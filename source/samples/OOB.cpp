/*
 * Modified version of the original OOB hex to support V2
 */

#include "MicroBit.h"
#include "Tests.h"
#include <cmath>
#include "Synthesizer.h"

#define OOB_SHAKE_OVERSAMPLING                  5
#define OOB_SHAKE_OVERSAMPLING_THRESHOLD        4
#define OOB_SHAKE_THRESHOLD                     1200

enum modes {
    WAKE = 0,
    INTRO,
    BUTTON_A,
    BUTTON_B,
    LOGO_BUTTON,
    TURN,
    DOTCHASER,
    CLAP,
    NEXT,
    SECRET
};
 
int mode;
int accelX, accelY;
int targetX, targetY;
bool flag = false;
MicroBitImage currentFrame;
bool mute = false;

int basenote = 52;
 
//shake game
    int shake_detected = 0;
    int shakeCount;
    int xMin;
    int xMax;
    int yMin;
    int yMax;
    int prevX;
    int prevY;
    
 
// Images and animations -----------------
 
const MicroBitImage dot("0,255,0,255,0\n255,255,255,255,255\n255,255,255,255,255\n0,255,255,255,0\n0,0,255,0,0\n");
 
const char *shake[] = {
    "0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n",
    "0,0,0,0,0\n0,0,0,0,0\n0,0,255,0,0\n0,0,0,0,0\n0,0,0,0,0\n",
    "0,0,0,0,0\n0,255,0,255,0\n0,0,255,0,0\n0,255,0,255,0\n0,0,0,0,0\n",
    "0,0,255,0,0\n0,255,0,255,0\n255,0,255,0,255\n0,255,0,255,0\n0,0,255,0,0\n",
    "255,0,255,0,255\n0,255,0,255,0\n255,0,255,0,255\n0,255,0,255,0\n255,0,255,0,255\n",
    "255,255,255,255,255\n255,255,255,255,255\n255,255,255,255,255\n255,255,255,255,255\n255,255,255,255,255\n"    
};
 
const char *wakeAnim[] = {
    "0,0,0,0,0\n0,0,0,0,0\n0,0,255,0,0\n0,0,0,0,0\n0,0,0,0,0\n",
    "0,0,0,0,0\n0,255,255,255,0\n0,255,255,255,0\n0,255,255,255,0\n0,0,0,0,0\n",
    "255,255,255,255,255\n255,255,255,255,255\n255,255,255,255,255\n255,255,255,255,255\n255,255,255,255,255\n"
};
 
const char *explosionTime[] = {
    "255,255,255,255,255\n255,0,0,0,255\n255,0,0,0,255\n255,0,0,0,255\n255,255,255,255,255\n",
    "255,255,255,255,255\n255,255,255,255,255\n255,255,0,255,255\n255,255,255,255,255\n255,255,255,255,255\n",
    "255,255,255,255,255\n255,255,255,255,255\n255,255,255,255,255\n255,255,255,255,255\n255,255,255,255,255\n",
    "0,0,0,0,0\n0,255,255,255,0\n0,255,255,255,0\n0,255,255,255,0\n0,0,0,0,0\n",
    "0,0,0,0,0\n0,0,0,0,0\n0,0,255,0,0\n0,0,0,0,0\n0,0,0,0,0\n",
    "0,0,0,0,0\n0,255,0,255,0\n0,0,0,0,0\n0,255,0,255,0\n0,0,0,0,0\n",
    "255,0,0,0,255\n0,0,255,0,0\n0,255,255,255,0\n0,0,255,0,0\n255,0,0,0,255\n",
    "0,0,255,0,0\n0,255,0,255,0\n255,0,0,0,255\n0,255,0,255,0\n0,0,255,0,0\n",
    "255,0,0,0,255\n0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n255,0,0,0,255\n",
    "0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n"    
};
 
