#ifndef _SPI_INTERFACE_H_
#define _SPI_INTERFACE_H_
#pragma once

#include <ucos.h>

class SpiInterface
{
public:
    /**
    * @brief Grabs the instance of the SPI interface.
    * 
    * @details SpiInterface is setup to function as a singleton, so that only one instance
    * of the interface should every be available.
    */
    static SpiInterface* GetSpiInterface();
    ~SpiInterface();

    /**
    * @brief This functions writes a number of bytes out to the DSPI module.
    *
    * @param[in] bytes The bytes to write out
    * @param[in] count The number of bytes to write
    */
    void WriteSpi( PBYTE bytes, int count );

    /**
    * @brief This is a function that can be run to ensure that DSPI is functioning correctly.  In order to use it,
    * the output on the pins must be monitored.
    */
    void TestSpi();

private:
    /**
     * @brief Constructor for SpiInterface. All of required pins are set here, as is calling the DSPIInit function
     *
     * @details It is private to facilitate the Singleton pattern.
     */ 
    SpiInterface();

    static SpiInterface* m_singleton;
};

#endif
