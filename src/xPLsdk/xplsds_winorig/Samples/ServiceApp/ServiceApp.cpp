/***************************************************************************
****																	****
****	ServiceApp.cpp  									            ****
****																	****
****	Example code demonstrating how to build an xPL Windows Service. ****
****																	****
****	Copyright (c) 2005 Mal Lansell.									****
****																	****
****	Permission is hereby granted, free of charge, to any person		****
****	obtaining a copy of this software and associated documentation	****
****	files (the "Software"), to deal in the Software without			****
****	restriction, including without limitation the rights to use,	****
****	copy, modify, merge, publish, distribute, sublicense, and/or	****
****	sell copies of the Software, and to permit persons to whom the	****
****	Software is furnished to do so, subject to the following		****
****	conditions:														****
****																	****
****	The above copyright notice and this permission notice shall		****
****	be included in all copies or substantial portions of the		****
****	Software.														****
****																	****
****	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY		****
****	KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE		****
****	WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR			****
****	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR	****
****	COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER		****
****	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR			****
****	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE		****
****	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.			****
****																	****
***************************************************************************/

#include "xplDevice.h"
#include "xplUDP.h"
#include "xplConfigItem.h"
#include "xplMsg.h"
#include "Service.h"
#include "ServiceApp.h"
#include "EventLog.h"

using namespace xpl;

// Set the config version here.  Each time a new version of the application
// is to be released, increment the version number.  This ensures that the
// new version will ignore any config items saved in the registry by
// previous versions, and instead enters config mode.
string const c_configVersion = "1.0.0";

// Change the appName to match your application.
string const c_appName = "ServiceApp";

// This text will appear as the service description in the management console.
string const c_appDescription = "Sample xPL Service";


/***************************************************************************
****																	****
****	main															****
****																	****
****	Parses the command line and installs, uninstalls or runs the	****
****	Service service as appropriate	 								****
****																	****
***************************************************************************/

int main ( int argc, char* argv[] )
{
    // Create the event log
    EventLog::Create ( c_appName );

    // Create the application
    ServiceApp* pApp = new ServiceApp();

    // Wrap the application in a service
    // Change "xPLApp" to the name you want to use for your service.
    Service* pService = Service::Create ( c_appName, c_appDescription, ServiceApp::MainProc, ( void* ) pApp );

    // Install, Uninstall or Run the service as determined by the command line
    int res = pService->ProcessCommandLine ( argc, argv );

    // Clean up
    Service::Destroy();
    delete pApp;
    EventLog::Destroy();

    return ( res );
}


/***************************************************************************
****																	****
****	ServiceApp Constructor         									****
****																	****
***************************************************************************/

ServiceApp::ServiceApp() :
    m_pDevice ( NULL ),
    m_pComms ( NULL )
{
    // Set default values for your application's variables here.
}


/***************************************************************************
****																	****
****	ServiceApp Destructor			            					****
****																	****
***************************************************************************/

ServiceApp::~ServiceApp()
{
    // Add clean-up code here
}


/***************************************************************************
****																	****
****	ServiceApp::Run							        				****
****																	****
***************************************************************************/