const char *twistyTime[] = {
    "0,0,0,0,0\n0,0,0,0,0\n255,0,0,0,255\n0,0,0,0,0\n0,0,0,0,0\n",
    "0,0,0,0,0\n255,0,0,0,255\n255,255,0,255,255\n255,0,0,0,255\n0,0,0,0,0\n",
    "255,0,0,0,255\n255,255,0,255,255\n255,255,255,255,255\n255,255,0,255,255\n255,0,0,0,255\n",
    "0,0,0,255,255\n255,0,0,255,255\n255,255,255,255,255\n255,255,0,0,255\n255,255,0,0,0\n",
    "0,255,255,255,255\n0,0,255,255,255\n255,0,255,0,255\n255,255,255,0,0\n255,255,255,255,0\n",
    "255,255,255,255,255\n0,0,255,255,255\n0,0,255,0,0\n255,255,255,0,0\n255,255,255,255,255\n",
    "255,255,255,255,255\n0,255,255,255,0\n0,0,255,0,0\n0,255,255,255,0\n255,255,255,255,255\n",
    "0,255,255,255,0\n0,0,255,0,0\n0,0,0,0,0\n0,0,255,0,0\n0,255,255,255,0\n",
    "0,0,255,0,0\n0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n0,0,255,0,0\n",
    "0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n"
};
 
const char *heart[] = { 
     "0,255,0,255,0\n255,255,255,255,255\n255,255,255,255,255\n0,255,255,255,0\n0,0,255,0,0\n"
};
 
// Arrow images and animations.
const MicroBitImage arrowUpTime("0,0,255,0,0\n0,255,255,255,0\n255,0,255,0,255\n0,0,255,0,0\n0,0,255,0,0\n");
 
const char *arrowDisintegrationTime[] = {
    "0,0,0,0,0\n0,0,255,0,0\n0,0,255,0,0\n255,0,255,0,255\n0,255,255,255,0\n",
    "0,0,0,0,0\n0,0,0,0,0\n0,0,255,0,0\n0,0,255,0,0\n255,0,255,0,255\n",
    "0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n0,0,255,0,0\n0,0,255,0,0\n",
    "0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n0,0,255,0,0\n",
    "0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n0,0,0,0,0\n"
};
 
// Bottom arrow from left to right
const char *bottomArrow[] = {
    "0,0,255,0,0\n0,255,0,0,0\n255,255,255,255,255\n0,255,0,0,0\n0,0,255,0,0\n",
    "0,0,0,0,255\n255,0,0,255,0\n255,0,255,0,0\n255,255,0,0,0\n255,255,255,255,0\n",
    "0,0,255,0,0\n0,0,255,0,0\n255,0,255,0,255\n0,255,255,255,0\n0,0,255,0,0\n",
    "255,0,0,0,0\n0,255,0,0,255\n0,0,255,0,255\n0,0,0,255,255\n0,255,255,255,255\n",
    "0,0,255,0,0\n0,0,0,255,0\n255,255,255,255,255\n0,0,0,255,0\n0,0,255,0,0\n"
};
 
const char *topArrow[] = {
    "0,0,255,0,0\n0,255,0,0,0\n255,255,255,255,255\n0,255,0,0,0\n0,0,255,0,0\n",
    "255,255,255,255,0\n255,255,0,0,0\n255,0,255,0,0\n255,0,0,255,0\n0,0,0,0,255\n",
    "0,0,255,0,0\n0,255,255,255,0\n255,0,255,0,255\n0,0,255,0,0\n0,0,255,0,0\n",
    "0,255,255,255,255\n0,0,0,255,255\n0,0,255,0,255\n0,255,0,0,255\n255,0,0,0,0\n",
    "0,0,255,0,0\n0,0,0,255,0\n255,255,255,255,255\n0,0,0,255,0\n0,0,255,0,0\n"
};

int target_freq;
int current_freq;
int playback_sleep = 5;
int slide = 1;
int chatter = false;
int chatter_toggle = false;

// ---------------------------
void playfreq(int freq)
{
  if (mute || freq == 0) { 
    uBit.io.speaker.setAnalogValue(0);
    uBit.io.P0.setAnalogValue(0);
    return;
  }
  uBit.io.speaker.setHighDrive(true);
  uBit.io.speaker.setAnalogValue(512);
  uBit.io.P0.setHighDrive(true);
  uBit.io.P0.setAnalogValue(512);
  int period = 1000000.0/(float)freq;

  uBit.io.speaker.setAnalogPeriodUs(period);
  uBit.io.P0.setAnalogPeriodUs(period);
  return;
}

