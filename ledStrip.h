#ifndef _LED_STRIP_H_
#define _LED_STRIP_H_
#pragma once

#include "led.h"

const int gLedCount = 64;

enum LedVis
{
    eLedVisNone = 0,
    eLedVisDefault,
    eLedVisKnightRider,
    eLedVisTwinkle,
    eLedVisBitCounter,
    eLedVisColorWave,
    eLedVisPulse,
    eLedVisColorChange,

    eLedVisCount
};

class LedStrip
{
public:
    LedStrip();
    ~LedStrip(){}

    /**
     * @brief Initializes the LED strip and sends the first two clear bytes via SPI.
     */
    bool InitializeLedStrip();

    /**
     * @brief Sets the color of the specified LED to a value between 0-127.
     *
     * @param[in] i The index of the LED to assign the color to
     * @param[in] r The value to assign red
     * @param[in] g The value to assign green
     * @param[in] b The value to assign blue
     */
    void SetLedValues( int i, uint8_t r, uint8_t g, uint8_t b );
    
    /**
     * @brief Sets the color of the specified LED to a value between 0.0-1.0.
     *
     * @param[in] i The index of the LED to assign the color to
     * @param[in] r The value to assign red
     * @param[in] g The value to assign green
     * @param[in] b The value to assign blue
     */
    void SetLedValues( int i, double r, double g, double b );
    
    /**
     * @brief Sets the active color that is used in animations, with color values between 0-127.
     *
     * @param[in] r The value to assign red
     * @param[in] g The value to assign green
     * @param[in] b The value to assign blue
     */
    void SetActiveColor( uint8_t r, uint8_t g, uint8_t b );
    
    /**
     * @brief Sets the active color that is used in animations, with color values between 0.0-1.0.
     *
     * @param[in] r The value to assign red
     * @param[in] g The value to assign green
     * @param[in] b The value to assign blue
     */
    void SetActiveColor( double r, double g, double b );

    /**
     * @brief Gets the color for a specified index in the LED strip.
     *
     * @param[in] i The index of the LED to get the color of
     * @param[out] v The array that will have the color values stored in RGB format
     */
    void GetLedValues( int i, uint8_t (&v)[3] );

    /**
     * @brief Get the active color used in animations.
     */
    const Led& GetActiveColor(){ return m_setColor; }

    /**
     * @brief Get the current visual that is set to play on the strip.
     */
    inline LedVis GetCurVis(){ return m_currentVisual;  }

    /**
     * @brief Set the current visualization to play on the strip
     *
     * @param[in] v The id of the visualization to play
     */
    void SetCurVis( LedVis v );

    /**
     * @brief Clear the color from all of the LEDs on the strip.
     */
    void ClearLeds();

    /**
     * @brief Write out the values of each of the LEDs to the strip via SPI.
     */
    void WriteLeds();

    /**
     * @brief Plays the Knight Rider animation.
     */
    void PlayKnightRiderAnim();
    
    /**
    * @brief Plays the Twinkle animation.
    */
    void PlayTwinkle();

    /**
    * @brief Plays the Bit Counter animation.
    */
    void PlayBitCounter();

    /**
    * @brief Plays the Color Wave animation.
    */
    void PlayColorWave();
    
    /**
    * @brief Plays the Pulse animation.
    */
    void PlayPulse();
    
    /**
    * @brief Plays the Color Change animation.
    */
    void PlayColorChange();

    /**
    * @brief The main loop of the class that detects when the set animation has changed, and calls the new animation.
    */
    void Main();

private:
    Led m_ledStrip[ gLedCount ];
    BYTE m_clearBits[ 2 ] = { 0, 0 };
    Led m_setColor;
    LedVis m_currentVisual = eLedVisDefault;
    bool m_reset = false;
    bool m_colorChanged = false;
};

#endif