void ServiceApp::Run
(
    HANDLE _hActive,
    HANDLE _hExit
)
{
    // _hActive is signalled when the service is running (not paused)
    // _hExit is signalled when the service stops.


    // ************************
    // Init
    // ************************

    // Create the xPL communications object
    m_pComms = xplUDP::Create ( true );
    if ( NULL == m_pComms )
    {
        return;
    }

    // Create the xPL Device
    // Change "vendor" to your own vendor ID.  Vendor IDs can be no more than 8 characters
    // in length.  Post a message to the xPL Yahoo Group (http://groups.yahoo.com/group/ukha_xpl)
    // to request a vendor ID.
    // Replace "device" with a suitable device name for your application - usually something
    // related to what is being xPL-enabled.  Device names can be no more than 8 characters.
    m_pDevice = xplDevice::Create ( "vendor", "device", c_configVersion, true, true, m_pComms );
    if ( NULL == m_pDevice )
    {
        return;
    }

    // Create any additional config items
    // As an example, the following code creates an item to hold the index
    // of a com port.  The value can be changed by the user in xPLHal.
    xplConfigItem* pItem = new xplConfigItem ( "comport", "reconf" );
    if ( NULL != pItem )
    {
        // Default the com port to COM1
        pItem->AddValue ( "1" );
        m_pDevice->AddConfigItem ( pItem );
    }

    // Get the message and config events
    // The xplMsgEvent is signalled when an xPL message is received by the xplDevice.
    // The configEvent is signalled when the config items have been changed.
    HANDLE xplMsgEvent = m_pDevice->GetMsgEvent();
    HANDLE configEvent = m_pDevice->GetConfigEvent();

    // Init the xplDevice
    // Note that all config items must have been set up before Init() is called.
    m_pDevice->Init();


    // ************************
    // Main loop
    // ************************

    while ( 1 )
    {
        // Wait for an event to occur.
        // If possible, design your application so that an event is signalled when
        // something needs to be taken care of.  That way, the application can
        // sit here not taking any CPU time, until there is work to do.
        // Add your own events to the handles array.  Don't forget to change the
        // number of events in the WaitForMultipleObjects call.
        HANDLE handles[3];
        handles[0] = _hExit;
        handles[1] = xplMsgEvent;
        handles[2] = configEvent;

        DWORD res = WaitForMultipleObjects ( 3, handles, FALSE, INFINITE );

        // First check that we're not actually meant to be paused
        handles[1] = _hActive;
        if ( WAIT_OBJECT_0 == WaitForMultipleObjects ( 2, handles, FALSE, INFINITE ) )
        {
            // Exit event was signalled
            break;
        }

        // Deal with any received xPL messages
        if ( ( WAIT_OBJECT_0 + 1 ) == res )
        {
            HandleMessages();
        }

        // Is configuration required?
        if ( ( WAIT_OBJECT_0 + 2 ) == res )
        {
            // Reset the configEvent before doing the configuration.
            // This ensures that if a config update is received by the
            // xPLDevice thread while we're in Configure(), it will
            // not be missed.
            ResetEvent ( configEvent );
            Configure();
        }

        // Deal with any ServiceApp specific stuff here
        // If we were waiting on an event, check it in the same way
        // that we did for the xplMsgEvent and configEvent above.
        //
        // if( ( WAIT_OBJECT_0 + 3 ) == res )
        // {
        // }
        //
    }

    // ************************
    // Clean up and exit
    // ************************

    // Destroy the xPL Device
    // This also deletes the hub and any config items we may have added
    xplDevice::Destroy ( m_pDevice );
    m_pDevice = NULL;

    // Destroy the comms object
    xplUDP::Destroy ( m_pComms );
    m_pComms = NULL;

    return;
}


/***************************************************************************
****																	****
****	ServiceApp::HandleMessages							            ****
****																	****
***************************************************************************/

void ServiceApp::HandleMessages()
{
    xplMsg* pMsg = NULL;

    // Get each queued message in turn
    while ( pMsg = m_pDevice->GetMsg() )
    {
        // Process the message here.
        // ...

        // Done with it.
        pMsg->Release();
    }
}


/***************************************************************************
****																	****
****	ServiceApp::Configure										    ****
****																	****
***************************************************************************/

void ServiceApp::Configure()
{
    // Examine each of our config items in turn, and if they have changed,
    // take the appropriate action.

    // As an example, the Com Port index.
    //
    //  xplConfigItem const* pItem = m_pDevice->GetConfigItem( "comport" );
    //  string value = pItem->GetValue();
    //  if( !value.empty() )
    //  {
    //      int comPort = atol( value.c_str() );
    //
    //      Deal with any change to the Com port number
    //      ...
    //  }
    //
}


/***************************************************************************
****																	****
****	ServiceApp::MainProc										    ****
****																	****
***************************************************************************/

void ServiceApp::MainProc
(
    HANDLE _hActive,
    HANDLE _hExit,
    void* pContext
)
{
    // Function called by the service when it starts
    // _hActive is signalled when the service is running (not paused)
    // _hExit is signalled when the service stops.
    ServiceApp* pApp = ( ServiceApp* ) pContext;
    pApp->Run ( _hActive, _hExit );
}
