
#include <Wire.h>
#include "Adafruit_MPRLS.h"

#include <Adafruit_CircuitPlayground.h>

// You dont *need* a reset and EOC pin for most uses, so we set to -1 and don't connect
#define RESET_PIN  -1  // set to any GPIO pin # to hard-reset on begin()
#define EOC_PIN    -1  // set to any GPIO pin to read end-of-conversion by pin
Adafruit_MPRLS mpr = Adafruit_MPRLS(RESET_PIN, EOC_PIN);

int ledPin = 10;   
int motorPin = 9;    
int heatPin = 6;

int heatSensorPin = 12;    
float restingPressure = 999;

int pulseDelay = 120;
int squeezeCountSession = 0;







boolean  isSqueezed(){ // k now contains 6

  float pressure_hPa = mpr.readPressure();
   float pressure_PSI = pressure_hPa / 68.947572932;

      
  if (pressure_PSI >= restingPressure + .02){
        return true;
  }else{
        return false;
}
}


void setup() {


  Serial.begin(115200);
  Serial.println("MPRLS Simple Test");
  if (! mpr.begin()) {
    Serial.println("Failed to communicate with MPRLS sensor, check wiring?");
  }

    
  Serial.println("Found MPRLS sensor");
  analogWrite(heatPin,0);

  restingPressure = mpr.readPressure() / 68.947572932;
  Serial.println("RESTING PRESSURE");
  Serial.println(restingPressure);
  
}



void loop() {
  float pressure_hPa = mpr.readPressure();
  Serial.print("Pressure (PSI): "); Serial.println(pressure_hPa / 68.947572932);
  float pressure_PSI = pressure_hPa / 68.947572932;


   
  //Hardness of squeezing -> the intensity of the light/vibration (Amplitude)
  //Length of interaction -> the rate of pulsing

  float pressureDelta = abs(pressure_PSI - restingPressure);
  Serial.println("pressureDelta");
  Serial.println(pressureDelta);
  //Map from .03 -> .12
  int pressureDeltaInt = (int) (pressureDelta*100); // now i is 3
  if (pressureDeltaInt > 12){
    pressureDeltaInt = 12;
  }
  int amplitude = map(pressureDeltaInt,2,12,100,255);
  
  pulseDelay = map(squeezeCountSession,0,60,120,800);

 
  if (isSqueezed()){
  Serial.println("isSqueezed is true");
    squeezeCountSession++;

//       Activate Heat
//      analogWrite(heatPin,255);
      for (int fadeValue = 60 ; fadeValue <= amplitude; fadeValue += 10) {
          Serial.println("fading Up");
          analogWrite(ledPin, fadeValue);
          int motorValue = map(fadeValue,0,amplitude,40,255);

          analogWrite(motorPin, motorValue);
          delay(pulseDelay);
          
      }
      delay(800);
      for (int fadeValue = amplitude ; fadeValue >= 60; fadeValue -=20) {
        Serial.println("fading down");
        analogWrite(ledPin, fadeValue);
        int motorValue = map(fadeValue,0,amplitude,40,255);
        analogWrite(motorPin, motorValue);
        delay(pulseDelay);
      }
      delay(800);

  


  } else {
    Serial.println("isSqueezed is False");
    if (squeezeCountSession > 1){
       squeezeCountSession = 0;

        //Initiate a slow windown sequence
       for (int iteration = 0; iteration < 3; iteration++){
        //Fade from the minimum value of Active state to 0
          for (int fadeValue = (60-iteration*3) ; fadeValue >= 0; fadeValue -=20) {
            analogWrite(ledPin, fadeValue);
             int motorValue = map(fadeValue,0,amplitude,40,255);
            analogWrite(motorPin, motorValue);
            delay(pulseDelay);

           if(isSqueezed()){
                break;
           }
          }
           delay(200);
          //pulse back up
          for (int fadeValue = 0 ; fadeValue <= (60-iteration*3); fadeValue += 10) {
            analogWrite(ledPin, fadeValue);
             int motorValue = map(fadeValue,0,amplitude,40,255);
            analogWrite(motorPin, motorValue);
            delay(pulseDelay);

              if(isSqueezed()){
                break;
              }
    
          }
           delay(200);

          if (iteration == 2){
              //When we get to the end of windown, do a final fadeout
             for (int fadeValue = (60-iteration*3) ; fadeValue >= 0; fadeValue -=20) {
              analogWrite(ledPin, fadeValue);
             int motorValue = map(fadeValue,0,amplitude,0,255);
            analogWrite(motorPin, motorValue);
              delay(pulseDelay);
    
               if(isSqueezed()){
                    break;
               }
            }
            
          }

          if(isSqueezed()){
                break;
           }
       }

    } else if ( squeezeCountSession == 1){
         squeezeCountSession = 0;
              //Don't need a full windown session if we tap the pillow
         for (int fadeValue = (60) ; fadeValue >= 0; fadeValue -=20) {
              analogWrite(ledPin, fadeValue);
             int motorValue = map(fadeValue,0,amplitude,0,255);
            analogWrite(motorPin, motorValue);
              delay(pulseDelay);
    
               if(isSqueezed()){
                    break;
               }
         }
      
    } else {

        analogWrite(ledPin, 0);
        analogWrite(motorPin, 0);
        analogWrite(heatPin,0);
      
    }
  


  }
  
}
