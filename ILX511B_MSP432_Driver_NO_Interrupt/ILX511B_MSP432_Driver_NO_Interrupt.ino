#include "msp.h"
#include <stdint.h>
/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>


#define VOut 42 // Analog input from CCD
#define ROG 63  // Pulse to initiate scan, P6_3
#define CLK 51 // Clock signal to trigger interrupts, P8_0
#define LAMP 64 // PWM signal to lamp, P7_2
#define LED1 78 // Onboard Red LED



//This is  a line to ensure that this doesn't compile properly until we have the right configuration for the ADC


volatile int data[2048];
volatile int pixel_num;
volatile bool reading;
int i_time = 50;

void setup() {
   // Halt the Watchdog timer
  MAP_WDT_A_holdTimer();
   /* Initializing ADC (MCLK/1/4) */
  MAP_ADC14_enableModule();
  MAP_ADC14_initModule(ADC_CLOCKSOURCE_ADCOSC, ADC_PREDIVIDER_1, ADC_DIVIDER_1, ADC_NOROUTE);
  /* Configuring GPIOs (5.5 A0) */
  MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P9, GPIO_PIN0, GPIO_TERTIARY_MODULE_FUNCTION);
  /* Configuring ADC Memory */
  MAP_ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A17, false); // false is for non differential Mode
  MAP_ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM0, true); // true is for repeat mode
  MAP_ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);
  MAP_ADC14_setResolution(ADC_14BIT);
  MAP_ADC14_setSampleHoldTime(ADC_PULSE_WIDTH_32, ADC_PULSE_WIDTH_32);
  MAP_Interrupt_enableMaster();
  /* Enabling/Toggling Conversion */
  MAP_ADC14_enableConversion();
  MAP_ADC14_toggleConversionTrigger();
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
    digitalWrite(LED1, !digitalRead(LED1));
    char command = char(Serial.read());
    if (command == 'S'){
      initiateScan(i_time);
      readLine();    
      sendData();
    }
    if (command == 'L'){
      digitalWrite(LAMP, HIGH);
    }
    if (command == 'F'){
      digitalWrite(LAMP, LOW);    
    }
    if (command == 'I'){
      i_time += 5;
    }
    if (command == 'D'){
      i_time -= 5;
      if (i_time < 5){
        i_time = 5;
      }
    }      
  }
}

void initiateScan(int i_time) {
  while ( P8IN == 0x1);
  while ( P8IN == 0x0);
  P6OUT == 0x8;
  delayMicroseconds(4);
  P6OUT == 0x0;
  delay(i_time);
  while ( P8IN == 0x1);
  while ( P8IN == 0x0);
  P6OUT == 0x8;
  delayMicroseconds(4);
  P6OUT == 0x0;
  pixel_num = 0;
  reading = true;
}

void readLine(){
  for(int i=0; i<2048; i++){
    while(P8IN == 0x0);
    data[i]= MAP_ADC14_getResult(ADC_MEM0);
    while(P8IN == 0x1);
  }
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

