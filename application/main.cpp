#include "CircuitPlayground.h"

CircuitPlayground cplay;

int main()
{
    int status = 1;

    while(1)
    {
        cplay.io.led.setDigitalValue(status);
        cplay.sleep(1000);

        status = !status;
    }
}