void play_note(uint8_t note) {
    if(note == 0) {
        target_freq = 0;
        return;
    }
            
    // A 440hz
    int f = 440.0 * pow(2, ((float)(note - 58)/12.0));
    target_freq = f;

    uBit.serial.printf("%d \r\n", note);
}
 
void playback_ticker() {
    // Thread forever
    while(1) {

        if(target_freq == 0) {
            current_freq = 0;
            playfreq(0);
        } else {
            current_freq += (target_freq - current_freq) / slide;
            playfreq(current_freq);
        }
        fiber_sleep(playback_sleep);
    }
}
 

// Wake up the device
void wake()
{
    uBit.display.setBrightness(0);
    // Turn on all pixels.
    for(int y=0; y<5; y++) {
        for(int x=0; x<5; x++) {
            uBit.display.image.setPixelValue(x, y, 255);
        }
    }
    
    // Fade in all LEDs.
    for(int i=0; i<255; i++) {
        uBit.display.setBrightness(i);
        uBit.sleep(10);
	    target_freq = i;
    }

    // Fade out all LEDs.
    for(int i=255; i>0; i--) {
        uBit.display.setBrightness(i);
        uBit.sleep(10);
	    target_freq = i;
    }
	play_note(0);
    
    // Set brightness back to full and clear screen.
    uBit.display.image.clear();
    uBit.display.setBrightness(255);
    
    slide = 1;
    // Pulsing animation.
    int animDelay = 100;
    for(int j=0; j<20; j++) {
        int k = 0;
        for(int i=0; i<3; i++) {
            currentFrame = MicroBitImage(wakeAnim[i]);
	        play_note((3*j) + k*5);
            k = k + 2;
            uBit.display.print(currentFrame,0,0,0,animDelay);
        }
        for(int i=2; i>-1; i--) {
            currentFrame = MicroBitImage(wakeAnim[i]);
	        play_note((3 * j) + k*5);
            k++;
            uBit.display.print(currentFrame,0,0,0,animDelay);
        }
        animDelay -= 5;
    }
        
	play_note(basenote);
    uBit.sleep(300);
    
    // Fade out last dot.
    //freq = 512;
    // Our frequency goes negative in this loop but it sounds cool!
    for(int i=255; i>=0; i--) {
        uBit.display.setBrightness(i);
        uBit.sleep(1);
	//freq = freq-10;
	//playfreq(freq);
    }
    
    play_note(0);
    // Clear display and set brightnes back to full.
    uBit.display.image.clear();
    uBit.display.setBrightness(255);
    
    uBit.sleep(500);
    
    // Proceed to the next mode.
    mode++;
}
 
void intro()
{
    // Introduce the micro:bit.
    uBit.display.image.clear();
    chatter = true;
    uBit.display.scroll("HELLO", 150);
    chatter = false;

    slide = 5;
    MicroBitImage smiley("0,0,0,0, 0\n0,255,0,255,0\n0,0,0,0,0\n255,0,0,0,255\n0,255,255,255,0\n");
    uBit.display.print(smiley);
    uBit.display.setBrightness(0);
    for(int b = 0; b < 255; b++) {
        uBit.sleep(2000 / 255);
        uBit.display.setBrightness(b);
    }

    play_note(basenote + 12);
    uBit.sleep(100);
    play_note(0);
    uBit.sleep(20);
    play_note(basenote + 5);
    uBit.sleep(300);
    play_note(0);
    uBit.sleep(1000);

    // Proceed to the next mode.
    mode++;
}

int button_a_pressed = false; 
void OOB_onButtonA(MicroBitEvent)
{
    button_a_pressed = true;
}

void pressButtonA() 
{
    // Give instruction
   // uBit.display.print("A");
   uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, OOB_onButtonA);
    
    // Wait for a button press.
    while(!button_a_pressed) {
        uBit.display.print("A");
        uBit.sleep(500);
        if(button_a_pressed) break;
    
        currentFrame = MicroBitImage(topArrow[0]);
        uBit.display.print(currentFrame,0,0,0,100);
       uBit.sleep(100);
        if(button_a_pressed) break;
    
       currentFrame = MicroBitImage(topArrow[0]);
        uBit.display.print(currentFrame,0,0,0,100);
       uBit.sleep(100);
        if(button_a_pressed) break;
    }
    
    // SADHBH'S animation goes here.
    for(int i=0; i<10; i++) {
        currentFrame = MicroBitImage(explosionTime[i]);
        uBit.display.print(currentFrame,0,0,0,100);
        play_note(basenote + (i * 5)) ;
    }
    play_note(0);
    
    uBit.display.stopAnimation();
    uBit.sleep(1000);
    
    // Proceed to the next mode.
    mode++;
}
 
