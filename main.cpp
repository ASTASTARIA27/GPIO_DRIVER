#include "gpio.hpp"
#include "UART.hpp"
#include <unistd.h>
#include <iostream>

int main() {
    //Initializing two GPIO Pins
    GPIO out(17);
    out.setDirection(GPIO::OUT);
    GPIO in(27);
    in.setDirection(GPIO::IN);

    //test
    //write high to GPIO17 then read GPIO 27
    out.write(true); //set output to high
    sleep(1);  //wait signal to stabilize
    std::cout << "Input reads (should be 1)" << in.read() << std::endl;

    //write low to GPIO 17 the read to GPIO27
    out.write(false); //set output to low
    sleep(1);
    std::cout << "Input reads (should be 0)" << in.read() << std::endl;

    uart ui(125000);
    ui.send('a');

    char receive = ui.receive();
    std::cout << "Received:" << receive << std::endl;
    return 0;


}
