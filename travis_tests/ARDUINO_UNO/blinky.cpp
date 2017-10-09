#include "ArduinoUno.h"

ArduinoUno uno;

int main()
{
    int status = 0;
    while(1)
    {
        uno.io.D13.setDigitalValue(status);
        uno.sleep(1000);
        status = !status;
    }
}