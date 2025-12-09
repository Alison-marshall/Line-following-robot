struct light_link
{
  int red;
  int green;
  int blue;
  long int duration;
  light_link* next;
};

class RGBLed_Manager
{
  private:
    light_link* first_link;
    light_link* last_link;
    light_link* current_link;
    int red_pin;
    int green_pin;
    int blue_pin;
    unsigned long next_light;
    bool active;

    void show_light(int red_value, int green_value, int blue_value)
    {
      analogWrite(red_pin, red_value);
      analogWrite(green_pin, green_value);
      analogWrite(blue_pin, blue_value);   
    }
    
  public:   
    int tag;
    
    RGBLed_Manager(int pred_pin, int pgreen_pin, int pblue_pin)
    {
      red_pin = pred_pin;
      green_pin = pgreen_pin;
      blue_pin = pblue_pin;
      first_link = NULL;
      last_link = NULL;
      current_link = NULL;
      next_light = 0;
      tag = 0;
      active = false;
      pinMode(red_pin, OUTPUT);
      pinMode(green_pin, OUTPUT);
      pinMode(blue_pin, OUTPUT);
    }

    bool isactive()
    {
      return active;
    }

    void add_light(int pred_value, int pgreen_value, int pblue_value, long int pduration)
    {
      light_link* temp_link;
      temp_link = new(light_link);
      temp_link->red = pred_value;
      temp_link->green = pgreen_value;
      temp_link->blue = pblue_value;
      temp_link->duration = pduration;
      temp_link->next = NULL;
      if (first_link == NULL)
      {
        first_link = temp_link;
        last_link = temp_link;
      }
      else
      {
        last_link->next = temp_link;
        last_link = temp_link;
      }
    }

    void play()
    {
      if (active == false)
      {
        current_link = first_link;
        active = true;
        if (current_link != NULL)
        {
          show_light(current_link->red, current_link->green, current_link->blue);
          next_light = millis() + current_link->duration;
        }
      }
    }

    void stop()
    {
      show_light(0,0,0);
      active = false;
    }

    void clear()
    {
      light_link* temp_link;
      light_link* next_link;
      temp_link = first_link;
      while (temp_link != NULL)
      {
        next_link = temp_link->next;
        delete temp_link;
        temp_link = next_link;
      }
      first_link = NULL;
      last_link = NULL;
      current_link = NULL;
    }

    void update()
    {
      if (active)
      {
        if (millis() >= next_light)
        {
          current_link = current_link->next;
          if (current_link == NULL)
          {
            current_link = first_link;
          }
          show_light(current_link->red, current_link->green, current_link->blue);
          next_light += current_link->duration;
        }
      }
    }
};