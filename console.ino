
// The liquid crystal lib. Allows me to use the screen easily.
#include <LiquidCrystal.h>
//The index of the custom char for each object
#define PADDLE_LEFT 3
#define PADDLE_RIGHT 4
//utility macro for array length
#define length(arr) (sizeof(arr) / sizeof(arr[0]))


/*

  The circuit:
  Display:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
  Joystick:
 *VRx to A1
 *VRy to A0 
*/

/* byte is an unsigned char */

/* Sprites (ends on line 27) */
const byte paddle_left_high[] = 
{
  0b00011,
  0b00011,
  0b00011,
  0b00011,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};
const byte paddle_left_low[] = 
{
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00011,
  0b00011,
  0b00011,
  0b00011
};

const byte paddle_left_mid[] =
{
  0b00000,
  0b00000,
  0b00011,
  0b00011,
  0b00011,
  0b00011,
  0b00000,
  0b00000
};



const byte paddle_right_high[] = 
{
  0b11000,
  0b11000,
  0b11000,
  0b11000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

const byte paddle_right_mid[] = 
{
  0b00000,
  0b00000,
  0b11000,
  0b11000,
  0b11000,
  0b11000,
  0b00000,
  0b00000
};

const byte paddle_right_low[] = 
{
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11000,
  0b11000,
  0b11000,
  0b11000
};

const byte ball_high[] =
{
  0b11100,
  0b11100,
  0b11100,
  0b00000,
  0b00000,
  0b00000,
  0000000,
  0b00000
};

const byte ball_mid[] = 
{
  0b00000,
  0b00000,
  0b11100,
  0b11100,
  0b11100,
  0b00000,
  0b00000,
  0b00000
};

const byte ball_low[] =
{
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11100,
  0b11100,
  0b11100
};
/************************/


/* Struct section */
//Simple struct with x and y coords for an object.
struct object
{
  byte x;
  byte y;
};

struct score 
{
  byte player;
  byte bot;
};

struct Inputs 
{
  byte x;
  byte y;
  byte button;
};
/****************/

/* Globals section */
//These hold the sprites for the left and right paddles respectively
const byte* paddle_left_sprites[3];
const byte* paddle_right_sprites[3];

int physicsInterval = 500;

//declare my objects
struct object left_paddle = {0,0};
struct object right_paddle = {15,0};

byte ball_direction_x = 1;
byte ball_direction_y = 1;
struct object ball = {1,0};

struct score scores = {0, 0};

//This just holds what I get from the stick.
struct Inputs inputs;

bool gameOver = false;


/*********************/



 /* stuff to do with timers (ends on line 231) */
struct timer
{
  int interval;
  int new_t;
  int old_t;
  void (*callback)(void);
  char times_left;
};

struct timer timers[9]; // I probably won't ever need more;

void setTimeOut(int interval, void (*callback)(void), char constant)
{ 
  if(!constant){
    for(int i = 1; i < length(timers); i++) {
      if(timers[i].times_left == 0) {
          timers[i].times_left = 1;  //if the timer is free than set the amount of times it runs at to one;
          timers[i].interval = interval;
          timers[i].callback = callback;
          break;
       }
   
    }
  } else {
      if(timers[0].times_left == 0) {
        timers[0].times_left = 1;  //if the timer is free than set the amount of times it runs at to one;
        timers[0].interval = interval;
        timers[0].callback = callback;
      }    

  }

}

void repeatTime(int interval, void (*callback)(void))
{
    for(int i = 1; i < length(timers); i++){
    if(timers[i].times_left == 0) 
      {
        timers[i].times_left = -1;  //if the timer is free than set the amount of times it runs at to one;
        timers[i].interval = interval;
        timers[i].callback = callback;
        break;
      }
    }
   
}

inline void timerLoop() {
     for(int i = 0; i < length(timers); i++) {
       if(timers[i].times_left != 0) {
         if(timers[i].new_t - timers[i].old_t > timers[i].interval) {
          timers[i].callback(); 
          timers[i].old_t = timers[i].new_t;
          if(timers[i].times_left > 0) timers[i].times_left--;
        }
         
         timers[i].new_t = millis();
         
     } else {
        timers[i].times_left = 0;
        timers[i].callback == NULL;
        timers[i].interval = 0;
        timers[i].old_t = 0;
        timers[i].new_t = 0;
      }
       

    }
}
/* End timers */



// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


void get_inputs(struct Inputs* inputs)
{    
      int xPosition = analogRead(A1); // the x Value of the joystick is grabbed from analog input 1
      int yPosition = analogRead(A0);
      int mapX = map(xPosition, 0, 1023, 16, 0);
      int mapY = map(yPosition, 0, 1023, 16, 0);
      inputs->x = mapX;
      inputs->y = mapY; 
}

inline void setGameOver()
{
  gameOver = true;
}

inline void onGameOver()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Player score: ");
  lcd.print(scores.player);
  lcd.setCursor(0,1);
  lcd.print("Enemy score: ");
  lcd.print(scores.bot);
  if(scores.bot > 9){
    delay(1000);
    lcd.setCursor(1,0);
    lcd.print("The enemy wins!");
    scores.bot = 0;
    scores.player = 0;
  }
  else if(scores.player > 9){
    delay(1000);
    lcd.setCursor(1,0);
    lcd.print("You win!");
    scores.bot = 0;
    scores.player = 0;
    
  }
  delay(2000);
  reset();
}



