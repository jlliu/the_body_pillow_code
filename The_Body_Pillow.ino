
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






//Determine if the pillow is squeezed
boolean  isSqueezed(){
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

boolean heatingMode = false;

void loop() {


  heatingMode = CircuitPlayground.slideSwitch();
  float pressure_hPa = mpr.readPressure();
  float pressure_PSI = pressure_hPa / 68.947572932;

  //Hardness of squeezing -> the intensity of the light/vibration (Amplitude)
  //Length of interaction -> the rate of pulsing

  float pressureDelta = abs(pressure_PSI - restingPressure);
  //Map from .03 -> .12
  int pressureDeltaInt = (int) (pressureDelta*100); 
  if (pressureDeltaInt > 12){
    pressureDeltaInt = 12;
  }
  int amplitude = map(pressureDeltaInt,2,12,100,255);
  int lowerBound = (int) floor(amplitude*0.25);
  
  pulseDelay = map(squeezeCountSession,0,60,120,800);

 
  if (isSqueezed()){
  Serial.println("isSqueezed is true");
    squeezeCountSession++;
//       Activate Heat
      if (heatingMode){
        analogWrite(heatPin,255);
      }

      for (int fadeValue = lowerBound ; fadeValue <= amplitude; fadeValue += 10) {
          Serial.println("fading Up");
          analogWrite(ledPin, fadeValue);
          int motorValue = map(fadeValue,0,amplitude,40,255);
          analogWrite(motorPin, motorValue);
          delay(pulseDelay);
          
      }
      delay(800);
      for (int fadeValue = amplitude ; fadeValue >= lowerBound; fadeValue -=20) {
        Serial.println("fading down");
        analogWrite(ledPin, fadeValue);
        int motorValue = map(fadeValue,0,amplitude,40,255);
        analogWrite(motorPin, motorValue);
        delay(pulseDelay);
      }
      delay(800);

  


  } else {
    //Pillow is not currently squeezed
    if (squeezeCountSession > 1)

        //Initiate a slow windown sequence
       for (int iteration = 0; iteration < 3; iteration++){
         int newAmplitude =  (int) floor(amplitude*(1/(.25+iteration)));
         if (iteration == 0){
               newAmplitude = amplitude;
              //When we get to the end of windown, do a final fadeout
             for (int fadeValue = lowerBound ; fadeValue >= 0; fadeValue -=10) {
              analogWrite(ledPin, fadeValue);
              int motorValue = map(fadeValue,0,amplitude,40,255);
               analogWrite(motorPin, motorValue);
               delay(pulseDelay+50*iteration);
               if(isSqueezed()){
                    break;
               }
             }
            
          }


          //pulse back up
          for (int fadeValue = 0 ; fadeValue <= newAmplitude; fadeValue += 10) {
            analogWrite(ledPin, fadeValue);
             int motorValue = map(fadeValue,0,amplitude,40,255);
            analogWrite(motorPin, motorValue);
            delay(pulseDelay+50*iteration);
              if(isSqueezed()){
                break;
              }
    
          }
           delay(200);

        //Fade from high to low
          for (int fadeValue  = newAmplitude ; fadeValue >= 0; fadeValue -=10) {

            analogWrite(ledPin, fadeValue);
            int motorValue = map(fadeValue,0,amplitude,40,255);
            analogWrite(motorPin, motorValue);
            delay(pulseDelay+100*iteration);

           if(isSqueezed()){
                break;
           }
          }
           delay(200);
          if(isSqueezed()){
                break;
           } else {
            squeezeCountSession = 0;
           }

             
       }

    } else if ( squeezeCountSession == 1){
       //Don't need a full windown session if we only tap the pillow
         squeezeCountSession = 0;
         for (int fadeValue = lowerBound ; fadeValue >= 0; fadeValue -=20) {
              analogWrite(ledPin, fadeValue);
             int motorValue = map(fadeValue,0,amplitude,0,255);
              analogWrite(motorPin, motorValue);
              delay(pulseDelay);
               if(isSqueezed()){
                    break;
               }
         }
      
    } else {
        //Pillow is not squeezed
        analogWrite(ledPin, 0);
        analogWrite(motorPin, 0);
        analogWrite(heatPin,0);
      
    }
  


  }
  
}
