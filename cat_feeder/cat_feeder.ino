// Automatic cat feeder which allows users to set feeding times based on real-time clock

#include <DS3231.h> // includes RTC library
#include <LiquidCrystal_I2C.h> // includes the LiquidCrystal_I2X Library 
DS3231  rtc(SDA, SSCL);

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

/*****************************************************************************
Example based off of demos by Brian Schmalz (designer of the Easy Driver).
http://www.schmalzhaus.com/EasyDriver/Examples/EasyDriverExamples.html
******************************************************************************/
//Declare pin functions on Redboard
#define stp 2
#define dir 3
#define MS1 4
#define MS2 5
#define EN  6

//Declare variables for functions
char user_input;
int x;
int y;
int state;
unsigned long starttime;
unsigned long endtime;

// Feeding parameters
char* feed_times[] = {"00:00:00", "08:00:00", "12:06:00", "16:00:00","20:00:00"}; //five feed_times -- can have more or less
int fd_times_length = sizeof(feed_times) / sizeof(feed_times[0]);
unsigned long feed_dur = 5000;

// runs stepper motor for given duration t in ms
void feed(unsigned long dur){
  StepForwardDefault(dur);
}

void lcd_disp() {
 static String feed_times_str = "";
 for (int i=0; i < sizeof(feed_times); i++) {
   String str(feed_times[i]);
   feed_times_str  = feed_times_str + " " + str.substring(0,2);
 }

 lcd.setCursor(0,0);
 lcd.print("TIME: ");
 lcd.print(rtc.getTimeStr());
 lcd.setCursor(0,1);
 lcd.print(feed_times_str);
 feed_times_str = "";
 //lcd.print(rtc.getDateStr());
}

bool check_time() {
  String curr_time = rtc.getTimeStr();
  bool ret = false;
  for (int j=0; j < sizeof(feed_times); j++) {
    if (curr_time == feed_times[j]) {
       ret = true;
    }
  }
  return ret;
}

//Reset Easy Driver pins to default states
void resetEDPins()
{
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(EN, HIGH);
}

//Default microstep mode function
void StepForwardDefault(unsigned long dur)
{
  starttime = millis();
  endtime = starttime;
  digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
  while ((endtime - starttime) <= dur) // do this loop for up to feed_dur (ms)
    {
      digitalWrite(stp,HIGH); //Trigger one step forward
      delay(1);    
      digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
      delay(1); 
      endtime = millis();
    }
}

//Reverse default microstep mode function
void ReverseStepDefault(unsigned long dur)
{
  starttime = millis();
  endtime = starttime;
  digitalWrite(dir, HIGH); //Pull direction pin low to move "reverse"
  while ((endtime - starttime) <= dur) // do this loop for up to feed_dur (ms)
    {
      digitalWrite(stp,HIGH); //Trigger one step forward
      delay(1);    
      digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
      delay(1); 
      endtime = millis();
    }
}

// 1/8th microstep foward mode function
void SmallStepMode(unsigned long dur)
{
  starttime = millis();
  endtime = starttime;
  digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
  digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
  digitalWrite(MS2, HIGH);
  while ((endtime - starttime) <= dur) // do this loop for up to feed_dur (ms)
    {
      digitalWrite(stp,HIGH); //Trigger one step forward
      delay(1);    
      digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
      delay(1); 
      endtime = millis();
    }
}

// qsort requires you to create a sort function
int sort_asc(const void *cmp1, const void *cmp2)
{
  // Need to cast the void * to char *
  char a = *((char *)cmp1);
  char b = *((char *)cmp2);
  // A simpler, probably faster way:
  return a - b;
}

void setup() { 
 rtc.begin(); // Initialize the rtc object
 //rtc.setTime(11,22,00); //set time U
 lcd.init();  //initialize the lcd
 lcd.backlight();  //open the backlight
 
 // for stepper motor control
 pinMode(stp, OUTPUT);
 pinMode(dir, OUTPUT);
 pinMode(MS1, OUTPUT);
 pinMode(MS2, OUTPUT);
 pinMode(EN, OUTPUT);
 resetEDPins(); //Set step, direction, microstep and enable pins to default states
 digitalWrite(EN, LOW); //Pull enable pin low to allow motor control

 qsort(feed_times, fd_times_length, sizeof(feed_times[0]), sort_asc);
}


void loop() {
  // put your main code here, to run repeatedly:
  lcd_disp();
//  if (check_time() == true) {
//    feed(feed_dur);
//    delay(1000); //delay 1000 ms
//  } 
  feed(feed_dur);
  delay(10000);
}
