#include <Arduino.h>
#include "lcd.h"

#define TFTW            80     // screen width
#define TFTH            160     // screen height
#define TFTW2           40     // half screen width
#define TFTH2           80     // half screen height
// game constant
#define SPEED            1
#define GRAVITY         9.8
#define JUMP_FORCE     2.15
#define SKIP_TICKS     20.0     // 1000 / 50fps
#define MAX_FRAMESKIP     5
// bird size
#define BIRDW             8     // bird width
#define BIRDH             8     // bird height
#define BIRDW2            4     // half width
#define BIRDH2            4     // half height
// pipe size
#define PIPEW            15     // pipe width
#define GAPHEIGHT        30     // pipe gap height
// floor size
#define FLOORH           20     // floor height (from bottom of the screen)
// grass size
#define GRASSH            4     // grass height (inside floor, starts at floor y)

// Color565
#define COLOR565(r,g,b)	( (r & 0xF8)<<8 | (g & 0xFC)<<5 | (b & 0xF8)>>4 )

#define BCKGRDCOL COLOR565(138,235,244)
#define BIRDCOL  COLOR565(255,254,174)
#define PIPECOL  COLOR565(99,255,78)
// pipe highlight
#define PIPEHIGHCOL COLOR565(250,255,250)
// pipe seam
#define PIPESEAMCOL  COLOR565(0,0,0)
// floor
#define FLOORCOL COLOR565(246,240,163)
// grass (col2 is the stripe color)
#define GRASSCOL COLOR565(141,225,87)
#define GRASSCOL2 COLOR565(156,239,88)

// bird sprite
// bird sprite colors (Cx name for values to keep the array readable)
#define C0 BCKGRDCOL
#define C1 COLOR565(195,165,75)
#define C2 BIRDCOL
#define C3 COLOR565(255,255,255)
#define C4 COLOR565(255,0,0)
#define C5 COLOR565(251,216,114)

const uint16_t birdcol[] =
{ C0, C0, C1, C1, C1, C1, C1, C0, C0, C0, C1, C1, C1, C1, C1, C0,
  C0, C1, C2, C2, C2, C1, C3, C1, C0, C1, C2, C2, C2, C1, C3, C1,
  C0, C2, C2, C2, C2, C1, C3, C1, C0, C2, C2, C2, C2, C1, C3, C1,
  C1, C1, C1, C2, C2, C3, C1, C1, C1, C1, C1, C2, C2, C3, C1, C1,
  C1, C2, C2, C2, C2, C2, C4, C4, C1, C2, C2, C2, C2, C2, C4, C4,
  C1, C2, C2, C2, C1, C5, C4, C0, C1, C2, C2, C2, C1, C5, C4, C0,
  C0, C1, C2, C1, C5, C5, C5, C0, C0, C1, C2, C1, C5, C5, C5, C0,
  C0, C0, C1, C5, C5, C5, C0, C0, C0, C0, C1, C5, C5, C5, C0, C0};


int maxScore = 0;

// bird structure
static struct BIRD {
  long x, y, old_y;
  long col;
  float vel_y;
} bird;

// pipe structure
static struct PIPES {
  long x, gap_y;
  long col;
} pipes;

// score
int score;
// temporary x and y var
static short tmpx, tmpy;

// ---------------
// draw pixel
// ---------------
// faster drawPixel method by inlining calls and using setAddrWindow and pushColor
// using macro to force inlining

ST7735  Lcd;
#define drawPixel(a, b, c) Lcd.fillRect(a, b, 1, 1, c)

void printScore(int sc)
{
	byte score[5] ={' ', ' ', ' ', ' ', 0};
	Lcd.drawString(2 ,4, (const char*)score, 0, BCKGRDCOL);
	sprintf((char *)score, "%04d", sc);
	Lcd.drawString(2 ,4, (const char*)score, WHITE, BCKGRDCOL);
}

void game_init() {
  // clear screen
  Lcd.fillScreen(BCKGRDCOL);
  // reset score
  score = 0;
  // init bird
  bird.x = 30;
  bird.y = bird.old_y = TFTH2 - BIRDH;
  bird.vel_y = -JUMP_FORCE;
  tmpx = tmpy = 0;
  // generate new random seed for the pipe gape
  //randomSeed(analogRead(0));
  // init pipe
  pipes.x = 0;
  pipes.gap_y = random(20, TFTH-60);
  printScore(score);
}

