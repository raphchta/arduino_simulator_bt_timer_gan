#ifndef simulator_bt_timer_gan_h
#define simulator_bt_timer_gan_h

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLE2901.h>
    
class timer_gan{
  private:
    BLEServer *pServer;
    BLECharacteristic *pCharacteristic_notify;
    BLECharacteristic *pCharacteristic_read;
    BLE2901 *descriptor_2901;
  public:
    timer_gan(char* name);
    void start_bt();
    unsigned long Get_timer();
    unsigned long timer_get();
    void timer_reset();
    void hands_on();
    bool ready(int delet_time);//delet_timer in ms 
    void hands_off();
    void start();
    void stop();
    void reset();
    unsigned long offset;
    static bool deviceConnected;
    bool is_runnig;
    bool is_HANDS_ON;
    bool is_ready;
    unsigned long list_timer;
    String gan_name;
};
    
#endif