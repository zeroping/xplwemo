// xPL Linux Hal Server basic types

#ifndef WEMONATDEVICE_H
#define WEMONATDEVICE_H

#include <string>
#include <vector>

#include "GPIOPin.h"

#include "Poco/SharedPtr.h"
#include "XplUDP.h"
#include "XplComms.h"
#include "XplDevice.h"
#include "Poco/Logger.h"
#include "Poco/NumberFormatter.h"

#include "Poco/Util/PropertyFileConfiguration.h"

using namespace Poco;
using namespace xpl;
using namespace std;
using Poco::AutoPtr;
using Poco::Util::PropertyFileConfiguration;
class WeMoDevice
{
public:
    
    WeMoDevice();
    ~WeMoDevice();

    void start();
    void HandleDeviceMessages ( MessageRxNotification* );
    
    static Path getConfigFileLocation();
    void setRelay ( const bool enabled );
    void inputLoop();
    void buttonPress ( const string buttonName, const bool pressed );
    void setLED ( const bool on );
private:
    Logger& hallog;

    //XplDevice* pDevice;
    SharedPtr<XplDevice> pDevice;
    GPIOPin relayPin;
    static const string configFilePath;
    int gpiopinnum;
    void loadConfiguration();
    AutoPtr<PropertyFileConfiguration> pConf;
    
};  




#endif // XPLHal_H
