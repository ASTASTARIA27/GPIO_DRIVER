#include "SPI.hpp"  
#include "gpio.hpp"
#include <fcntl.h> //open
#include <sys/mman.h> //mmap,munmap
#include <unistd.h> //close()
#include <stdexcept>

#define SPI_BLOCK_SIZE  4096
#define core_clock      250000000
#define SPI_BASE_ADDRESS 0xFE204000 //base address
#define Control_Status   0x00   //control and status
#define TXRX_FIFO        0x04   //Master TX and RX FIFO
#define CLK_DIV          0x08   //Master Clock DIvider
#define DLEN             0x0c   //Master Data Length 
#define LTOH             0x10   //LoSSI mode TOH
#define DC               0x14   //SPI DMA DREQ controls

#define CS_TA           (1<<7)  //Transfer is active
#define CS_RX           (1<<17) //RX FIFO contains data
#define CS_TX           (1<<18) //TX FIFO can accept data
#define CS_RF           (1<<20) //RX FIFO is not full
#define CS_CLEAR        (3<<4)  //clear FIFO (bits 4 and 5)
 
spi::spi(int clock_speed): clock_speed(clock_speed),spi_base(nullptr) {
    mapSpi();
}
spi::~spi() {
    unmapSpi();
}
void spi::mapSpi() {
    int mem = open("/dev/mem",O_RDWR | O_SYNC);
    if(mem < 0) {
        throw std::runtime_error("Unable to open mem because of SPI");
    }
    spi_base = static_cast<volatile uint32_t*>(
        mmap(
            nullptr,
            SPI_BLOCK_SIZE,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            mem,
            SPI_BASE_ADDRESS
        )
    );
    close(mem);
    if(spi_base == MAP_FAILED) {
        throw std::runtime_error("Mapping failed because of I2C");
    }

    int Divider = core_clock/clock_speed;

    *(spi_base + CLK_DIV/4) = Divider;
}

void spi::unmapSpi() {
    if(spi_base) {
        munmap(const_cast< uint32_t*>(spi_base),SPI_BLOCK_SIZE);
        spi_base = nullptr;
    }
}

uint8_t spi::transfer(uint8_t data) {
    //activate transfer and clear FIFOs
    *(spi_base + Control_Status/4) = CS_TA | CS_CLEAR;

    //wait for TX FIFO to have space
    while(!(*(spi_base + Control_Status/4) & CS_TX)) {
        //wait
    }
    
    //write data in to FIFO
    *(spi_base + TXRX_FIFO/4) = data;

    //wait for RX FIFO to have data
    while(!(*(spi_base + Control_Status/4) & CS_RX)) {
        //wait
    }

    //Read data from FIFO

    uint32_t received = *(spi_base + TXRX_FIFO/4);

    //Deactivate Transfer
    *(spi_base + Control_Status/4) = 0;
    return (uint8_t)(received & 0xFF);
}