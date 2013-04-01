#include "WeMoDevice.h"


#include <vector>
#include <string>
#include <iostream>


#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include "Poco/Net/TCPServerParams.h"
#include "Poco/String.h"
#include <Poco/Path.h>
#include <Poco/Process.h>


using namespace std;
using Poco::Net::ServerSocket;
using Poco::Net::TCPServer;
using Poco::Net::TCPServerConnection;
using Poco::Net::TCPServerConnectionFactory;
using Poco::Net::TCPServerConnectionFactoryImpl;
using Poco::Net::TCPServerParams;
using Poco::Net::IPAddress;
using Poco::Process;
;

WeMoDevice::WeMoDevice() :
    hallog ( Logger::get ( "xplwemonat" ) )
{

}

void WeMoDevice::start()
{

    //xplUDP::instance()->rxNotificationCenter.addObserver ( Observer<WeMoDevice, MessageRxNotification> ( *this,&WeMoDevice::HandleAllMessages ) );
    //just create it
    xplUDP::instance();
    
    poco_debug ( hallog, "xplUDP created" );
    pDevice.assign ( new xplDevice ( "smgpoe", "wemonat", "1.0", true, true, xplUDP::instance() ) );
    if ( NULL == pDevice )
    {
        poco_error ( hallog, "no device" );
        return;
    }

    pDevice->rxNotificationCenter.addObserver ( Observer<WeMoDevice, MessageRxNotification> ( *this,&WeMoDevice::HandleDeviceMessages ) );

    pDevice->Init();


}

WeMoDevice::~WeMoDevice()
{
    poco_debug ( hallog, "destroying xplHal" );

}

void WeMoDevice::setRelay(bool enabled){
    if (enabled) {
        poco_debug ( hallog, "monitor on" );
//         std::vector<std::string> args;
//         args.push_back("dpms");
//         args.push_back("force");
//         args.push_back("on");
//         Process::launch("/usr/bin/xset", args);
    } else {
        poco_debug ( hallog, "monitor off" );
//         std::vector<std::string> args;
//         args.push_back("dpms");
//         args.push_back("force");
//         args.push_back("off");
//         Process::launch("/usr/bin/xset", args);
    }
}

//messages that are local to us
void WeMoDevice::HandleDeviceMessages ( MessageRxNotification* mNot )
{
    poco_debug ( hallog, "got directed message: " + mNot->message->GetSchemaClass() + " " + mNot->message->GetSchemaType() );

    if ( (icompare(mNot->message->GetSchemaClass(), "Control")==0) && ( icompare(mNot->message->GetSchemaType(), "Basic")==0))  {
        const xplMsgItem* dev = mNot->message->GetMsgItem("device");
        if (dev != NULL && icompare(dev->GetValue(),"relay")==0 ) {
            const xplMsgItem* dtype = mNot->message->GetMsgItem("type");
            if (dtype != NULL && icompare(dtype->GetValue(),"output")==0 ) {
                const xplMsgItem* current = mNot->message->GetMsgItem("current");
                if (current != NULL) {
                    if ( (icompare(current->GetValue(),"enable")==0) 
                        || (icompare(current->GetValue(),"high")==0)) {
                    
                        setRelay(true);
                    } else  if ( (icompare(current->GetValue(),"disable")==0) 
                        || (icompare(current->GetValue(),"low")==0)) {
                        setRelay(false); 
                    }
                }
            } 
        }
    }
    
    mNot->release();
}

Path WeMoDevice::getConfigFileLocation()
{
    Poco::Path p ( Poco::Path::home() );
    p.pushDirectory ( ".xPL" );
    p.pushDirectory ( "xplhallite" );
    return p;
}


