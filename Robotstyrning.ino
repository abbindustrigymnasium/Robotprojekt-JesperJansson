#include <Wire.h>
#include <VL53L0X.h>
#include <Servo.h>

#define DO_MotorEnable 0
#define DO_MotorInput3 4
#define DO_MotorInput4 5  
#define XSHUT_pin2 16 //pinnar som styr vilken sensor som kortet ska läsa
#define XSHUT_pin1 10
#define SensorLeft_newAddress 42 //alla sensorer utom den första behöver nya adresser

VL53L0X SensorRight;
VL53L0X SensorLeft;
Servo myservo;

int RightSensor = 0;
int LeftSensor = 0;
int DistanceFram = 0;
int DistanceRight = 0;
int DistanceLeft = 0;
int Pos = 90;

typedef enum RobotStates{
  StateInit,
  StateStopp,
  StateFram,
  StateBak,
};

RobotStates RState;

void measure_distances(){ //funktion för att mäta avstånd
  RightSensor = SensorRight.readRangeContinuousMillimeters();
  LeftSensor = SensorLeft.readRangeContinuousMillimeters();
  if (RightSensor < 140 and LeftSensor < 140){ //vägg kan ej vara så nära vägg både till vänster och höger vilket betyder att avståndet är fram
    DistanceFram = LeftSensor;
  }
  else if (RightSensor - LeftSensor < 20 and RightSensor > LeftSensor){ //om båda sensorerna ger lika värden ger de samma avstånd fram
    DistanceFram = LeftSensor;
  }
  else if (LeftSensor - RightSensor < 20 and LeftSensor > RightSensor){
    DistanceFram = LeftSensor;
  }
  else { //Största värdet är fram och en sida, lägsta är en sida
    if (RightSensor > LeftSensor){
      DistanceRight = LeftSensor;
      DistanceFram = RightSensor;
      DistanceLeft = RightSensor;
    }
    if (LeftSensor > RightSensor){
      DistanceRight = LeftSensor;
      DistanceFram = LeftSensor;
      DistanceLeft = RightSensor;
    }
  }
}


void check_stopp(){ //kollar om bilen ska stanna
  if (RState == StateFram){
    //stannar bilen och kollar värden igen även om den ska fram( DistanceFram > 100)
    if (DistanceFram < 100 or DistanceLeft < 130 or DistanceRight < 130){
      RState = StateStopp; 
    }
  }
  else if (RState == StateBak){ 
    if (DistanceFram > 100){
      RState = StateStopp;
    }
  }
}


void check_fram_bak(){ // kollar om bilen ska köra fram eller bak
  if (DistanceFram > 100){
    RState = StateFram;
  }
  else{
    RState = StateBak;
  }
}


void check_styrning(){
  if(DistanceFram < 800 or DistanceRight < 500 or DistanceLeft < 500){ 
    digitalWrite(DO_MotorEnable, 0); //stannar först så att bilen inte sladdar
    Serial.println("Svänger");
    if (DistanceLeft < DistanceRight){
      Serial.println("Höger");
      while(Pos < 110){ //svänger en grad i taget med delay emellan för mjukare svängar
        Pos += 1;
        myservo.write(Pos);
        delay(5);
      }
    }
    else{
      Serial.println("Vänster");
      while(Pos > 70){
        Pos -= 1;
        myservo.write(Pos);
        delay(5);
      }
    }
  }
  else{
    Pos = 90;
    myservo.write(Pos);
  }
}

void setup() {
  Wire.begin(12,13); //SDA och CLK för sensorerna
  Serial.begin(9600);
  pinMode(DO_MotorEnable, OUTPUT);
  pinMode(DO_MotorInput3, OUTPUT);
  pinMode(DO_MotorInput4, OUTPUT);
  pinMode(XSHUT_pin1, OUTPUT);
  pinMode(XSHUT_pin2, OUTPUT);


  SensorLeft.setAddress(SensorLeft_newAddress);
  pinMode(XSHUT_pin1, INPUT);

  SensorRight.init();
  SensorLeft.init();

  SensorRight.setTimeout(500);
  SensorLeft.setTimeout(500);

  SensorRight.startContinuous();
  SensorLeft.startContinuous();

  myservo.attach(15); //servot har IO-pinne 15

  RState = StateInit;
}

void loop() {

  switch (RState){

    case StateInit:
      delay(5000);
      RState = StateStopp;
      myservo.write(90); //rätar styrningen
    break;

    case StateStopp:
      digitalWrite(DO_MotorEnable, 0);
      digitalWrite(DO_MotorInput3, LOW);
      digitalWrite(DO_MotorInput4, LOW);
      

      measure_distances();

      check_fram_bak();

      check_styrning();

      delay(1000);
    break;

    case StateFram:
      digitalWrite(DO_MotorEnable, 1.0);
      digitalWrite(DO_MotorInput3, HIGH);
      //Serial.println("Fram");
      measure_distances();

      check_stopp();

      check_styrning();
    break;

    case StateBak:
      Pos = 90
      myservo.write(Pos); //rätar upp styrningen så att bilen svänger rakt bakåt
      digitalWrite(DO_MotorEnable, 1.0);
      digitalWrite(DO_MotorInput4, HIGH);
      //Serial.println("Bak");

      measure_distances();

      check_stopp();
  }

}