int button_b_pressed = false; 
void OOB_onButtonB(MicroBitEvent)
{
    button_b_pressed = true;
}

void pressButtonB()
{
    // Give instruction
   
   uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, OOB_onButtonB);
    
    // Wait for a button press.
    while(!button_b_pressed) {
        uBit.display.print("B");
        uBit.sleep(500);
        if(button_b_pressed) break;
    
         currentFrame = MicroBitImage(topArrow[4]);
        uBit.display.print(currentFrame,0,0,0,100);
       uBit.sleep(100);
       if(button_b_pressed)break;
    
       currentFrame = MicroBitImage(topArrow[4]);
        uBit.display.print(currentFrame,0,0,0,100);
       uBit.sleep(100);
        if(button_b_pressed)break;
    }
    
    // SADHBH'S animation goes here.
    for(int i=0; i<10; i++) {
        currentFrame = MicroBitImage(twistyTime[i]);
        uBit.display.print(currentFrame,0,0,0,100);
        play_note(basenote + (9*5) - (i * 5)) ;
    }
    play_note(0);
    
    uBit.sleep(2000);
    
    // Proceed to the next mode.
    mode++;
}

int button_logo_pressed = false; 
void OOB_onButtonLogo(MicroBitEvent)
{
    button_logo_pressed = true;
}

void pressLogoButton()
{
    // Give instruction
   
   uBit.messageBus.listen(MICROBIT_ID_LOGO, MICROBIT_BUTTON_EVT_CLICK, OOB_onButtonLogo);
    
    // Wait for a button press.
    while(!button_b_pressed) {
        uBit.display.print("Logo");
        uBit.sleep(500);
        if(button_logo_pressed) break;
    
        currentFrame = MicroBitImage(topArrow[2]);
        uBit.display.print(currentFrame,0,0,0,100);
        uBit.sleep(100);
        if(button_logo_pressed)break;
    
        currentFrame = MicroBitImage(topArrow[2]);
        uBit.display.print(currentFrame,0,0,0,100);
        uBit.sleep(100);
        if(button_logo_pressed)break;
    }
    
    // SADHBH'S animation goes here.
    for(int i=0; i<10; i++) {
        currentFrame = MicroBitImage(twistyTime[i]);
        uBit.display.print(currentFrame,0,0,0,100);
        play_note(basenote + (9*5) - (i * 5)) ;
    }
    play_note(0);
    
    uBit.sleep(2000);
    
    // Proceed to the next mode.
    mode++;
}
 
void updateAccelPosition()
{
    accelX = 0;
    accelY = 0;
 
    if (uBit.accelerometer.getX() > 600)
        accelX++;
    if (uBit.accelerometer.getX() > 250)
        accelX++;
    if (uBit.accelerometer.getX() > -250)
        accelX++;
    if (uBit.accelerometer.getX() > -600)
        accelX++;
 
    if (uBit.accelerometer.getY() > 600)
        accelY++;
    if (uBit.accelerometer.getY() > 250)
        accelY++;
    if (uBit.accelerometer.getY() > -250)
        accelY++;
    if (uBit.accelerometer.getY() > -600)
        accelY++;
}

