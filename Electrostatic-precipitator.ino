//Control board for Electrostatic Precipitator V2 for Gateway to Science Bismarck, ND 
//Created by Joe Meyer (Fab III) for the SMM on 4/23/2019

#include "Button.h"

//#define A0 0 // potentiometer on board to adj something
//#define A1 1 // potentiometer on board to adj something
//#define A2 2 // potentiometer on board to adj something

#define PERCENT_HEATER_WARMUP 100
#define PERCENT_HEATER_SMOKE 40
#define PERCENT_HEATER_IDLE 0

#define BACK_FILTER_PIN 4 //digital out for fan and BACK ESP on/off.
#define HEATER_PIN 5 //digital out for heater pwm signal
#define FRONT_ESP_RELAY_PIN 6
#define SMOKE_START_BTN_PIN 11 //digital in for smoke start button
#define SMOKE_STOP_BTN_PIN 12 //TODO REMOVE this digital in for smoke start button 
#define SMOKE_LED_PIN 10
#define ESP_START_BTN_PIN 8
#define ESP_STOP_BTN_PIN 9 //Better to just toggle with momentary switch?
#define ESP_LED_PIN 7  // investigate LED output on HV supply board

Button Smoke_Start_Btn;
Button ESP_Start_Btn;
Button Smoke_Stop_Btn;  //TODO remove
Button ESP_Stop_Btn;


bool is_Smoking = false, is_Warming = false;
int timeDelayFastWarm = 30000; //time in msec to wait after smoke stops that will trigger a fast warm up on next btn press.
int heatUpTimeOut = 8000; //time in msec to run heater high for quicker warmup.
int heaterTimeOut = 30000; //time in msec that heater should run after last btn press.
int backFilterTimeOut = 300000; // 5 min
long millisSmokeStopped = 0, currentMillis = 0, millisSmokeBtnPressed=0;


void stopSmoking(){
  Serial.println("Smoke off. Heater to idle temp..."); 
  digitalWrite(SMOKE_LED_PIN,LOW); //tun off indicator light
  analogWrite(HEATER_PIN,(PERCENT_HEATER_IDLE *2.55));   
  millisSmokeStopped = millis();
  is_Smoking = false;
}

void stopFiltering(){ //called after inactivity for BackFilterTimeOut time
  Serial.println("Turn off fan, back ESP, and heater"); 
  digitalWrite(BACK_FILTER_PIN,LOW);
  analogWrite(HEATER_PIN,0);    // turn off heater if idling
}



void setup() {
  Serial.begin(9600); // open the serial port at 9600 bps:
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(SMOKE_START_BTN_PIN, INPUT); //digital in for smoke start button
  pinMode(SMOKE_STOP_BTN_PIN, INPUT); //digital in for smoke stop button
  pinMode(SMOKE_LED_PIN, OUTPUT); //indicator light for smoke
  pinMode(ESP_START_BTN_PIN, INPUT); //digital in for ESP start button
  pinMode(ESP_STOP_BTN_PIN, INPUT); //digital in for ESP stop button
  pinMode(ESP_LED_PIN, OUTPUT); //indicator light for ESP
  pinMode(FRONT_ESP_RELAY_PIN, OUTPUT); //indicator light for ESP

  millisSmokeStopped = millis() - timeDelayFastWarm;
  
  
//  pinMode(A0, INPUT); // analog input on board to adj something
//  pinMode(A1, INPUT); // analog input on board to adj something
//  pinMode(A2, INPUT); // analog input on board to adj something

  digitalWrite(FRONT_ESP_RELAY_PIN,LOW);

  Smoke_Start_Btn.setup(SMOKE_START_BTN_PIN, [](int state){
    if (!state){
      currentMillis = millis();

      if ((is_Smoking == false) && ((currentMillis - millisSmokeStopped) >= timeDelayFastWarm)){
        Serial.println("Smoke on, fast warm up"); 
        is_Warming = true;
        analogWrite(HEATER_PIN, (PERCENT_HEATER_WARMUP * 2.55));
      }else{
        Serial.println("Smoke on."); 
        analogWrite(HEATER_PIN, (PERCENT_HEATER_SMOKE * 2.55));   
      }       
      digitalWrite(SMOKE_LED_PIN,HIGH); //tun on indicator light
      is_Smoking = true;      
      millisSmokeBtnPressed = currentMillis;
    }
  }, 50);

  Smoke_Stop_Btn.setup(SMOKE_STOP_BTN_PIN, [](int state){  //TODO remove button
    if (!state){
      stopSmoking();
    }
  }, 50);

  ESP_Start_Btn.setup(ESP_START_BTN_PIN, [](int state){
    if (!state){
      Serial.println("ESP Start");
      digitalWrite(ESP_LED_PIN,HIGH);
      digitalWrite(FRONT_ESP_RELAY_PIN,HIGH);
    }
  }, 50);

  ESP_Stop_Btn.setup(ESP_STOP_BTN_PIN, [](int state){  
    if (!state){
      Serial.println("ESP STOP");
      digitalWrite(ESP_LED_PIN,LOW);    
      digitalWrite(FRONT_ESP_RELAY_PIN,LOW);
    }
  }, 50);

  Serial.println("ESP Controller V2");
  Serial.println("Preheating... ");
  
  analogWrite(HEATER_PIN,(PERCENT_HEATER_IDLE*2.55));    // set heater to X/255 power idle temp
}

void loop() { 
  currentMillis = millis();
  Smoke_Start_Btn.idle();
  ESP_Start_Btn.idle();
  Smoke_Stop_Btn.idle();
  ESP_Stop_Btn.idle();  

  if ((is_Smoking == true) && (heaterTimeOut < (currentMillis - millisSmokeBtnPressed))){
    stopSmoking();
  }
  if ((is_Warming == true) && (heatUpTimeOut < (currentMillis - millisSmokeBtnPressed))){
    Serial.println("Heater warm up over.");
    is_Warming = false;
    analogWrite(HEATER_PIN,(PERCENT_HEATER_SMOKE*2.55));    
  }
  if (backFilterTimeOut > (currentMillis - millisSmokeBtnPressed)){
    stopFiltering();
  }
  
}
