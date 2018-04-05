#include "ledStrip.h"

#include <constants.h>
#include <HiResTimer.h>
#include <stdio.h>
#include <stdlib.h>

#include "spiInterface.h"

extern SpiInterface *gSpi;

HiResTimer* hrTimer = nullptr;

LedStrip::LedStrip()
{
}

bool LedStrip::InitializeLedStrip()
{
    if( gSpi == nullptr )
    {
        iprintf( "Unable to initialize LED strip, SPI interface is unavailable.\r\n" );
        return false;
    }

    gSpi->WriteSpi( m_clearBits, 2 ); // Send the first two clear bytes
    m_setColor.SetLedValues( double( 0.5 ), double( 0.5 ), double( 0.5 ) );

    return true;
}

void LedStrip::SetLedValues( int i, uint8_t r, uint8_t g, uint8_t b )
{
    if( i < 0 || i > gLedCount )
    {
        iprintf( "SetLed: Index outside of led range %d\r\n", i );
        return;
    }

    m_ledStrip[ i ].SetLedValues( r, g, b );
}

void LedStrip::SetLedValues( int i, double r, double g, double b )
{
    if( i < 0 || i > gLedCount )
    {
        iprintf( "SetLed: Index outside of led range %d\r\n", i );
        return;
    }
    m_ledStrip[ i ].SetLedValues( r, g, b );
}

void LedStrip::SetActiveColor( uint8_t r, uint8_t g, uint8_t b )
{
    m_setColor.SetLedValues( r, g, b );
    m_colorChanged = true;
    //iprintf( "Active Color set to %d %d %d\r\n", m_setColor.GetRed(), m_setColor.GetGreen(), m_setColor.GetBlue() );
}

void LedStrip::SetActiveColor( double r, double g, double b )
{
    m_setColor.SetLedValues( r, g, b );
    m_colorChanged = true;
    //printf( "Active Color set to %f %f %f\r\n", m_setColor.GetRedD(), m_setColor.GetGreenD(), m_setColor.GetBlueD() );
}

void LedStrip::GetLedValues( int i, uint8_t( &v )[ 3 ] )
{
    if( i < 0 || i > gLedCount )
    {
        iprintf( "GetLedValues: Index outside of led range %d\r\n", i );
        return;
    }

    v[ 0 ] = m_ledStrip[ i ].GetRed();
    v[ 1 ] = m_ledStrip[ i ].GetGreen();
    v[ 2 ] = m_ledStrip[ i ].GetBlue();
}

void LedStrip::SetCurVis( LedVis v )
{
    m_reset = true; 
    m_currentVisual = v;
}

void LedStrip::ClearLeds()
{
    // Write color bits
    for( int i = 0; i < gLedCount; i++ )
    {
        m_ledStrip[ i ].SetLedValues( uint8_t( 0 ), uint8_t( 0 ), uint8_t( 0 ) );
    }

    WriteLeds();
    //OSTimeDly( 1 );
}

void LedStrip::WriteLeds()
{
    // Write color bits
    for( int i = 0; i < gLedCount; i++ )
    {
        m_ledStrip[ i ].WriteLedValues();
    }

    // Write clear bits
    //iprintf( "Writing clear bit\r\n" );
    gSpi->WriteSpi( m_clearBits, 2 ); // Have to send one final byte to latch the last byte
    //OSTimeDly( 1 );
}