// ---------------
// game start
// ---------------
void game_start() {
  Lcd.fillScreen(BLACK);
  Lcd.fillRect(0, TFTH2 - 10, TFTW, 1, WHITE);
  Lcd.fillRect(0, TFTH2 + 15, TFTW, 1, WHITE);
  
  Lcd.drawString( TFTW2-8*3, TFTH2 - 6, "FLAPPY", WHITE, BLACK);
  Lcd.drawString( TFTW2-8*3, TFTH2 + 6, "-BIRD-", WHITE, BLACK);
  
  while (1) {
    // wait for push button
     if(digitalRead(PA8) == HIGH){
      while(digitalRead(PA8) == HIGH);
      break;
    }
  }
  // init game settings
  game_init();
}

// ---------------
// game over
// ---------------
void game_over() {
  Lcd.fillScreen(BLACK);
 
#if 0
  EEPROM_Read(&maxScore,0);
  
  if(score>maxScore)
  {
    EEPROM_Write(&score,0);
    maxScore = score;
    M5.Lcd.setTextColor(TFT_RED);
    M5.Lcd.setTextSize(1); 
    M5.Lcd.setCursor( 0, TFTH2 - 16);
    M5.Lcd.println("NEW HIGHSCORE");
  }
#endif
	Lcd.drawString( TFTW2 - 32, TFTH2 - 6,"GameOver", WHITE,BLACK);
  delay(1000);
  while(1) {
    // wait for push button
    if(digitalRead(PA8) == HIGH){
      while(digitalRead(PA8) == HIGH);
      break;
    }
  }
}


