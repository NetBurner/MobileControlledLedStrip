#include <predef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <basictypes.h>
#include <htmlfiles.h>
#include <http.h>
#include <iosys.h>

#include "led.h"
#include "ledStrip.h"
#include "spiInterface.h"

// Buffers to hold data posted from form
#define MAX_BUF_LEN 80
char gWebBuffer1[MAX_BUF_LEN];

double gRedLedVal = 0.5;
double gGreenLedVal = 0.5;
double gBlueLedVal = 0.5;

extern SpiInterface* gSpi;
extern LedStrip gLedStrip;
extern const int gLedCount;

// These are used to keep track of which LEDs should be updated when the user has selected to
// change them individually through the web control panel.
uint32_t gLedChecked1 = 0XFFFFFFFF;
uint32_t gLedChecked2 = 0XFFFFFFFF;

// Functions called from a web page must be declared extern C
extern "C"
{
   void LedDataBuffer( int sock, PCSTR url );
   void GetRedLedValue( int sock, PCSTR url );
   void GetGreenLedValue( int sock, PCSTR url );
   void GetBlueLedValue( int sock, PCSTR url );
   void GetCurrentVisualization( int sock, PCSTR url );
};


// Function to display value in web page
void LedDataBuffer( int sock, PCSTR url )
{
   writestring( sock, gWebBuffer1 );
}

void GetRedLedValue( int sock, PCSTR url )
{
    snprintf( gWebBuffer1, MAX_BUF_LEN, "%f", gLedStrip.GetActiveColor().GetRedD() );
    writestring( sock, gWebBuffer1 );
}

void GetGreenLedValue( int sock, PCSTR url )
{
    snprintf( gWebBuffer1, MAX_BUF_LEN, "%f", gLedStrip.GetActiveColor().GetGreenD() );
    writestring( sock, gWebBuffer1 );
}

void GetBlueLedValue( int sock, PCSTR url )
{
    snprintf( gWebBuffer1, MAX_BUF_LEN, "%f", gLedStrip.GetActiveColor().GetBlueD() );
    writestring( sock, gWebBuffer1 );
}

void GetCurrentVisualization( int sock, PCSTR url )
{
    switch( gLedStrip.GetCurVis() )
    {
        case eLedVisKnightRider:
        {
            snprintf( gWebBuffer1, MAX_BUF_LEN, "Knight Rider" );
            break;
        }
        case eLedVisTwinkle:
        {
            snprintf( gWebBuffer1, MAX_BUF_LEN, "Twinkle" );
            break;
        }
        case eLedVisBitCounter:
        {
            snprintf( gWebBuffer1, MAX_BUF_LEN, "Bit Counter" );
            break;
        }
        case eLedVisColorWave:
        {
            snprintf( gWebBuffer1, MAX_BUF_LEN, "Color Wave" );
            break;
        }
        case eLedVisPulse:
        {
            snprintf( gWebBuffer1, MAX_BUF_LEN, "Pulse" );
            break;
        }
        case eLedVisColorChange:
        {
            snprintf( gWebBuffer1, MAX_BUF_LEN, "Color Change" );
            break;
        }
        case eLedVisDefault: // This was removed for the demo
        case eLedVisNone:
        default:
        {
            snprintf( gWebBuffer1, MAX_BUF_LEN, "None Selected" );
            break;
        }
    }
    writestring( sock, gWebBuffer1 );
}

/*-------------------------------------------------------------------
 * MyDoPost() callback function
 * The NetBurner web server will call this function whenever a
 * web browser posts form data.
 * - This is where you have the ability to parse the url and
 *   form data sent in the HTML Post.
 * - The TCP connection will be held open while you are in this function
 *   so you can take whatever action you wish, or send any data you
 *   want to the web browser.
 * - IMPORTANT: you need to provide some type of content (eg web page)
 *   to the web browser or send a redirect command to the web browser
 *   so that it will issue an HTML GET request to obtain a web page.
 *   Otherwise the use will be left looking at a blank page.
 * - This example illustrates how to process multiple HTML Forms,
 *   so it parses the url to determine the name of the form, which
 *   is defined by the HTML Form action tag.
 *-----------------------------------------------------------------*/