void LedStrip::PlayKnightRiderAnim()
{
    iprintf( "Starting KnightRiderAnim\r\n" );
    int activeIndex = 0;
    bool inc = true;
    while( m_currentVisual == eLedVisKnightRider && !m_reset )
    {
        for( int i = inc ? 0 : gLedCount - 1;
             inc ? i < gLedCount : i >= 0; 
             inc ? i++ : i-- )
        {
            int step = ( ( activeIndex - i ) > 0 ) ? ( activeIndex - i ) : ( i - activeIndex );
            double stepVal =  step * .25;
            uint8_t actColRed = m_setColor.GetRed();
            uint8_t actColGreen = m_setColor.GetGreen();
            uint8_t actColBlue = m_setColor.GetBlue();

            // If the led is close enough to the active led and has a high enough color value, reduce it, otherwise
            // just turn it off.
            if( actColRed != 0 )
            {
                actColRed = ( ( actColRed > stepVal ) && ( step <= 3 ) ) ? uint8_t( actColRed - ( actColRed * stepVal ) ) : 0;
            }
            if( actColGreen != 0 )
            {
                actColGreen = ( ( actColGreen > stepVal ) && ( step <= 3 ) ) ? uint8_t( actColGreen - ( actColGreen * stepVal ) ) : 0;
            }
            if( actColBlue != 0 )
            {
                actColBlue = ( ( actColBlue > stepVal ) && ( step <= 3 ) ) ? uint8_t( actColBlue - ( actColBlue * stepVal ) ) : 0;
            }

            m_ledStrip[ i ].SetLedValues( actColRed, actColGreen, actColBlue );
        }

        WriteLeds();

        // Delay so we can process we posts
        OSTimeDly( 1 );

        if( (activeIndex == 0 && !inc) || (activeIndex == 63 && inc) )
        {
            inc = !inc;
        }

        activeIndex = inc ? activeIndex + 1 : activeIndex - 1;
    }
    iprintf( "Leaving knight rider\r\n" );
    m_reset = false;
}