void turn()
{
    int timeout = 0;
    int samples_high;
    int x, y, z, magnitude;
    
    playback_sleep = 100;

    uBit.display.scroll("SHAKE!", 200);

    uBit.accelerometer.setRange(8);

    xMax = uBit.accelerometer.getX();
    yMax = uBit.accelerometer.getY();

    shake_detected = 0;
    slide = 1;

    while(timeout < 20000 || shake_detected == 0) {

        samples_high = 0;
        for (int samples = 0; samples < OOB_SHAKE_OVERSAMPLING; samples++)
        {
            x = uBit.accelerometer.getX();
            y = uBit.accelerometer.getY();
            z = uBit.accelerometer.getZ();
            magnitude = sqrt((x*x)+(y*y)+(z*z));

            if (magnitude > OOB_SHAKE_THRESHOLD)
                samples_high++;
        }

        if (samples_high >= OOB_SHAKE_OVERSAMPLING_THRESHOLD)
            shakeCount++;
        else
            shakeCount--;

        // Clamp shakeCount within range 0..4 
        shakeCount = min(shakeCount, 4);
        shakeCount = max(shakeCount, 0);
        
        // Display an image matching the shake intensity measured
        currentFrame = MicroBitImage(shake[shakeCount]);
        uBit.display.print(currentFrame);
        if(shakeCount > 0) {
            play_note(basenote + 7*shakeCount);
        } else {
            play_note(0);
        }

        if(shakeCount == 4) shake_detected = 1; // Exit on large shake

         // Wait a while.
         uBit.sleep(150);
         timeout += 150;

         if(((timeout % 3000) == 0) && !shake_detected) {
            uBit.display.scroll("SHAKE!", 200);
         }

    }
    playback_sleep = 5;
    play_note(0);
    
    uBit.accelerometer.setRange(2);
    uBit.display.image.clear();
    uBit.sleep(1000);

    mode++;
}

void insertNewTarget()
{
    targetX = uBit.random(4);
    targetY = uBit.random(4);
    if(targetX == 0 && targetY == 0) targetY++;
    else if(targetX == 0 && targetY == 4) targetY--;
    else if(targetX == 4 && targetY == 0) targetY++;
    else if(targetX == 4 && targetY == 4) targetY--;
}
 
void dotChaser()
{
    uBit.display.scroll("TILT", 200);
    
    slide = 1;
    int score = 0;
    int toggle = 0;
    int toggleCount = 0;

    // Get initial positions
    updateAccelPosition();
    int last_x = accelX;
    int last_y = accelY; 

    // Timeout
    int timeout = 0;
   
    slide = 3; 
    while(score < 3) {
        if(toggleCount % 5 == 0) toggle = 255-toggle;
        
        updateAccelPosition();
        
        uBit.display.image.clear();
        uBit.display.image.setPixelValue(accelX, accelY, 255);
        uBit.display.image.setPixelValue(targetX, targetY, toggle);
        
        if(targetX == accelX && targetY == accelY) {
            play_note(0);
            uBit.sleep(100);
            for(int z = 0; z < 4; z++) {
                current_freq = 2000;
                play_note(basenote + 12*z);
                uBit.sleep(100);
            }
            insertNewTarget();
            score++;
            play_note(0);
            uBit.sleep(300);
        }
        
        if(last_x != accelX || last_y != accelY) {
            play_note(basenote - 12 + (12 * accelX) + (3 * accelY));
            last_x = accelX;
            last_y = accelY;
            timeout = 0;
        } else {
            play_note(0);
        }

        if(timeout > 5000) {
            uBit.display.scroll("TILT", 200);
            timeout = 0;
        }
        
        uBit.sleep(100);
        timeout += 100;
        
        toggleCount++;
    }
   
    play_note(0); 
    // Fade out last dot.
    for(int z = 0; z < 10; z++) {
           current_freq = 2000;
           play_note(basenote + 12*(z%5));
           uBit.sleep(100);
    }
    play_note(0);
    for(int i=255; i>=0; i--) {
        uBit.display.setBrightness(i);
        uBit.sleep(1);
    }
    
    // Clear display and set brightnes back to full.
    uBit.display.image.clear();
    uBit.display.setBrightness(255);
    
    uBit.sleep(2000);
    
    mode++;
}
 
#define SNAKE_EMPTY 0
#define SNAKE_UP    1
#define SNAKE_LEFT  2
#define SNAKE_RIGHT 3
#define SNAKE_DOWN  4
 
 
#define SNAKE_FRAME_DELAY 350
 
struct Point
{
    int     x;
    int     y;
};
 
