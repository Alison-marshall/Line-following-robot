const byte mstop = 0;
const byte mforward = 1;
const byte mbackwards = 2;

class motor_controller{
  public: 
    byte status = -1;
    void forward(byte speed);
    void backwards(byte speed);
    void stop();
    motor_controller(byte ipin1, byte ipin2, byte ienA);
  private:  
    byte pin1;
    byte pin2;
    byte enA;
};
motor_controller::motor_controller(byte ipin1, byte ipin2, byte ienA){
  pin1 = ipin1;
  pin2 = ipin2;
  enA = ienA ;
  status = mstop;
  pinMode( enA , OUTPUT);
  pinMode( pin1 , OUTPUT);
  pinMode( pin2 , OUTPUT);
  stop();
}

void motor_controller::forward(byte speed){
  if (status != mforward){
    analogWrite(enA,0);
    digitalWrite(pin1,LOW);
    digitalWrite(pin2,HIGH);
  }
  analogWrite(enA,speed);
  status = mforward;
}

void motor_controller::backwards(byte speed){
  if (status != mbackwards){
    analogWrite(enA,0);
    digitalWrite(pin1,HIGH);
    digitalWrite(pin2,LOW);
  }
  analogWrite(enA,speed);
  status = mbackwards;
}

void motor_controller::stop(){
  analogWrite(enA,0);
  digitalWrite(pin1,LOW);
  digitalWrite(pin2,HIGH);
  status = mstop;
}