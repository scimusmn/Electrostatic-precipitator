//Control board for Electrostatic Precipitator V2 for Gateway to Science Bismarck, ND 
//Created by Joe Meyer (Fab III) for the SMM on 7/9/2019

#define PERCENT_HEATER_WARMUP 100
#define PERCENT_HEATER_SMOKE 35

#define LED_LIGHTING 4 //Cabinet lighting at the top
#define HEATER_PIN 5 //digital out for heater pwm signal
#define FRONT_ESP_RELAY_PIN 6
#define ESP_LED_PIN 7  // investigate LED output on HV supply board
#define ESP_START_BTN_PIN 8
#define ESP_STOP_BTN_PIN 9 
#define SMOKE_LED_PIN 10  // LED indicator on smoke control box
#define SMOKE_START_BTN_PIN 11 //digital in for smoke start button


bool is_Smoking = false, is_Warming = false, ESP_enabled = false;
int smoke_Duration = 30000, front_ESP_timeout = 60000; //time in msec that heater should run after last btn press.
unsigned long warm_up_duration = 0; //variable for calculated warmup.
unsigned long millisSmokeStopped = 0, millisSmokeBtnPressed=0, millisSmokeStarted = 0; //time stamps for calc duration

void stopSmoking(){
  Serial.println("Smoke off"); 
  digitalWrite(SMOKE_LED_PIN,LOW); //turn off indicator light
  analogWrite(HEATER_PIN,0);   //turn off heating element
  if (is_Smoking) millisSmokeStopped = millis();
  is_Smoking = false;
  is_Warming = false;
}

void stopFrontESP(){
  Serial.println("ESP STOP");
  digitalWrite(ESP_LED_PIN,LOW);     //turn off indicator light
  digitalWrite(FRONT_ESP_RELAY_PIN,LOW);
}

void fastWarmUp(){
  Serial.println("Warming...");
  is_Warming = true;
  analogWrite(HEATER_PIN,255);  // turn on heating element 100%
  long timeSinceSmoke = millis() - millisSmokeStopped; // calc cool down time
  warm_up_duration = ((timeSinceSmoke)/8); //one sec of warm up for every 8 sec of cool down
  if (warm_up_duration > 10000) warm_up_duration = 10000; //cap at 10 sec
}


void setup() {
  Serial.begin(9600); // open the serial port at 9600 bps:
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(LED_LIGHTING, OUTPUT);
  pinMode(SMOKE_START_BTN_PIN, INPUT); //digital in for smoke start button
  pinMode(SMOKE_LED_PIN, OUTPUT); //indicator light for smoke
  pinMode(ESP_START_BTN_PIN, INPUT); //digital in for ESP start button
  pinMode(ESP_STOP_BTN_PIN, INPUT); //digital in for ESP stop button
  pinMode(ESP_LED_PIN, OUTPUT); //indicator light for ESP
  pinMode(FRONT_ESP_RELAY_PIN, OUTPUT); //indicator light for ESP


//Start conditions
  millisSmokeStopped = millis() - 100000; // say smoke stopped 100s ago for max warm up duration.
  Serial.println("ESP Controller V2");  // show sign of life!
  digitalWrite(FRONT_ESP_RELAY_PIN,LOW);  
  analogWrite(HEATER_PIN,0);  
  digitalWrite(LED_LIGHTING, HIGH);
}

void loop() { 
  if (!digitalRead(ESP_STOP_BTN_PIN)) stopFrontESP();

  if (!digitalRead(SMOKE_START_BTN_PIN)){
    millisSmokeBtnPressed = millis();  // store millis at btn press
    ESP_enabled = true;
    if (!is_Smoking){
      millisSmokeStarted = millis();  // store millis when smoke starts
      Serial.println("Smoke on."); 
      digitalWrite(SMOKE_LED_PIN,HIGH); //turn on indicator light
      is_Smoking = true;     
      fastWarmUp();
    }
  }

  if (!digitalRead(ESP_START_BTN_PIN)){
    if (ESP_enabled){
        Serial.println("ESP Start");
        digitalWrite(ESP_LED_PIN,HIGH);
        digitalWrite(FRONT_ESP_RELAY_PIN,HIGH);
      }else{
        digitalWrite(ESP_LED_PIN,HIGH);
        delay(150);
        digitalWrite(ESP_LED_PIN,LOW);
        delay(150);
        digitalWrite(ESP_LED_PIN,HIGH);
        delay(150);
        digitalWrite(ESP_LED_PIN,LOW);
        delay(150);
        digitalWrite(ESP_LED_PIN,HIGH);
        delay(150);
        digitalWrite(ESP_LED_PIN,LOW);
      }
  }

  if ((is_Smoking == true) && (smoke_Duration < (millis() - millisSmokeBtnPressed))){ //timeout smoke
    stopSmoking();
  }
  
  if ((is_Warming == true) && (warm_up_duration < (millis() - millisSmokeStarted))){ //warm up timeout
    Serial.println("Heater warm up over.");
    is_Warming = false;
    analogWrite(HEATER_PIN,(PERCENT_HEATER_SMOKE*2.55));  // turn smoke down to maintainable temp.
    digitalWrite(SMOKE_LED_PIN,HIGH); //turn on indicator light
  }

  if (!is_Smoking && ESP_enabled){  //disable the front ESP if no smoke is being produced
    if (front_ESP_timeout < (millis() - millisSmokeStopped)){
      stopFrontESP();
      ESP_enabled = false;
    }
  }
  
}
