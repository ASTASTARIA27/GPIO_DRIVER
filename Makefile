#1.Compiler and Flags
#CXX = c++ compiler for g++
#CXXFLAGS = flags passed to the compiler
#Wall = to show all warnings
#o2 = optimize for speed
CXX = g++
CXXFLAGS = -Wall -O2

#2.Target Name (FInal Output file)
TARGET = gpio_app

#3.Source and object files
SRCS = main.cpp gpio.cpp UART.cpp I2C.cpp SPI.cpp
OBJS = $(SRCS:.cpp=.o)

#4.Defalut Target Rule
# this tell make to build the final executable when you just run make
all:$(TARGET)

#5.How to build the executable
#$@ means the target(gpio_app)
#$^ means all dependicies(main.cpp,gpio.cpp)
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

#6.How to build object files
#This tells 'make' how to turn .cpp files into .o filesbject
#$< = source file
#$@ = object file
&.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o$@

#7.Clean
#so we can run make clean to remove build artificats
clean:
	rm -f $(OBJS) $(TARGET)