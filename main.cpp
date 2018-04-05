#include <init.h>
#include <predef.h>
#include <stdio.h>
#include <startnet.h>
#include <taskmon.h>
#include <ucos.h>

#include "led.h"
#include "ledStrip.h"
#include "spiInterface.h"

extern "C"
{
   void UserMain( void *pd );
}

void RegisterPost();

const char *AppName = "LED Strip App";

SpiInterface *gSpi = nullptr;
LedStrip gLedStrip;

extern const int gLedCount;

DWORD gTestTaskStack[ USER_TASK_STK_SIZE ] __attribute__( (aligned( 4 )) );

void CallLedStripMain( void* data )
{
    gLedStrip.Main();
}

/**
 *  UserMain
 *
 *  Main entry point for the example
 */
void UserMain( void *pd )
{   
    // Initialize the stack, get an IP address, set up the web server, etc.
    initWithWeb();
    
    // Initialize SPI
    gSpi = SpiInterface::GetSpiInterface();
    
    gLedStrip.InitializeLedStrip();
    
    RegisterPost();  // Assign HTML Post handling callback function
    
    // This task runs the code that sends data to the LED strip
    OSTaskCreate( CallLedStripMain, nullptr, (void*)&gTestTaskStack[ USER_TASK_STK_SIZE ], (void*)gTestTaskStack, MAIN_PRIO - 1 );
    
    // All the action happens in the web page processing, so just loop here
    while ( 1 )
    {
        OSTimeDly( TICKS_PER_SECOND );
    }
}
