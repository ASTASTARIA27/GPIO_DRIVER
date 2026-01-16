#include "gpio.hpp"
#include "UART.hpp"
#include "I2C.hpp"
#include <unistd.h>
#include <iostream>

int bcdToDecimal(uint8_t hex) {
    return (hex & 0x0F) + ((hex >> 4) * 10);
}

uint8_t decToBcd(int val) {
    return (uint8_t)( (val/10*16) + (val%10) );
}

int main() {
    //Initializing two GPIO Pins
    std::cout << "--- Starting GPIO Test ---" << std::endl;
    GPIO out(17);
    out.setDirection(GPIO::OUT);
    
    GPIO in(27);
    in.setDirection(GPIO::IN);

    // Test High
    out.write(true); 
    sleep(1); 
    std::cout << "Input reads (expect 1): " << in.read() << std::endl;

    // Test Low
    out.write(false); 
    sleep(1);
    std::cout << "Input reads (expect 0): " << in.read() << std::endl;

    // --- 2. UART Setup ---
    std::cout << "--- Starting UART Test ---" << std::endl;

    GPIO tx(14);
    GPIO rx(15);
    tx.setFunction(GPIO::ALT5); // Pin 14 -> TXD1
    rx.setFunction(GPIO::ALT5); // Pin 15 -> RXD1

    uart ui(115200);//baudrate set
    ui.send('a');
    std::cout << "Sent 'a' over UART" << std::endl;
    char received_char = ui.receive();
    std::cout << "Received back: " << received_char << std::endl;

    if (received_char == 'a') {
        std::cout << "UART Loopback Test PASSED!" << std::endl;
    } else {
        std::cout << "UART Loopback Test FAILED!" << std::endl;
    }

    // --- 3. I2C Setup ---
    std::cout << "--- Starting I2C Setup ---" << std::endl;
    GPIO sda(2);
    GPIO scl(3);
    sda.setFunction(GPIO::ALT0); // Pin 2 -> SDA1
    scl.setFunction(GPIO::ALT0); // Pin 3 -> SCL1

    std::cout << "--- Reading Full Time from DS3231 ---" << std::endl;
    try {
        I2C i2c;
        uint8_t device_add = 0x68;
        
        // 1. Set the register pointer back to 0x00 (Seconds)
        uint8_t start_reg = 0x00;
        i2c.write(device_add, &start_reg, 1);

        // 2. Read 3 bytes (Seconds, Minutes, Hours)
        uint8_t time_data[3];
        i2c.read(device_add, time_data, 3);

        // 3. Convert BCD to Decimal
        int seconds = bcdToDecimal(time_data[0]);
        int minutes = bcdToDecimal(time_data[1]);
        int hours   = bcdToDecimal(time_data[2]); 
        // Note: 'hours' might need extra masking if 12/24h bit is set, 
        // but for now, this usually works for 24h mode.

        // 4. Print formatted time
        std::cout << "Current Time: " 
                  << hours << ":" 
                  << minutes << ":" 
                  << seconds << std::endl;
        
    } catch (const std::exception& exc) {
        std::cerr << "I2C Failure: " << exc.what() << std::endl;
    }
    
    return 0;

}
