
#include <Arduino.h>
#include "simulator_bt_timer_gan.h"


#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID               "0000fff0-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID_notify "0000fff5-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID_read   "0000fff2-0000-1000-8000-00805f9b34fb"


bool timer_gan::deviceConnected = false;
class MyServerCallbacks: public BLEServerCallbacks{
  void onConnect(BLEServer* pServer) {
    timer_gan::deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    timer_gan::deviceConnected = false;
    pServer->getAdvertising()->start();
  }
};


BLECharacteristic *pCharacteristic_notify = NULL;
BLECharacteristic *pCharacteristic_read = NULL;
BLE2901 *descriptor_2901 = NULL;
void uint16ToUint8Array(uint16_t value, uint8_t *array) {
    array[0] = (uint8_t)(value >> 8); // Octet de poids fort
    array[1] = (uint8_t)(value & 0xFF); // Octet de poids faible
}
uint16_t crc16(char* pData, int length){
    uint8_t i;
    uint16_t wCrc = 0xffff;
    while (length--) {
        wCrc ^= *(unsigned char *)pData++ << 8;
        for (i=0; i < 8; i++)
            wCrc = wCrc & 0x8000 ? (wCrc << 1) ^ 0x1021 : wCrc << 1;
    }
    return wCrc & 0xffff;
}
timer_gan::timer_gan(char* name){
  bool is_runnig = false;
  bool is_HANDS_ON = false;
  bool is_ready = false;
  unsigned long list_timer = 0;
  unsigned long offset =0;

  
  gan_name = String("gan-" + String(name));
}
void timer_gan::start_bt(){

  BLEDevice::init(gan_name);
  
  /*
  pCharacteristic_read = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_read,
                      BLECharacteristic::PROPERTY_READ);
  */

  BLEServer *pServer = BLEDevice::createServer();
  
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic_notify = pService->createCharacteristic(CHARACTERISTIC_UUID_notify, BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic_notify->addDescriptor(new BLE2902());
  pService->start();
  pServer->getAdvertising()->start();
};
unsigned long int timer_gan::timer_get(){
  return millis() - offset;
}
void timer_gan::timer_reset(){
  offset = millis();
};
unsigned long timer_gan::Get_timer(){
  if (is_runnig){
    return timer_get();
  }
  else{
    return list_timer;
  }
};
void timer_gan::hands_on(){
  if (!is_runnig && !is_HANDS_ON){
    timer_reset();
    is_HANDS_ON = true;
    if (deviceConnected){
      uint8_t HANDS_ON[] = {0xfe,0x04,0x01,0x06,0xf8,0x4e};
      int int_HANDS_ON = 6;
      pCharacteristic_notify->setValue(HANDS_ON, int_HANDS_ON);
      pCharacteristic_notify->notify();
    }
  }
};
bool timer_gan::ready(int delet_time=1200){
  if (!is_ready){
    if (is_HANDS_ON && timer_get() >= delet_time){
      if (deviceConnected){
        uint8_t GET_SET[] = {0xfe,0x04,0x01,0x01,0x1f,0x3e};
        int int_GET_SET = 6;
        pCharacteristic_notify->setValue(GET_SET, int_GET_SET);
        pCharacteristic_notify->notify();
      }
      is_ready = true;
      return true;
    }else{
      return false;
    }
  }else{
    return true;
  }
};//delet_timer in ms 
void timer_gan::hands_off(){
  if (!is_runnig && is_HANDS_ON){
    timer_reset();
    is_ready = false;
    is_HANDS_ON = !is_HANDS_ON;
    if (deviceConnected){
      uint8_t HANDS_OFF[] = {0xfe,0x04,0x01,0x02,0x7C,0x0e};
      int int_HANDS_OFF = 6;
      pCharacteristic_notify->setValue(HANDS_OFF, int_HANDS_OFF);
      pCharacteristic_notify->notify();
    }
  }
};
void timer_gan::start(){
  if (is_runnig && !is_ready){
    return;
  }else{
    is_runnig = true;
    is_ready = false;
    is_HANDS_ON = false;
    timer_reset();
    if (deviceConnected){
      uint8_t runing[] = {0xfe,0x04,0x01,0x03,0x5d,0x1e};
      int int_runing = 6;
      pCharacteristic_notify->setValue(runing, int_runing);
      pCharacteristic_notify->notify();
    }
    return;
  }
};
void timer_gan::stop(){
  if (is_runnig){
    is_runnig = false;
    uint8_t stops[] = {0xfe,0x08,0x01,0x04,0x00,0x00,0x00,0x00,0x00,0x00};
    int int_stops = 10;
    int fs = timer_get();
    list_timer =fs;
    int m = fs/ 1000 /60;
    int s = fs/ 1000 - m * 60;
    int ms = (fs - (m * 60 + s)*1000);
    char pData[6];
    stops[4] = m;
    stops[5] = s;
    uint16_t ms16 = ms;
    uint8_t ms8[2];
    uint16ToUint8Array(ms16,ms8);
    stops[6] = ms8[1];
    stops[7] = ms8[0];
    int i = 0;
    for (i = 0; i < 6;i++){
      pData[i] = stops[i+2];
    }
    uint8_t byteArdray [2];
    uint16ToUint8Array(crc16(pData,6), byteArdray);
    stops[8] = byteArdray[1];
    stops[9] = byteArdray[0];
    if (deviceConnected){
      pCharacteristic_notify->setValue(stops,int_stops);
      pCharacteristic_notify->notify();
    }
  }
};
void timer_gan::reset(){
  is_runnig = false;
  int m = list_timer/ 1000 /60;
  int s = list_timer/ 1000 - m * 60;
  int ms = (list_timer/ 1000 - (m * 60 + s)*1000);
  uint8_t IDLE[] = {0xfe,0x08,0x01,0x05,0x00,0x00,0x00,0x00,0x00,0x00};
  int int_IDLE = 10;
  char pData[6];
  IDLE[4] = m;
  IDLE[5] = s;
  uint16_t ms16 = ms;
  uint8_t ms8[2];
  uint16ToUint8Array(ms16,ms8);
  IDLE[6] = ms8[1];
  IDLE[7] = ms8[0];
  int i = 0;
  for (i = 0; i < 6;i++){
    pData[i] = IDLE[i+2];
  }
  uint8_t byteArdray [2];
  uint16ToUint8Array(crc16(pData,6), byteArdray);
  IDLE[8] = byteArdray[1];
  IDLE[9] = byteArdray[0];
  if (deviceConnected){
    pCharacteristic_notify->setValue(IDLE,int_IDLE);
    pCharacteristic_notify->notify();
  }
};
