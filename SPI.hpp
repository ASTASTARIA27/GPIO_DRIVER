#ifndef SPI_HPP
#define SPI_HPP
#include <iostream>
#include <cstdint>

class spi {
    private:
    int clock_speed;
    volatile uint32_t* spi_base;
    void mapSpi();
    void unmapSpi();

    public:
    spi(int clock_speed);
    uint8_t transfer(uint8_t data);
    ~spi();
};

#endif