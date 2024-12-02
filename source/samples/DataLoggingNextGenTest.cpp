#include "MicroBit.h"
#include "Tests.h"

// Requires MICROBIT_LOG_MODE value 2
// Sample data for nextgen data logging for testing the MicroBitLog mode.
// Actual next gen data uses a custom HEX build in
// https://github.com/microbit-foundation/adl-recognition/
const char* sample = \
"movement_level,acceleration_mean,model_prediction,model_label1,model_label2,model_label3,model_label4,model_label5,model_label6,model_label7,metadata_key,metadata_value\n"
",,,,,,,,,,version,1\n"
",,,,,,,,,,row_interval_ms,1000\n"
",,,,,,,,,,display_mode,full\n"
",,,,,,,,,,model_version,6\n"
",,,,,,,,,,dev_mode,1\n"
",,,,,,,,,,start,7851\n"
"0,1030\n"
"1,1109\n"
"1,1087\n"
"0,1075\n"
"2,1510,2,27,816,11,144,0,0,0\n"
"2,1536\n"
"2,1479\n"
"2,1789\n"
"1,1261\n"
"2,1593,1,746,0,187,66,0,0,0\n"
"0,1017\n"
"2,1411\n"
"2,1788\n"
"2,1507\n"
"1,1286,1,828,19,148,7,0,0,0\n"
"1,1271\n"
"1,1147\n"
"0,1036\n"
"0,1036\n"
"0,1036,2,0,996,0,0,0,0,0\n"
"0,1036\n"
"0,1036\n"
"0,1036\n"
"0,1037\n"
"0,1036,2,0,996,0,0,0,0,0\n"
",,,,,,,,,,end,37451\n";


void data_logging_timeseries_next_gen()
{
    while (1)
    {
        if (uBit.buttonA.wasPressed())
        {
            uBit.display.scroll("L");
            uBit.log.logString(sample);
        }
        if (uBit.buttonB.wasPressed())
        {
            uBit.display.scroll("C");
            uBit.log.clear(false);
        }
        uBit.sleep(1);
    }
}