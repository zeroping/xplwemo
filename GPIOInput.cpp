#include "GPIOInput.h"

#define GPIO_INPUT_DEV  "/dev/input/event0"

#include <iostream>

#include <exception>
using namespace std;


GPIOInput::GPIOInput()
{
    
    gpiofile.open(GPIO_INPUT_DEV, ios::in | ios::binary);
    if(!gpiofile.is_open())
    {
        throw GPIOKeysException("Can't init keys");
    }
    
    
}

GPIOInput::~GPIOInput()
{
    gpiofile.close();
}

struct input_event GPIOInput::waitForInput() {
    struct input_event ev;
    
    while(true){
        //keep going till we get an actual event
        gpiofile.read((char *)&ev, sizeof(ev));
        return ev;
//         if (ev.type != EV_SYN && ev.value == 1) {
//             /* button pressed, do something ... */
//             return ev;
//             
//         }
    }
}

