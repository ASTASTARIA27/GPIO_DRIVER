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
    void setDirection(Direction dir);
    void write(bool value);
    bool read();
};



#endif
