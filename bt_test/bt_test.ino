#include <SoftwareSerial.h>

#define BT_RX 2
#define BT_TX 3
#define BX_AT 4

SoftwareSerial bluetooth(BT_TX, BT_RX);

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
  
  pinMode(BX_AT, OUTPUT);
  digitalWrite(BX_AT,1);  

  bluetooth.println("AT+NAME=backzoid-1");

  Serial.begin(9600);

  digitalWrite(BX_AT, 0); 
  Serial.println(F("Done setup..."));
}

void loop() {
  while (bluetooth.available())
  {
    int bt_input = (int) bluetooth.read();
    Serial.print((char) bt_input);
  }
}