// ---------------
// game loop
// ---------------
void game_loop() {
  // ===============
  // prepare game variables
  // draw floor
  // ===============
  // instead of calculating the distance of the floor from the screen height each time store it in a variable
  unsigned char GAMEH = TFTH - FLOORH;
  // draw the floor once, we will not overwrite on this area in-game
  // black line
  Lcd.drawFastHLine(0, GAMEH, TFTW, BLACK);
  // grass and stripe
  Lcd.fillRect(0, GAMEH+1, TFTW2, GRASSH, GRASSCOL);
  Lcd.fillRect(TFTW2, GAMEH+1, TFTW2, GRASSH, GRASSCOL2);
  // black line
  Lcd.drawFastHLine(0, GAMEH+GRASSH, TFTW, BLACK);
  // mud
  Lcd.fillRect(0, GAMEH+GRASSH+1, TFTW, FLOORH-GRASSH, FLOORCOL);
  // grass x position (for stripe animation)
  long grassx = TFTW;
  // game loop time variables
  double delta, old_time, next_game_tick, current_time;
  next_game_tick = current_time = millis();
  int loops;
  // passed pipe flag to count score
  bool passed_pipe = false;
  // temp var for setAddrWindow
  unsigned char px;
  unsigned char bpx;
  unsigned char jump = 0;

  while (1) {
    loops = 0;
    delay(50);
    //while( millis() > next_game_tick && loops < MAX_FRAMESKIP) {
      if(digitalRead(PA8) == HIGH){
      	if(!jump){
          jump = 1;
          if (bird.y > BIRDH2*0.5) bird.vel_y = -JUMP_FORCE;
          // else zero velocity
          else bird.vel_y = 0;
        }
      }else{
        jump = 0;
      }
      // LED-B ON
      if(bird.vel_y < 0){
        digitalWrite(PA2,LOW);
      }else{
        digitalWrite(PA2,HIGH);
      }
      // ===============
      // update
      // ===============
      // calculate delta time
      // ---------------
      //old_time = current_time;
      //current_time = millis();
      delta = 0.02; //(current_time-old_time)/1000;

      // bird
      // ---------------
      bird.vel_y += 0.5;//GRAVITY * delta;
      bird.y += bird.vel_y;

      // pipe
      // ---------------
      
      pipes.x -= SPEED;
      // if pipe reached edge of the screen reset its position and gap
      if (pipes.x < -PIPEW) {
        pipes.x = TFTW;
        pipes.gap_y = random(10, GAMEH-(10+GAPHEIGHT));
      }

      // ---------------
      next_game_tick += SKIP_TICKS;
      loops++;
    //}

    // ===============
    // draw
    // ===============
    // pipe
    // ---------------
    // we save cycles if we avoid drawing the pipe when outside the screen

    if (pipes.x >= 0 && pipes.x < TFTW) {
      // pipe color
      Lcd.drawFastVLine(pipes.x+3, 0, pipes.gap_y, PIPECOL);
      Lcd.drawFastVLine(pipes.x+3, pipes.gap_y+GAPHEIGHT+1, GAMEH-(pipes.gap_y+GAPHEIGHT+1), PIPECOL);
      // highlight
      Lcd.drawFastVLine(pipes.x, 0, pipes.gap_y, PIPEHIGHCOL);
      Lcd.drawFastVLine(pipes.x, pipes.gap_y+GAPHEIGHT+1, GAMEH-(pipes.gap_y+GAPHEIGHT+1), PIPEHIGHCOL);
      // bottom and top border of pipe
      drawPixel(pipes.x, pipes.gap_y, PIPESEAMCOL);
      drawPixel(pipes.x, pipes.gap_y+GAPHEIGHT, PIPESEAMCOL);
      // pipe seam
      drawPixel(pipes.x, pipes.gap_y-6, PIPESEAMCOL);
      drawPixel(pipes.x, pipes.gap_y+GAPHEIGHT+6, PIPESEAMCOL);
      drawPixel(pipes.x+3, pipes.gap_y-6, PIPESEAMCOL);
      drawPixel(pipes.x+3, pipes.gap_y+GAPHEIGHT+6, PIPESEAMCOL);
    }
#if 1
    // erase behind pipe
    if (pipes.x <= TFTW)
     Lcd.drawFastVLine(pipes.x+PIPEW, 0, GAMEH, BCKGRDCOL);
    // PIPECOL
#endif
    // bird
    // ---------------
    // clear bird at previous position stored in old_y
    // we can't just erase the pixels before and after current position
    // because of the non-linear bird movement (it would leave 'dirty' pixels)
    Lcd.fillRect(bird.x, bird.old_y, BIRDW, BIRDH, BCKGRDCOL);
    Lcd.drawBitmap(bird.x, bird.old_y, BIRDW, BIRDH, (uint16_t *)birdcol);
    tmpx = BIRDW-1;
    // save position to erase bird on next draw
    bird.old_y = bird.y;

    // grass stripes
    // ---------------
    grassx -= SPEED;
    if (grassx < 0) grassx = TFTW;
    Lcd.drawFastVLine( grassx    %TFTW, GAMEH+1, GRASSH-1, GRASSCOL);
    Lcd.drawFastVLine((grassx+64)%TFTW, GAMEH+1, GRASSH-1, GRASSCOL2);

    // ===============
    // collision
    // ===============
    // if the bird hit the ground game over
    if (bird.y > GAMEH-BIRDH) break;
    // checking for bird collision with pipe
    if (bird.x+BIRDW >= pipes.x-BIRDW2 && bird.x <= pipes.x+PIPEW-BIRDW) {
      // bird entered a pipe, check for collision
      if (bird.y < pipes.gap_y || bird.y+BIRDH > pipes.gap_y+GAPHEIGHT) break;
      else passed_pipe = true;
    }
    // if bird has passed the pipe increase score
    else if (bird.x > pipes.x+PIPEW-BIRDW && passed_pipe) {
      passed_pipe = false;
      score++;
    }

    // update score
    // ---------------
    printScore(score);
  }
  
  // add a small delay to show how the player lost
  delay(1200);
}


void setup()
{
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PA1, OUTPUT);
  pinMode(PA2, OUTPUT);
  
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(PA1, HIGH);
  digitalWrite(PA2, HIGH);

  // Initialize BOOT0 Button
  pinMode(PA8, INPUT);
  
  //SPI.begin();
  Lcd.begin();
  Lcd.fillScreen(BLACK);
  
}

void loop()
{
	game_start();
	game_loop();
  game_over();
}