void LedStrip::PlayTwinkle()
{
    uint32_t activeLowInc = 0;
    uint32_t activeHighInc = 0;
    uint32_t activeLowDec = 0;
    uint32_t activeHighDec = 0;

    uint32_t spawnCount = 0;
    uint32_t maxAge = 0;
    
    bool changeRed = false;
    bool changeGreen = false;
    bool changeBlue = false;

    double redMod = 0.0;
    double greenMod = 0.0;
    double blueMod = 0.0;
    m_colorChanged = true;

    ClearLeds();

    while( m_currentVisual == eLedVisTwinkle && !m_reset )
    {
        // If the set color has changed, then we need to compute all of these values
        if( m_colorChanged )
        {
            maxAge = m_setColor.GetRed() + m_setColor.GetGreen() + m_setColor.GetBlue() + 1;
            // Put restrictions on the maxage so it isn't too short or too long
            maxAge = (maxAge < 100) ? 100 : maxAge;
            maxAge = (maxAge > 250) ? 250 : maxAge;

            changeRed = m_setColor.GetRed() != 0;
            changeGreen = m_setColor.GetGreen() != 0;
            changeBlue = m_setColor.GetBlue() != 0;

            redMod = changeRed ? m_setColor.GetRedD() * .5 : 0.0;
            greenMod = changeGreen ? m_setColor.GetGreenD() * .5 : 0.0;
            blueMod = changeBlue ? m_setColor.GetBlueD() * .5 : 0.0;
            while( greenMod > 0.01 || redMod > 0.01 || blueMod > 0.01 )
            {
                greenMod *= 0.5;
                redMod *= 0.5;
                blueMod *= 0.5;
            }
            m_colorChanged = false;
        }

        // Find next twinkle start index
        int spawned = 0;
        // Don't spawn more than 48 at any given time, and no more than 5 in one cycle
        while( ( spawnCount < 48 ) && ( spawned < 5 ) )
        {
            bool foundSpawn = false;
            int spawnInd = rand() % 64;

            // Don't reset LED that are in the process of decrementing
            if( ( spawnInd < 32 ) &&
                ( ( activeLowInc & ( 1 << spawnInd ) ) == 0) &&
                ( ( activeLowDec & ( 1 << spawnInd ) ) == 0) )
            {
                activeLowInc |= (1 << spawnInd);
                foundSpawn = true;
            }
            else if( ( spawnInd >= 32 ) &&
                ( ( activeHighInc & ( 1 << ( spawnInd % 32 ) ) ) == 0 ) &&
                ( ( activeHighDec & ( 1 << ( spawnInd % 32 ) ) ) == 0 ) )
            {
                activeHighInc |= ( 1 << ( spawnInd % 32 ) );
                foundSpawn = true;
            }
            else
            {
                spawnInd = rand() % 64;
            }

            if( foundSpawn )
            {
                // Set the base color
                m_ledStrip[ spawnInd ].SetLedValues( uint8_t( 0 ), uint8_t( 0 ), uint8_t( 0 ) );
                // Ensure we at least get 10
                uint32_t curMaxAge = ( rand() % ( maxAge - 10 ) ) + 10;
                // This is just to cut way down on the max age, which gives a nicer affect
                double ageScaled = ( curMaxAge * ( double( ( rand() % 91 ) + 10 ) / 100.0 ) );
                m_ledStrip[ spawnInd ].SetMaxAge( uint32_t( ageScaled ) + 1 );
                spawnCount++;
                spawned++;
            }
        }

        // Increment and decrement set twinkles as needed
        for( int i = 0; i < gLedCount; i++ )
        {
            bool inc = false;
            bool dec = false;
            if( ( i < 32 ) ? activeLowInc & ( 1 << i ) : activeHighInc & ( 1 << ( i % 32 ) ) )
            {
                inc = true;
            }
            // Decrement color
            else if( ( i < 32 ) ? activeLowDec & ( 1 << i ) : activeHighDec & ( 1 << ( i % 32 ) ) )
            {
                dec = true;
            }

            // These values are used to determine how long a light should be on, and how fast it should inc and dec
            uint32_t curAge = m_ledStrip[ i ].GetRed() + m_ledStrip[ i ].GetGreen() + m_ledStrip[ i ].GetBlue();
            double modScale = ( inc || dec ) ? ( curAge / double( m_ledStrip[ i ].GetMaxAge() ) ) * 2.0 : 0.0;

            if( inc )
            {

                m_ledStrip[ i ].SetLedValues( changeRed ? m_ledStrip[ i ].GetRedD() + redMod + ( redMod * modScale ) : double( 0 ),
                                              changeGreen ? m_ledStrip[ i ].GetGreenD() + greenMod + ( greenMod * modScale ) : double( 0 ),
                                              changeBlue ? m_ledStrip[ i ].GetBlueD() + blueMod + ( blueMod * modScale ) : double( 0 ) );
                
                // Move lights that have hit the peak from inc to dec
                bool ledIsMax = ( changeRed && ( m_ledStrip[ i ].GetRed() >= m_setColor.GetRed() ) ) ||
                                ( changeGreen && ( m_ledStrip[ i ].GetGreen() >= m_setColor.GetGreen() ) ) ||
                                ( changeBlue && ( m_ledStrip[ i ].GetBlue() >= m_setColor.GetBlue() ) );

                if( ledIsMax || ( m_ledStrip[ i ].GetMaxAge() < curAge ) )
                {
                    if( i < 32 )
                    {
                        activeLowInc &= ~( 1 << i );
                        activeLowDec |= ( 1 << i );
                    }
                    else
                    {
                        activeHighInc &= ~( 1 << ( i % 32 ) );
                        activeHighDec |= ( 1 << ( i % 32 ) );
                    }
                }
            }
            else if( dec )
            {
                m_ledStrip[ i ].SetLedValues( changeRed ? double( m_ledStrip[ i ].GetRedD() - redMod - ( redMod * modScale ) ) : double( 0 ),
                                              changeGreen ? double( m_ledStrip[ i ].GetGreenD() - greenMod - ( greenMod * modScale ) ) : double( 0 ),
                                              changeBlue ? double( m_ledStrip[ i ].GetBlueD() - blueMod - ( blueMod * modScale ) ) : double( 0 ) );

                // Remove lights that have finished dec
                bool ledIsBase = ( m_ledStrip[ i ].GetRed() <= 0 ) &&
                                 ( m_ledStrip[ i ].GetGreen() <= 0 ) &&
                                 ( m_ledStrip[ i ].GetBlue() <= 0 ) ;

                if( ledIsBase )
                {
                    if( i < 32 )
                    {
                        activeLowDec &= ~( 1 << i );
                    }
                    else
                    {
                        activeHighDec &= ~( 1 << ( i % 32 ) );
                    }

                    // Turn off if we have hit the base
                    m_ledStrip[ i ].SetLedValues( uint8_t( 0 ), uint8_t( 0 ), uint8_t( 0 ) );
                    spawnCount--;
                }
            }
            else
            {
                m_ledStrip[ i ].SetLedValues( uint8_t( 0 ), uint8_t( 0 ), uint8_t( 0 ) );
            }
        }

        WriteLeds();
        OSTimeDly( 1 );
    }

    m_reset = false;
}

