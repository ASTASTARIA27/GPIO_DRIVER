#include <iostream>
#include "gpio.hpp"
#include <fcntl.h> //open
#include <sys/mman.h> //mmap,munmap
#include <unistd.h> //close()
#include <stdexcept>
#include "I2C.hpp"

#define I2C_BASE_ADDRESS 0xFE804000
#define I2C_BLOCK_SIZE 4096
#define CONTROL 0x00
#define STATUS  0x04
#define DLEN 0x08
#define SLAVE_ADD 0x0c
#define DATA_FIFO 0x10
#define DIV 0x14
#define DEL 0x18 //data delay
#define CLKT 0x1c //clock stretch timeout

I2C::I2C():i2c_base (nullptr){
    mapi2c();
}

I2C::~I2C() {
   unmapi2c();
}

void I2C::mapi2c() {
    int mem = open("/dev/mem", O_RDWR | O_SYNC);
    if(mem < 0) {
        throw std::runtime_error("Unable to open dev/mem because of i2c");
    }

    i2c_base = static_cast<volatile uint32_t*>(
    mmap(
        nullptr,
        I2C_BLOCK_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        mem,
        I2C_BASE_ADDRESS
    )
    );
    close(mem);

    if(i2c_base == MAP_FAILED) {
        throw std::runtime_error("Mapping failed because of I2C");
    }
}

void I2C::unmapi2c() {
    if(i2c_base) {
        munmap(const_cast<uint32_t *>(i2c_base),I2C_BLOCK_SIZE);
        i2c_base = nullptr;
    }
}