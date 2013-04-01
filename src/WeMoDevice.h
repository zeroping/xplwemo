// xPL Linux Hal Server basic types

#ifndef WEMONATDEVICE_H
#define WEMONATDEVICE_H

#include <string>
#include <vector>

#include "Poco/SharedPtr.h"
#include "xplUDP.h"
#include "xplComms.h"
#include "xplDevice.h"
#include "Poco/Logger.h"
#include "Poco/NumberFormatter.h"

using namespace Poco;
using namespace xpl;
using namespace std;

class WeMoDevice
{
public:
    
    WeMoDevice();
    ~WeMoDevice();

    void start();
    void HandleDeviceMessages ( MessageRxNotification* );
    
    static Path getConfigFileLocation();
    void setRelay ( bool enabled );
private:
    Logger& hallog;

    //xplDevice* pDevice;
    SharedPtr<xplDevice> pDevice;
    
};


#endif // XPLHal_H
