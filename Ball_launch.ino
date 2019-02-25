#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.
#include <mpu9255_esp32.h>
// #include <math.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
 
#define BACKGROUND TFT_GREEN
#define BALL_COLOR TFT_BLUE
 
const int DT = 40; //milliseconds
const int EXCITEMENT = 2000; //how much force to apply to ball
const uint8_t input_pin1 = 16; // apply force
const uint8_t input_pin2 = 5; // reset screen

bool pushed_last_time; //for finding change of button (using bool type...same as uint8_t)
uint8_t push_count;

uint32_t primary_timer; //main loop timer
 
//physics constants:
const float MASS = 1; //for starters
const int RADIUS = 5; //radius of ball
const float K_FRICTION = 0.15;  //friction coefficient
const float K_SPRING = 0.9;  //spring coefficient
 
//boundary constants:
const int LEFT_LIMIT = RADIUS; //left side of screen limit
const int RIGHT_LIMIT = 127-RADIUS; //right side of screen limit
const int TOP_LIMIT = RADIUS; //top of screen limit
const int BOTTOM_LIMIT = 159-RADIUS; //bottom of screen limit

//state variables:
// default values
const float x_pos_d = 64; //x position
const float y_pos_d = 80; //y position
const float x_vel_d = 0; //x velocity
const float y_vel_d = 0;  //y velocity
const float x_accel_d = 0; //x acceleration
const float y_accel_d = 0; //y acceleration
const uint8_t state_default = 0;
// ball 1
float x_pos_1 = 64;
float y_pos_1 = 80;
float x_vel_1 = 0;
float y_vel_1 = 0; 
float x_accel_1 = 0;
float y_accel_1 = 0;
uint8_t state_1 = 0;
// ball 2
float x_pos_2 = 64;
float y_pos_2 = 80;
float x_vel_2 = 0;
float y_vel_2 = 0; 
float x_accel_2 = 0;
float y_accel_2 = 0;
uint8_t state_2 = 0;
// ball 3
float x_pos_3 = 64;
float y_pos_3 = 80;
float x_vel_3 = 0;
float y_vel_3 = 0; 
float x_accel_3 = 0;
float y_accel_3 = 0;
uint8_t state_3 = 0;
// ball 4
float x_pos_4 = 64;
float y_pos_4 = 80;
float x_vel_4 = 0;
float y_vel_4 = 0; 
float x_accel_4 = 0;
float y_accel_4 = 0;
uint8_t state_4 = 0;

MPU9255 imu; //imu object called, appropriately, imu


void step(float x_force=0, float y_force=0 ){
  //update acceleration (from f=ma)
  x_accel = (x_force-K_FRICTION*x_vel)/MASS;
  y_accel = (y_force-K_FRICTION*y_vel)/MASS;
  //integrate to get velocity from current acceleration
  x_vel = x_vel + 0.001*DT*x_accel; //integrate, 0.001 is conversion from milliseconds to seconds
  y_vel = y_vel + 0.001*DT*y_accel; //integrate
  //
  moveBall();
}
void moveBall(){
  float nextX = x_pos + 0.001*DT*x_vel;
  float nextY = y_pos + 0.001*DT*y_vel;
  if (nextX < LEFT_LIMIT) {
    x_pos = LEFT_LIMIT+ K_SPRING*(LEFT_LIMIT - nextX);
    x_vel = -K_SPRING*x_vel;
  } else if (nextX > RIGHT_LIMIT) {
    x_pos = RIGHT_LIMIT- K_SPRING*(nextX - RIGHT_LIMIT);
    x_vel = -K_SPRING*x_vel;
  } else {
    x_pos = nextX;
  }

  if (nextY > BOTTOM_LIMIT) {
    y_pos = BOTTOM_LIMIT- K_SPRING*(nextY - BOTTOM_LIMIT);
    y_vel = -K_SPRING*y_vel;
  } else if (nextY < TOP_LIMIT) {
    y_pos = TOP_LIMIT+K_SPRING*(TOP_LIMIT - nextY);
    y_vel = -K_SPRING*y_vel;
  } else {
    y_pos = nextY;
  }
}

