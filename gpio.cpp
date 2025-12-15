/*On a Raspberry Pi 4, the "offset" of a GPIO refers to 
its specific position or identification number within a hardware bank or software driver
*/
#include <iostream>
#include "gpio.hpp"
#include <fcntl.h> //open
#include <sys/mman.h> //mmap,munmap
#include <unistd.h> //close()
#include <stdexcept> //exceptions
#include <cstring> //memset - used to set a block of memory to a value

//Defining GPIO base address
#define GPIO_BASE_ADDRESS 0xFE200000
#define GPIO_BLOCK_SIZE 4096

//Defining offsets in bytes
#define GPFSEL0 0x00
#define GPSET0  0x1c
#define GPCLR0  0x28
#define GPLEV0  0x34

//constructor
GPIO::GPIO(int pin): pin_(pin), gpio_base_(nullptr){
    mapGPIO();
}
//destructor
GPIO::~GPIO() {
    unmapGPIO();
}

//RDWR -Read or write access
//O_SYNC - access is synchronous (prevents cpu caching)
void GPIO::mapGPIO() {
    int mem = open("/dev/mem", O_RDWR | O_SYNC);
    if(mem < 0) {
        throw std::runtime_error("failed to open /dev/mem");
    }
    //mapping the physical memory at GPIO_BASE
    gpio_base_ = static_cast<volatile uint32_t*>(
        mmap(
            nullptr, //let kernel pick the size
            GPIO_BLOCK_SIZE, //how many bytes to map(4kb)
            PROT_READ | PROT_WRITE, //Read and write access
            MAP_SHARED, //Shared mapping
            mem,//share changes with other copies
            GPIO_BASE_ADDRESS //offset where gpio register begin

        )
    );

    close(mem);

    if(gpio_base_ == MAP_FAILED) {
        throw std::runtime_error("failed to mmap gpio");
    }
}

void GPIO::unmapGPIO() {
    if(gpio_base_) {
        munmap(const_cast<uint32_t*>(gpio_base_),GPIO_BLOCK_SIZE);
        gpio_base_ = nullptr;
    }
}