#include <AccelStepper.h>
#include "parser.h"

//AccelStepper Xaxis(1, 2, 5); // pin 2 = step, pin 5 = direction
//AccelStepper Yaxis(1, 3, 6); // pin 3 = step, pin 6 = direction
//AccelStepper Zaxis(1, 4, 7); // pin 4 = step, pin 7 = direction
//AccelStepper Yaxis(1, 3, 6); // pin 3 = step, pin 6 = direction
//AccelStepper Xaxis(1, 4, 7); // pin 4 = step, pin 7 = direction

class StepperDriver {
public:
  StepperDriver(uint8_t stepPin, uint8_t dirPin, uint8_t msg1Pin, uint8_t msg2Pin, uint8_t msg3Pin) {
    
    stp = AccelStepper(1, stepPin, dirPin);
    microstepPins[0] = msg1Pin;
    microstepPins[1] = msg2Pin;
    microstepPins[2] = msg3Pin;
    microStepRes = 8;
    microStepStates[0] = LOW;
    microStepStates[1] = LOW;
    microStepStates[2] = LOW;
    
    for(int i=0; i<3; i++) {
      pinMode(microstepPins[i], OUTPUT);
    }
 
  }
  

  
  void setMicroStepResolution(int res) { //1, 2, 4, 8, 16
    
    if(res == 1) {
      setMicroStepStates(LOW, LOW, LOW);
    } else if (res == 2) {
      setMicroStepStates(HIGH, LOW, LOW);
    } else if (res == 4) {
      setMicroStepStates(LOW, HIGH, LOW);
    } else if (res == 8) {
      setMicroStepStates(HIGH, HIGH, LOW);
    } else if (res == 16) {
      setMicroStepStates(HIGH, HIGH, HIGH);
    }
    
    // don't forget where we are
    int newStepTarget = (stp.targetPosition()/microStepRes)*res;
    int newCurrentPositon = (stp.currentPosition()/microStepRes)*res;
    
    stp.setCurrentPosition(newCurrentPositon);
    stp.moveTo(newStepTarget);    
    
    microStepRes = res;
    writeMicroStepPins();
    
  }
  
  void writeMicroStepPins() {
    for(int i=0; i<3; i++) {
       digitalWrite(microstepPins[i], microStepStates[i]);
    }
  }
  
  void moveTo(float angle) {
    stp.moveTo(angle*microStepRes*1.0f);
  }
  
  AccelStepper stp;
  
protected:
  void setMicroStepStates(uint8_t msg1, uint8_t msg2, uint8_t msg3) {
       microStepStates[0] = msg1;
       microStepStates[1] = msg2;
       microStepStates[2] = msg3; 
  }

  int microstepPins[3];
  int microStepRes = 1;
  int microStepStates[3];
  
  // 
  // range
};

StepperDriver tilt(3,6,16,15,14);
StepperDriver  pan(4,7,19,18,17);

float targetX;
float targetY;
float resolution = 16;
unsigned int long lastMessageTime;

unsigned int long lastSpeedChangeTime;
unsigned int long lastTargetChangeTime;

void setup() {
  
  Serial.begin(38400);
  reset();
   
}

void reset() {
  
  tilt.stp.setMaxSpeed(4000);
  pan.stp.setMaxSpeed(4000);
  
  tilt.stp.setAcceleration(400.0);
  pan.stp.setAcceleration(400.0);
  
  tilt.setMicroStepResolution(1);
  pan.setMicroStepResolution(1);
   
  // reset
  tilt.stp.runToNewPosition(30);
  pan.stp.runToNewPosition(90); 
  tilt.stp.runToNewPosition(0);
  pan.stp.runToNewPosition(0);
  
  tilt.setMicroStepResolution(resolution);
  pan.setMicroStepResolution(resolution);

}

