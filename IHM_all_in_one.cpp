
#include <Wire.h>   
#include <XInput.h>
//#include <LiquidCrystal.h>
#include "HX711.h"
              
#include "SparkFun_MMA8452Q.h"

MMA8452Q accel;                   // create instance of the MMA8452 class

float ax,ay,az;
float angle_x,angle_y,angle_z;
int map_angle;
bool button_flag;

/*--------------- loadCells------------------------*/
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN_R = A0;
const int LOADCELL_SCK_PIN_R = A1;

const int LOADCELL_DOUT_PIN_L = A2;
const int LOADCELL_SCK_PIN_L = A3;

HX711 scale_right;
HX711 scale_left;

/*--------------- Setup controller------------------------*/
const boolean UseLeftJoystick   = true;  // set to true to enable left joystick
const boolean InvertLeftYAxis   = false;  // set to true to use inverted left joy Y
const boolean UseRightJoystick  = true;  // set to true to enable right joystick
const boolean InvertRightYAxis  = false;  // set to true to use inverted right joy Y
const boolean UseTriggerButtons = false;   // set to false if using analog triggers

const int ADC_Max = 255;  // 10 bit
const int32_t ADC_Max_loadCell = 2147483647;

/*--------------- Variables ------------------------*/
int32_t leftJoyX;
int32_t leftJoyX_right;
int32_t leftJoyX_left;
int32_t freinage;
int leftJoyX_M;
int time_last;
                                 
//  Pins allocation
const int Pin_ButtonA = 6;
/*--------------- LCD------------------------*/
const int rs = 9, en = 8, d4 = 7, d5 = 6, d6 = 5, d7 = 3;
//LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  // set up the LCD's number of columns and rows:
  Wire.begin();
  if (accel.begin() == false) {
    //Serial.println("Not Connected. Please check connections and read the hookup guide.");
    while (1);
  }
  time_last = millis();
  /*lcd.begin(16, 2);
  lcd.setCursor(0, 1);
  // Print a message to the LCD.
  lcd.print("IHM dragon !");*/
  
  scale_right.begin(LOADCELL_DOUT_PIN_R, LOADCELL_SCK_PIN_R);
  scale_left.begin(LOADCELL_DOUT_PIN_L, LOADCELL_SCK_PIN_L);
  delay(1000); 
/*--------------- Xinput ------------------------*/  
  XInput.setTriggerRange(0, ADC_Max);
  // Set buttons as inputs, using internal pull-up resistors
  pinMode(Pin_ButtonA, INPUT);  
  XInput.setJoystickRange(-(ADC_Max), ADC_Max);  // Set joystick range 
  XInput.setAutoSend(false);  // Wait for all controls before sending 
  XInput.begin();
  
}

void loop() {
  int current = millis();
  if(accel.available()){
    // Acceleration of x, y, and z directions in g units
    ax = accel.getCalculatedX()*9.87;
    ay = accel.getCalculatedY()*9.87;
    az = accel.getCalculatedZ()*9.87;
    
    //angle_x = atan(ax/az);
    angle_x = atan(ax/(sqrt(square(ay)+square(az))));
    //angle_y = atan(ay/az);
    angle_y = atan(ay/(sqrt(square(ax)+square(az))));
    
    angle_x *= 180;
    angle_x /= 3.141592;
    
    angle_y *= 180;
    angle_y /= 3.141592;
    
    map_angle = map(angle_x,-90,90,-255,255);
    if(az>19.5){
      button_flag = true;
    }
    else{
      button_flag = false;
    }
  }
  
  if (scale_right.is_ready() and scale_left.is_ready() and ((time_last+100)<current)) {
    leftJoyX_right = scale_right.read();
    leftJoyX_left = scale_left.read();
    
    if(leftJoyX_right > 0 and leftJoyX_left > 100000){
      if(leftJoyX_right>1000000){
        leftJoyX_right = 1000000;
      }
      if(leftJoyX_left>1000000){
        leftJoyX_left = 1000000;
      }
      //lcd.setCursor(0, 1);
      //lcd.print("freinage");
      freinage = map(leftJoyX_left,100000,1000000,0,255);
    }
  
    else if(leftJoyX_right > 0 and leftJoyX_left < 100000){
      freinage = 0;
      if(leftJoyX_right>1000000){
        leftJoyX_right = 1000000;
      }
      leftJoyX = leftJoyX_right;
      leftJoyX_M =  map(leftJoyX,0,1000000,0,255);
    }
  else if(leftJoyX_right < 0 and leftJoyX_left > 100000){
    freinage = 0;
    if(leftJoyX_left>1000000){
      leftJoyX_left = 1000000;
    }
    leftJoyX = leftJoyX_left;
    leftJoyX_M = map(leftJoyX,100000,1000000,0,-255);
  }
  else{
    leftJoyX_M = 0;
    /*lcd.setCursor(0, 1);
    lcd.print(leftJoyX_M);*/
  }    
  time_last = millis();
  }


  if (UseTriggerButtons == true) {
    // Read trigger buttons
    boolean triggerLeft;
    boolean triggerRight;
    // Set the triggers as if they were buttons
    XInput.setButton(TRIGGER_LEFT, triggerLeft);
    XInput.setButton(TRIGGER_RIGHT, triggerRight);
  }
  else {
    // Read trigger potentiometer values
    // Set the trigger values as analog
    XInput.setTrigger(TRIGGER_RIGHT, freinage);
  }
  
  // Set left joystick 
  if (UseLeftJoystick == true) {    
    boolean invert = !InvertLeftYAxis;
    XInput.setJoystickX(JOY_LEFT, leftJoyX_M);
    if(button_flag){
      map_angle = 0;
    }
    XInput.setJoystickY(JOY_LEFT, map_angle);
  }
  // Read pin values and store in variables
  // (Note the "!" to invert the state, because LOW = pressed)
  boolean buttonA = !digitalRead(Pin_ButtonA);
  // Set XInput buttons
  XInput.setButton(BUTTON_A, buttonA);
  XInput.setButton(BUTTON_Y, button_flag);  
  // Send control data to the computer
  XInput.send();
}