Point           head;                 // Location of the head of our snake.
Point           tail;                 // Location of the tail of our snake.
Point           food;                 // Location of food.
MicroBitImage   map(5,5);  
 
 
void place_food()
{
    int r = uBit.random(24);
    int x = 0; int y = 0;
    
    while (r > 0)
    {
        x = (x+1) % 5;
        if (x == 0)
            y = (y+1) % 5;
            
        if(map.getPixelValue(x,y) == SNAKE_EMPTY)
            r--;
    }
    
    food.x = x;
    food.y = y;
}
 
void snake()
{   
    Point newHead;              // Calculated placement of new head position based on user input.    
    int hdirection;             // Head's direction of travel
    int tdirection;             // Tail's direction of travel
    int snakeLength;            // number of segments in the snake.
    int growing;                // boolean state indicating if we've just eaten some food.
    
    // Start in the middle of the screen.
    tail.x = tail.y = 2;    
    head.x = head.y = 2;
    snakeLength = 1;
    growing = 0;
    map.clear();
        
    uBit.display.image.setPixelValue(head.x, head.y, 255);
        
    // Add some random food.    
    place_food();
        
    while (1)
    {    
        // Flash the food is necessary;       
        uBit.display.image.setPixelValue(food.x, food.y, uBit.systemTime() % 1000 < 500 ? 0 : 255);
          
        int dx = uBit.accelerometer.getX();
        int dy = uBit.accelerometer.getY();
        
        newHead.x = head.x;
        newHead.y = head.y;
        
        if (abs(dx) > abs(dy))
        {
            if(dx < 0)
            {
                hdirection = SNAKE_LEFT;
                newHead.x = newHead.x == 0 ? 4 : newHead.x-1;
            }
            else
            {
                hdirection = SNAKE_RIGHT;
                newHead.x = newHead.x == 4 ? 0 : newHead.x+1;
            }            
        }
        else    
        {
            if(dy < 0)
            {
                hdirection = SNAKE_UP;
                newHead.y = newHead.y == 0 ? 4 : newHead.y-1;
            }
            else
            {
                hdirection = SNAKE_DOWN;
                newHead.y = newHead.y == 4 ? 0 : newHead.y+1;
            }
        }           
        
        int status = map.getPixelValue(newHead.x, newHead.y);
        if (status == SNAKE_UP || status == SNAKE_DOWN || status == SNAKE_LEFT || status == SNAKE_RIGHT)
        {
            ManagedString s("GAME OVER! SCORE: ");
            ManagedString s2(snakeLength-1);
            
            uBit.display.scroll(s);
            uBit.display.scroll(s2);
            
            return;            
        }
                                          
        // move the head.       
        map.setPixelValue(head.x, head.y, hdirection);
        uBit.display.image.setPixelValue(newHead.x, newHead.y, 255);
 
        if (growing)
        {
            growing = 0;
            snakeLength++;
        }
        else
        {        
            // move the tail.
            tdirection = map.getPixelValue(tail.x,tail.y);     
            map.setPixelValue(tail.x, tail.y, SNAKE_EMPTY);         
            uBit.display.image.setPixelValue(tail.x, tail.y, 0);
    
            // Move our record of the tail's location.        
            if (snakeLength == 1)
            {
                tail.x = newHead.x;
                tail.y = newHead.y;
            }
            else
            {
                if (tdirection == SNAKE_UP)
                    tail.y = tail.y == 0 ? 4 : tail.y-1;
                
                if (tdirection == SNAKE_DOWN)
                    tail.y = tail.y == 4 ? 0 : tail.y+1;
            
                if (tdirection == SNAKE_LEFT)
                    tail.x = tail.x == 0 ? 4 : tail.x-1;
                
                if (tdirection == SNAKE_RIGHT)
                    tail.x = tail.x == 4 ? 0 : tail.x+1;
            }
        }
 
        // Update our record of the head location and away we go!
        head.x = newHead.x;
        head.y = newHead.y;
      
        // if we've eaten some food, replace the food and grow ourselves!
        if (head.x == food.x && head.y == food.y)
        {
            growing = 1;
            place_food();
        }
      
        uBit.sleep(SNAKE_FRAME_DELAY);   
    }   
}

void OOB_onButtonAExtra() {
    uBit.display.stopAnimation();
    for(int i=0; i<10; i++) {
        currentFrame = MicroBitImage(explosionTime[i]);
        uBit.display.print(currentFrame,0,0,0,100);
        play_note(basenote + (i * 5)) ;
    }
    currentFrame = MicroBitImage(heart[0]);     
    play_note(0);
    uBit.display.image.clear();
    uBit.display.print(currentFrame,0,0,0,400); 
    mode++;
}