void ball_update(state) { // pass in the state of the ball (declared globally)
  switch (state) {
    case 0: //initialize ball (stationary)
      if (!launch) { // if pushed
        state = 1; //change state
      } else {
        tft.fillCircle(x_pos,y_pos,RADIUS,BALL_COLOR); //draw ball
      }
    break;
    
    case 1: // update acceleration
      if (!launch) { // if pushed
        tft.fillCircle(x_pos,y_pos,RADIUS,BALL_COLOR);
        // calculate acceleration
        imu.readAccelData(imu.accelCount);//read imu
        float y = -imu.accelCount[0]*imu.aRes;
        float x = -imu.accelCount[1]*imu.aRes;
        tft.fillCircle(x_pos,y_pos,RADIUS,BALL_COLOR);
      } else {
        tft.fillCircle(x_pos,y_pos,RADIUS,BALL_COLOR);
        state = 2;
      }
    break;

    case 2: // call step() with updated acceleration once released botton
      tft.fillCircle(x_pos,y_pos,RADIUS,BALL_COLOR);
      if (push_count < 4){
        push_count++;
      }
      step(x*EXCITEMENT, y*EXCITEMENT);//apply force based on last acceleration
      tft.fillCircle(x_pos,y_pos,RADIUS,BALL_COLOR);
      state = 3;
    break;

    case 3: // final state, stop updating accel
      tft.fillCircle(x_pos,y_pos,RADIUS,BALL_COLOR);
      step();
      tft.fillCircle(x_pos,y_pos,RADIUS,BALL_COLOR);
    break;

    default: // handling unexpected behavior
      Serial.println("state machine not working");
    break;
  }
}

void ball_1(float x_pos, float y_pos, float x_vel, float y_vel, float x_accel, float y_accel) {
  if (push_count == 0) {
    ball_update(state_1);
  }
}

void ball_2(float x_pos, float y_pos, float x_vel, float y_vel, float x_accel, float y_accel) {
  if (push_count == 1) {
    ball_update(state_2);
  }
}

void ball_3(float x_pos, float y_pos, float x_vel, float y_vel, float x_accel, float y_accel) {
  if (push_count == 2) {
    ball_update(state_3);
  }
}

void ball_4(float x_pos, float y_pos, float x_vel, float y_vel, float x_accel, float y_accel) {
  if (push_count == 3) {
    ball_update(state_4);
  }
}

void run_all_balls() {
  ball_1(x_pos_1, y_pos_1, x_vel_1, y_vel_1, x_accel_1, y_accel_1);
  ball_2(x_pos_2, y_pos_2, x_vel_2, y_vel_2, x_accel_2, y_accel_2);
  ball_3(x_pos_3, y_pos_3, x_vel_3, y_vel_3, x_accel_3, y_accel_3);
  ball_4(x_pos_4, y_pos_4, x_vel_4, y_vel_4, x_accel_4, y_accel_4);
}

void setup() {
  Serial.begin(115200); //for debugging if needed.
  pinMode(input_pin1, INPUT_PULLUP); //set input pin as an input!
  pinMode(input_pin2, INPUT_PULLUP); //set input pin as an input!
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(BACKGROUND);
  if (imu.setupIMU(1)){
    Serial.println("IMU Connected!");
  }else{
    Serial.println("IMU Not Connected :/");
    Serial.println("Restarting");
    ESP.restart(); // restart the ESP (proper way)
  }
  push_count = 0;
  pushed_last_time = false;
  primary_timer = millis();
}
 
void loop() {
  uint8_t launch = digitalRead(input_pin1);
  uint8_t resetS = digitalRead(input_pin2);

  /*state machine for resettting
  once pushed, before even release, will reset the program
  */
  if (!resetS){ //if pushed
    if(!pushed_last_time){ //if not previously pushed
      pushed_last_time = true;
      push_count = 0;
      //run_all_balls();
    }else{
      //run_all_balls();
    }
  }else{ //else not pushed
    pushed_last_time = false; //mark not pushed
    //run_all_balls();
  }

  while (millis()-primary_timer < DT); //wait for primary timer to increment
  primary_timer = millis();
}