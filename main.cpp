#include "gpio.hpp"
#include <unistd.h>
#include <iostream>

int main() {
    //Initializing two GPIO Pins
    GPIO out(17);
    GPIO in(27);

    //setting their directions
    out.setDirection(GPIO::OUT);
    in.setDirection(GPIO::IN);

    //test
    out.write(true); //set output to high
    sleep(1);  //wait signal to stabilize
    std::cout << "Input reads (should be 1)" << in.read() << std::endl;

    out.write(false); //set output to low
    sleep(1);
    std::cout << "Input reads (should be 0)" << in.read() << std::endl;
    return 0;


}
