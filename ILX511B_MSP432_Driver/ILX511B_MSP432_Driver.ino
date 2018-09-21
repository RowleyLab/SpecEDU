#define VOut A0 // Analog input from CCD
#define ROG 63  // Pulse to initiate scan
#define CLK 51 // Clock signal to trigger interrupts
#define LAMP 64 // PWM signal to lamp
#define LED1 78 // Onboard Red LED


//This is  a line to ensure that this doesn't compile properly until we have the right configuration for the ADC


volatile int data[2048];
volatile int pixel_num;
volatile bool reading;

void setup() {
  analogReadResolution(14);
  Serial.begin(115200); //230400
  establishContact();  
  pinMode(CLK, INPUT);
  pinMode(ROG, OUTPUT);
  digitalWrite(ROG, LOW);
  pinMode(LAMP, OUTPUT);
  digitalWrite(LAMP, LOW);
  delay(2000);
  reading = false;
  pixel_num = 0;
  pinMode(LED1, OUTPUT);
  digitalWrite(LED1, HIGH);
  //attachInterrupt(CLK, readPixel, RISING);
  delay(1000);
}

void loop() {
  if (Serial.available() > 0){
    digitalWrite(LED1, LOW);
    int i_time = Serial.parseInt();
    while (Serial.available() > 0 ){
      Serial.read();
    }
    initiateScan(i_time);
    readLine();    
    sendData();
  }
}

void initiateScan(int i_time) {
  digitalWrite(ROG, HIGH);
  delayMicroseconds(8);
  digitalWrite(ROG,LOW);
  pixel_num = 0;
  reading = true;
}

void readPixel(){  
  if (pixel_num < 2048 && reading){
    data[pixel_num]=analogRead(VOut);
    pixel_num++;    
  }
  else{
    reading = false;
  }
  
}



void readLine() {
  while(reading);
}

void sendData() {
  for (int i=0; i<2048;i++){
    Serial.write(byte(data[i]>>8)); // high byte
    Serial.write(byte(data[i])); // low byte  
  }
  Serial.flush();
}

void establishContact(){
  Serial.println("Spec");
  delay(20);
  Serial.println("Spec");
}

