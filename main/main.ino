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

#define SPEED_ZERO 64
#define SPEED_RATIO 5

#define SPEED_ZERO_THRESHOLD 50

#define MAX_SPEED_DELTA 80

#define SIGN(v) ((v < 0) ? -1 : 1)

enum MotorSide
{
  LEFT_MOTOR,
  RIGHT_MOTOR
};

MFRC522 rfid_top(RFID_T_SS, RFID_RST);     // Create MFRC522 instance
MFRC522 rfid_bottom(RFID_B_SS, RFID_RST);  // Create MFRC522 instance

SoftwareSerial bluetooth(BT_TX, BT_RX);

const int driver_left[3] = {DRIVER_L_IN4, DRIVER_L_IN3, DRIVER_L_ENB};
const int driver_right[3] = {DRIVER_R_IN1, DRIVER_R_IN2, DRIVER_R_ENA};

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
  int status = rfid.PICC_RequestA(bufferATQA, &bufferSize);
  
  bool result = rfid.PICC_IsNewCardPresent();
  return result;
};

void dump_serial(MFRC522 rfid){

  if ((!rfid.PICC_IsNewCardPresent()) || (!rfid.PICC_ReadCardSerial())) {
    // Serial.println(F("No card"));
    return ;
  }

  log("collected");
  log(String(rfid.uid.uidByte[0], HEX).c_str());
  bluetooth.print('C');
  
//  for (byte i = 0; i < rfid.uid.size; i++) {
//    log(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
//    log(String(rfid.uid.uidByte[i], HEX));
//  }

  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

//left, right
int last_speed[] = {0, 0};

void set_speed(MotorSide side, int speed)
{
  const int* driver = (side == LEFT_MOTOR) ? driver_left : driver_right;

  if (speed == 0)
  {
    digitalWrite(driver[0], LOW);
    digitalWrite(driver[1], LOW);
  }
  else
  {
 	  digitalWrite(driver[0], (speed > 0 ? HIGH : LOW));
  	digitalWrite(driver[1], (speed > 0 ? LOW : HIGH));

  	analogWrite(driver[2], min(255, abs(speed) + SPEED_ZERO_THRESHOLD));
  }
}

void set_speed(int speed)
{
  set_speed(LEFT_MOTOR, speed);
  set_speed(RIGHT_MOTOR, speed);
}

void set_speed(int left, int right)
{
	set_speed(LEFT_MOTOR, left);
	set_speed(RIGHT_MOTOR, right);
}

void log(const char* s)
{
  Serial.println(s);
}

void loop()
{
 log("Waiting for card...");
 bluetooth.print('W');

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
  bluetooth.print('G');

  set_speed(0);
  
 while (card_present(rfid_top))
 {
   dump_serial(rfid_bottom);
   
   if (bluetooth.available())
   {
     const byte value = bluetooth.read();

     //[-320; 315]
     const int speed = SPEED_RATIO * ( (value & 127) - SPEED_ZERO );

     const MotorSide side = (value & 128) ? RIGHT_MOTOR : LEFT_MOTOR;
     const MotorSide other_side = (side == RIGHT_MOTOR) ? LEFT_MOTOR : RIGHT_MOTOR;

     last_speed[side] = speed;
     
     const int speed_delta = last_speed[side] - last_speed[other_side];
    
      if(abs(speed_delta) < MAX_SPEED_DELTA)
      {
      	set_speed(side, last_speed[side]);
      }
      else
      {
      	const int mid_speed = (last_speed[side] + last_speed[other_side])/2;
      
      	set_speed(side, mid_speed + MAX_SPEED_DELTA * SIGN(speed_delta) / 2);
      	set_speed(other_side, mid_speed - MAX_SPEED_DELTA * SIGN(speed_delta) / 2);
      }
      
//      Serial.print(last_speed[LEFT_MOTOR], DEC);
//      Serial.print(" ");
//      Serial.print(last_speed[RIGHT_MOTOR], DEC);
//      Serial.println();
   }
 }
  
//  log("Died");
//  bluetooth.print(F("Died ..."));
 
 set_speed(0);
}