void OOB_onButtonBExtra() {
    uBit.display.stopAnimation();
    for(int i=0; i<10; i++) {
        currentFrame = MicroBitImage(twistyTime[i]);
        uBit.display.print(currentFrame,0,0,0,100);
        play_note(basenote + (9*5) - (i * 5)) ;
    }
    play_note(0);
    currentFrame = MicroBitImage(heart[0]);     
    uBit.display.image.clear();
    uBit.display.print(currentFrame,0,0,0,400); 
    mode++;
}
 
void next()
{
     if(flag == false){
        flag = true;
        for(int i = 0; i < 5; i++) {
            play_note(basenote + i * 5);
            uBit.sleep(100);
        }
        play_note(0);
        uBit.display.scroll("WOW!", 200);
     }

    int nRuns = 0;
    while(!uBit.buttonA.isPressed() && !uBit.buttonB.isPressed() && mode == NEXT){
        for(int i=0; i<10; i++) {
            if(nRuns<3) play_note(basenote + (3 * (i % 4))); 
            currentFrame = MicroBitImage(twistyTime[i]);
            uBit.display.print(currentFrame,0,0,0,100);
             if(uBit.buttonA.isPressed() && uBit.buttonB.isPressed()){
                uBit.display.stopAnimation();
                 break;
                 }
            if(uBit.buttonA.isPressed()) OOB_onButtonAExtra();
            if(uBit.buttonB.isPressed()) OOB_onButtonBExtra();
        }
         for(int i=0; i<10; i++) {
            if(nRuns<3) play_note(basenote + (5 * (i % 4))); 
            currentFrame = MicroBitImage(explosionTime[i]);
            uBit.display.print(currentFrame,0,0,0,100);
           if(uBit.buttonA.isPressed() && uBit.buttonB.isPressed()){
                uBit.display.stopAnimation();
                 break;
                 }
            if(uBit.buttonA.isPressed()) OOB_onButtonAExtra();
            if(uBit.buttonB.isPressed()) OOB_onButtonBExtra();
        }
        play_note(0); 
        currentFrame = MicroBitImage(heart[0]);     
        uBit.display.print(currentFrame,0,0,0,400); 
        uBit.sleep(100);
        if(uBit.buttonA.isPressed() && uBit.buttonB.isPressed()){
            uBit.display.stopAnimation();
             break;
        }
            if(uBit.buttonA.isPressed()) OOB_onButtonAExtra();
            if(uBit.buttonB.isPressed()) OOB_onButtonBExtra();
        nRuns++;
    }
    mode++;
}

void make_noise() {
    uBit.display.scroll("MAKE NOISE!", 200);
    level_meter();
    mode++;
}

void clap() {
    uBit.display.scroll("CLAP!", 200);
    mems_clap_test(1);
    mode++;
}

void onLogoTouch(MicroBitEvent e) {
    uBit.serial.printf("logo touch \r\n");
    mute = !mute;
}

void
out_of_box_experience_v2()
{   
    target_freq = 0;
    current_freq = 0;

    create_fiber(playback_ticker);
   
    /* Disable logo touch to mute
    uBit.io.logo.isTouched();
    uBit.messageBus.listen(uBit.io.logo.id, MICROBIT_BUTTON_EVT_CLICK, onLogoTouch);
    */

   mode = WAKE;

    if(uBit.buttonA.isPressed()) mode = SECRET;
    while(1)
    {   
        switch(mode) {
            
            case WAKE:
                wake();
                break;
                
            case INTRO:
                intro();
                break;
                
            case BUTTON_A:
                pressButtonA();
                break;
                
            case BUTTON_B:
                pressButtonB();
                break;
            
            case LOGO_BUTTON:
                pressLogoButton();
                break;
            
            case TURN:
                turn();
                break;
                
            case DOTCHASER:
                dotChaser();
                break;

            case CLAP:
                clap();
                break;

            case NEXT:
                next();
                break;  
            
            case SECRET:
            default:
                snake();
                break;
        }
    }      
}


