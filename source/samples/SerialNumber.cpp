#include "MicroBit.h"
#include "Tests.h"

#define MEMS_MIC                1
#define PIEZO_MIC               2

#define SETTLE_DELAY            1000
#define FACTORY_DATA            0x0007FC00


//
// Configuration to map a given pin to it's associated NRF52 analog ID.
//
static const KeyValueTableEntry nrf52_id_to_mic_config_data[] = {
    {2,PIEZO_MIC},
    {0x3892F209,MEMS_MIC}
};
CREATE_KEY_VALUE_TABLE(nrf52_id_to_mic_config, nrf52_id_to_mic_config_data);

int isMemsMic()
{
    if (nrf52_id_to_mic_config.hasKey(microbit_serial_number()))
        return nrf52_id_to_mic_config.get(microbit_serial_number()) == MEMS_MIC;

    return 0;
}

int isPiezoMic()
{
    if (nrf52_id_to_mic_config.hasKey(microbit_serial_number()))
        return nrf52_id_to_mic_config.get(microbit_serial_number()) == PIEZO_MIC;

    return 0;
}

int isPiezoMic2()
{
    int sense1;
    int sense2;

    uBit.io.speaker.setDigitalValue(0);
    uBit.io.runmic.setDigitalValue(1);
    uBit.sleep(100);

    uBit.io.speaker.setDigitalValue(1);
    target_wait_us(SETTLE_DELAY);

    sense1 = uBit.io.microphone.getAnalogValue();
    {
        uBit.io.speaker.setDigitalValue(0);
        target_wait_us(SETTLE_DELAY);
        sense2 = uBit.io.microphone.getAnalogValue();
    }

    DMESG("[SENSE1: %d] [SENSE2: %d]", sense1, sense2);

    if (abs(sense1 - sense2) > 40)
        return 1;

    return 0;
}

void showSerialNumber()
{
    int piezo = isPiezoMic2();

    DMESG("MBNEXT: [SERIAL_NUMBER:%x] [%s]", microbit_serial_number(), piezo ? "PIEZO" : "MEMS");

    if (piezo)
        uBit.display.print("P");
    else
        uBit.display.print("M");

    while(1)
        uBit.sleep(1000);
}

int 
hasPassedFactoryTests()
{
    uint32_t *state = (uint32_t *)FACTORY_DATA;

    return ((*state == 0) ? 1 : 0);
}

void
record_factory_pass()
{
    uint32_t *state = (uint32_t *)FACTORY_DATA;
    DMESG("FACTORY_PASS");

    if (*state)
    {
        DMESG("STORING...\n");
        // Enable Flash Writes
        NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos);
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy);

        // Configure PINS for GPIO use.
        *state = 0;

        // Disable Flash Writes
        NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
    }
}
