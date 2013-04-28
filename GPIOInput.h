#ifndef GPIO_INPUT_H
#define GPIO_INPUT_H

#include "linux/input.h"
#include <string>
#include <fstream>
#include <stdexcept>
using namespace std;
/* GPIO Input
 * Purpose: Opens /dev/input/eventX to read the input events from the gpio_keys driver
 */



class GPIOKeysException : public std::runtime_error
{
public:
    GPIOKeysException(const std::string& msg) 
    : std::runtime_error(msg)
    { }
};


class GPIOInput
{
public:
    GPIOInput();  // create a GPIOInput object to watch the default input
    ~GPIOInput();
    /**
     * @brief block until input is seen, then return the button press that ocurred
     *
     **/
    struct input_event waitForInput();
private:
    ifstream gpiofile;
    
};

#endif