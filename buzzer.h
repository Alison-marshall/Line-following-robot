struct note_link
{
  int note;
  unsigned long int duration;
  unsigned long int delay;
  note_link* next;
};

class buzzer_manager
{
  private:
    note_link* first_note;
    note_link* last_note;
    note_link* current_note;
    int outputpin;
    unsigned long playnext;
    bool active;
    
  public:   
    bool loop;
    int tag;

    buzzer_manager(int buzzerpin)
    {
      outputpin = buzzerpin;
      first_note = NULL;
      last_note = first_note;
      current_note = first_note;
      playnext = 0;
      loop = false;
      tag = 0;
      active = false; 
    }

    bool isactive()
    {
      return active;
    }

    void add_note(int pnote, long int pduration, long int pdelay)
    {
      note_link* temp_link;
      temp_link = new(note_link);
      temp_link->note = pnote;
      temp_link->duration = pduration;
      temp_link->delay = pdelay;
      temp_link->next = NULL;
      if (first_note == NULL)
      {
        first_note = temp_link;
        last_note = temp_link;
      }
      else
      {
        last_note->next = temp_link;
        last_note = temp_link;
      }
    }

    void play(bool ploop)
    {
      if (active == false)
      {
        current_note = first_note;
        loop = ploop;
        active = true;
        if (current_note != NULL)
        {
          tone(outputpin, current_note->note, current_note->duration);
          playnext = millis() + current_note->duration + current_note->delay;
        }
      }
    }

    void stop()
    {
      noTone(outputpin);
      active = false;
    }

    void clear()
    {
      note_link* temp_link;
      note_link* nextlink;
      temp_link = first_note;
      while (temp_link != NULL)
      {
        nextlink = temp_link->next;
        delete temp_link;
        temp_link = nextlink;
      }
      first_note = NULL;
      last_note = NULL;
      current_note = NULL;
    }

    void update()
    {
      if (active)
      {
        if (millis() >= playnext)
        {
          current_note = current_note->next;
          if (current_note == NULL)
          {
            if (loop)
            {
              current_note = first_note;
            }
            else
            {
              stop();
            }
          }
          if (active)
          {
            noTone(outputpin);
            tone(outputpin, current_note->note, current_note->duration);
            playnext += current_note->duration + current_note->delay;
          }
        }
      }
    }
};