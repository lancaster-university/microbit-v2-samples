#ifndef MB_NEXT_TESTS_H
#define MB_NEXT_TESTS_H

void fxos_test();
void blinky();
void mbed_blinky();
void button_test1();
void button_test2();
void button_test3();
void display_test1();
void display_test2();
void fade_test();
void mems_mic_test();
void piezo_mic_test();
void speaker_test(int plays);
void speaker_test2(int plays);
void gpio_test();
void radio_rx_test();
void radio_rx_test2();
void radio_tx_test();
void temperature_test();
void accelerometer_test1();
void compass_test1();
void compass_test2();
void button_blinky_test();
void spirit_level();
void edge_connector_test();
void analog_test();
void piezo_clap_test(int wait_for_clap = 0);
void mems_clap_test(int wait_for_clap = 0);
void showSerialNumber();
void display_wink();
void display_tick();
void display_radio();
void spirit_level2();
void button_blinky_test2();
int isPiezoMic2();
int out_of_box_experience();
int hasPassedFactoryTests();
void record_factory_pass();
void display_arrows();
void factory_radio_transmitter();
void square_wave_test();
void red_power_test();
void green_power_test();
void off_power_test();

int read_light_level();
#endif
