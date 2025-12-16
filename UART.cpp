#include "UART.hpp"
#include <fcntl.h> //open
#include <sys/mman.h> //mmap,munmap
#include <unistd.h> //close()
#include <stdexcept> //exceptions

#define BASE_UART_ADDRESS 0xFE215000
#define UART_BLOCK_SIZE 4096
#define AUX_ENABLES 0x04
#define AUX_MU_IO_REG 0x40
#define AUX_MU_IER_REG 0x44
#define AUX_MU_IIR_REG 0x48
#define AUX_MU_LCR_REG 0x4c
#define AUX_MU_MCR_REG 0x50
#define AUX_MU_LSR_REG 0x58
#define AUX_MU_CNTL_REG 0x60
#define AUX_MU_BAUD_REG 0x68

//constructor 
uart::uart(int baudrate) :baudrate(baudrate) , uart_base(nullptr) {
    mapUart();
}

//destructor
uart::~uart() {
    unmapUart();
}

void uart::mapUart() {
    int mem = open("/dev/mem" , O_RDWR | O_SYNC);
    if(mem < 0) {
        throw std::runtime_error("unable to open mem for the uart");
    }
    uart_base = static_cast<volatile uint32_t*>(
        mmap(
            nullptr,
            UART_BLOCK_SIZE,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            mem,
            BASE_UART_ADDRESS

        )
    );
    close(mem);

    if(uart_base == MAP_FAILED) {
        throw std::runtime_error("Mapping failed for UART");
    }
}

void uart::unmapUart() {
    if(uart_base) {
        munmap(const_cast< uint32_t*> (uart_base),UART_BLOCK_SIZE);
        uart_base = nullptr;
    }
}

void init() {

}