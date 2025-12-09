class timer
{
  public:
    void (*trigger_func)(timer &ptimer);

    timer(unsigned long pinterval, void(*pfunct)(timer &ptimer), bool pactive,int pmax_count);

    bool is_active();
    void start();
    void stop();
    void reset();
    void change_interval(unsigned long pinterval);
    void update();
    int counter();
  private:
    bool active;
    unsigned long interval;
    unsigned long next_trigger;
    int max_count;
    int timer_counter;
};

timer::timer(unsigned long pinterval, void(*pfunct)(timer &ptimer), bool pactive,int pmax_count)
{
  active = pactive;
  interval = pinterval;
  trigger_func = pfunct;
  timer_counter = 0;
  max_count = pmax_count;
  reset();
}

int timer::counter(){
  return timer_counter;
}

bool timer::is_active(){
  return active;
}

void timer::start(){
  if (active != true){
    active = true;
    reset();
  }
}

void timer::stop(){
  active = false;
}

void timer::reset(){
  next_trigger = millis() + interval;
  timer_counter = 0;
}

void timer::change_interval(unsigned long pinterval){
  interval = pinterval;
}

void timer::update(){
  if (active){
    if (timer_counter == max_count){
      active = false;
    }else{
      if (millis() >= next_trigger){
        trigger_func(*this);
        next_trigger += interval;
        timer_counter ++;
      }
    }

  }

};
