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

void I2C::write(uint8_t add,uint8_t *data,uint8_t len) {
    //wait until i2c bus is idle
    while(*(i2c_base + STATUS/4) & (1<<0)) {
        //wait
    }

    //7-bit slave address to the register
    *(i2c_base + SLAVE_ADD/4) = 0x01;

    //length of data to dlen registrer
    *(i2c_base + DLEN/4) = len;

    //clear FIFO
    /*there is a specific bit in control offset called clear fifo we used that to clear fifo*/
    *(i2c_base + CONTROL/4) = 1<<4;

    //pushing data into fifo register
    for(int i =0 ; i<len; ++i) {
        *(i2c_base + DATA_FIFO/4) = data[i];
    }

    //write transfer
    *(i2c_base + CONTROL/4) = (1<<15) | (1<<7);

    //wait for done
    while(!(*(i2c_base + STATUS/4) &(1<<1))) {
        //wait
    }

    //check for errors
    if(*(i2c_base + STATUS/4) & ((1<<8)| (1<<9))) {
        throw std::runtime_error("I2C WRITE ERROR");
    }

    //clear status 
    *(i2c_base + STATUS/4) = 0xFFFF;
}

void I2C::read(uint8_t add,uint8_t *buff,uint8_t len) {


}