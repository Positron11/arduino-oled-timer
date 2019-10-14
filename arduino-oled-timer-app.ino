// libraries ======================================================================================================================================================
#include <Button.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// define oled display constants ==================================================================================================================================
#define OLED_RESET 4
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// define pins
#define BUZZER 11

// initialize encoder button and oled display =====================================================================================================================
Button right = Button(8, PULLUP); // right button
Button select = Button(9, PULLUP); // select button
Button left = Button(10, PULLUP); // left button
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // display

// global variables ===============================================================================================================================================
String screen;

// functions ======================================================================================================================================================

// pad number with leading zeroes
String pad_zeroes(int number, int len) {
  String text;
  for (int i = 0; i < len - String(number).length(); i++) {
    text += "0";
  }
  text += String(number);
  return text;
}

// incrementing
int increment(int value, int maximim, int minimum) {
  if (value < maximim) {
    value ++;
  }
  else {
    value = minimum;
  }
  return value;
}

// decrementing
int decrement(int value, int maximum, int minimum) {
  if (value > minimum) {
    value --;
  }
  else {
    value = maximum;
  }
  return value;
}

// classes ========================================================================================================================================================

class Menu {

  public:

    // highlighted button variable
    int highlighted_button;

    // buttons count variable
    int buttons;

    Menu(int buttons) {
      this-> buttons = buttons;
      highlighted_button = 1;
    }

    // scroll through buttons
    void scroll() {
      if (right.uniquePress()) {
        highlighted_button = increment(highlighted_button, buttons, 1);
      }
      else if (left.uniquePress()) {
        highlighted_button = decrement(highlighted_button, buttons, 1);
      }
    }
};

class MenuButton { //----------------------------------------------------------------------------------------------------------------------------------------------

    // button id variable
    int id;

  public:

    // button coordinates variables
    int x;
    int y;

    // button text variable
    String text;

    MenuButton(String text, int x, int y, int id) {
      this->text = text;
      this->x = x;
      this->y = y;
      this->id = id;
    }

    void show(int highlighted_button) {
      // set button coordinates
      display.setCursor(x + 2, y + 2);

      // highlight button if required
      if (id == highlighted_button) {
        display.setTextColor(BLACK, WHITE);
      }
      else {
        display.setTextColor(WHITE);
      }

      // print button
      display.print(text);

      // draw borders
      display.drawRect(x, y, text.length() * 6 + 4, 12, WHITE);

      // reset text color
      display.setTextColor(WHITE);
    }
};

class Timer: public Menu { //-------------------------------------------------------------------------------------------------------------------------------------

    // timer set time variables
    int hours;
    int minutes;
    int seconds;

    // timer elapsed time variables
    int current_hours = 0;
    int current_minutes = 0;
    int current_seconds = 0;

    // remaining time variable
    long remaining_seconds;

    // menu buttons
    MenuButton back_btn, toggle_btn;

  public:

    // timer state variable
    bool time_up;
    bool timer_on;

    Timer(long hours, long minutes, long seconds):
      Menu(2),
      back_btn("Menu", 0, 52, 1),
      toggle_btn("Pause", 60, 52, 2)
    {
      this->hours = hours;
      this->minutes = minutes;
      this->seconds =  seconds;
      remaining_seconds = (seconds) + (minutes * 60L) + (hours * 60L * 60L);
      timer_on = true;
      time_up = false;
    }

    // update timer
    void update() {
      // intitalize timer
      static unsigned long nextSecond = millis() + 1000;

      // count seconds
      if (millis() >= nextSecond && timer_on) {
        current_seconds ++;
        remaining_seconds --;
        nextSecond = millis() + 1000;
      }

      // count minutes
      if (current_seconds >= 60) {
        current_seconds = 0;
        current_minutes ++;
      }

      // count hours
      if (current_minutes >= 60) {
        current_minutes = 0;
        current_hours ++;
      }

      // check if time up
      if (remaining_seconds == 0) {
        timer_on = false;
        time_up = true;
        screen = "Flash";
      }
    }

    // toggle timer between on / off state
    void toggle() {
      if (timer_on) {
        timer_on = false;
        toggle_btn.text = "Resume";
      }
      else {
        timer_on = true;
        toggle_btn.text = "Pause";
      }
    }

