#include "MicroBit.h"
#include "Tests.h"

#include "MicroBitUSBFlashManager.h"

static void
onReadFlashStatus()
{
    DMESG("Reading CONFIG:");    

    //MicroBitUSBFlashConfig config = uBit.flash.getConfiguration();
    MicroBitUSBFlashGeometry geometry = uBit.flash.getGeometry();
    //DMESG("FLASH CONFIG: [filename: %s] [size:%d] [visible:%d]", config.fileName.toCharArray(), config.fileSize, config.visible ? 1 : 0);
    DMESG("FLASH GEOMETRY: [blockSize: %d][blockCount: %d]", geometry.blockSize, geometry.blockCount);
}

static void
onWriteFlashStatus()
{
    DMESG("witeFlashStatus:");

    DMESG("Erasing CONFIG:");
    uBit.flash.eraseConfig();

    MicroBitUSBFlashConfig config;

    config.fileName = "JOE.TXT";
    config.fileSize = 8;
    config.visible = true;

    DMESG("Writing CONFIG:");

    uBit.flash.setConfiguration(config, true);

    DMESG("Done");
}

static void
onReadFlash()
{    
    int address = 90;
    int size = 30;

    DMESG("Reading FLASH... [address: %d] [length: %d]", address, size);

    ManagedBuffer response(size);
    response = uBit.flash.read(address, size);

    /*
    for (int i=0; i<response.length(); i++)
        DMESG("  %x [%c]", response[i], response[i]);
    */
}

static void
onEraseFlash()
{    
    int err;

    DMESG("Erasing FLASH pages...");

    ManagedBuffer response(4);
    err = uBit.flash.erase(0, 1024*4);
    if(err != DEVICE_OK)
        DMESG(" FLASH_ERASE ERROR %d", err);
}

static void
onPartialEraseFlash()
{    
    int err;
    ManagedString s = "*** :) ***";
    ManagedBuffer b((uint8_t *)s.toCharArray(), 10);

    DMESG("Partial FLASH erase...");

    err = uBit.flash.erase(100, 10);
    if(err != DEVICE_OK)
        DMESG(" FLASH_ERASE ERROR: %d", err);

    err = uBit.flash.write(b, 100);
    if(err != DEVICE_OK)
        DMESG(" WRITE ERROR: %d", err);
}

static void
onWriteFlash()
{
    int address = 0;
    int size = 1024;

    DMESG("Writing FLASH...");

    ManagedBuffer b(size);
    char c = 'a';

    for (int i=0; i<size; i++)
    {
        b[i] = c++;
        if (c > 'z')
            c = 'a';
    }

    uBit.flash.write(b, address);
}

static void
onRemount(MicroBitEvent)
{
    DMESG("remount:");
    uBit.flash.remount();
}

static void
onStartFlashTest(MicroBitEvent)
{
   DMESG("FLASH_STORAGE_TEST: STARTING IN 3 SECONDS");
    uBit.sleep(3000);

    DMESG("FLASH_STORAGE_TEST: ERASING 4K PAGES");
    onEraseFlash();
    
    DMESG("FLASH_STORAGE_TEST: WRITING");
    onWriteFlash();

    DMESG("FLASH_STORAGE_TEST: READING");
    onReadFlash();

    DMESG("FLASH_STORAGE_TEST: ERASING bytes");
    onPartialEraseFlash();

    DMESG("FLASH_STORAGE_TEST: READING (AGAIN)");
    onReadFlash();

    DMESG("FLASH_STORAGE_TEST: DONE");

    microbit_dmesg_flush();

}

void flash_storage_test()
{  
    //UNUSED FUNCTIONS
    (void) onReadFlashStatus;
    (void) onWriteFlashStatus;

    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, onStartFlashTest);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, onRemount);

    while(1)
        uBit.sleep(10000);
}