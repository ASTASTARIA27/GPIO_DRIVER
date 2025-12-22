#ifndef I2C_HPP
#define I2C_HPP
#include <cstdint>
#include <iostream>

class I2C {
    private:
    volatile uint32_t* i2c_base;
    void mapi2c();
    void unmapi2c();

    public:
    I2C();
    ~I2C();

    void write(uint8_t add,uint8_t *data,uint8_t len);
    void read(uint8_t add,uint8_t *buff,uint8_t len);


};

#endif