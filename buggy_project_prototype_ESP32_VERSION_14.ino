//imported libaries
#include "SR04.h"
#include "string.h"
#include "BluetoothSerial.h"
#include "pitches.h"
#include <Preferences.h>

// self created libaries
#include "motor.h"
#include "button.h"
#include "timer.h"
#include "RGBled.h"
#include "buzzer.h"



String device_name = "Miranda";

byte TRIG_PIN = 13;
byte ECHO_PIN = 14;

byte BUTTON_PIN = 4;

byte IR_PIN = 33;
byte CAL_PIN = 32;

byte pin1 = 17;
byte pin2 = 18;
byte enA = 19;

byte pin3 = 21;
byte pin4 = 22;
byte enB = 23;

byte BUZ_PIN = 16;

byte R_PIN = 25;
byte G_PIN = 26;
byte B_PIN = 27;


void serial_input();

int calc_pid(); 

void buzzer_sequence();


void init_standby();
void update_standby();
void button_standby(unsigned long length);

void init_active();
void update_active();
void button_active(unsigned long timer);
void led_active();

void init_calibration();
void update_calibration();
void button_calibration1(timer &ptimer);
void button_calibration2(timer &ptimer);

void init_testing();
void update_testing();

void timer_init_lost(timer &ptimer);
void init_lost();
void update_lost();

motor_controller l_motor(pin1, pin2, enA), r_motor(pin3, pin4, enB);
button_controller button_main(BUTTON_PIN);
SR04 distance_sensor(ECHO_PIN,TRIG_PIN);
RGBLed_Manager mode_led(R_PIN, G_PIN, B_PIN);
timer calibration_timer1(3000,button_calibration1,false,1);
timer calibration_timer2(6500,button_calibration2,false,1);
timer lost_timer(5000,timer_init_lost,false,1);
buzzer_manager buzzer(BUZ_PIN);
Preferences preferences;
BluetoothSerial SerialBT;

const String GET_MODE = "GET MODE";
const String SET_MODE = "SET MODE";

long unsigned loop_delay = 200;


float IR_input;

byte mode = 0;
const byte standby_mode = 0;
const byte calibration_mode = 1;
const byte active_mode = 2;
const byte test_mode = 3;
const byte lost_mode = 4;

const int max_speed = 255;
int default_speed = 120;
const String SAVE_DEFAULT_SPEED = "DS";
const String SET_DEFAULT_SPEED = "SET DEFAULT SPEED";
const String GET_DEFAULT_SPEED = "GET DEFAULT SPEED";
int right_motor_speed;
const String SET_R_SPEED = "SET R SPEED";
const String GET_R_SPEED = "GET R SPEED";
int left_motor_speed;
const String SET_L_SPEED = "SET L SPEED";
const String GET_L_SPEED = "GET L SPEED";
int dead_zone  = -1;
const String SET_DEAD_ZONE = "SET DEAD ZONE";
const String GET_DEAD_ZONE = "GET DEAD ZONE";
const char END_LINE = '\n';
float p_control = 0.5;
const String SAVE_P_CONTROL = "P";
const String SET_P_CONTROL = "SET P CONSTANT";
const String GET_P_CONTROL = "GET P CONSTANT";
float error_total = 0;
float i_control = 0;
const String SAVE_I_CONTROL = "I";
const String SET_I_CONTROL = "SET I CONSTANT";
const String GET_I_CONTROL = "GET I CONSTANT";
float last_error = 0;
float d_control = 0.5;
const String SAVE_D_CONTROL = "D";
const String SET_D_CONTROL = "SET D CONSTANT";
const String GET_D_CONTROL = "GET D CONSTANT";
bool ir_report = false;
const String GET_SENSOR_DATA = "GET SENSOR DATA";

void setup() {
  Serial.begin(115200);
  pinMode(IR_PIN,INPUT);
  pinMode(CAL_PIN,OUTPUT);
  SerialBT.begin(device_name);
  while (! Serial); // Wait untilSerial is ready
  SerialBT.println("Setup complete"); // This sends an output to the serial board just to signal that the programme has been setup/has completed this stage
  init_standby();
  buzzer_sequence();
  preferences.begin("miranda",false);
  p_control = preferences.getFloat(SAVE_P_CONTROL.c_str(),0.5);
  i_control = preferences.getFloat(SAVE_I_CONTROL.c_str(),0);
  d_control = preferences.getFloat(SAVE_D_CONTROL.c_str(),0.5);
  default_speed = preferences.getInt(SAVE_DEFAULT_SPEED.c_str(),120);

}