void LedStrip::PlayBitCounter()
{
    uint32_t lowRange = 1;
    uint32_t highRange = 0;

    uint32_t bitTest = 0;

    while( m_currentVisual == eLedVisBitCounter  && !m_reset )
    {
        for( int i = 0; i < gLedCount; i++ )
        {
            bitTest = 1 << ( i % 32 );
            bool setBit = ( ( i < 32 ) && ( bitTest & lowRange ) ) ||
                          ( ( i >= 32 ) && ( bitTest & highRange ) );
            if( setBit )
            {
                m_ledStrip[ gLedCount - i - 1 ].SetLedValues( m_setColor );
            }
            else
            {
                m_ledStrip[ gLedCount - i - 1 ].SetLedValues( uint8_t( 0 ), uint8_t( 0 ), uint8_t( 0 ) );
            }
        }
        lowRange++;
        // If we have gone through the low range, add one to our high range counter
        if( lowRange == 0 )
        {
            highRange++;
        }

        WriteLeds();

        // Delay so we can process we posts
        OSTimeDly( 5 );
    }
    m_reset = false;
}

void LedStrip::PlayColorWave()
{
    uint32_t curIndex = 0;
    uint32_t curIndex2 = 38;
    Led curColor;
    Led curColor2;

    while( m_currentVisual == eLedVisColorWave && !m_reset )
    {
        // If we are at the beginning, set the color
        if( curIndex == 0 )
        {
            curColor.SetLedValues( uint8_t( rand() % 64 ), uint8_t( rand() % 64 ), uint8_t( rand() % 64 ) );
        }

        if( curIndex2 == 0 )
        {
            curColor2.SetLedValues( uint8_t( rand() % 64 ), uint8_t( rand() % 64 ), uint8_t( rand() % 64 ) );
        }

        bool curIndHigh = curIndex > curIndex2;

        for( uint32_t i = 0; i < gLedCount; i++ )
        {
            bool useCurInd = false;
            bool useCurInd2 = false;
            if( curIndHigh && i <= curIndex2 )
            {
                useCurInd2 = true;
            }
            else if( curIndHigh && i <= curIndex )
            {
                useCurInd = true;
            }
            else if( !curIndHigh && i <= curIndex )
            {
                useCurInd = true;
            }
            else if( !curIndHigh && i <= curIndex2 )
            {
                useCurInd2 = true;
            }

            if( useCurInd )
            {
                uint32_t scaleIndex = ( ( curIndex - i ) < 9 ) ? curIndex - i : 9;
                double colorScale = scaleIndex / double( 10.0 );
                double  newRed = curColor.GetRedD() - ( curColor.GetRedD() * colorScale );
                double  newGreen = curColor.GetGreenD() - ( curColor.GetGreenD() * colorScale );
                double  newBlue = curColor.GetBlueD() - ( curColor.GetBlueD() * colorScale );

                m_ledStrip[ i ].SetLedValues( newRed, newGreen, newBlue );
            }
            else if( useCurInd2 )
            {
                uint32_t scaleIndex = ((curIndex2 - i) < 9) ? curIndex2 - i : 9;
                double colorScale = scaleIndex / double( 10.0 );
                double  newRed = curColor2.GetRedD() - (curColor2.GetRedD() * colorScale);
                double  newGreen = curColor2.GetGreenD() - (curColor2.GetGreenD() * colorScale);
                double  newBlue = curColor2.GetBlueD() - (curColor2.GetBlueD() * colorScale);

                m_ledStrip[ i ].SetLedValues( newRed, newGreen, newBlue );
            }
        }
         
        curIndex = ( curIndex + 1 ) % ( gLedCount + 10 );
        curIndex2 = ( curIndex2 + 1) % ( gLedCount + 10 );
        WriteLeds();
        OSTimeDly( 1 );
    }

    m_reset = false;
}

