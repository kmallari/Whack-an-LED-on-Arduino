// Whack an LED
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

//PIN SETUP =============================================

//initialize game LEDs (CCW starting from RIGHT)
int ledPins[] = {6,11,13,8};

//LEDs for remaining lives
int HP_LED[] = {5,4,3};

//joystick
int Vx = A3;  //x-axis
int Vy = A2;  //y-axis
int SW = 0;   //button

//INPUT VARIABLES =======================================
//x and y reading of joystick input
int Sx = 0;
int Sy = 0;
int mapX = 0;
int mapY = 0;

int SSW; //switch state
int prevState = 1; //for checking button presses
int prevDir = 8; //prev input direction of joystick
int prevMode = 3; //prevents printing multiple times on LCD

//GAME VARIABLES ========================================
int rNum;       //random num for light to turn on
int score;      //current score
int highScores[3];  //easy, normal, hard highscores
int showHS = 0; //determines which highscore to show 
int cycleTime = 2000; //time to cycle between highscores while idle

unsigned long prevTime = 0; //for modifying certain timed states
unsigned long currTime;

int HP; //number of lives

// Difficulty constants, time to react in ms
const int easy = 500;
const int medium = 325;
const int hard = 250;

int diffValue = 0; //for difficulty selection

int diffTime[] = {easy, medium, hard}; //duration for current diff
int diffTime_curr;

//difficulty names
String diffName[] = {"EASY", "MEDIUM", "HARD"};
String diffShort[] = {"EZ", "MD", "HD"};

//program state: 0 = idle | 1 = select diff | 2 = in-game | 3 = game over
int mode = 0;

bool activeLED = 0; //if an LED is on during the game

//SETUP =================================================
void setup() {
  Serial.begin(9600);

  //initialize joystick input
  pinMode(Vx, INPUT);
  pinMode(Vy, INPUT);
  pinMode(SW, INPUT_PULLUP);

  //initializing LEDs
  for(int i = 3; i <= 13; i++)
  {
    pinMode(i, OUTPUT);
  }
  
  lcd.init();  //initializing LCD
  lcd.begin(16, 2);  
  lcd.backlight();
}

//MAIN =================================================
void loop()
{
  //read position of joystick
  Sx = analogRead(Vx);
  Sy = analogRead(Vy);
  
  //re-mapping of values
  mapX = map(Sx, 0, 1023, -512, 512);
  mapY = map(Sy, 0, 1023, 512, -512);

  SSW = digitalRead(SW); //check for button press

  //IDLE ===============================================
  if(mode == 0)
  {
    if(modeChange(mode))
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Hi-Score: ");
      lcd.setCursor(0, 1);
      lcd.print("PRESS TO START");
    }

    //cycles through high scores
    lcd.setCursor(10,0);
    lcd.print(highScores[realDiff(showHS % 3)]);
    lcd.setCursor(14,0);
    lcd.print(diffShort[realDiff(showHS % 3)]);
    
    currTime = millis();
    if(currTime - prevTime >= cycleTime)
    {
      lcd.setCursor(10,0);
      lcd.print("    ");
      showHS++;
      prevTime = currTime;
    }
    
    //press button to select diff
    if(buttonPress(SSW))
    {
      mode = 1;
    }
  }

  //DIFFICULTY SELECT ==================================
  if(mode == 1)
  {
    //difficulty screen
    if(modeChange(mode))
    {
      //show current difficulty
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Select Diff: ");
      lcd.setCursor(0, 1);
      lcd.print("<- ");
      lcd.print(diffName[realDiff(diffValue)]);
      lcd.setCursor(14,1);
      lcd.print("->");
    }

    //if stick is moved
    if(stickDir(mapX,mapY) != prevDir)
    {
      //moving right increments difficulty
      if(stickDir(mapX,mapY) == 0)
      {
        diffValue++;
      }
      //moving left decrements difficulty
      if(stickDir(mapX,mapY) == 4)
      {
        diffValue--;
      }
      diffSelect(diffValue); //change difficulty
    }
    prevDir = stickDir(mapX,mapY);

    //press button to start game
    if(buttonPress(SSW))
    {
      mode = 2;
    }
  }

  //IN-GAME ============================================
  if(mode == 2)
  {
    if(modeChange(mode))
    {
      //3 seconds to get ready
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("READY?");
      lcd.setCursor(0,1);
      lcd.print("3");
      delay(1000);
      lcd.setCursor(0,1);
      lcd.print("2");
      delay(1000);
      lcd.setCursor(0,1);
      lcd.print("1");
      delay(1000);

      //display highscore for current difficulty
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Hi-Score: ");
      lcd.print(highScores[realDiff(diffValue)]);
      lcd.setCursor(14,0);
      lcd.print(diffShort[realDiff(diffValue)]);
      lcd.setCursor(0, 1);
      lcd.print("Score: 0");
      
      //set lives to max and turn on indicators
      HP = 3;
      digitalWrite(HP_LED[0], HIGH);
      digitalWrite(HP_LED[1], HIGH);
      digitalWrite(HP_LED[2], HIGH);
      score = 0; //reset score
    }    
    game(); //start game
  }

  //GAME OVER ==========================================
  if(mode == 3)
  {
    if(modeChange(mode))
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("GAME OVER");
      lcd.setCursor(14,0);
      lcd.print(diffShort[realDiff(diffValue)]);

      //update highscore if new record, else show final score
      lcd.setCursor(0,1);
      if(score > highScores[realDiff(diffValue)])
      {
        highScores[realDiff(diffValue)] = score;
        lcd.print("NEW HI-SCORE:");
        lcd.print(highScores[realDiff(diffValue)]);
      }
      else
      {
        lcd.print("Score: ");
        lcd.print(score);
      }
    }
    
    if(buttonPress(SSW))
    {
      mode = 0;
    }
  }
}