void loop() {
  if (mode == standby_mode){
    update_standby();
  } else if (mode == active_mode){
    update_active();
  } else if (mode == calibration_mode){
    update_calibration();
  } else if (mode == test_mode){
    update_testing();
  } else if (mode == lost_mode){
    update_lost();
  }
  button_main.check();
  mode_led.update();
  buzzer.update();
  lost_timer.update();
  if (SerialBT.available() > 0){//This simply checks if something has been inputed. Therefore if something has been inputted we check what the input was.
    serial_input(); //This was a previously declared procedure which is written below the programme. It's main purpose is to read input to change the danger or warning distance.
  }
}

void buzzer_sequence(){
  buzzer.add_note(NOTE_E5,300,100);
  buzzer.add_note(NOTE_E5,300,100);
  buzzer.add_note(NOTE_E5,300,100);
  buzzer.add_note(NOTE_E5,900,100);
  buzzer.add_note(NOTE_E5,900,100);
  buzzer.add_note(NOTE_E5,900,100);
  buzzer.add_note(NOTE_E5,300,100);
  buzzer.add_note(NOTE_E5,300,100);
  buzzer.add_note(NOTE_E5,300,1000);
}

int calc_pid(){
  float p_val;
  float i_val;
  float d_val;
  IR_input = analogRead(IR_PIN);
  IR_input = 4095 - IR_input;
  IR_input = IR_input - 2048;
  if (IR_input < -1638){
    IR_input = last_error;
  }else if(IR_input > 1638){
    if (last_error < 0){
      IR_input = -1638;
    }else{
      IR_input = 1638;
    }
    lost_timer.start();
  }else{
    lost_timer.stop();
  }
  p_val = p_control * IR_input;
  error_total += IR_input;
  i_val = i_control * error_total;
  d_val = d_control * (IR_input - last_error);
  last_error = IR_input;
  return (p_val + i_val + d_val);
}

void init_standby(){
  digitalWrite(CAL_PIN,HIGH);
  mode = standby_mode;
  l_motor.stop();
  r_motor.stop();
  button_main.on_release = button_standby;
  button_main.on_press = NULL;
  mode_led.stop();
  mode_led.clear();
  mode_led.add_light(150,100,0,500);
  mode_led.add_light(75,50,0,250);
  mode_led.play();
  mode_led.tag = 0;
  buzzer.stop();
}

void update_standby(){
}

void button_standby(unsigned long length){
  if (length < 4000){
    init_active();
  } else {
    init_calibration();
  }

}

void init_active(){
  mode = active_mode;
  l_motor.forward(default_speed);
  r_motor.forward(default_speed);
  button_main.on_release = button_active;
  button_main.on_press = NULL;
  last_error = 0;
  error_total = 0;
  led_active();
}

void update_active(){
  float pid_val;
  if (distance_sensor.Distance() > 20){
    pid_val = calc_pid();
    left_motor_speed = constrain(default_speed + pid_val,(-max_speed),max_speed);
    right_motor_speed = constrain(default_speed - pid_val,(-max_speed),max_speed);
    if  (left_motor_speed > 0){
      l_motor.forward(left_motor_speed);
    }else{
      left_motor_speed = 0 - left_motor_speed;
      l_motor.backwards(left_motor_speed);
    }
    if  (right_motor_speed > 0){
      r_motor.forward(right_motor_speed);
    }else{
      right_motor_speed = 0 - right_motor_speed;
      r_motor.backwards(right_motor_speed);
    }
    if (mode_led.tag != 2){
      led_active();
    }
    buzzer.stop();
    } else{
      l_motor.stop();
      r_motor.stop();
      buzzer.play(true);
      if (mode_led.tag != 3){
        mode_led.stop();
        mode_led.clear();
        mode_led.add_light(250,0,0,300);
        mode_led.add_light(0,0,0,100);
        mode_led.add_light(250,0,0,300);
        mode_led.add_light(0,0,0,100);
        mode_led.add_light(250,0,0,300);
        mode_led.add_light(0,0,0,100);
        mode_led.add_light(250,0,0,900);
        mode_led.add_light(0,0,0,100);
        mode_led.add_light(250,0,0,900);
        mode_led.add_light(0,0,0,100);
        mode_led.add_light(250,0,0,900);
        mode_led.add_light(0,0,0,100);
        mode_led.add_light(250,0,0,300);
        mode_led.add_light(0,0,0,100);
        mode_led.add_light(250,0,0,300);
        mode_led.add_light(0,0,0,100);
        mode_led.add_light(250,0,0,300);
        mode_led.add_light(0,0,0,1000);
        mode_led.play();
        mode_led.tag = 3;
      }
  }
}