void loop() {  
   
   while (parser::read(100)) {
    
     lastMessageTime = millis();
     
     if (parser::nextTokenIf("m")) {
       int res = parser::nextToken().toInt();
       pan.setMicroStepResolution(res);
       tilt.setMicroStepResolution(res);
       resolution = res;
     }
     
     if (parser::nextTokenIf("a")) {
       float a = parser::nextToken().toFloat();
       pan.stp.setAcceleration(a);
       tilt.stp.setAcceleration(a);
     }

     
     if (parser::nextTokenIf("s")) {
       float s = parser::nextToken().toFloat();
       
       pan.stp.setMaxSpeed(s);
       tilt.stp.setMaxSpeed(s);
        
     }
           

    if (parser::nextTokenIf("p")) {
      float x = parser::nextToken().toFloat();
      float y = parser::nextToken().toFloat();

      if (x >= 0 && x <= 90*16 && y >= 0 && y <= 90*16) {
        //Serial.println("Position: " + String(x) + ", " + String(y));
        targetX = x;
        targetY = y;
        
        tilt.moveTo(y);
        pan.moveTo(x);
        
      }
    }
  }
  
  if(millis() - lastMessageTime > 15000) {
    
    
    if(millis() - lastSpeedChangeTime > 4000) {
      
        tilt.stp.setMaxSpeed(random(10,600));
        tilt.stp.setAcceleration(random(2,400));
      
        pan.stp.setMaxSpeed(random(10,600));
        pan.stp.setAcceleration(random(2,400));
        
        tilt.setMicroStepResolution(pow(2,random(4)));
        pan.setMicroStepResolution(pow(2,random(4)));
        
        lastSpeedChangeTime = millis();
        
    }
    
    
    if(millis() - lastTargetChangeTime > 30000) {
      
      tilt.moveTo(random(0,29.0));
      pan.moveTo(random(0,89.0));
      
      lastTargetChangeTime = millis() + random(15000);
    
    }
    
    
    
    
    
    if(tilt.stp.distanceToGo() == 0 && pan.stp.distanceToGo() == 0) {
      
      //lastTargetChangeTime = millis();
      //tilt.moveTo(random(0,29.0));
      //pan.moveTo(random(0,89.0));
      //tilt.setMicroStepResolution(pow(2,random(4)));

    }
    
    if(pan.stp.distanceToGo() == 0) {
      //pan.moveTo(random(0,89.0));
      //tilt.moveTo(random(0,29.0));
      //pan.setMicroStepResolution(pow(2,random(4)));
      
    }  
  }
  
  tilt.stp.run(); //runSpeed
  pan.stp.run();
  
   /*tilt.stp.runToNewPosition(x*8);
   tilt.stp.runToNewPosition(0);
      
   pan.stp.runToNewPosition(y*8-1);
   pan.stp.runToNewPosition(0);*/
  
  
  
  /* tilt.setMicroStepResolution(1);
   pan.setMicroStepResolution(1);
   
   tilt.stp.runToNewPosition(30);
   tilt.stp.runToNewPosition(0);
  
   pan.stp.runToNewPosition(89);
   pan.stp.runToNewPosition(0);
   
   
   tilt.setMicroStepResolution(2);
   pan.setMicroStepResolution(2);
   
   tilt.stp.runToNewPosition(30*2);
   tilt.stp.runToNewPosition(0);
   
   pan.stp.runToNewPosition(90*2-1);
   pan.stp.runToNewPosition(0);
   
   
  tilt.setMicroStepResolution(4);
   pan.setMicroStepResolution(4);
   
    tilt.stp.runToNewPosition(30*4);
   tilt.stp.runToNewPosition(0);
  
   pan.stp.runToNewPosition(90*4-1);
   pan.stp.runToNewPosition(0);*/
   
   /*
   tilt.setMicroStepResolution(8);
   pan.setMicroStepResolution(8);
   
   tilt.stp.runToNewPosition(30*8);
   tilt.stp.runToNewPosition(0);
      
   pan.stp.runToNewPosition(90*8-1);
   pan.stp.runToNewPosition(0);*/
   
   
   /*
   tilt.setMicroStepResolution(16);
   pan.setMicroStepResolution(16);
   
   tilt.stp.runToNewPosition(30*16);
   tilt.stp.runToNewPosition(0);
   
   pan.stp.runToNewPosition(90*16-1);
   pan.stp.runToNewPosition(0);*/
   
   
   /*pan.stp.moveTo(15);
   Yaxis.runSpeedToPosition();
   
   pan.stp.moveTo(90);
    pan.stp.setSpeed(10);
   pan.stp.runSpeedToPosition();
   
   pan.stp.moveTo(10);
     pan.stp.setSpeed(20);

   pan.stp.runSpeedToPosition();
   
     pan.stp.setSpeed(30);

   pan.stp.moveTo(20);
   pan.stp.runSpeedToPosition();
   
     pan.stp.setSpeed(40);
   pan.stp.moveTo(30);
   pan.stp.runSpeedToPosition();
  */
   
      /*Yaxis.runToNewPosition(0);
   Xaxis.runToNewPosition(0);
   
  
   Yaxis.runToNewPosition(100);
   Xaxis.runToNewPosition(100);
  
   Yaxis.moveTo(0);
   Yaxis.runSpeedToPosition();
  
   Xaxis.moveTo(45);
   Xaxis.runSpeedToPosition();
   
   Yaxis.moveTo(-15);
   Yaxis.runSpeedToPosition();

   Xaxis.moveTo(-45);
   Xaxis.runSpeedToPosition();*/
}
