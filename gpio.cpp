/*On a Raspberry Pi 4, the "offset" of a GPIO refers to 
its specific position or identification number within a hardware bank or software driver

please refer to BCM2711 Peripherals pdf for pi
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
   int mem1 = open("/dev/mem", O_RDWR | O_SYNC);
   if(mem1 < 0) {
    throw std::runtime_error("Failed to open dev/mem");
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


/*
Turn a GPIO ON or OFF by writing to the correct hardware register.
This is only for output pins
GPSET0  Setpins 0 to 31 (sets selected pins to 3.3)   make selected pin HIGH(1<<pin(what writing this does))
GPSET1  Setpins 32 to 53
GPCLR0  Clearpins 0 to 31 make selected pin LOW (sets selected pins to 0v)
GPCLR1 Clearpins 32 to 53

GPSET0 OFFSET IS 0x1c which is 28 in decimal
GPSET1 OFFSET IS 0x20 which is 32 in decimal so each register is 4 bytes apart
*/

/*
(1<<bit)Create a number where only bit number bit is 1, and all other bits are 0.
example bit = 17
1<<17 = 0x00020000
this creates a bit mask that targets only GPIO Pin 17
*/
void GPIO::write(bool value) {
    int register_offset = (pin<32) ? GPSET0 : GPSET0+4;
    int clear_offset = (pin < 32)? GPCLR0 : GPCLR0+4;

    //Compute the bit to write
    int bit = pin%32;

    if(value) {
        *(gpio_base + (register_offset/4)) = (1 << bit);
    } else {
        *(gpio_base + (clear_offset/4)) = (1 << bit);
    }
}

/*
now gpio read is to check whether the GPIO pin is in high or low
GPLEV0  Reads GPIO 0-31   contains(Bitfield : 1 bit per pin)
GPLEV1  Reads GPIO 32-53

int bit = pin%32  if GPIO 17 32/17 = 0 so GPLEV0
IF GPIO 33  bit = 1 so GPLEV1

Apply bitmask 1<<bit to isolate pins value
if the result is non zero pin is High

(1 << bit) → non‑zero

(1 << bit) != 0 → true → becomes 1

level & 1 → checks only bit 0, not bit
*/

bool GPIO::read() {
    int reg_offset = (pin<32) ? GPLEV0 : GPLEV0+4;
    int bit = pin%32;

    uint32_t level = *(gpio_base + reg_offset/4);
    return ((level & (1<<bit)) != 0);
}

void GPIO::setALTFunction(int alt) {
    int reg_index = pin/10;
    int bit_pos = (pin%10)*3;
    volatile uint32_t *gpfsel = gpio_base + (GPFSEL0/4) + reg_index;

    uint32_t value = *gpfsel;
    value &=~(0b111 << bit_pos);
    value |= (alt << bit_pos);
    *gpfsel = value;
}