void button_active(unsigned long timer){
  init_standby();
}

void led_active(){
  mode_led.stop();
  mode_led.clear();
  mode_led.add_light(0,240,0,1000);
  mode_led.add_light(0,100,0,250);
  mode_led.play();
  mode_led.tag = 2;
}

void init_calibration(){
  l_motor.stop();
  r_motor.stop();
  button_main.on_press = NULL;
  button_main.on_release = NULL;
  mode_led.stop();
  mode_led.clear();
  mode_led.add_light(150,0,150,1000);
  mode_led.add_light(50,0,50,250);
  mode_led.play();
  mode_led.tag = 1;
  mode = calibration_mode;
  digitalWrite(CAL_PIN,LOW);
  calibration_timer1.start();
}

void update_calibration(){
  if (calibration_timer2.is_active()){
    r_motor.forward(150);
    l_motor.backwards(150);
  }
  calibration_timer1.update();
  calibration_timer2.update();
}

void button_calibration1(timer &ptimer){
  digitalWrite(CAL_PIN,HIGH);
  calibration_timer2.start();
}

void button_calibration2(timer &ptimer){
  digitalWrite(CAL_PIN,LOW);
  delay(20);
  digitalWrite(CAL_PIN,HIGH);
  init_standby();
}


void init_testing(){
  mode = test_mode;
  l_motor.stop();
  r_motor.stop();
  button_main.on_press = NULL;
  button_main.on_release = NULL;
  mode_led.stop();
  mode_led.clear();
  mode_led.add_light(0,150,150,1000);
  mode_led.add_light(0,50,50,1000);
  mode_led.play();
  mode_led.tag = 3;
}

void update_testing(){ 
  if (right_motor_speed > 0){
    r_motor.forward(right_motor_speed);
  }else if (right_motor_speed < 0){
    right_motor_speed = 0 - right_motor_speed ;
    r_motor.backwards(right_motor_speed);
  } else{
    r_motor.stop();
  }
  if (left_motor_speed > 0){
    l_motor.forward(left_motor_speed);
  }else if (left_motor_speed < 0){
    left_motor_speed = 0 - left_motor_speed ;
    l_motor.backwards(left_motor_speed);
  } else{
    l_motor.stop();
  }
}

void timer_init_lost(timer &ptimer){
  init_lost();
}

void init_lost(){
  mode = lost_mode;
  l_motor.stop();
  r_motor.stop();
  button_main.on_release = button_lost;
  button_main.on_press = NULL;
  mode_led.stop();
  mode_led.clear();
  mode_led.add_light(250,0,0,300);
  mode_led.add_light(0,0,0,100);
  mode_led.add_light(250,0,0,300);
  mode_led.add_light(0,0,0,100);
  mode_led.add_light(250,0,0,300);
  mode_led.add_light(0,0,0,100);
  mode_led.add_light(250,0,0,900);
  mode_led.add_light(0,0,0,100);
  mode_led.add_light(250,0,0,900);
  mode_led.add_light(0,0,0,100);
  mode_led.add_light(250,0,0,900);
  mode_led.add_light(0,0,0,100);
  mode_led.add_light(250,0,0,300);
  mode_led.add_light(0,0,0,100);
  mode_led.add_light(250,0,0,300);
  mode_led.add_light(0,0,0,100);
  mode_led.add_light(250,0,0,300);
  mode_led.add_light(0,0,0,1000);
  mode_led.play();
  mode_led.tag = 4;
  buzzer.play(true);
}

void update_lost(){

}

void button_lost(unsigned long timer){
  init_standby();
}

