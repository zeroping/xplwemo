#include "WeMoDevice.h"
#include "GPIOInput.h"


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

extern const string WeMoDevice::configFilePath = "/etc/xplwemo.conf";


WeMoDevice::WeMoDevice() :
    hallog ( Logger::get ( "xplwemonat" ) )
{

}

void WeMoDevice::start()
{
    loadConfiguration();
    
    //start by claiming our GPIO
    relayPin = GPIOPin(pConf->getString("relay.gpio","13"));
    relayPin.export_gpio();
    relayPin.setdir_gpio("out");
    
 
    
    //xplUDP::instance()->rxNotificationCenter.addObserver ( Observer<WeMoDevice, MessageRxNotification> ( *this,&WeMoDevice::HandleAllMessages ) );
    //just create it
    xplUDP::instance();
    
    poco_debug ( hallog, "xplUDP created" );
    string xplvend = pConf->getString("xplname.vendor","smgpoe");
    string xpldevi = pConf->getString("xplname.device","wemo");
    string xplinst = pConf->getString("xplname.instance","default");
    
    pDevice.assign ( new xplDevice ( xplvend, xpldevi, "1.0", true, true, xplUDP::instance() ) );
    if ( NULL == pDevice )
    {
        poco_error ( hallog, "no device" );
        return;
    }
    pDevice->SetInstanceId(xplinst);

    pDevice->rxNotificationCenter.addObserver ( Observer<WeMoDevice, MessageRxNotification> ( *this,&WeMoDevice::HandleDeviceMessages ) );

    pDevice->Init();
    
    Poco::RunnableAdapter<WeMoDevice> inputRunnable(*this, &WeMoDevice::inputLoop);
    inputRunnable.run();

}

WeMoDevice::~WeMoDevice()
{
    poco_debug ( hallog, "destroying xplHal" );
    relayPin.setdir_gpio("in");
    relayPin.unexport_gpio();

}


void WeMoDevice::loadConfiguration(){
    pConf = new PropertyFileConfiguration(configFilePath);
}

void WeMoDevice::setLED(const bool on)
{
    if(pConf->hasProperty("relay.ledname")) {
        string ledpath = pConf->getString("relay.ledname");
        ofstream ledfile(ledpath.c_str()); // Open led" file. Convert C++ string to C string. Required for all Linux pathnames
        if (ledfile < 0){
            poco_warning ( hallog, "failed to turn on/off LED for relay" );
            return;
        }
        
        //write 0 or 1
        if(on) {
            ledfile << "1";
        } else {
            ledfile << "0";
        }
        ledfile.close(); //close export file
    }
}

void WeMoDevice::setRelay(const bool enabled){
    if (enabled) {
        poco_warning ( hallog, "relay on" );
//         std::vector<std::string> args;
//         args.push_back("dpms");
//         args.push_back("force");
//         args.push_back("on");
//         Process::launch("/usr/bin/xset", args);
        relayPin.setval_gpio("1");
        setLED(true);
        
    } else {
        poco_warning ( hallog, "relay off" );
//         std::vector<std::string> args;
//         args.push_back("dpms");
//         args.push_back("force");
//         args.push_back("off");
//         Process::launch("/usr/bin/xset", args);
        relayPin.setval_gpio("0");
        setLED(false);
    }
}

//messages that are local to us
void WeMoDevice::HandleDeviceMessages ( MessageRxNotification* mNot )
{
    poco_debug ( hallog, "got directed message: " + mNot->message->GetSchemaClass() + " " + mNot->message->GetSchemaType() );

    if ( (icompare(mNot->message->GetSchemaClass(), "Control")==0) && ( icompare(mNot->message->GetSchemaType(), "Basic")==0))  {
        const xplMsgItem* dev = mNot->message->GetMsgItem("device");
        if (dev != NULL && icompare(dev->GetValue(),pConf->getString("relay.devname","relay"))==0 ) {
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


void WeMoDevice::buttonPress(const string buttonName, const bool pressed) {
    
    AutoPtr<xplMsg> message = new xplMsg ( "xpl-trig",  pDevice->GetCompleteId(), "*", "sensor", "basic" );
    message->AddValue ( "device",buttonName );
    message->AddValue ( "type","input" );
    if(pressed){
        message->AddValue ( "current","high" );
    } else {
        message->AddValue ( "current","low" );
    }
    
    xplUDP* comm = xplUDP::instance();
    comm->TxMsg ( *message );
    poco_information ( hallog, " sending a message:" );
    poco_trace ( hallog, message->GetRawData() );
    
}

void WeMoDevice::inputLoop()
{
    try {
        GPIOInput gin;
        while(true) {
            input_event ev = gin.waitForInput();
            poco_debug ( hallog, "got input, val: " + NumberFormatter::format(ev.value)  + " : " + NumberFormatter::format(ev.type) + " : " + NumberFormatter::format(ev.code)) ;
            if(ev.type==1){
                vector < string > keys;
                pConf->keys(keys);
                vector<string>::iterator keysIt;
                for(keysIt = keys.begin(); keysIt != keys.end(); keysIt++)
                {
                    string buttonstr = "button";
                    if(icompare(*keysIt,6,buttonstr,6) == 0){
//                         poco_debug ( hallog, "key has button " + *keysIt);
                        if (ev.code == pConf->getInt(*keysIt+".code",0)) {
                            if(ev.value) {
                                poco_debug ( hallog, "key press" + pConf->getString(*keysIt+".name","namedef"));
                                buttonPress(pConf->getString(*keysIt+".name","namedef"),1);
                            } else {
                                poco_debug ( hallog, "key release" + pConf->getString(*keysIt+".name","namedef"));
                                buttonPress(pConf->getString(*keysIt+".name","namedef"),0);
                            }
                        }
                    }
                }
                
            }
        }
    } catch (GPIOKeysException e) {
        poco_error ( hallog, "cannot open button input device" );
        return ;
    }    
}