inline void physicsLoop()
{   
  
    ball.x += ball_direction_x; // Don't question the logic. It works.
    ball.y += ball_direction_y;

    if(ball.x >= 14) {
      //makes the game gradually harder
      physicsInterval-= 10;
      if(ball.y >= right_paddle.y -1 && ball.y <= right_paddle.y + 1) { 
        ball_direction_x = -1;
        ball_direction_y = random(-1,1);  
        
      } else {
        scores.player++;
        setTimeOut(200, setGameOver, 0);
      }
    }

    else if(ball.x <= 1) {

      if(ball.y >= left_paddle.y -1 && ball.y <= left_paddle.y + 1) { 
        ball_direction_x = 1;
        if(ball.y > left_paddle.y) ball_direction_y = 1;
        else if(ball.y < left_paddle.y) ball_direction_y = -1;
        else ball_direction_y = 0;  
      } else {
        scores.bot++;
        setTimeOut(200, setGameOver, 0);
      }
    }

    //If it hits the ceiling it goes down. If it hits the floor it goes up.
    if(ball.y == 0) ball_direction_y = 1;
    else if(ball.y == 5) ball_direction_y = -1; 
    
}

inline void render()
{
      //left paddle  
      lcd.setCursor(0, (left_paddle.y < 3 ? 0 : 1));
      lcd.write(byte(PADDLE_LEFT)); 
      
      //ball
      lcd.setCursor(ball.x, (ball.y < 3 ? 0 : 1));
      lcd.write(byte(ball.y < 3 ? ball.y : ball.y - 3));

      //right paddle
      lcd.setCursor(15, (right_paddle.y < 3 ? 0 : 1));
      lcd.write(byte(PADDLE_RIGHT));
   
} 
inline void inputLoop() {
      get_inputs(&inputs);

      if(inputs.y <= 6 && left_paddle.y < 5) {
        left_paddle.y++;
        uint8_t* sprite = (uint8_t*) paddle_left_sprites[left_paddle.y < 3 ? left_paddle.y : left_paddle.y - 3];
        lcd.createChar(byte(PADDLE_LEFT), sprite);
      }

      else if(inputs.y >= 12 && left_paddle.y > 0) { 
        left_paddle.y--;
        uint8_t* sprite = (uint8_t*) paddle_left_sprites[left_paddle.y < 3 ? left_paddle.y : left_paddle.y - 3];
        lcd.createChar(byte(PADDLE_LEFT), sprite);
      }
}

inline void reset() {   
  right_paddle.y = 0;
    left_paddle.y = 0;
    ball.x = 1;
    ball.y = 0;
    ball_direction_x = 1;
    ball_direction_y = 1;
    physicsInterval  = 500;
    gameOver = false;
}
inline void botLoop() { //A simple bot. Ball go up, it go up. Ball go down, it go down.
  if(ball.x + 1 > 5) {
    
    if(ball.y > right_paddle.y && right_paddle.y < 5) right_paddle.y++;
    else if(ball.y < right_paddle.y && right_paddle.y > 1) right_paddle.y--;  
    
    /* Very important. Updates the sprite for the right paddle*/
    const byte* sprite = paddle_right_sprites[(right_paddle.y < 3) ? right_paddle.y : (right_paddle.y - 3)]; 
    lcd.createChar(byte(PADDLE_RIGHT), sprite);

  }
}


void setup() {

  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  //set up pins for the joystick
  pinMode(A1, INPUT);
  pinMode(A0, INPUT);

  //set up sprites for each object
  paddle_left_sprites[0] = paddle_left_high;
  paddle_left_sprites[1] = paddle_left_mid;
  paddle_left_sprites[2] = paddle_left_low;

  paddle_right_sprites[0] = paddle_right_high;
  paddle_right_sprites[1] = paddle_right_mid;
  paddle_right_sprites[2] = paddle_right_low;
  
  //set starting sprites.
  lcd.createChar(byte(0), (uint8_t*) ball_high);  
  lcd.createChar(byte(1), (uint8_t*) ball_mid);
  lcd.createChar(byte(2), (uint8_t*) ball_low);
  lcd.createChar(byte(PADDLE_LEFT), (uint8_t*) paddle_left_sprites[0]);
  lcd.createChar(byte(PADDLE_RIGHT), (uint8_t*) paddle_right_sprites[0]);
  setTimeOut(physicsInterval, physicsLoop, 1);
  repeatTime(400, botLoop);
  randomSeed(analogRead(0));
  reset();
 
}



void loop() {
  
 lcd.clear();
 if(!gameOver)
 { 
     inputLoop();
     render();
     timerLoop();  
    if(timers[0].times_left == 0) {
      setTimeOut(physicsInterval, physicsLoop, 1); // and I'll do it again!
    }  
 }
 else
 {
    onGameOver();
 }
 delay(80); //there had to be some amount of time between the iterations of the game loop, and it might as well be 80ms.
}
