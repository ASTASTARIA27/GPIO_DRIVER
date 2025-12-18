#ifndef UART_HPP
#define UART_HPP
#include <iostream>
#include <cstdint>

class uart {
    private:
    int baudrate;
    volatile uint32_t* uart_base;
    void mapUart();
    void unmapUart();

    public:
    uart(int baudrate); //constructor
    void send(char c); //sould send a char
    char receive(); //should recceive a char
    enum mode {start,stop};
    ~uart(); //destructor
};


#endif