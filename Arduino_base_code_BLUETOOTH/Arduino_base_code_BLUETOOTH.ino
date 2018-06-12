
// Add Libraries for LCD and Software Serial Connection
#include <rgb_lcd.h>
#include <SoftwareSerial.h>

//Define pins
#define LeftEnable 11
#define LeftPin1 10
#define LeftPin2 9
#define RightEnable 4
#define RightPin1 6
#define RightPin2 5
#define LeftEncoder 3
#define RightEncoder 2
#define BluetoothRxPin 12
#define BluetoothTxPin 13

//Define variables
int RotationCountLeft = 0;       //variable to count number of interrupts on left encoder
int RotationCountRight = 0;      //variable to count number of interrupts on right encoder
int arrayposition = 0;           //variable to locate position to write into the array
int readposition = 0;            //variable to locate position to read from the array
boolean paused = false;

//Initialize the array for commands
String commands[50];

//Initialize Software serial and LCD
SoftwareSerial Bluetooth(BluetoothRxPin, BluetoothTxPin);
rgb_lcd lcd;


void setup() {

  //Initialize the pins
  pinMode(LeftEnable, OUTPUT);
  pinMode(LeftPin1, OUTPUT);
  pinMode(LeftPin2, OUTPUT);
  pinMode(RightEnable, OUTPUT);
  pinMode(RightPin1, OUTPUT);
  pinMode(RightPin2, OUTPUT);
  pinMode(LeftEncoder, INPUT);
  pinMode(RightEncoder, INPUT);

  //Initialize the Serial connections
  Serial.begin(19200);
  Bluetooth.begin(9600);

  //start the LCD and set background colour to red
  lcd.begin(16, 2);
  lcd.setRGB(255, 0, 0); //R, G, B, values from 0 to 255

  //Print the text to the display
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("WAITING");

  //Initialize the interrupts and set their pins and methods
  attachInterrupt(digitalPinToInterrupt(LeftEncoder), Left, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RightEncoder), Right, CHANGE);

}

//Method called by the interrupt to count the left encoder changes
void Left() {
  RotationCountLeft++; 
}

//Method called by interrupt to count the right encoder changes
void Right() {
  RotationCountRight++;

}

//method to check if there is data avaliable on the Serial connection 
void serialCheck() {

  //Check if there is data on the Serial Connection
  if (Bluetooth.available()) {

    //read the string until a newline character is read
    String data = Bluetooth.readStringUntil('\n');

    //to stop the program
    if (data == "STOP") {

      //set the LCD
      lcd.clear();
      lcd.print("WAITING");
      lcd.setRGB(255, 0, 0);

      //write over commands in array
      for (int i = 0; i <= arrayposition; i++) {
        commands[i] = "";
      }

      //reset variables 
      arrayposition = 0;
      readposition = 0;
      paused = false;

      //back to loop
      loop();

    //to pause the program
    } else if (data == "PAUSE") {

      //set pause variable to true
      paused = true;

      //inform the user
      Bluetooth.println("PROGRAM PAUSED");

      //write to the LCD and set the colour to Yellow
      lcd.clear();
      lcd.print("PAUSED");
      lcd.setRGB(255, 255, 0);  //R, G, B, values from 0 to 255
      

      //back to loop
      loop();

    //to restart the program
    }else if (data == "GO") {

        //set pause variable to false
        paused = false;

        //start the program
        go();
      }
    }
  
}

//main loop
void loop() {

  //check if system is paused
  if (paused == false) {

    //run add commands method if system is not paused
    addCommands();

  //if paused, check the serial
  } else {

    //run the serial check method
    serialCheck();
  }
}


//method for adding commands to array
void addCommands() {

  //check if there is data on the serial connection
  if (Bluetooth.available()) {

    //read the string until a newline character is read
    String data = Bluetooth.readStringUntil('\n');

    //check if the data is a commands i.e. starting with a f, b, l, or r
    if (data.startsWith("f") || data.startsWith("b") || data.startsWith("l") || data.startsWith("r")) {

      //write the command to the current array position
      commands[arrayposition] = data;

      //increment the array position
      arrayposition++;

      //inform the user that the command has been added
      Bluetooth.print("COMMAND ADDED:    ");
      Bluetooth.println(data);

      //if the data says GO, inform the user and run the program
    } else if (data == "GO") {

      //inform the user
      Bluetooth.println("RUNNING PROGRAM");

      //write to the LCD and change the colour to green
      lcd.clear();
      lcd.print("RUNNING");
      lcd.setRGB(0, 255, 0);  //R, G, B, values from 0 to 255

      //start the program
      go();

      //if the data says STOP, clear the array and variables
    } else if (data == "STOP") {

      //inform the user
      Bluetooth.println("STOPPING PROGRAM");

      //write to the LCD and change to colour to RED
      lcd.clear();
      lcd.print("WAITING");
      lcd.setRGB(255, 0, 0);  //R, G, B, values from 0 to 255

      //write over commands in the array
      for (int i = 0; i <= arrayposition; i++) {
        commands[i] = "";
      }

      //reset variables 
      arrayposition = 0;
      readposition = 0;
      paused = false;

      //restart the loop
      loop();
    } 
    

  }
}