    // reset timer
    void reset() {
      current_hours = 0;
      current_minutes = 0;
      current_seconds = 0;
      remaining_seconds = (seconds) + (minutes * 60L) + (hours * 60L * 60L);
      timer_on = false;
      time_up = false;
      toggle_btn.text = "Resume";
    }

    // quit timer
    void stop() {
      reset();
      time_up = true;
    }

    // show timer
    void show() {
      // initialize display
      display.clearDisplay();
      display.setCursor(0, 0);

      // print header
      display.setTextSize(2);
      display.print("TiMER");
      display.setTextSize(1);
      display.print(" Timer");

      // show timer (elapsed time)
      display.setTextSize(2);
      display.setCursor(0, 25);
      display.print(pad_zeroes(current_hours, 2) + ":" + pad_zeroes(current_minutes, 2) + ":" + pad_zeroes(current_seconds, 2));

      // show remaining time
      display.setTextSize(1);
      display.setCursor(104, 0);
      display.print("h:" + pad_zeroes((remaining_seconds / (60 * 60)), 2));
      display.setCursor(104, 16);
      display.print("m:" + pad_zeroes(((remaining_seconds / 60) % 60), 2));
      display.setCursor(104, 32);
      display.print("s:" + pad_zeroes((remaining_seconds % 60), 2));

      // show buttons
      back_btn.show(highlighted_button);
      toggle_btn.show(highlighted_button);

      display.display();
    }

    // take input
    void take_input() {
      scroll();
      if (select.uniquePress()) {
        switch (highlighted_button) {
          case 1:
            screen = "Main Menu";
            break;
          case 2:
            toggle();
            break;
        }
      }
    }
};

class MainMenu: public Menu { //-----------------------------------------------------------------------------------------------------------------------------------

    // menu buttons
    MenuButton create_btn, timer_btn, reset_btn, stop_btn;

  public:

    MainMenu():
      Menu(3),
      create_btn("Create", 0, 52, 1),
      timer_btn("Timer", 0, 20, 1),
      reset_btn("Reset", 0, 36, 2),
      stop_btn("Stop", 0, 52, 3)
    {}

    // show main menu
    void show(bool timer_exists) {
      // initialize display
      display.clearDisplay();
      display.setCursor(0, 0);

      // print header
      display.setTextSize(2);
      display.print("TiMER");
      display.setTextSize(1);
      display.print(" Menu");

      // show buttons
      if (!timer_exists) {
        create_btn.show(highlighted_button);
      }
      else {
        timer_btn.show(highlighted_button);
        reset_btn.show(highlighted_button);
        stop_btn.show(highlighted_button);
      }

      display.display();
    }

    // take input
    void take_input(Timer *timer) {
      scroll();
      if (select.uniquePress()) {
        switch (highlighted_button) {
          case 1:
            if (timer == nullptr) {
              screen = "Create";
            }
            else {
              screen = "Timer";
            }
            break;
          case 2:
            timer->reset();
            timer->highlighted_button = 2;
            screen = "Timer";
            break;
          case 3:
            timer->stop();
            break;
        }
      }
    }
};

class BlinkerNum {

    int id;

  public:

    BlinkerNum(int id) {
      this->id = id;
    }

    void show(int num, int id) {
      static unsigned long nextSwitchTime = millis() + 200;
      static bool hidden = false;
      static String alt_text;

      if (this->id == id) {
        if (millis() >= nextSwitchTime) {
          if (hidden) {
            hidden = false;
            alt_text = pad_zeroes(num, 2);
          }
          else {
            hidden = true;
            alt_text = "  ";
          }
          nextSwitchTime = millis() + 200;
        }
        display.print(alt_text);
      }
      else {
        display.print(pad_zeroes(num, 2));
      }
    }
};

class CreateMenu: public Menu { //---------------------------------------------------------------------------------------------------------------------------------

    // selected button variable
    int selected_button = 0;

    // menu buttons
    MenuButton hrs_btn, min_btn, sec_btn, confirm_btn;

    // indicator text
    BlinkerNum hrs, mins, sec;

  public:

    // time store variables
    int hours = 0;
    int minutes = 0;
    int seconds = 0;

    // creation status variable
    bool creation_completed = false;

    CreateMenu():
      Menu(4),
      hrs(1),
      mins(2),
      sec(3),
      hrs_btn("Hrs", 0, 52, 1),
      min_btn("Min", 36, 52, 2),
      sec_btn("Sec", 73, 52, 3),
      confirm_btn("Back", 100, 52, 4)
    {}

