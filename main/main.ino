#define DRIVER_L_IN1 A0
#define DRIVER_L_IN2 A1
#define DRIVER_L_ENA 5

#define DRIVER_R_IN3 A2
#define DRIVER_R_IN4 A3
#define DRIVER_R_ENB 6

void setup() {
  // put your setup code here, to run once:
  pinMode(DRIVER_L_IN1, OUTPUT);
  pinMode(DRIVER_L_IN2, OUTPUT);
  pinMode(DRIVER_L_ENA, OUTPUT);

  pinMode(DRIVER_R_IN3, OUTPUT);
  pinMode(DRIVER_R_IN4, OUTPUT);
  pinMode(DRIVER_R_ENB, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(DRIVER_L_IN1, LOW);
  digitalWrite(DRIVER_L_IN2, HIGH);

  analogWrite(DRIVER_L_ENA, 64);
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(1000);

  analogWrite(DRIVER_L_ENA, 255);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
