// Whack a LED
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD variable
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Joystick variables
int sX = A0;     //joystick x axis, analog input
int sY = A1;     //joystick y axis, analog in put
int sSX;         //state of x, reading from sX
int sSY;         //state of y, reading from sY
int sS;          //converted state (may not be most efficient)

// Game variables
int rNum;        //random int choosing the random light to turn on
int wins=0;      //counting consecutive wins
int highScore=0; //saving the highest score of consecutive wins

// Difficulty constants, time to react
const int easy = 500;
const int medium = 325;
const int hard = 250;

// Difficulty and Start pin
const int difficultyPin = 8;
const int startPin = 12;

int difficultyState = 0;
int startState = 0;

// Variable of the time to use
int whackTime = easy; // easy by default
int difficulty = 0;

int ledPins[]={5,2,4,3,6,7,8}; //initializing led's
int pinCount=7;                //number of led pins

boolean gameTime = true;

void setup() {
  Serial.begin(9600);
  pinMode(sX, INPUT);
  pinMode(sY, INPUT);

  // initialize the push buttons
  pinMode(difficultyPin, INPUT);
  pinMode(startPin, INPUT);

  for (int thisPin = 0; thisPin < pinCount; thisPin++) { //assigning all the pins as outputs in a for loop
    pinMode(ledPins[thisPin], OUTPUT);
  }

  // sets the initial text for the lcd
  lcd.init();  //initialize the lcd
  lcd.begin(16, 2);  
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Score: ");
  lcd.setCursor(0, 1);
  lcd.print("High Score: ");
}

void loop() {
  // prints score and high score on the lcd
  
  difficultyState = digitalRead(difficultyPin);
  startState = digitalRead(startPin);

  if (difficultyState == HIGH) {
    // turn LED on:
    delay(500);
    difficulty++;
    determineDifficulty();
    Serial.println("DOES THIS PRINT");
  } 
  if (startState == HIGH) {
    // turn LED on:
    Serial.println("Start print");
    gameTime = true;
    Serial.print("DIFFICULTY: ");
    Serial.println(difficulty);
    
    delay(1000); // gives players time to prepare
    game();
  }
}

void determineDifficulty() {
  switch (difficulty) {
     case 0:
       whackTime = easy;
       break;
     case 1:
       whackTime = medium;
       break;
     case 2:
       whackTime = hard;
       break;
     default:
       difficulty = 0;
       break;
  }
}

void game() {
  while(gameTime){
    rNum=random(4); //generating random choice
    delay(1000);
    digitalWrite(ledPins[rNum], HIGH); //lighting the randomly chosen bulb
  
    Serial.println(whackTime);
    delay(whackTime); //difficulty
    
    // reading the value of the analog stick
    sSX = analogRead(sX); // reading x axis input
    delay(100);
    sSY = analogRead(sY); // reading y axis input
  
    // converting y and x inputs to 4 possibilities. 0 and 1023 are the maximum values on each axis of the joystick, as measured.
    sS=0;
    switch (sSX) {
      case 0:
        sS=1;      // Left
        break;
      case 1023:
        sS=2;      // Right
        break;
    }
  
    switch (sSY) {
      case 0:
        sS=3;      // Up
        break;
      case 1023:
        sS=4;      // Down
        break;
    }
  
    digitalWrite(ledPins[rNum], LOW); // turning off the randomly selected bulb, after the joystick choice was made
    if (sS-1==rNum) { // checking if the user input the correct direction of the lit bulb
      wins++;
      for (int k=0; k<=3; k++) {     // blinking green light indicating correct choice
        digitalWrite(ledPins[4], HIGH);
        delay(50);
        digitalWrite(ledPins[4], LOW);
        delay(50);
      }
    }
    else {
      if (wins>highScore) { // if the consecutive wins are more than the previous highscore, the new highscore is set.
        highScore=wins;
        wins=0;
      }
      for (int i=0; i<=3; i++) {       // blinking red light indicating incorrect choice
        digitalWrite(ledPins[5], HIGH);
        delay(50);
        digitalWrite(ledPins[5], LOW);
        delay(50);
        wins = 0;
        clearScore(); // clear the score line on LCD
      }
      gameTime = false;
    }
    printScores();
  }
}

void clearScore() {
  lcd.setCursor(7, 0);
  lcd.print("         ");
}

void printScores() { // prints the score and high score on lcd
  lcd.setCursor(7, 0);
  lcd.print(wins);
  lcd.setCursor(12, 1);
  lcd.print(highScore);
}

void turnOn(int startLED, int endLED){
  for (int ledPin = startLED; ledPin <= endLED; ledPin++){
    digitalWrite(ledPin, HIGH);
  }
}

void turnOff(int startLED, int endLED){
  for (int ledPin = startLED; ledPin <= endLED; ledPin++){
    digitalWrite(ledPin, LOW);
  }
}
