#include "simulator_bt_timer_gan.h"

timer_gan my_timer("name");
int incomingByte = 0;
void setup(){
  Serial.begin(115200);
  my_timer.start_bt();
} 

void loop() {
  Serial.println(my_timer.deviceConnected);
  if (my_timer.deviceConnected){
      my_timer.hands_on();
      delay(500);
      while (!my_timer.ready(1200)){
        delay(500);
      }
      Serial.println("ready");
      delay(500);
      my_timer.start();
      Serial.println("start");
      delay(5340);
      my_timer.stop();
      Serial.println("stop");
      delay(1000);
      my_timer.reset();
      Serial.println("reset");
      delay(100000);
  }
  
}