//DETECTS BUTTON PRESSES ===============================
bool buttonPress(int SSW)
{
  if((SSW != prevState) && (SSW == 0))
  {
    prevState = SSW;
    return 1;
  }
  else
  {
    prevState = SSW;
    return 0;
  }
}

//DETECTS MODE CHANGES =================================
bool modeChange(int mode)
{
  if(mode != prevMode)
  {
    prevMode = mode;
    return 1;
  }
  else
  {
    prevMode = mode;
    return 0;
  }
}

//FOR CHANGING DIFFICULTY ==============================
int realDiff(int diffValue) //0 = EZ | 1 = MD | 2 = HD
{
  //get positive modulus of number of diff changes
  return (diffValue % 3 + 3) % 3;
}

void diffSelect(int diffValue)
{
  //clear previous diff upon changing
  lcd.setCursor(3, 1);
  lcd.print("          ");
  lcd.setCursor(3, 1);
  
  //show difficulty and set time accordingly
  diffTime_curr = diffTime[realDiff(diffValue)];
  lcd.print(diffName[realDiff(diffValue)]);
}

//ACTUAL GAME ==========================================
void game()
{
  currTime = millis();

  if(!activeLED) //if all LEDs are off
  {
    rNum = random(4); //random LED to turn on
    digitalWrite(ledPins[rNum], HIGH); //turn on LED
    prevTime = currTime;
    activeLED = 1;
  }

  //if time is not up and the correct directional input is made
  if((currTime - prevTime <= diffTime_curr) && (stickDir(mapX,mapY) == rNum))
  {
    score++; //add score
    lcd.setCursor(7, 1);
    lcd.print(score);
    digitalWrite(ledPins[rNum], LOW); //turn off LED
    activeLED = 0;
    delay(500); //wait for next LED to turn on
  }

  //lose a life upon running out of time
  if(currTime - prevTime >= diffTime_curr)
  {
    HP--;
    digitalWrite(HP_LED[HP], LOW);
    digitalWrite(ledPins[rNum], LOW); //turn off LED
    activeLED = 0;

    if(HP == 0) //game over upon losing all lives
    {
      mode = 3;
    }
    
    delay(500);
  }
}

//FOR CHECKING JOYSTICK INPUT DIRECTION ================
int stickDir(int x, int y)
{
  //RIGHT
  if((x > 256) && (y <= 256) && (y >= -256))
  {
    Serial.println("RIGHT");
    return 0;
  }
  
  //UP-RIGHT
  else if((x > 256) && (y > 256))
  {
    Serial.println("UP-RIGHT");
    return 1;
  }

  //UP
  else if((x >= -256) && (x <= 256) && (y > 256))
  {
    Serial.println("UP");
    return 2;
  }

  //UP-LEFT
  else if((x < -256) && (y > 256))
  {
    Serial.println("UP-LEFT");
    return 3;
  }
  
  //LEFT
  else if((x < -256) && (y <= 256) && (y >= -256))
  {
    Serial.println("LEFT");
    return 4;
  }

  //DOWN-LEFT
  else if((x < -256) && (y < -256))
  {
    Serial.println("DOWN-LEFT");
    return 5;
  }

  //DOWN
  else if((x >= -256) && (x <= 256) && (y < -256))
  {
    Serial.println("DOWN");
    return 6;
  }

  //DOWN-RIGHT
  else if((x > 256) && (y < -256))
  {
    Serial.println("DOWN-RIGHT");
    return 7;
  }
  
  //IDLE
  else
  {
    return 8;
  }
}
