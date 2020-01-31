#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

#define DRIVER_L_IN1 A0
#define DRIVER_L_IN2 A1
#define DRIVER_L_ENA 5

#define DRIVER_R_IN3 A2
#define DRIVER_R_IN4 A3
#define DRIVER_R_ENB 6

#define RFID_T_SS 8
#define RFID_B_SS 10
#define RFID_RST 9

#define BT_RX 2
#define BT_TX 3

MFRC522 rfid_top(RFID_T_SS, RFID_RST);     // Create MFRC522 instance
MFRC522 rfid_bottom(RFID_B_SS, RFID_RST);  // Create MFRC522 instance

SoftwareSerial bluetooth(BT_TX, BT_RX);

void setup() {
  // put your setup code here, to run once:
  pinMode(DRIVER_L_IN1, OUTPUT);
  pinMode(DRIVER_L_IN2, OUTPUT);
  pinMode(DRIVER_L_ENA, OUTPUT);

  pinMode(DRIVER_R_IN3, OUTPUT);
  pinMode(DRIVER_R_IN4, OUTPUT);
  pinMode(DRIVER_R_ENB, OUTPUT);

  Serial.begin(9600);
  SPI.begin();

  rfid_top.PCD_Init(RFID_T_SS, RFID_RST);
  rfid_top.PCD_DumpVersionToSerial();

  rfid_bottom.PCD_Init(RFID_B_SS, RFID_RST);
  rfid_bottom.PCD_DumpVersionToSerial();

  bluetooth.begin(9600);
  Serial.println(F("Done setup..."));
}

void dump_serial(MFRC522 rfid){
  if ((!rfid.PICC_IsNewCardPresent()) || (!rfid.PICC_ReadCardSerial())) {
    // Serial.println(F("No card"));
    return ;
  }

  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }

  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(DRIVER_L_IN1, LOW);
  digitalWrite(DRIVER_L_IN2, HIGH);

  digitalWrite(DRIVER_R_IN3, HIGH);
  digitalWrite(DRIVER_R_IN4, LOW);
  
  analogWrite(DRIVER_L_ENA, 255);
  analogWrite(DRIVER_R_ENB, 255);

  dump_serial(rfid_top);
  dump_serial(rfid_bottom);

  // Если пришли данные по Bluetooth
  while (bluetooth.available())
  {
    // Записываем, что пришло от Bluetooth в переменную bt_input
    int bt_input = (int) bluetooth.read();
    analogWrite(DRIVER_L_ENA, (bt_input - 49) * 64);
    dump_serial(rfid_top);
  
    Serial.println(bt_input);
    Serial.println((char) bt_input);
  }
}