//method for running the program
void go() {

  //while there are commands to be run
  while (readposition <= arrayposition) {

    //write command into local variable
    String input = commands[readposition];

    //check for new data on serial
    serialCheck();

    //see if the input starts with f  -  for forward motion
    if (input.startsWith("f")) {

      //write distance forward into local varible
      int distanceForward = input.substring(1).toInt();

      //run method forward, passing it the distance needed
      forward(distanceForward);

      //inform the user of the command 
      Bluetooth.print("forward\t\t");
      Bluetooth.println(distanceForward);

      //seee if the input starts with b  -  for backward motion
    } else if (input.startsWith("b")) {

      //write the distance backward into local variable
      int distanceBackward = input.substring(1).toInt();

      //run method backward, passing it the distance needed
      backward(distanceBackward);

      //inform the user of the command 
      Bluetooth.print("backward\t\t");
      Bluetooth.println(distanceBackward);

      //see if the input starts with l  -  for left turning 
    } else if (input.startsWith("l")) {

      //write the degrees into local variable
      int degree = input.substring(1).toInt();

      //run method for left turn, passing it the degrees needed
      left(degree);

      
      //inform the user of the command 
      Bluetooth.print("left turn\t\t");
      Bluetooth.println(degree);

      //see if the input starts with r  -  for right turning
    } else if (input.startsWith("r")) {

      //write the degrees into local variable
      int degree = input.substring(1).toInt();

      //run method for right turn, passing it the degrees needed
      right(degree);

      //inform the user of the command 
      Bluetooth.print("right turn\t\t");
      Bluetooth.println(degree);
    }

    //increment the read posiiton
    readposition++;

    //add slight delay between commands
    delay(500);
  }

  //after all commands have been run, inform the user and reset array and variables

  Bluetooth.println("PROGRAM ENDED");

  //write to the LCD and set the colour to red
  lcd.clear();
  lcd.print("WAITING");
  lcd.setRGB(255, 0, 0);  //R, G, B, values from 0 to 255

  //write over the array of commands
  for (int i = 0; i <= arrayposition; i++) {
    commands[i] = "";
  }

  //reset the variables 
  arrayposition = 0;
  readposition = 0;

  //reset the loop
  loop();

}

//method for forward motion
void forward(int distance) {

  //multiply variable by modifier to get correct distance for robot
  distance = distance * 1.7;

  //while the distance is less than the required, keep moving forward
  while ((RotationCountLeft < distance) && (RotationCountRight < distance)) {

    //enable both motors, and engage forward motion
    digitalWrite(LeftEnable, HIGH);
    digitalWrite(LeftPin1, LOW);
    analogWrite(LeftPin2, 190); //motor power inbalance requires different values 
    digitalWrite(RightEnable, HIGH);
    digitalWrite(RightPin1, LOW);
    analogWrite(RightPin2, 200);
  }

  //disable motors at end of travel
  digitalWrite(LeftEnable, LOW);
  digitalWrite(RightEnable, LOW);

  //reset rotation variables
  RotationCountLeft = 0;
  RotationCountRight = 0;
}

//method for backward motion
void backward(int distance) {

    //multiply variable by modifier to get correct distance for robot
  distance = distance * 1.7;

    //while the distance is less than the required, keep moving backward
  while ((RotationCountLeft < distance) && (RotationCountRight < distance)) {

    //enable both motors and engage backward motion
    digitalWrite(LeftEnable, HIGH);
    digitalWrite(LeftPin2, LOW);
    analogWrite(LeftPin1, 190); //motor power inbalance requires different values 
    digitalWrite(RightEnable, HIGH);
    digitalWrite(RightPin2, LOW);
    analogWrite(RightPin1, 200);
  }

  //disable motors at end of travel 
  digitalWrite(LeftEnable, LOW);
  digitalWrite(RightEnable, LOW);

  //reset rotation variables 
  RotationCountLeft = 0;
  RotationCountRight = 0;
}

//method for right turning
void right(int degree) {

  //divide the degrees by a modifier to get correct turn for robot
  degree = degree / 4.5;


  //while the degrees is less than required, keep turning
  while ((RotationCountLeft < degree) && (RotationCountRight < degree)) {

    //enable both motors and engage turning motion
    digitalWrite(LeftEnable, HIGH);
    digitalWrite(LeftPin2, LOW);
    analogWrite(LeftPin1, 190); //motor power inbalance requires different values 
    digitalWrite(RightEnable, HIGH);
    digitalWrite(RightPin1, LOW);
    analogWrite(RightPin2, 200);
  }

  //disable motors at end of turn
  digitalWrite(LeftEnable, LOW);
  digitalWrite(RightEnable, LOW);

  //reset rotation variables 
  RotationCountLeft = 0;
  RotationCountRight = 0;
}

//method for left turning 
void left(int degree) {


  //divide the degrees by a modifier to get correct turn for robot
  degree = degree / 4.3;

  //while the degrees is less than required, keep turning
  while ((RotationCountLeft < degree) && (RotationCountRight < degree)) {

    //enable both motors and engage turning motion
    digitalWrite(LeftEnable, HIGH);
    digitalWrite(LeftPin2, 190); //motor power inbalance requires different values 
    analogWrite(LeftPin1, LOW);
    digitalWrite(RightEnable, HIGH);
    digitalWrite(RightPin1, 200);
    analogWrite(RightPin2, LOW);
  }

  //disable motors at end of turn
  digitalWrite(LeftEnable, LOW);
  digitalWrite(RightEnable, LOW);

  //reset rotation variables 
  RotationCountLeft = 0;
  RotationCountRight = 0;
}