int MyDoPost( int sock, char *url, char *pData, char *rxBuffer )
{
    // Display the data passed in just for the purpose of this example
    iprintf( "----- Processing Post -----\r\n" );
    iprintf( "Post URL: %s\r\n", url );
    iprintf( "Post Data: %s\r\n", pData );

    /* Parse the url to determine which form was used. The item
     * we are parsing for is defined by the HTML Form tag called
     * "action". It can be any name, but in this example we are
     * using it to tell us what web page the form was sent from.
     * Note that httpstricmp() requires the search string to
     * be in upper case, even if the action tag is lower case.
     */
    if( httpstricmp( url + 1, "LEDFORM" ) )
    {
        ExtractPostData( "red_slider", pData, gWebBuffer1, MAX_BUF_LEN );
        gRedLedVal = atof( gWebBuffer1 );
        printf("red value set to: \"%f\"\r\n", gRedLedVal );

        ExtractPostData( "green_slider", pData, gWebBuffer1, MAX_BUF_LEN );
        gGreenLedVal = atof( gWebBuffer1 );
        printf( "green value set to: \"%f\"\r\n", gGreenLedVal );

        ExtractPostData( "blue_slider", pData, gWebBuffer1, MAX_BUF_LEN );
        gBlueLedVal = atof( gWebBuffer1 );
        printf( "blue value set to: \"%f\"\r\n", gBlueLedVal );

        ExtractPostData( "visualization", pData, gWebBuffer1, MAX_BUF_LEN );

        gLedStrip.SetActiveColor( gRedLedVal, gGreenLedVal, gBlueLedVal );

        // Set the LED values
        if( httpstricmp( gWebBuffer1, "DEFAULT" ) )
        {
            iprintf( "Default Visualization\r\n" );
            gLedStrip.SetCurVis( eLedVisDefault );
            for( int i = 0; i < gLedCount; i++ )
            {
                char nameBuf[ 15 ] = {};
                snprintf( nameBuf, 15, "led_%02d", i + 1 );
                if( ExtractPostData( nameBuf, pData, gWebBuffer1, MAX_BUF_LEN ) > 0 )
                {
                    // Make sure we have the bit set
                    uint32_t temp = 1 << ( i % 32 );
                    if( i < 32 )
                    {
                        gLedChecked1 |= temp;
                    }
                    else
                    {
                        gLedChecked2 |= temp;
                    }

                    //iprintf( "Setting %s\r\n", nameBuf );
                    gLedStrip.SetLedValues( i, gRedLedVal, gGreenLedVal, gBlueLedVal );
                }
                else
                {
                    // Unset the bit, but leave the color value alone
                    uint32_t temp = ~(1 << ( i % 32 ) );
                    if( i < 32 )
                    {
                        gLedChecked1 &= temp;
                    }
                    else
                    {
                        gLedChecked2 &= temp;
                    }
                }
            }
        }
        else if( httpstricmp( gWebBuffer1, "KNIGHTRIDER" ) )
        {
            iprintf( "Knight Rider\r\n" );
            gLedStrip.SetCurVis( eLedVisKnightRider );
        }
        else if( httpstricmp( gWebBuffer1, "TWINKLE" ) )
        {
            iprintf( "Twinkle\r\n" );
            gLedStrip.SetCurVis( eLedVisTwinkle );
        }
        else if( httpstricmp( gWebBuffer1, "BITCOUNTER" ) )
        {
            iprintf( "Bit Counter\r\n" );
            gLedStrip.SetCurVis( eLedVisBitCounter );
        }
        else if( httpstricmp( gWebBuffer1, "COLORWAVE" ) )
        {
            iprintf( "Color Wave\r\n" );
            gLedStrip.SetCurVis( eLedVisColorWave );
        }
        else if( httpstricmp( gWebBuffer1, "COLORCHANGE" ) )
        {
            iprintf( "Color Change\r\n" );
            gLedStrip.SetCurVis( eLedVisColorChange );
        }
        else if( httpstricmp( gWebBuffer1, "PULSE" ) )
        {
            iprintf( "Pulse\r\n" );
            gLedStrip.SetCurVis( eLedVisPulse );
        }

        printf( "LedChecked1: %X\r\n", uint( gLedChecked1 ) );
        printf( "LedChecked2: %X\r\n", uint( gLedChecked2 ) );
        
        // Tell the web browser to issue a GET request for the next page
        RedirectResponse( sock, "index.htm" );
    }
    else if( httpstricmp( url + 1, "SETREDVALUE" ) )
    {
        Led newColor = gLedStrip.GetActiveColor();
        ExtractPostData( "red_slider", pData, gWebBuffer1, MAX_BUF_LEN );
        gRedLedVal = atof( gWebBuffer1 );
        gLedStrip.SetActiveColor( gRedLedVal, newColor.GetGreenD(), newColor.GetBlueD() );
    }
    else if( httpstricmp( url + 1, "SETGREENVALUE" ) )
    {
        Led newColor = gLedStrip.GetActiveColor();
        ExtractPostData( "green_slider", pData, gWebBuffer1, MAX_BUF_LEN );
        gGreenLedVal = atof( gWebBuffer1 );
        gLedStrip.SetActiveColor( newColor.GetRedD(), gGreenLedVal, newColor.GetBlueD() );
    }
    else if( httpstricmp( url + 1, "SETBLUEVALUE" ) )
    {
        Led newColor = gLedStrip.GetActiveColor();
        ExtractPostData( "blue_slider", pData, gWebBuffer1, MAX_BUF_LEN );
        gBlueLedVal = atof( gWebBuffer1 );
        gLedStrip.SetActiveColor( newColor.GetRedD(), newColor.GetGreenD(), gBlueLedVal );
    }

    return 1;
}

void RegisterPost()
{
   SetNewPostHandler( MyDoPost );
}
