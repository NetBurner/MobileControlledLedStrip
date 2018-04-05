#ifndef _LED_H_
#define _LED_H_
#pragma once

#include <basictypes.h>

class Led
{
public:
    /**
     * @brief Set the color values of the LED
     *
     * @param[in] r The value to assign red
     * @param[in] g The value to assign green
     * @param[in] b The value to assign blue
     */
    void SetLedValues( uint8_t r, uint8_t g, uint8_t b );

    /**
     * @brief Set the color values of the LED
     *
     * @param[in] r The value to set red to, from 0.0 to 1.0
     * @param[in] g The value to set green to, from 0.0 to 1.0
     * @param[in] b The value to set blue to, from 0.0 to 1.0
     */
    void SetLedValues( double r, double g, double b );

    /**
     * @brief Set the color values of the LED
     *
     * @param[in] l The led to copy the values from
     */
    void SetLedValues( const Led& l );

    /**
     * @brief Write the led values out to SPI pins
     */
    void WriteLedValues();

    // Don't send the color set bit, just the color value
    inline uint8_t GetRed() const{ return m_red - 128; }
    inline uint8_t GetGreen() const{ return m_green - 128; }
    inline uint8_t GetBlue() const{ return m_blue - 128; }

    inline double GetRedD() const { return m_redD; }
    inline double GetGreenD() const { return m_greenD; }
    inline double GetBlueD() const { return m_blueD; }

    /**
    * @brief Gets the max age, in ticks, that this LED will be let for particular animations.
    */
    inline uint32_t GetMaxAge() const { return m_maxAge; }

    /**
    * @brief Sets the max age, in ticks, that this LED will be let for particular animations.
    *
    * @param[in] a The max age of the LED to set.
    */
    inline void SetMaxAge( uint32_t a ){ m_maxAge = a;  }

    bool operator==( const Led& lhs ) const;

private:
    // The first bit is used as a reset bit, so it must always be set to give a color value
    uint8_t m_red = 128;
    uint8_t m_green = 128;
    uint8_t m_blue = 128;

    double m_redD = 0.0;
    double m_greenD = 0.0;
    double m_blueD = 0.0;

    uint32_t m_maxAge = 0;
};

#endif
