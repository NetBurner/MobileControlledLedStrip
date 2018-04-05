#include "spiInterface.h"

#include <basictypes.h>
#include <dspi.h>
#include <pins.h>
#include <pinconstant.h>
#include <stdio.h>

SpiInterface* SpiInterface::m_singleton = nullptr;

SpiInterface* SpiInterface::GetSpiInterface()
{
    if( m_singleton == nullptr )
    {
        m_singleton = new SpiInterface();
    }

    return m_singleton;
}

SpiInterface::~SpiInterface()
{
    delete m_singleton;
}

SpiInterface::SpiInterface()
{
    J2[ 25 ].function( PINJ2_25_DSPI1_SCK );
    J2[ 27 ].function( PINJ2_27_DSPI1_SIN );
    J2[ 28 ].function( PINJ2_28_DSPI1_SOUT );

    /* Initialize DSPI options
    void DSPIInit( BYTE SPIModule = DEFAULT_DSPI_MODLE, DWORD Baudrate = 2000000,
    BYTE QueueBitSize = 0x8, BYTE CS = 0x0F,
    BYTE CSPol = 0x0F, BYTE ClkPolarity = 0x0,
    BYTE ClkPhase = 0x1, BOOL DoutHiz = TRUE,
    BYTE csToClockDelay = 0, BYTE delayAfterTransfer = 0 )

    DEFAULT_DSPI_MODULE is defined in <dspi.h>, and is use to selet the
    default DSPI module if no argument is given. The standard implementation
    defines this macro to be 1
    */
    DSPIInit( DEFAULT_DSPI_MODULE, 2000000, 0x8, 0x0F, 0x0F, 0, 0, true, 0, 0 );
}

void SpiInterface::TestSpi()
{
    iprintf( "SPI test starting..." );
    const int arraySize = 10;
    BYTE testArray[ arraySize ] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    WriteSpi( testArray, arraySize );
    iprintf( "complete\r\n" );
}

void SpiInterface::WriteSpi( PBYTE bytes, int count )
{
   DSPIStart( DEFAULT_DSPI_MODULE, bytes, nullptr, count, nullptr );
   while( !DSPIdone( DEFAULT_DSPI_MODULE ) )
   {}
}
