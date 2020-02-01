#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

#define DRIVER_R_IN1 A0
#define DRIVER_R_IN2 A1
#define DRIVER_R_ENA 5

#define DRIVER_L_IN3 A2
#define DRIVER_L_IN4 A3
#define DRIVER_L_ENB 6

#define RFID_T_SS 8
#define RFID_B_SS 10
#define RFID_RST 9

#define BT_RX 2
#define BT_TX 3

MFRC522 rfid_top(RFID_T_SS, RFID_RST);     // Create MFRC522 instance
MFRC522 rfid_bottom(RFID_B_SS, RFID_RST);  // Create MFRC522 instance

SoftwareSerial bluetooth(BT_TX, BT_RX);

int driver_left[3] = {DRIVER_R_IN2, DRIVER_R_IN1, DRIVER_R_ENA};
int driver_right[3] = {DRIVER_L_IN3, DRIVER_L_IN4, DRIVER_L_ENB};

void setup() {
  // put your setup code here, to run once:
  pinMode(driver_left[0], OUTPUT);
  pinMode(driver_left[1], OUTPUT);
  pinMode(driver_left[2], OUTPUT);

  pinMode(driver_right[0], OUTPUT);
  pinMode(driver_right[1], OUTPUT);
  pinMode(driver_right[2], OUTPUT);

  Serial.begin(9600);
  SPI.begin();

  rfid_top.PCD_Init(RFID_T_SS, RFID_RST);
  rfid_top.PCD_DumpVersionToSerial();

  rfid_bottom.PCD_Init(RFID_B_SS, RFID_RST);
  rfid_bottom.PCD_DumpVersionToSerial();

  bluetooth.begin(9600);
  Serial.println(F("Done setup..."));
}

bool card_present(MFRC522 rfid) {
  byte bufferATQA[2];
  byte bufferSize = sizeof(bufferATQA);
  rfid.PCD_WriteRegister(0x24, 0x26);
  int status = rfid.PICC_RequestA(bufferATQA, bufferSize);
  
  bool result = rfid.PICC_IsNewCardPresent();
  return result;
};

void dump_serial(MFRC522 rfid){

  if ((!rfid.PICC_IsNewCardPresent()) || (!rfid.PICC_ReadCardSerial())) {
    // Serial.println(F("No card"));
    return ;
  }

  log("collected ");
  log(String(rfid.uid.uidByte[0], HEX).c_str());
  
//  for (byte i = 0; i < rfid.uid.size; i++) {
//    log(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
//    log(String(rfid.uid.uidByte[i], HEX));
//  }

  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

void set_speed(int driver[3], int speed) {
  if (speed == 0) {
    digitalWrite(driver[0], LOW);
    digitalWrite(driver[1], LOW);

    return ;
  }

  digitalWrite(driver[0], (speed > 0 ? HIGH : LOW));
  digitalWrite(driver[1], (speed > 0 ? LOW : HIGH));

  analogWrite(driver[2], min(255, abs(speed)));
}

void log(char* s) {
  Serial.println(s);
  bluetooth.print(s); 
}

void loop() {
  log("Waiting for card...");
  while (!rfid_top.PICC_IsNewCardPresent()) {
    delay(100);
     // Halt PICC
    rfid_top.PICC_HaltA();
    // Stop encryption on PCD
    rfid_top.PCD_StopCrypto1();

    // clear bluetooth
    while (bluetooth.available()) {
      bluetooth.read();
    };
  };

  log("GOOOOOO");

  set_speed(driver_left, 0);
  set_speed(driver_right, 0);
  
  while (card_present(rfid_top)) {
    dump_serial(rfid_bottom);
    if (bluetooth.available() >= 2)
    {
      int left = (int) bluetooth.read();
      int right = (int) bluetooth.read();

      set_speed(driver_left, (left - 128) * 3);
      set_speed(driver_right, (right - 128) * 3);

      Serial.print(left, DEC);
      Serial.print(" "); 
      Serial.print(right, DEC);
      Serial.println(); 
    }
  }
  log("Died");
  bluetooth.print(F("Died ...")); 
  set_speed(driver_left, 0);
  set_speed(driver_right, 0);
}
