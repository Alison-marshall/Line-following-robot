

class button_controller{
  public:
  button_controller(byte button_pin);
  void check();
  void (*on_press)();
  void (*on_release)(unsigned long duration);
  private:
  byte input_pin;
  bool low;
  unsigned long time_pressed;
};

button_controller::button_controller(byte button_pin){
  input_pin = button_pin;
  low = false;
  time_pressed = 0;
  on_press = NULL;
  on_release = NULL;
  pinMode(input_pin,INPUT_PULLUP);
}

void button_controller::check(){
  if (digitalRead(input_pin) == LOW) {
    if (low == false ){
      low = true;
      time_pressed = millis();
      if (on_press != NULL){
        on_press();
      }

    }

  }else{
    if (low == true){
      low = false;
      if (on_release != NULL){
        on_release(millis()-time_pressed);
      }
    }
  } 
}