void serial_input(){
  bool valid = true;
  String command;
  String value;
  int index;
  int int_value;
  float flt_value;

  String input = SerialBT.readStringUntil('\n');//This reads the string inputted in the serial monitor until the next line.
  input.toUpperCase();//This changes the string to uppercase ensuring there will be no errors when comparing it to other strings.
  SerialBT.println(input);//This simply prints the input so I can see it has been capitalized properly
  
  index = input.indexOf(':');
  SerialBT.println(index);
  command =  input.substring(0,index);
  value = input.substring(index+1,input.length());
  SerialBT.println(command);
  SerialBT.println(value);
  if (command.equals(SET_DEFAULT_SPEED)){//This ensure the person has entered the correct code to change the warning distance
    valid = true;
    for (int i = 0; i == value.length(); i++){//this goes through the string to check the inputs are numbers
      if (!isdigit(value[i])) {
        valid = false;// it changes valid to false if it isn't to stop the code proceeding
      }
    }
    if (valid){
      int_value = value.toInt();//This sets the end of the string containg the numbers to an integer.
      if (int_value > dead_zone){// This ensures the warning distance is larger than the danger distance
        default_speed = int_value; // this sets the warning distance to the new value after it has passed the previous checks.
        SerialBT.print("New default set to: ");
        SerialBT.println(default_speed);//This simply confirms the new warning distance has been set
        preferences.putInt(SAVE_DEFAULT_SPEED.c_str(), default_speed);
      }
      else{
        SerialBT.println("speed is smaller than dead zone. No new speed set.");
        SerialBT.println(int_value);// This tells us why the new distance wasn't set if it wasn't
        
      } 
    }
  }else if (command.equals(GET_DEFAULT_SPEED)){//This ensure the person has entered the correct code to change the warning distance
      SerialBT.print("Default speed: ");
      SerialBT.println(default_speed);//This simply confirms the new warning distance has been set
  }else if (command.equals(SET_R_SPEED)){//This ensure the person has entered the correct code to change the warning distance
      if (mode == test_mode){
        valid = true;
        for (int i = 0; i == value.length(); i++){//this goes through the string to check the inputs are numbers
          if (!isdigit(value[i])) {
            valid = false;// it changes valid to false if it isn't to stop the code proceeding
          }
        }
        if (valid){
          int_value = value.toInt();//This sets the end of the string containg the numbers to an integer.
          if (int_value > dead_zone){// This ensures the warning distance is larger than the danger distance
            right_motor_speed = int_value; // this sets the warning distance to the new value after it has passed the previous checks.
            SerialBT.print("New right speed set to: ");
            SerialBT.println(right_motor_speed);//This simply confirms the new warning distance has been set
          }
          else{
            SerialBT.println("speed is smaller than dead zone. No new speed set.");
            SerialBT.println(int_value);// This tells us why the new distance wasn't set if it wasn't
            
          } 
        }
      } else{
        SerialBT.println("Only valid in test mode.");
      }
    }else if (command.equals(GET_R_SPEED)){//This ensure the person has entered the correct code to change the warning distance
      SerialBT.print("Right speed: ");
      SerialBT.println(right_motor_speed);
    }else if (command.equals(SET_L_SPEED)){//This ensure the person has entered the correct code to change the warning distance
      if (mode == test_mode){
        valid = true;
        for (int i = 0; i == value.length(); i++){//this goes through the string to check the inputs are numbers
          if (!isdigit(value[i])) {
            valid = false;// it changes valid to false if it isn't to stop the code proceeding
          }
        }
        if (valid){
          int_value = value.toInt();//This sets the end of the string containg the numbers to an integer.
          if (int_value > dead_zone){// This ensures the warning distance is larger than the danger distance
            left_motor_speed = int_value; // this sets the warning distance to the new value after it has passed the previous checks.
            SerialBT.print("New left speed set to: ");
            SerialBT.println(left_motor_speed);//This simply confirms the new warning distance has been set
          }
          else{
            SerialBT.println("speed is smaller than dead zone. No new speed set.");
            SerialBT.println(int_value);// This tells us why the new distance wasn't set if it wasn't
            
          } 
        }
     }else{
      SerialBT.println("This command is only valid during test mode.");
     }
  }else if (command.equals(GET_L_SPEED)){//This ensure the person has entered the correct code to change the warning distance
    SerialBT.print("left speed: ");
    SerialBT.println(left_motor_speed);
  }else if(command.equals(GET_MODE)){
    SerialBT.print("get mode");
    SerialBT.println(mode);
  }else if(command.equals(SET_MODE)){
        valid = true;
    for (int i = 0; i == value.length(); i++){//this goes through the string to check the inputs are numbers
      if (!isdigit(value[i])) {
        valid = false;// it changes valid to false if it isn't to stop the code proceeding
      }
    }
    if (valid){
      int_value = value.toInt();
      if ((int_value >= 0) and (int_value <= 3)){
        mode = int_value;
        SerialBT.print("New mode: ");
        SerialBT.println(mode);
        if (mode == standby_mode){
          init_standby();
        } else if (mode == active_mode){
          init_active();
        } else if (mode == calibration_mode){
          init_calibration();
        } else if (mode == test_mode){
          init_testing();
       } else if (mode == lost_mode){
          init_lost();
        }
      }
    } else {
      SerialBT.println("Invalid input");
    }
  }else if(command.equals(GET_DEAD_ZONE)){
    SerialBT.print("get dead zone: ");
    SerialBT.println(dead_zone);
  }else if(command.equals(SET_DEAD_ZONE)){
    valid = true;
    for (int i = 0; i == value.length(); i++){//this goes through the string to check the inputs are numbers
      if (!isdigit(value[i])) {
        valid = false;// it changes valid to false if it isn't to stop the code proceeding
      }
    }
    if (valid){
      int_value = value.toInt();//This sets the end of the string containg the numbers to an integer.
      if (int_value < default_speed){// This ensures the warning distance is larger than the danger distance
        dead_zone = int_value; // this sets the warning distance to the new value after it has passed the previous checks.
        SerialBT.print("New dead zone set to: ");
        SerialBT.println(dead_zone);//This simply confirms the new warning distance has been set
      }
      else{
        SerialBT.println("dead zone is larger than default speed. No new dead zone set.");
        SerialBT.println(int_value);// This tells us why the new distance wasn't set if it wasn't
        
      } 
    }
  }else if(command.equals(GET_SENSOR_DATA)){
    SerialBT.print("sensor data: ");
    SerialBT.println(IR_input);
  }else if(command.equals(GET_P_CONTROL)){
    SerialBT.print("p constant: ");
    SerialBT.println(p_control);
  }else if(command.equals(SET_P_CONTROL)){
    flt_value = value.toFloat();//This sets the end of the string containg the numbers to a float
    if (flt_value != 0){
      p_control = flt_value; // this sets the warning distance to the new value after it has passed the previous checks.
      SerialBT.print("New P constant set to: ");
      SerialBT.println(p_control);//This simply confirms the new warning distance has been set}
      preferences.putFloat(SAVE_P_CONTROL.c_str(), p_control);
    }else{
      SerialBT.print("invalid input");
    }
  }else if(command.equals(GET_I_CONTROL)){
    SerialBT.print("I constant: ");
    SerialBT.println(i_control);
  }else if(command.equals(SET_I_CONTROL)){
    flt_value = value.toFloat();//This sets the end of the string containg the numbers to an integer.
      i_control = flt_value; // this sets the warning distance to the new value after it has passed the previous checks.
      SerialBT.print("New I constant set to: ");
      SerialBT.println(i_control);//This simply confirms the new warning distance has been set}
      preferences.putFloat(SAVE_I_CONTROL.c_str(), i_control);
    
  }else if(command.equals(GET_D_CONTROL)){
     SerialBT.print("d constant: ");
    SerialBT.println(d_control);
  }else if(command.equals(SET_D_CONTROL)){
    flt_value = value.toFloat();//This sets the end of the string containg the numbers to an integer.
    if (flt_value != 0){
      d_control = flt_value; // this sets the warning distance to the new value after it has passed the previous checks.
      SerialBT.print("New D constant set to: ");
      SerialBT.println(d_control);//This simply confirms the new warning distance has been set}
      preferences.putFloat(SAVE_D_CONTROL.c_str(), d_control);
    }else{
      SerialBT.print("invalid input");
    }
  }else{
    SerialBT.println("This command was not recognised.");
  }

}