    // show create menu
    void show() {
      // initialize display
      display.clearDisplay();
      display.setCursor(0, 0);

      // print header
      display.setTextSize(2);
      display.print("TiMER");
      display.setTextSize(1);
      display.print(" Timer");

      // show set time
      display.setTextSize(2);
      display.setCursor(0, 25);
      hrs.show(hours, selected_button);
      display.print(":");
      mins.show(minutes, selected_button);
      display.print(":");
      sec.show(seconds, selected_button);

      // show buttons
      display.setTextSize(1);
      hrs_btn.show(highlighted_button);
      min_btn.show(highlighted_button);
      sec_btn.show(highlighted_button);
      if (hours == 0 && minutes == 0 && seconds == 0) {
        confirm_btn.text = "Back";
        confirm_btn.x = 100;
      }
      else {
        confirm_btn.text = "Ok";
        confirm_btn.x = 112;
      }
      confirm_btn.show(highlighted_button);

      display.display();
    }

    void take_input() {
      if (selected_button == 0) {
        scroll();
        if (select.uniquePress()) {
          if (highlighted_button == 4) {
            if (hours == 0 && minutes == 0 && seconds == 0) {
              screen = "Main Menu";
            }
            else {
              creation_completed = true;
            }
          }
          else {
            selected_button = highlighted_button;
          }
        }
      }

      else {
        if (right.uniquePress()) {
          switch (selected_button) {
            case 1:
              hours = increment(hours, 24, 0);
              break;
            case 2:
              minutes = increment(minutes, 59, 0);
              break;
            case 3:
              seconds = increment(seconds, 59, 0);
              break;
            case 4:
              break;
          }
        }

        else if (left.uniquePress()) {
          switch (selected_button) {
            case 1:
              hours = decrement(hours, 24, 0);
              break;
            case 2:
              minutes = decrement(minutes, 59, 0);
              break;
            case 3:
              seconds = decrement(seconds, 59, 0);
              break;
            case 4:
              break;
          }
        }

        // deselect button
        if (select.uniquePress()) {
          selected_button = 0;
        }
      }
    }
};

// setup ==========================================================================================================================================================
void setup() {
  // intitalize display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE);
  // set buzzer output
  pinMode(BUZZER, OUTPUT);
}

// main loop ======================================================================================================================================================
void loop() {
  // change screen
  screen = "Main Menu";

  // main menu and create menu objects
  MainMenu menu;
  menu.buttons = 1;
  CreateMenu create;

  // until timer created, switch between creation menu and main menu
  while (create.creation_completed == false) {
    if (screen == "Main Menu") {
      menu.show(false);
      menu.take_input(nullptr);
    }
    else if (screen == "Create") {
      create.show();
      create.take_input();
    }
  }

  // timer object
  Timer timer(create.hours, create.minutes, create.seconds);
  // timer pointer
  Timer *timer_ptr;
  timer_ptr = &timer;

  // add timer buttons
  menu.buttons = 3;

  // change screen
  screen = "Timer";

  // until time up, switch between timer screen and main menu
  while (timer.time_up == false) {
    timer.update();
    if (screen == "Main Menu") {
      menu.show(true);
      menu.take_input(timer_ptr);
    }
    else if (screen == "Timer") {
      timer.show();
      timer.take_input();
    }
  }

  // flashing screen to indicate time up
  if (screen == "Flash") {
    unsigned long offTime = millis() + 30000;
    while (millis() < offTime) {
      static unsigned long nextSwitchTime = millis() + 100;
      static bool flash = false;
      if (millis() >= nextSwitchTime) {
        if (flash) {
          display.clearDisplay();
          display.display();
          // buzzer on
          digitalWrite(BUZZER, HIGH);
          flash = false;
        }
        else {
          display.clearDisplay();
          display.fillRect(0, 0, 128, 64, WHITE);
          display.display();
          // buzzer off
          digitalWrite(BUZZER, LOW);
          flash = true;
        }
        nextSwitchTime = millis() + 100;
      }
      if (select.uniquePress() || right.uniquePress() || left.uniquePress()) {
        // buzzer off
        digitalWrite(BUZZER, LOW);
        break;
      }
    }
  }
}
