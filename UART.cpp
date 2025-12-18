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
#define AUX_MU_LSR_REG 0x54
#define AUX_MU_CNTL_REG 0x60
#define AUX_MU_BAUD_REG 0x68
#define system_clock_freq 250000000

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

    //enabling UART
    *(uart_base+AUX_ENABLES/4) |= 1; //if you dont use '|' it will overwrite all bits in the register to 1.


    /*
    0x3  in hex = 0000 0011 in binary
                    ↑ ↑
                    | └── Bit 0 = 1 → enables **bit 0**
                    └──── Bit 1 = 1 → enables **bit 1**

        Bits (1:0)  Word Length 
        
        `00`        7 bits      
        `**11**`    **8 bits**  


    */

    //configuring data format
    *(uart_base+AUX_MU_LCR_REG/4) =0x3;

    //disabling interrupts
    *(uart_base+AUX_MU_IER_REG/4) =0x0;

    
    /*Bit:     7 6 5 4 3 2 1 0
      Value:   1 1 0 0 0 1 1 0
      Binary:  11000110
      Hex:     0xC6
    */
    //Clearing FIF0 TX RX bits and enabling FIFO
    *(uart_base+AUX_MU_IIR_REG/4) = 0xC6;

    //setting baudrate (check the formula in pdf)
    uint32_t baud_reg = (system_clock_freq/(8*baudrate)) -1;
    *(uart_base + AUX_MU_BAUD_REG/4) = baud_reg;

    //re enabling RX and TX 
    *(uart_base+AUX_MU_CNTL_REG/4) =0x3;
}

void uart::unmapUart() {
    if(uart_base) {
        munmap(const_cast< uint32_t*> (uart_base),UART_BLOCK_SIZE);
        uart_base = nullptr;
    }
}

//send
void uart::send(char c) {
    while(!(*(uart_base + AUX_MU_LSR_REG/4) & (1<<5)));

    *(uart_base + AUX_MU_IO_REG/4) = c;
} 

char uart::receive() {
    int timeout = 1000000;
    while (!(*(uart_base + AUX_MU_LSR_REG/4) & 1) && timeout--);
    if (timeout <= 0) {
        throw std::runtime_error("UART receive timeout");
    }
    return *(uart_base + AUX_MU_IO_REG/4) & 0xFF;
}


