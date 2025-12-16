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
#define gpio_baseADDRESS 0xFE200000
#define GPIO_BLOCK_SIZE 4096

//Defining offsets in bytes
#define GPFSEL0 0x00
#define GPSET0  0x1c
#define GPCLR0  0x28
#define GPLEV0  0x34

//constructor
GPIO::GPIO(int pin): pin(pin), gpio_base(nullptr){
    mapGPIO();
}
//destructor
GPIO::~GPIO() {
    unmapGPIO();
}

//RDWR -Read or write access
//O_SYNC - access is synchronous (prevents cpu caching)
void GPIO::mapGPIO() {
   int mem1 = open("dev/mem", O_RDWR | O_SYNC);
   if(mem1 < 0) {
    throw std::runtime_error("Failed yo open dev/mem");
   }

   /*gpio_base holds a virtual memory address — chosen by the kernel — that corresponds to the 
   physical memory starting at 0xFE200000, the base of the GPIO registers.*/

   gpio_base = static_cast<volatile uint32_t*>(
        mmap(
            nullptr, //let kerner choose virtual address
            GPIO_BLOCK_SIZE, //Size of GPIO (4kb)
            PROT_READ | PROT_WRITE, //Used to give access permissions of specific regian of memory mainly in mmap() or mprotect()
            MAP_SHARED, //used it to say mapping maybe shared between many processes
            mem1, //file
            gpio_baseADDRESS
        )
    );

   close(mem1);
   if(gpio_base == MAP_FAILED) {
    throw std::runtime_error("Mapping failed");
   }
}

void GPIO::unmapGPIO() {
    if(gpio_base) {
        munmap(const_cast<uint32_t*>(gpio_base),GPIO_BLOCK_SIZE); //const cast is used to remove or add a const or volatile modifier to or form a type
        gpio_base = nullptr;
    }
}

/*
you want to tell whether your pi is an input or output
each GPIO Pin has 3-bit field 000->input and 001->output and other values alternate functions like UART ,SPI etc..
Check BCM2711 Peripherals pdf
GPIO Direction control is controlled by GPIO Function select registers
6 funtion select registers GPFSEL0 TO GPFSEL6 each has nine GPIO pins controlled like GPIO 0-9 etc...
*/

/*
maths
PIN Number 'N'
Register index = N/10 (because each register handle pins)
Bit position = (N%10)*3 (Each pin uses 3 bits)

Pin = 17

17 / 10 = 1 → use GPFSEL1 (which GPFSEL register)

17 % 10 = 7 → pin 7 in this register (where in that register)

7 * 3 = 21 → use bits 21–23
*/

void GPIO::setDirection(Direction dir) {
    int reg_index = pin/10;
    int bit_pos = (pin%10)*3;

    // /4 is because we access memory in 32 bit units(1 word to 4 bytes)
    //we are giving it from the gpio base address
    volatile uint32_t* gpfsel = gpio_base + (GPFSEL0/4) + reg_index;

    /*
    0b111 = binary 111 = deciman 7 = hex 0x7
    it represents a 3-bit mask where all three bits are set to one
    000(fn)      0b000(3-bit binary)  input
    001(fn)      0b001(3-bit binary)  output
    100+(fn)     0b100(3-bit binary)  alternate functions

    so when we set the direction we only want to affect the 3 bits targeting the pin and 
    leave other 29 bits untoched

    for example GPIO17
    int bit_pos = 21;
    value &= ~(0b111 << 21);
    */

    uint32_t value = *gpfsel; //reading current regi(ster value to avoid disturbing other pins
    value &= ~(0b111 << bit_pos);

    //set output bits
    if(dir == OUT) {
        value |= (0b001 << bit_pos); //001 is output
    }
    *gpfsel = value; //assigning modified value to gpfsel
}

