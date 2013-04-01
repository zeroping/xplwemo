// xPL LCD monitor controller

#include <string>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <pthread.h>
#include <signal.h>
#include <syslog.h>
#include <iostream>
#include <fstream>
#include "WeMoDevice.h"

#include "Poco/Logger.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FormattingChannel.h"
#include "Poco/NumberFormatter.h"
#include "Poco/Thread.h"


using namespace Poco;


bool running = true;

void shutdown_handler ( int s )
{
    Logger& rootlogger = Logger::root();
    poco_information ( rootlogger, "  caught signal" + s );
    running = false;
}

void setup_singnal_handler()
{
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = shutdown_handler;
    sigemptyset ( &sigIntHandler.sa_mask );
    sigIntHandler.sa_flags = 0;

    sigaction ( SIGINT, &sigIntHandler, NULL );
}

int main ( int argc,const char * argv[] )
{


    AutoPtr<ConsoleChannel> pCons ( new ConsoleChannel );
    AutoPtr<PatternFormatter> pPF ( new PatternFormatter );
    pPF->setProperty ( "pattern", "%H:%M:%S %s %I %p: %t" );
    AutoPtr<FormattingChannel> pFC ( new FormattingChannel ( pPF, pCons ) );
    Logger& rootlogger = Logger::root();
    //Logger::root().setChannel(pFC);
    rootlogger.setChannel ( pFC );
    rootlogger.setLevel ( "debug" );

    poco_warning ( rootlogger, "starting logger" );
    poco_information ( rootlogger, "Starting up");

    openlog ( "xplwemonat", LOG_PID, LOG_DAEMON );

//     WeMoDevice dev;
//     
//     dev.start();
    poco_information ( rootlogger, "Main thread created" );

    setup_singnal_handler();
    
    while ( running )
    {

        Thread::sleep ( 1000 );
        poco_information ( rootlogger, "test" );

    }

    return 0;
    // exit(0);
}