void LedStrip::PlayPulse()
{
    ClearLeds();

    bool changeRed = false;
    bool changeGreen = false;
    bool changeBlue = false;

    double redMod = 0.0;
    double greenMod = 0.0;
    double blueMod = 0.0;

    bool inc = true;
    double step = 0.5;
    m_colorChanged = true;

    while( m_currentVisual == eLedVisPulse && !m_reset )
    {
        if( m_colorChanged )
        {
            changeRed = m_setColor.GetRed() != 0;
            changeGreen = m_setColor.GetGreen() != 0;
            changeBlue = m_setColor.GetBlue() != 0;

            redMod = changeRed ? m_setColor.GetRedD() * .025 : 0.0;
            greenMod = changeGreen ? m_setColor.GetGreenD() * .025 : 0.0;
            blueMod = changeBlue ? m_setColor.GetBlueD() * .025 : 0.0;
            m_colorChanged = false;
        }

        bool setLongDelay = false;
        // Increment and decrement as needed
        for( int i = 0; i < gLedCount; i++ )
        {
            // These values are used to determine how long a light should be on, and how fast it should inc and dec
            if( inc )
            {
                m_ledStrip[ i ].SetLedValues( changeRed ? m_ledStrip[ i ].GetRedD() + redMod + ( redMod * step ) : double( 0 ),
                    changeGreen ? m_ledStrip[ i ].GetGreenD() + greenMod + ( greenMod * step ) : double( 0 ),
                    changeBlue ? m_ledStrip[ i ].GetBlueD() + blueMod + ( blueMod * step ) : double( 0 ) );
            }
            else
            {
                m_ledStrip[ i ].SetLedValues( changeRed ? double( m_ledStrip[ i ].GetRedD() - redMod - ( redMod * step ) ) : double( 0 ),
                    changeGreen ? double( m_ledStrip[ i ].GetGreenD() - greenMod - ( greenMod * step ) ) : double( 0 ),
                    changeBlue ? double( m_ledStrip[ i ].GetBlueD() - blueMod - ( blueMod * step ) ) : double( 0 ) );
            }
        }


        if( inc )
        {
            // Move lights that have hit the peak from inc to dec
            bool ledIsMax = (changeRed && (m_ledStrip[ 0 ].GetRed() >= m_setColor.GetRed())) ||
                (changeGreen && (m_ledStrip[ 0 ].GetGreen() >= m_setColor.GetGreen())) ||
                (changeBlue && (m_ledStrip[ 0 ].GetBlue() >= m_setColor.GetBlue()));
            step += 0.5;

            if( ledIsMax )
            {
                inc = false;
            }
        }
        else
        {
            bool ledIsBase = (m_ledStrip[ 0 ].GetRed() <= 0) &&
                (m_ledStrip[ 0 ].GetGreen() <= 0) &&
                (m_ledStrip[ 0 ].GetBlue() <= 0);
            step -= 0.5;

            if( ledIsBase )
            {
                inc = true;
                setLongDelay = true;
            }
        }

        WriteLeds();
        OSTimeDly( setLongDelay ? 5 : 1 );
    }
    m_reset = false;
}

