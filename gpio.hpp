#ifndef GPIO_HPP
#define GPIO_HPP
#include <cstdint>

class GPIO{
    private:
    int pin;
    volatile uint32_t* gpio_base; //we use volatile to say the compiler this maybe used for by something later
    void mapGPIO();
    void unmapGPIO();

    public:
    GPIO(int pin); //constructor
    ~GPIO(); //destructor
    enum Direction {IN,OUT};
    enum FunctionMode {
        INPUT  = 0b000,
        OUTPUT = 0b001,
        ALT0   = 0b100, // 4
        ALT1   = 0b101, // 5
        ALT2   = 0b110, // 6
        ALT3   = 0b111, // 7
        ALT4   = 0b011, // 3
        ALT5   = 0b010  // 2  <-- You need this one for Mini UART
    };
    void setDirection(Direction dir);
    void setFunction(FunctionMode mode);
    void write(bool value);
    bool read();
};



#endif
