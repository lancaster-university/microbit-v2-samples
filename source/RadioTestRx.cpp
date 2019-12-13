#include "MicroBit.h"
#include "Tests.h"

int data_received;

void onData(MicroBitEvent)
{
    PacketBuffer b = uBit.radio.datagram.recv();

    if (b[0] == '1')
        uBit.display.print("A");

    if (b[0] == '2')
        uBit.display.print("B");
}

void onData2(MicroBitEvent)
{
    PacketBuffer b = uBit.radio.datagram.recv();
    DMESG("RECV");

    if (b[0] =='M' && b[1] =='B' && b[2] =='N' && b[3] =='E' && b[4] =='X' && b[5] =='T')
        data_received = 1;
}

void radio_rx_test()
{
    uBit.messageBus.listen(DEVICE_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
    uBit.radio.enable();

    while(1)
        uBit.sleep(1000);
}

void radio_rx_test2()
{
    data_received = 0;

    uBit.messageBus.listen(DEVICE_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData2);
    uBit.radio.enable();

    while(!data_received)
        uBit.sleep(1000);

    uBit.messageBus.ignore(DEVICE_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData2);
}

void radio_tx_test()
{
    uBit.radio.enable();

    while(1)
    {
        if (uBit.buttonA.isPressed())
            uBit.radio.datagram.send("1");

        else if (uBit.buttonB.isPressed())
            uBit.radio.datagram.send("2");

        uBit.sleep(100);
    }
}