void LedStrip::PlayColorChange()
{
    ClearLeds();

    Led changeColor1;
    Led changeColor2;
    double redDif = 0.0;
    double greenDif = 0.0;
    double blueDif = 0.0;

    uint32_t center = gLedCount / 2;
    bool evenLedCount = ( gLedCount % 2 ) == 0;
    uint32_t step = 0;
    bool setLongDelay = false;
    bool inc = true;
    while( m_currentVisual == eLedVisColorChange && !m_reset )
    {
        if( step == 0 )
        {
            changeColor1.SetLedValues( uint8_t( rand() % 128 ), uint8_t( rand() % 128 ), uint8_t( rand() % 128 ) );
            do
            {
                changeColor2.SetLedValues( uint8_t( rand() % 128 ), uint8_t( rand() % 128 ), uint8_t( rand() % 128 ) );

                redDif = changeColor1.GetRedD() - changeColor2.GetRedD();
                greenDif = changeColor1.GetGreenD() - changeColor2.GetGreenD();
                blueDif = changeColor1.GetBlueD() - changeColor2.GetBlueD();
            } while( ( ( redDif < 0 ? -1 * redDif : redDif ) < 0.5 ) &&
                     ( ( greenDif < 0 ? -1 * greenDif : greenDif ) < 0.5 ) &&
                     ( ( blueDif < 0 ? -1 * blueDif : blueDif ) < 0.5 ) );
            setLongDelay = false;
        }

        for( uint32_t i = 0; i < gLedCount; i++ )
        {
            uint32_t distFromSource = 0;
            // This is a modifier so that we can get both of the middle LEDs in strips with an even number
            uint32_t distMod = evenLedCount ? 1 : 0;
            if( inc )
            {
                // Distance from center
                distFromSource = ( i <= center - distMod ) ? ( center - distMod ) - i : i - center;
            }
            else
            {
                // Distance from closest end
                distFromSource = ( i <= center - distMod ) ? i : ( gLedCount - distMod ) - i;
            }

            if( distFromSource <= step )
            {
                double newRed = changeColor2.GetRedD() + ( redDif * ( distFromSource / double( center ) ) );
                double newGreen = changeColor2.GetGreenD() + ( greenDif * ( distFromSource / double( center ) ) );
                double newBlue = changeColor2.GetBlueD() + ( blueDif * ( distFromSource / double( center ) ) );
                m_ledStrip[ i ].SetLedValues( newRed, newGreen, newBlue );
            }
        }
        step = ( step + 1 ) % ( gLedCount / 2 );
        if( step == 0 )
        {
            setLongDelay = true;
            inc = !inc;
        }

        WriteLeds();
        OSTimeDly( setLongDelay ? 5 : 1 );
    }
    m_reset = false;
}

void LedStrip::Main()
{
    while( 1 )
    {
        switch( m_currentVisual )
        {
            case eLedVisDefault:
            {
                m_reset = false;
                WriteLeds();
                break;
            }
            case eLedVisKnightRider:
            {
                PlayKnightRiderAnim();
                break;
            }
            case eLedVisTwinkle:
            {
                PlayTwinkle();
                break;
            }
            case eLedVisBitCounter:
            {
                PlayBitCounter();
                break;
            }
            case eLedVisColorWave:
            {
                PlayColorWave();
                break;
            }
            case eLedVisPulse:
            {
                PlayPulse();
                break;
            }
            case eLedVisColorChange:
            {
                PlayColorChange();
                break;
            }
            case eLedVisNone:
            default:
            {
                break;
            }
        }
        OSTimeDly( TICKS_PER_SECOND / 2 );
    }
}
