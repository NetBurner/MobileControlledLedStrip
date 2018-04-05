#include "led.h"

#include <stdio.h>

#include "spiInterface.h"

extern SpiInterface gSpi;

void Led::SetLedValues( uint8_t r, uint8_t g, uint8_t b )
{
    // Always make sure the first bit is set
    m_red = ( r < 128 ) ? 128 + r : r;
    m_green = ( g < 128 ) ? 128 + g : g;
    m_blue = ( b < 128 ) ? 128 + b : b;

    m_redD = double( m_red - 128 ) / 127.0;
    m_greenD = double( m_green - 128 ) / 127.0;
    m_blueD = double( m_blue - 128 ) / 127.0;
}

void Led::SetLedValues( double r, double g, double b )
{
    // Ensure the value passed in is between a range of 0.0 and 1.0
    r = ( r < 1.0 ) ? r : 1.0;
    r = ( r > 0.0 ) ? r : 0.0;
    m_redD = r;

    g = ( g < 1.0 ) ? g : 1.0;
    g = ( g > 0.0 ) ? g : 0.0;
    m_greenD = g;

    b = ( b < 1.0 ) ? b : 1.0;
    b = ( b > 0.0 ) ? b : 0.0;
    m_blueD = b;

    // Always make sure the first bit is set
    m_red = 128 + uint32_t( m_redD * 127 );
    m_green = 128 + uint32_t( m_greenD * 127 );
    m_blue = 128 + uint32_t( m_blueD * 127 );

}

void Led::SetLedValues( const Led& l )
{
    m_red = l.GetRed() + 128;
    m_green = l.GetGreen() + 128;
    m_blue = l.GetBlue() + 128;
}

void Led::WriteLedValues()
{
    //iprintf( "  g: %d   r: %d   b: %d\r\n", m_green, m_red, m_blue );
    BYTE colors[3] = { m_green, m_red, m_blue };
    gSpi.WriteSpi( colors, 3 );
}

bool Led::operator==( const Led& lhs ) const
{
    // We hide the visibility bit from the users, so we have to add it back when making a comparison
    return m_red == ( lhs.GetRed() + 128 ) && 
           m_green == ( lhs.GetGreen() + 128 ) && 
           m_blue == ( lhs.GetBlue() + 128 );
}
