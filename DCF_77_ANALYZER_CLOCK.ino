/*
  This sketch is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This sketch is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 
 :() :() :() :() :() :() :() :() :() :() :() :() :() :() :() :() :() :() :() :() :() :() :() :() :() :() :()
 
 This is a sketch, aimed at people who are just beginning to discover the wonderful DCF77 / Radio Clock world.
 The C++ code is far from optimized because I myself am an Arduino and C++ newbie! ;)
 But in writing this code and learning a LOT from others, I gradually comprehended the inner workings of
 DCF and C++.
 
 If you enjoy exploring the Arduino and DCF77 as much as I did, maybe by using and/or learning from this
 sketch, my goal is reached. :o)
 
 Erik de Ruiter
 2014
 
 Edit:
 2014-12-01 changed code to accomodate Adafruit Audio FX Sound Board for Chime sound
 
 
 CREDITS:
 I learned a lot from the work of Matthias Dalheimer and Thijs Elenbaas who made their own DCF77 decoders.
 Without their work I would not have known where to start.
 I ended up writing my own code (using bits and pieces of their ideas) so I could understand what is happening...
 My code is far, very far from efficient or advanced but it does work and I know what is going on.
 
 */

//----------------------------------------------------------------------------------------------------------
// Libraries
//----------------------------------------------------------------------------------------------------------

// Arduino (new) Time library .................................... http://www.pjrc.com/teensy/td_libs_Time.html
#include <Time.h>
// Enable this line if using Arduino Uno, Mega, etc.
#include <Wire.h>
// Maxim 7219 displays library ................................... http://playground.arduino.cc/Main/LEDMatrix
#include <DS1307RTC.h>
// a basic DS1307 library that returns time as a time_t .......... http://www.pjrc.com/teensy/td_libs_DS1307RTC.html
#include <LedControl.h>
//SPI interface library .......................................... http://arduino.cc/en/Reference/SPI
#include <SPI.h>
//STREAMING.h .................................................... http://arduiniana.org/libraries/streaming/
/*
New users sometimes wonder why the “Arduino language” doesn’t provide the kind
 of concatenation or streaming operations they have become accustomed to in Java/VB/C#/C++, etc.
 lcd << "GPS #" << gpsno << " date: " << day << "-" << month << "-" << year << endl;
 This library works for any class that derives from Print:
 Serial << "Counter: " << counter;
 lcd << "Temp: " << t.get_temperature() << " degrees";
 my_pstring << "Hi Mom!" << endl;
 With the new library you can also use formatting manipulators like this:
 Serial << "Byte value: " << _HEX(b) << endl;
 lcd << "The key pressed was " << _BYTE(c) << endl;
 This syntax is familiar to many, is easy to read and learn, and, importantly, *** consumes no resources ***
 (Because the operator functions are essentially just inline aliases for their print() counterparts,
 no sketch gets larger or consumes more RAM as a result of their inclusion.)
 */
#include <Streaming.h>
// OneWire lets you access 1-wire devices made by Maxim/Dallas,
// such as DS18S20, DS18B20, DS1822 .............................. http://www.pjrc.com/teensy/td_libs_OneWire.html
// The DallasTemperature library can do all this work for you! ... http://milesburton.com/Dallas_Temperature_Control_Library
#include <OneWire.h>

//----------------------------------------------------------------------------------------------------------
// DS18B20 initialization
//----------------------------------------------------------------------------------------------------------
OneWire  ds(8); // define Onewire instance DS on pin 8

//----------------------------------------------------------------------------------------------------------
// Maxim 7219 Matrix Display initialization
//----------------------------------------------------------------------------------------------------------
/*
  clearDisplay(int addr) ........................................ clears the selected display
 lc.shutdown(int addr, boolean) ................................ wake up the MAX72XX from power-saving mode (true = sleep, false = awake)
 lc.setIntensity(int addr, value) .............................. set a medium brightness for the Leds (0=min - 15=max)
 lc.setLed(int addr, int row, int col, boolean state) .......... switch on the led in row, column. remember that indices start at 0!
 lc.setRow(int addr, int row, byte value) ...................... this function takes 3 arguments. example: lc.setRow(0,2,B10110000);
 lc.setColumn(int addr, int col, byte value) ................... this function takes 3 arguments. example: lc.setColumn(0,5,B00001111);
 lc.setDigit(int addr, int digit, byte value, boolean dp) ....... this function takes an argument of type byte and prints the corresponding digit on the specified column.
 The range of valid values runs from 0..15. All values between 0..9 are printed as digits,
 values between 10..15 are printed as their hexadecimal equivalent
 lc.setChar(int addr, int digit, char value, boolean dp) ....... will display: 0 1 2 3 4 5 6 7 8 9 A B C D E F H L P; - . , _ <SPACE> (the blank or space char)
 
 pin 12/7 is connected to the DataIn
 pin 11/6 is connected to the CLK
 pin 10/5 is connected to CS/LOAD
 
 ***** Please set the number of devices you have *****
 But the maximum default of 8 MAX72XX wil also work.
 LedConrol(DATAIN, CLOCK, CS/LOAD, NUMBER OF MAXIM CHIPS)
 */

// lc is for the Maxim Common CATHODE displays
LedControl lc = LedControl(12, 11, 10, 8, false); // Define pins for Maxim 72xx and how many 72xx we use
// lc1 is for the Maxim Common ANODE displays
LedControl lc1 = LedControl(7, 6, 5, 1, true); // Define pins for Maxim 72xx and how many 72xx we use


//----------------------------------------------------------------------------------------------------------
// Variable and array defenitions
//----------------------------------------------------------------------------------------------------------
// Define DCF routine parameters
#define DCFRejectionTime     700      // Pulse-to-Pulse rejection time. 
#define DCFRejectPulseWidth  50       // Minimal pulse width
#define DCFSplitTime         180      // distinguishes pulse width 100 ms and 200 ms. In practice we see 130 ms and 230
#define DCFSyncTime          1600     // defines 2000 ms pulse for end of sequence

// define Arduino Pins
#define BUZZER         A1             // OUTPUT: Piezo buzzer on Analog port
#define CHIMESWITCHPIN A2             // INPUT: Chime on/off switch
#define CHIMEPIN       A3             // OUTPUT: Chime Activate     LOW=CHIME ON
#define DCF_INTERRUPT   0             // Interrupt number associated with pin
#define SPEAKERVOLPIN  13             // After power on, set the speaker volume of the Adafruit Adio Board via this pin
#define DCF77PIN        2             // INPUT: Connection pin to DCF 77 device. Must be pin 2 or 3!
#define NOTUSEDPIN      3             // INPUT: not used at this moment
#define DCF77SOUNDPIN   4             // INPUT: Switch DCF77 Sound ON/OFF
#define TEMPRESETPIN    9             // INPUT: Push button to reset min/max temp memory


// define miscellaneous parameters
#define DEBUG_FLOW          0         // 1 = show info about functions calls 
#define DS1307_I2C_ADDRESS  0x68      // define the RTC I2C address

// definition of Maxim 7219 display number sequence
// first Maxim 7219 in 'daisychain' must be '0', next '1' etc.
// COMMON CATHODE DISPLAYS
#define MaximLedRingInner   0
#define MaximLedRingOuter   1
#define MaximPeriodPulse    2
#define MaximBufferBitError 3
#define MaximWeek           4
#define MaximDate           5
#define MaximRtcTime        6
#define MaximLeds           7

// definition of Maxim 7219 display number sequence
// first Maxim 7219 in 'daisychain' must be '0', next '1' etc.
// COMMON ANODE DISPLAYS
#define MaximDcfTime       0

// definition of display brighness levels
#define BrightnessMaximLedRingOuter     1
#define BrightnessMaximLedRingInner     1
#define BrightnessMaximPeriodPulse      2
#define BrightnessMaximWeek             3
#define BrightnessMaximDate             9
#define BrightnessMaximRtcTime          1
#define BrightnessMaximLeds             6
#define BrightnessMaximDcfTime          4
#define BrightnessMaximBufferBitError  15

// After power on, set the speaker volume of the Adafruit Adio Board via this pin
#define SPEAKERVOLUME       12

// definition of Status/Error Led's. Use division and modulo to seperate Row/Col values!!
// to lit a LED you need a ROW and COLUMN value.
// so, for example: BFLed/10 results in the value '1' (row)
// and BFLed%10 results in the value '3' (column)

/* Row/Col LED numbers of Maxim 7219 chip
 Row  Col
 Sunday      0    0
 Monday      0    1
 Tuesday     0    2
 Wednesday   0    3
 Thursday    0    4
 Friday      0    5
 Saturday    0    6
 Synced      0    7
 RTCError    1    0
 SummerTime  1    1
 WinterTime  1    2
 LeapYear    1    3
 BF          1    4
 EoM         1    5
 EoB         1    6
 rPW         1    7
 rPT         2    0
 DCFOk       2    1
 Chime       2    2
 
 */
#define SyncedLed      07
#define RTCError       10
#define SummerTimeLed  11
#define WinterTimeLed  12
#define LeapYearLed    13
#define BFLed          14
#define EoMLed         15
#define EoBLed         16
#define rPWLed         17
#define rPTLed         20
#define DCFOKLed       21
#define ChimeLed       22


// Pulse flanks
static   int leadingEdge         = 0;
static   int trailingEdge        = 0;
static   int previousLeadingEdge = 0;

// used in <Int0handler>
volatile unsigned char DCFSignalState = 0;

// used in <loop>
int ss              = 0;
int previousSecond  = 0;
int previousSecond2 = 0;
unsigned char previousSignalState;

// DCF Buffers and indicators
static int DCFbitBuffer[59]; // here, the received DCFbits are stored
const int bitValue[] = {
  1, 2, 4, 8, 10, 20, 40, 80}; // these are the decimal values of the received DCFbits

/* not used at this moment
 // Inserted a 'null' value  ("") because an array starts with position zero '0' and I need it to start at '1'
 const char dayNameLong[][10] = {"", "Maandag", "Dinsdag", "Woensdag", "Donderdag", "Vrijdag", "Zaterdag", "Zondag"};
 const char dayNameShort[][3] = {"", "Ma", "Di", "Wo", "Do", "Vr", "Za", "Zo"};
 const char monthNameLong [][10] = {"", "Januari", "Februari", "Maart", "April", "Mei", "Juni", "Juli", "Augustus", "September", "Oktober", "November", "December"};
 const char monthNameShort [][4] = {"", "Jan", "Feb", "Mrt", "Apr", "Mei", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dec"};
 */

static int bufferPosition = 0;
static int endOfMinute    = 0;
static int previousMinute = 0;
static int previousHour   = 0;

// used to check if switch state is changed
int buttonChimeToggle = 0;
int buttonDCF77SoundToggle = 0;
int buttonDisplayOffToggle = 0;

// dcf variables to store decoded DCF time in
int dcfMinute  = 0;
int dcfHour    = 0;
int dcfDay     = 0;
int dcfWeekDay = 0;
int dcfMonth   = 0;
int dcfYear    = 0;
int dcfDST     = 0;
int leapYear   = 0;

// variables used to store weeknumber and daynumer values
int dayNumber;   //Returns the number of day in the year
int weekNumber;   //Returns the number of the week in the year

// error counter variable
int errorCounter = 0;

// miscelleanous variables
boolean daytimeChange    = 1;
boolean dayTime          = 0;        
boolean chimeSwitch      = 0;
boolean dcf77SoundSwitch = 0;

// temperature variables
byte present            = 0;
byte DS18B20Data[12];
byte addr[8]            = {0x28, 0x13, 0x95, 0x7B, 0x05, 0x00, 0x00, 0x70};
int maxTemp             = 0;
int minTemp             = 0;
int lsByte              = 0;
int msByte              = 0;
int tempReading         = 0;
int tempCelcius         = 0;
boolean tempResetButton = 0;


//==============================================================================
// SETUP
//==============================================================================
void setup()
{
  // initialize Serial communication
  Serial.begin(115200);

  pinMode(DCF77PIN, INPUT);            // define DCF77PIN as input
  pinMode(NOTUSEDPIN, INPUT);          // define pin to shut off display as input
  pinMode(TEMPRESETPIN, INPUT);        // define pin to reset High / Low temperature memory
  pinMode(DCF77SOUNDPIN, INPUT);       // define pin for switch to shut off second beep sound
  pinMode(CHIMESWITCHPIN, OUTPUT);     // output to activate hourly chime sound
  pinMode(CHIMEPIN, OUTPUT);           // output to activate ticking sound
  pinMode(SPEAKERVOLPIN, OUTPUT);      // output to set LOWER speaker volume on startup
  digitalWrite(SPEAKERVOLPIN, LOW);
 
  // Initialize all variables and LED displays
  initialize();
  // Initialize DCF77 signal listener interrupt
  attachInterrupt(DCF_INTERRUPT, int0handler, CHANGE);

  // Initialize RTC and set as SyncProvider.
  // Later RTC will be synced with DCF time
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  // check if RTC has set the system time
  if (timeStatus() != timeSet)
  { // Unable to sync with the RTC - activate RTCError LED
    lc.setLed(MaximLeds, RTCError / 10, RTCError % 10, true);
  } 
  else {
    // RTC has set the system time - dim RTCError LED
    lc.setLed(MaximLeds, RTCError / 10, RTCError % 10, false);
  }

  // After power on, set the speaker volume of the Adafruit Adio Board
  for(int i=0; i<=SPEAKERVOLUME; i++)
  {
      digitalWrite(SPEAKERVOLPIN, HIGH);
      delay(100);
      digitalWrite(SPEAKERVOLPIN, LOW);
      delay(100);
  }

  // use for test purposes
  //setTime(23, 59, 40, 31, 12, 13);
  //RTC.set(now());

}


//==============================================================================
// LOOP
//==============================================================================
void loop()
{
  if (DEBUG_FLOW) Serial.println(" <Loop> ");

  // check if switches are changed and act upon it
  checkSwitches();


  // check if time is changed
  if (second() != previousSecond)
  {
    //display -Real Time Clock- Time
    displayRtcTime();

    // display 'HI' and 'LO' temperature on specific moments
    switch (second())
    {
    case 5:
      // hourly chime OFF
      digitalWrite(CHIMEPIN, HIGH);
      break;
    case 30:
      // display 'HI' on display for Hi temperature
      lc1.clearDisplay(MaximDcfTime); // clear display
      lc1.setChar(MaximDcfTime, 3, 'H', false); // Display 'H' (Celcius)  character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
      lc1.setChar(MaximDcfTime, 2, '1', false); // Display 'I' (Celcius)  character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
      break;
    case 31:
    case 32:
      // display Max temperature on DCF display LED display
      lc1.setChar(MaximDcfTime, 3, (maxTemp / 1000), false);
      lc1.setChar(MaximDcfTime, 2, (maxTemp % 1000) / 100, true);
      lc1.setChar(MaximDcfTime, 1, (maxTemp % 100) / 10, false);
      lc1.setRow(MaximDcfTime, 0, B01001110); // Display 'C' (Celcius)  character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
      lc1.setChar(MaximDcfTime, 5, 1, false); // activate top dot
      break;
    case 33:
      // display 'LO' on display for Low temperature
      lc1.clearDisplay(MaximDcfTime); // clear display
      lc1.setChar(MaximDcfTime, 3, 'L', false); // Display 'L' (Celcius)  character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
      lc1.setChar(MaximDcfTime, 2, '0', false); // Display 'O' (Celcius)  character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
      break;
    case 34:
    case 35:
      // display Min temperature on DCF display LED display
      lc1.setChar(MaximDcfTime, 3, (minTemp / 1000), false);
      lc1.setChar(MaximDcfTime, 2, (minTemp % 1000) / 100, true);
      lc1.setChar(MaximDcfTime, 1, (minTemp % 100) / 10, false);
      lc1.setRow(MaximDcfTime, 0, B01001110); // Display 'C' (Celcius)  character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
      lc1.setChar(MaximDcfTime, 5, 1, false); // activate top dot
      break;
    case 36:
      // display 'CU' on display for Current temperature
      lc1.clearDisplay(MaximDcfTime); // clear display
      lc1.setColumn(MaximDcfTime, 3, B01001110); // Display 'C' (Celcius)  character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
      lc1.setColumn(MaximDcfTime, 2, B00111110); // Display 'O' (Celcius)  character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
      break;
    case 58:
      // hourly chime ACTIVATE
      if(chimeSwitch == 1 && dayTime == 1 && minute() == 59) digitalWrite(CHIMEPIN, LOW);
      break;
    default:
      // display current temperature
      displayTemp();
      break;
    } //switch
    previousSecond = second();
  } //(second() != previousSecond)



  // call scanSignal if interrupt is triggered by incoming DCF77 pulse
  if (DCFSignalState != previousSignalState)
  {
    scanSignal();

    previousSignalState = DCFSignalState;
  } //if (DCFSignalState != previousSignalState)




  // display date, week LED's, week nr etc. if time is changed
  if (minute() != previousMinute)
  {
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //           DISPLAY SHUTS DOWN FROM 8 PM UNTIL 8 AM
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // check wether it is Day- or Nighttime
    if (hour() >= 8 && hour() < 21)
    {
      // it's DAYTIME so time to activate displays
      if (daytimeChange == 1) // test variable because daytime routine is needed only once
      {
        // activate all the displays and status LED's
        for (int i = 0; i < 8; i++)
        {
          lc.shutdown(i, false);  // Common Cathode displays wakeup
        } //for
        // Maxim Common Anode
        lc1.shutdown(0, false);  // Common Anode display wake up
      } //if (daytimeChange == 0)

      daytimeChange = 0; // set variable so daytime routine is performed only once
      dayTime = 1;
      displayData(); // display date, week LED's, week nr etc.

    } //if (hour() >= 8 && hour() < 21)

    else

    {
      // it's NIGHTTIME so time to deactivate displays
      if (daytimeChange == 0) // test variable because nighttime routine is needed only once
      {
        // deactivate all the displays and status LED's
        for (int i = 0; i < 8; i++)
        {
          lc.shutdown(i, true);  // Common Cathode displays wakeup
        } //for
        // Maxim Common Anode
        lc1.shutdown(0, true);  // Common Anode display wake up
      } //if (daytimeChange == 0)

      dayTime = 0;
      daytimeChange = 1; // set variable so nighttime routine is performed only once

    } //else

    previousMinute = minute();

  } //if (minute() != previousMinute)


  // reset errorCounter display every hour
  if (dcfHour != previousHour)
  {
    errorCounter = 0;
    ledDisplay(MaximBufferBitError, "R", 0);
    previousHour = dcfHour;
  }


} //loop


//================================================================================================================
//================================================================================================================
//================================================================================================================
//================================================================================================================
//================================================================================================================


//================================================================================================================
//
// scanSignal
//
// called from <loop>
//================================================================================================================
/*
       pulse                 pulse
       width                 width
       |- -|               |--   --|           |----- END OF MINUTE marker:2000ms -----|
        ___                 _______             ___                                     ___                 _______
       | 0 |               |   1   |           | 0 |                                   | 0 |               |   1   |
       |   |               |       |           |   |                                   |   |               |       |
       |   |               |       |           |   |                                   |   |               |       |
 ______|   |_______________|       |___________|   |___________________________________|   |_______________|       |__ _ _ _
       ^   ^               ^       ^           ^   ^               ^                   ^   ^               ^       ^
       1000 2100           2000     2200       3000 3100         NO PULSE              5000 5100           6000     6200     << example millis() value
                                                                 = end of Minute indication
       ^                   ^                   ^                                       ^                   ^
       DCFbit# 56          DCFbit# 57          DCFbit# 58                               DCFbit# 0           DCFbit# 1  etc...   << DCF bit received
       
       ^                   ^        ^
       previous             leading  trailing
       leading edge         edge     edge
       
       ^
       flanktime
 */



// Evaluates the signal as it is received. Decides whether we received a "1" or a "0"

void scanSignal()
{

  if (DEBUG_FLOW) Serial.print(" <scanSignal> ");

  // store time of received pulse
  int flankTime = millis();


  ///////////////////////////////////////////////////////////////
  // Check for flank UP and perform checks on received DCF signal
  if (DCFSignalState == 1) {

    leadingEdge = flankTime; // store flankTime to check validity of received period

    // If this flank UP is detected quickly after previous flank up this
    // will be an incorrect period lenght that we shall reject
    if ((leadingEdge - previousLeadingEdge) < DCFRejectionTime) // DCFRejectionTime = 700, should be 1000ms
    {
      // rPT - ERROR: FLANK TO FLANK TIME IS TO SHORT -> REJECTED
      error(rPTLed);
      return;
    } // if ((leadingEdge - previousLeadingEdge) < DCFRejectionTime)



    // No further action is taken because pulse information is not complete yet, only after receiving DOWN flank
    // we will know if the pulse is a '0' or '1' and we can do several checks.
    return;

  } // if (DCFSignalState == 1)


  ///////////////////////////////////////////////////////////
  // Check for DOWN and perform checks on received DCF signal
  if (DCFSignalState == 0) {


    trailingEdge = flankTime; // store flankTime to calculate pulsewidth.


    // If the detected pulse is too short it will be an incorrect pulse that we shall reject as well
    if ((trailingEdge - leadingEdge) < DCFRejectPulseWidth) // DCFRejectPulseWidth = 50. should be 100 and 200 ms ideally
    {
      //rPW - ERROR: DETECTED PULSE IS TO SHORT -> REJECTED
      error(rPWLed);
      return;
    } // if ((trailingEdge - leadingEdge) < DCFRejectPulseWidth)

    // display pulse width on LED display
    ledDisplay(MaximPeriodPulse, "R", (trailingEdge - leadingEdge));
    // display period width on LED display
    ledDisplay(MaximPeriodPulse, "L", (leadingEdge - previousLeadingEdge));

    // END OF MINUTE check
    //if (bufferPosition > 0 && (leadingEdge - previousLeadingEdge) > DCFSyncTime) // DCFSyncTime is 1600ms
    if ((leadingEdge - previousLeadingEdge) > DCFSyncTime) // DCFSyncTime is 1600ms
    {
      // end of minute detected:
      finalizeBuffer(); //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    } // if


    // refresh previousLeadingEdge time
    previousLeadingEdge = leadingEdge;


    // PROCESS RECEIVED DCFbit, distinguish between long and short pulses
    if (trailingEdge - leadingEdge < DCFSplitTime) // DCFSplitTime == 180
    {
      appendSignal(0); //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
      if (dcf77SoundSwitch == 1) buzzer(100);
    }  //if
    else
    {
      appendSignal(1); //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
      if (dcf77SoundSwitch == 1) buzzer(200);
    } //else
  } // if (DCFSignalState == 0)
} // void scanSignal();



//================================================================================================================
//
// appendSignal
//
// called from <scanSignal>
//================================================================================================================


/**
 * Add new bit to buffer
 */
void appendSignal(unsigned char signal)
{
  if (DEBUG_FLOW) Serial.print(" <appendSignal> ");

  // display bufferPosition on LED display
  lc.setChar(MaximBufferBitError, 7, bufferPosition / 10, false);
  lc.setChar(MaximBufferBitError, 6, bufferPosition % 10, false);
  // display received DCFbit on LED Display
  lc.setChar(MaximBufferBitError, 4, signal, false);

  // Fill array with DCFbit signal
  DCFbitBuffer[bufferPosition] = signal;

  // display received bits on 1st Matrix displays
  lc.setLed(MaximLedRingInner, bufferPosition / 8, bufferPosition % 8, signal);

  // increment bufferposition counter
  bufferPosition++;


  if (bufferPosition > 1)
  {
    // clear error messages on the screen. clear at second '1' to leave them a while the display...
    lc.setLed(MaximLeds, BFLed / 10, BFLed % 10, false);
    lc.setLed(MaximLeds, EoMLed / 10, EoMLed % 10, false);
    lc.setLed(MaximLeds, EoBLed / 10, EoBLed % 10, false);
    lc.setLed(MaximLeds, rPWLed / 10, rPWLed % 10, false);
    lc.setLed(MaximLeds, rPTLed / 10, rPTLed % 10, false);

  }

  if (bufferPosition == 6)
  {
    // clear time status messages

  }

  if (bufferPosition == 56)
  {
    // warn for coming minute transition
    //buzzer(100);
  }

  if (bufferPosition > 59)
  {
    // EoB ERROR - clear both DCFbit displays because buffer is full before at end of time-sequence
    lc.clearDisplay(MaximLedRingInner); // clear display
    lc.clearDisplay(MaximLedRingOuter); // clear display
    lc.clearDisplay(MaximDcfTime); // clear display

    // EoB - BUFFER IS FULL BEFORE END OF TIME-SEQUENCE, THIS MAY BE DUE TO NOISE GIVING ADDITIONAL PEAKS
    error(EoBLed);
    return;

  } // if (bufferPosition > 59)

  // buzzer sound with duration depending on DCFsignal
  //signal == 0 ? buzzer(100) : buzzer(200);

} // void


//================================================================================================================
//
// finalizeBuffer
//
//================================================================================================================

/**
 * Finalize filled buffer
 */
void finalizeBuffer(void) {

  if (DEBUG_FLOW) Serial.print(" <finalizeBuffer> ");

  if (bufferPosition == 59)
  {

    // BUFFER FULL

    // reset RTC seconds to '0'

    // display BF on TFT screen
    lc.setLed(MaximLeds, BFLed / 10, BFLed % 10, true);

    // display DCF 'OK' status TRUE
    lc.setLed(MaximLeds, DCFOKLed / 10, DCFOKLed % 10, true);


    // clear DCFbit led ring inner
    lc.clearDisplay(MaximLedRingInner);
    //copy minute information to DCFbt led ring outer
    for (int i = 0; i < 59; i++) {
      lc.setLed(MaximLedRingOuter, i / 8, i % 8, DCFbitBuffer[i]);
    } // for


    // process buffer and extract data
    processBuffer();


    // Reset running buffer
    bufferPosition   = 0;
    // Reset DCFbitBuffer array, positions 0-58 (=59 bits)
    for (int i = 0; i < 59; i++) {
      DCFbitBuffer[i] = 0;
    } // for


  } // if (bufferPosition == 59)
  else
  {

    // BUFFER NOT YET FULL at end of time-sequence
    lc.setLed(MaximLeds, EoMLed / 10, EoMLed % 10, true);

    // CLEAR DISPLAYS
    // clear both DCFbit displays because Buffer is not yet full at end of time-sequence
    lc.clearDisplay(MaximLedRingInner);
    lc.clearDisplay(MaximLedRingOuter);

    // Reset running buffer
    bufferPosition   = 0;
    // Reset DCFbitBuffer array, positions 0-58 (=59 bits)
    for (int i = 0; i < 59; i++) {
      DCFbitBuffer[i] = 0;
    }


  } // else
} // void finalizeBuffer(void)




//================================================================================================================
//
// processBuffer
//
// called from <finalizeBuffer>
//================================================================================================================

/**
 * Evaluates the information stored in the buffer. This is where the DCF77
 * signal is decoded
 */
void processBuffer(void) {

  if (DEBUG_FLOW) Serial.print(" <processBuffer> ");

  // Buffer is full and ready to be decoded
  dcfMinute  = bitDecode(21, 27);
  dcfHour    = bitDecode(29, 34);
  dcfDay     = bitDecode(36, 41);
  dcfWeekDay = bitDecode(42, 44);
  dcfMonth   = bitDecode(45, 49);
  dcfYear    = bitDecode(50, 57);

  //call function to calculate day of year and weeknumber
  dayWeekNumber(dcfYear, dcfMonth, dcfDay, dcfWeekDay);

  //Determine Summer- or Witertime
  dcfDST     = bitDecode(17, 17);

  // determine Leap Year
  leapYear   = calculateLeapYear(dcfYear);

  // set Arduino time and after that set RTC time
  setTime(dcfHour, dcfMinute, 0, dcfDay, dcfMonth, dcfYear);
  RTC.set(now());

  // activate Synced LED
  lc.setLed(MaximLeds, SyncedLed / 10, SyncedLed % 10, true);



} // return to finalizeBuffer()

//================================================================================================================
//
// bitDecode
//
// called from <processBuffer>
//================================================================================================================


int bitDecode (int bitStart, int bitEnd) {

  if (DEBUG_FLOW) Serial.print(" <bitDecode> ");

  // reset 'bitValue-array' counter
  int i = 0;
  int value = 0;

  // process bitrange bitStart > bitEnd
  while (bitStart <= bitEnd)
  {
    // check if DCFbit in buffer is '1', discard when '0'
    if (DCFbitBuffer[bitStart] == 1) {
      // DCFbit in buffer == 1 so append its corresponding value to the variable 'value'
      value = value + bitValue[i];
    }
    // increment 'bitValue-array' counter
    i++;
    // increment bit-range counter
    bitStart++;
  }
  return value;
}





//================================================================================================================
//
// buzzer
//
//================================================================================================================

void buzzer(int duration)
{
  tone(BUZZER, 1500, duration);
}

//================================================================================================================
//
// initialize
//
// calles from <setup>
//================================================================================================================

/**
 * Initialize parameters
 */
void initialize(void)
{
  if (DEBUG_FLOW) Serial.print(" <initialize> ");

  //---------------------------------------------------
  // Initialize Variables
  //---------------------------------------------------
  leadingEdge           = 0;
  trailingEdge          = 0;
  previousLeadingEdge   = 0;
  bufferPosition        = 0;

  digitalWrite(CHIMEPIN, HIGH); // Set Chimepin to default state

  // Reset DCFbitBuffer array, positions 0-58 (=59 bits)
  for (int i = 0; i < 59; i++) {
    DCFbitBuffer[i] = 0;
  }


  //---------------------------------------------------
  // Initialize Maxim 72xx IC's
  //---------------------------------------------------
  // Maxim Common Cathode
  for (int i = 0; i < 8; i++)
  {
    lc.shutdown(i, true);  // display shut down
    delay(100);
    lc.shutdown(i, false);  // display wake up
    lc.clearDisplay(i);     // clear display
  }
  // Maxim Common Anode
  lc1.shutdown(0, true);  // display shut down
  delay(100);
  lc1.shutdown(0, false);  // display wake up
  lc1.clearDisplay(0);     // clear display

  // Maxim Common Cathode
  lc.setIntensity(MaximLedRingOuter, BrightnessMaximLedRingOuter);
  lc.setIntensity(MaximLedRingInner, BrightnessMaximLedRingInner);
  lc.setIntensity(MaximPeriodPulse, BrightnessMaximPeriodPulse);
  lc.setIntensity(MaximWeek, BrightnessMaximWeek);
  lc.setIntensity(MaximDate, BrightnessMaximDate);
  lc.setIntensity(MaximRtcTime, BrightnessMaximRtcTime);
  lc.setIntensity(MaximLeds, BrightnessMaximLeds);
  lc.setIntensity(MaximBufferBitError, BrightnessMaximBufferBitError);

  // Maxim Common Anode
  lc1.setIntensity(MaximDcfTime, BrightnessMaximDcfTime);


  //---------------------------------------------------
  // Initialize Displays
  //---------------------------------------------------

  // set errorCounter display to '0'
  ledDisplay(MaximBufferBitError, "R", 0);


}


//================================================================================================================
//
// int0handler
//
//================================================================================================================

/**
 * Interrupt handler that processes up-down flanks into pulses and stores these in the buffer
 */
void int0handler()
{
  if (DEBUG_FLOW) Serial.print(" <int0handler ");
  DCFSignalState = digitalRead(DCF77PIN);
} // int0handler()




//================================================================================================================
//
// displayBuffer
//
//================================================================================================================
void displayBuffer(String message)
{
  Serial << endl << message << endl;
  Serial << "01234567890123456789012345678901234567890123456789012345678" << endl;
  Serial << "     |    1    |    2    |    3    |    4    |    5    |   " << endl;
  Serial << "                         124     12     12       1    1248 " << endl;
  Serial << "                     1248000 124800 1248001241248012480000 " << endl;
  Serial << "                 ZW  mmmmmmmPhhhhhhPDDDDDDwwwMMMMMYYYYYYYYP" << endl;
  // display DCFbit buffer contents
  for (int i = 0; i < 59; i++) {
    Serial << DCFbitBuffer [i];
  }
  Serial << endl;
}


//================================================================================================================
//
// ledDisplay on Maxim 8 digit displays
//
// called from processBuffer
//================================================================================================================
void ledDisplay(int addr, String leftOrRight, int value)
{

  int ones;
  int tens;
  int hundreds;
  int thousands;
  int shift;

  //break down value in seperate digits for ones, tens, etc
  int v = value; // 'value' is needed later so copy it in 'v'
  ones = v % 10;
  v = v / 10;
  tens = v % 10;
  v = v / 10;
  hundreds = v % 10;
  thousands = v / 10;

  //Select which side of the 8 digit display to be used
  (leftOrRight == "L" ? shift = 4 : shift = 0);

  //Now print the number digit by digit
  //preceding zero's are removed with tenary operator by 'printing' a space character.
  lc.setChar(addr, 3 + shift, ((value < 1000) ? ' ' : thousands), false);
  lc.setChar(addr, 2 + shift, ((value < 100) ? ' ' : hundreds), false);
  lc.setChar(addr, 1 + shift, ((value < 10) ? ' ' : tens), false);
  lc.setChar(addr, 0 + shift, ones, false);
}

//================================================================================================================
//
// checkSwitches
//
// called from scanSignal
//================================================================================================================

void checkSwitches(void)
{


  // read stat of temp rest push button
  tempResetButton = digitalRead(TEMPRESETPIN);
  // reset temperature min/max values when push-button is pressed
  if (tempResetButton == 1)
  {
    maxTemp = 0;
    minTemp = 0;
  }



  //read state of switch CHIME
  chimeSwitch = digitalRead(CHIMESWITCHPIN);

  if (chimeSwitch == 1)
  {
    lc.setLed(MaximLeds, ChimeLed / 10, ChimeLed % 10, true);
  } // if
  else
  {
    lc.setLed(MaximLeds, ChimeLed / 10, ChimeLed % 10, false);
  } // else




    //read state of switch DCF77SOUNDPIN
  int buttonDCF77Sound = digitalRead(DCF77SOUNDPIN);

  if (buttonDCF77Sound == 1)
  {
    dcf77SoundSwitch = 1;
  } // if
  else
  {
    dcf77SoundSwitch = 0;
  } // else


} // void checkButtons(void)


//================================================================================================================
//
// error
//
//================================================================================================================

// errorLed parameter is LED to be switched on
int error(int errorLed)
{
  lc.setLed(MaximLeds, errorLed / 10, errorLed % 10, true);

  // clear Led's/displays because of error condition
  lc.setLed(MaximLeds, DCFOKLed / 10, DCFOKLed % 10, false);
  lc.setLed(MaximLeds, SyncedLed / 10, SyncedLed % 10, false);
  lc.clearDisplay(MaximLedRingOuter); // clear display

  // increase errorCounter and display errorCount
  errorCounter++;
  ledDisplay(MaximBufferBitError, "R", errorCounter);


}

//================================================================================================================
//
// dayWeekNumber
//
//================================================================================================================
/*
Code from: http://forum.arduino.cc/index.php/topic,44476.0.html
 
 you need to declare two global variables:
 Code:
 short dayNumber;   //Returns the number of day in the year
 short weekNumber;  //Returns the number of the week in the year
 
 and this is how you need to call the function in your loop:
 Code:
 dayWeekNumber(year(),month(),day(),weekday());
 
 Take into account that i use TIME library in my sketch, so how you call to year, month, day and day of week could change for other libraries.
 
 and this is an example of how to call the function and show the results:
 Code:
 dayWeekNumber(year(),month(),day(),weekday());
 Serial.print("Day #");
 Serial.print(dayNumber);
 Serial.print(" at week # ");
 Serial.print(weekNumber);
 Serial.print(" of ");
 Serial.println(year());
 
 Be aware than in the function, there is a problematic line:
 Code:
 WN = (DN-7+10)/7;
 This is because in the library the week starts on Sunday (0), however, in the ISO standard,
 week start on monday (1), and sunday is the last day (7). For that reason, i included this especial case in the code.
 */
int dayWeekNumber(int y, int m, int d, int w)
{
  int days[] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334            }; // Number of days at the beginning of the month in a (not leap) year.
  //Start to calculate the number of day
  if (m == 1 || m == 2) {
    //for any type of year, it calculate the number of days for January or february
    dayNumber = days[(m - 1)] + d;
  } // if
  // Now, try to calculate for the other months
  else if ((y % 4 == 0 && y % 100 != 0) ||  y % 400 == 0) { //those are the conditions to have a leap year
    // if leap year, calculate in the same way but increasing one day
    dayNumber = days[(m - 1)] + d + 1;
  } // else if
  else
  {
    //if not a leap year, calculate in the normal way, such as January or February
    dayNumber = days[(m - 1)] + d;
  } // else
  // Now start to calculate Week number
  if (w == 0) {
    //if it is sunday (time library returns 0)
    weekNumber = (dayNumber - 7 + 10) / 7;
  } // if
  else
  {
    // for the other days of week
    weekNumber = (dayNumber - w + 10) / 7;
  } // else
  return weekNumber;
}

//================================================================================================================
//
// leapYear
//
//================================================================================================================

/*****
 * Purpose: Determine if a given year is a leap year
 * Parameters:    int year  // The year to test
 * Return value:  int       // '1' if the year is a leap year, '0'otherwise
 *****/
int calculateLeapYear(int year)
{
  if (year % 4 == 0 && year % 100 != 0 || year % 400 == 0) {
    return 1;
  } 
  else {
    return 0;
  }
}

//================================================================================================================
//
// displayDCFTime and day led's
//
//================================================================================================================

void displayData(void)
{

  // display Day of Week on LED's
  // first, clear all the 'Day' Led's (row '0') before displaying new value
  for (int i = 0; i < 7; i++) {
    lc.setLed(MaximLeds, 0, i, false);
  }
  // Maxim 7219 LED's are numbered from zero so sunday '7' must be changed to '0'
  lc.setLed(MaximLeds, 0, ((dcfWeekDay == 7) ? 0 : dcfWeekDay), true);

  // display Weeknumber
  lc.setChar(MaximWeek, 1, ((weekNumber < 10) ? ' ' : (weekNumber / 10)), false);
  lc.setChar(MaximWeek, 0, weekNumber % 10, false);


  // display Date - with dashes between D-M-Y
  //lc.setChar(MaximDate, 7, ((dcfDay < 10) ? ' ' : (dcfDay / 10)), false);
  lc.setChar(MaximDate, 7, dcfDay / 10, false);
  lc.setChar(MaximDate, 6, dcfDay % 10, false);
  lc.setChar(MaximDate, 5, '-', false);
  lc.setChar(MaximDate, 4, dcfMonth / 10, false);
  lc.setChar(MaximDate, 3, dcfMonth % 10, false);
  lc.setChar(MaximDate, 2  , '-', false);
  lc.setChar(MaximDate, 1, dcfYear / 10, false);
  lc.setChar(MaximDate, 0, dcfYear % 10, false);


  /*
  // display Date - with dots between D.M.Y
   lc.setChar(MaximDate, 7, dcfDay / 10, false);
   lc.setChar(MaximDate, 6, dcfDay % 10, true);
   lc.setChar(MaximDate, 5, dcfMonth / 10, false);
   //lc.setChar(MaximDate, 5, ((dcfMonth < 10) ? ' ' : (dcfMonth / 10)), false);
   lc.setChar(MaximDate, 4, dcfMonth % 10, true);
   lc.setChar(MaximDate, 3, 2, false);
   lc.setChar(MaximDate, 2, 0, false);
   lc.setChar(MaximDate, 1, dcfYear / 10, false);
   lc.setChar(MaximDate, 0, dcfYear % 10, false);
   */

  /*
  // display Date - moved day to right if month is <10
   lc.setChar(MaximDate, 7, ((dcfDay < 10) ? ' ' : ((dcfMonth < 10) ? ' ' : (dcfDay / 10))), false);
   lc.setChar(MaximDate, 6, ((dcfMonth < 10) ? ((dcfDay < 10) ? ' ' : (dcfDay / 10)) : (dcfDay % 10)), ((dcfMonth < 10) ? false : true));
   lc.setChar(MaximDate, 5, ((dcfMonth < 10) ? (dcfDay % 10) : (dcfMonth / 10)), ((dcfMonth < 10) ? true : false));
   lc.setChar(MaximDate, 4, dcfMonth % 10, true);
   lc.setChar(MaximDate, 3, 2, false);
   lc.setChar(MaximDate, 2, 0, false);
   lc.setChar(MaximDate, 1, dcfYear / 10, false);
   lc.setChar(MaximDate, 0, dcfYear % 10, false);
   */

  // display Summer- or Wintertime LED
  if (dcfDST == 1) {
    lc.setLed(MaximLeds, SummerTimeLed / 10, SummerTimeLed % 10, true);
  } 
  else {
    lc.setLed(MaximLeds, WinterTimeLed / 10, WinterTimeLed % 10, true);
  }

  // display Leap Year LED
  if (leapYear = 1)  {
    lc.setLed(MaximLeds, LeapYearLed / 10, LeapYearLed % 10, false);
  } // if
  else
  {
    lc.setLed(MaximLeds, LeapYearLed / 10, LeapYearLed % 10, true);
  } // else




}

//================================================================================================================
//
// displayDCFTime (NOT USED - TEMPERATURE IS DISPLAYED INSTEAD)
//
//================================================================================================================

void displayDCFTime(void)
{
  // display DCF time
  lc1.setChar(MaximDcfTime, 3, ((dcfHour < 10) ? ' ' : (dcfHour / 10)), false);
  lc1.setChar(MaximDcfTime, 2, (dcfHour % 10), false);
  lc1.setChar(MaximDcfTime, 1, (dcfMinute / 10), false);
  lc1.setChar(MaximDcfTime, 0, (dcfMinute % 10), false);
  lc1.setChar(MaximDcfTime, 4, 1, false);
}


//================================================================================================================
//
// displayRTCTime
//
//================================================================================================================

void displayRtcTime() {

  //if (timeStatus() == timeSet) {

  lc.setChar(MaximRtcTime, 3, ((hour() < 10) ? ' ' : (hour() / 10)), false);
  lc.setChar(MaximRtcTime, 2, (hour() % 10), false);
  lc.setChar(MaximRtcTime, 1, (minute() / 10), false);
  lc.setChar(MaximRtcTime, 0, (minute() % 10), false);
  lc.setChar(MaximRtcTime, 5, (second() / 10), false);
  lc.setChar(MaximRtcTime, 4, (second() % 10), false);

  //} else {

  //Serial.println("The time has not been set.  Please run the Time");
  //Serial.println("TimeRTCSet example, or DS1307RTC SetTime example.");
  //Serial.println();
  //delay(4000);
  //}

}


//================================================================================================================
//
// displayTemp - get temperature from DS18B20 sensor and display it
//
// called from loop() every 1 second
//================================================================================================================

void displayTemp(void)
{

  ds.reset();            // The initialization sequence consists of a reset pulse transmitted by the bus master followed by presence pulse(s) transmitted by the slave(s).
  //ds.skip();           // use this command when only 1 sensor is used to avoid specifying the address!
  ds.select(addr);       // select specific device
  ds.write(0x44);        // start conversion

  delay(100);            // delay is dependent on resolution!

  present = ds.reset();  // The return value of ds.reset() command is stored in the variable PRESENT - The initialization sequence consists of a reset pulse transmitted by the bus master followed by presence pulse(s) transmitted by the slave(s).
  ds.select(addr);       // select specific device
  ds.write(0xBE);        // Read Scratchpad

  //Serial.print("  Device Present = ");
  //Serial.print(present, HEX); // show the result of RESET command: 1=present; 0=not present
  //Serial.print("     Data = ");
  for ( byte i = 0; i < 9; i++) {           // we need 9 bytes
    DS18B20Data[i] = ds.read(); // Read a Byte a a time
  } // for

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  lsByte = DS18B20Data[0];
  msByte = DS18B20Data[1];
  tempReading = (msByte << 8) + lsByte;
  tempCelcius = (6 * tempReading) + tempReading / 4;    // multiply by (100 * 0.0625) or 6.25

  // store min and max temperatures
  if (minTemp == 0) minTemp = tempCelcius;
  if (tempCelcius > maxTemp) maxTemp = tempCelcius;
  if (tempCelcius < minTemp) minTemp = tempCelcius;


  // display temperature on DCF display LED display
  lc1.setChar(MaximDcfTime, 4, ' ', false); // deactivate colon digits
  lc1.setChar(MaximDcfTime, 3, (tempCelcius / 1000), false);
  lc1.setChar(MaximDcfTime, 2, (tempCelcius % 1000) / 100, true);
  lc1.setChar(MaximDcfTime, 1, (tempCelcius % 100) / 10, false);
  lc1.setRow(MaximDcfTime, 0, B01001110); // Display 'C' (Celcius)  character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
  lc1.setChar(MaximDcfTime, 5, 1, false); // activate top dot
}

/*
//================================================================================================================
 //
 // writeScratchpad - change data in memory of DS18B20 sensor
 //
 // USE ONLY once in LOOP() IF YOU WANT TO CHANGE RESOLUTION
 //================================================================================================================
 
 void writeScratchpad(void)
 {
 // This is done ONCE in setup()
 
 // INITIALIZATION
 // All transactions on the 1-Wire bus begin with an initialization sequence.
 // The initialization sequence consists of a reset pulse transmitted by the bus master followed by presence pulse(s) transmitted by the slave(s).
 // The presence pulse lets the bus master know that slave devices (such as the DS18B20) are on the bus and are ready to operate.
 ds.reset();      // The initialization sequence consists of a reset pulse transmitted by the bus master followed by presence pulse(s) transmitted by the slave(s).
 
 // Select a specific DS18x20 device
 byte addr[8] = {
 0x28, 0x13, 0x95, 0x7B, 0x05, 0x00, 0x00, 0x70          };
 ds.select(addr);
 
 // WRITE SCRATCHPAD
 // This command allows the master to write 3 bytes of data to the DS18B20’s scratchpad.
 // The first data byte is written into the TH register (byte 2 of the scratchpad), the second byte is written into the TL register (byte 3),
 // and the third byte is written into the configuration register (byte 4).
 // Data must be transmitted least significant bit first. All three bytes MUST be written before the master issues a reset, or the data may be corrupted.
 ds.write(0x4E);
 
 ds.write(0); // write zero into the alarm register HIGH
 ds.write(0); // write zero into the alarm register LOW
/* and write R1 and R2 into the configuration register to select the precision of the temperature
 R1/R2 value | resolution | conversion time | increments
 HEX   DEC
 0  0   0   =   9 bits   |   93,75 ms      | 0,5 Celsius
 0  1   1   =  10 bits   |   187,5 ms      | 0,25
 1  0   2   =  11 bits   |   375 ms        | 0,125
 1  1   3   =  12 bits   |   750 ms        | 0,0625
 CONFIGURATION REGISTER:
 ------ Bit ------
 7  6  5  4  3  2  1  0
 0  R1 R0 1  1  1  1  1
 
 // CURRENT SETTING:
 ds.write(0 << 5); // << 5 means shift value 5 positions to the left. ds.write(B00011111) does the same
 
 
 // WRITE SCRATCHPAD DATA TO EEPROM
 ds.reset();
 ds.select(addr);
 // COPY SCRATCHPAD [48h]
 // This command copies the contents of the scratchpad TH, TL and configuration registers (bytes 2, 3 and 4) to EEPROM.
 // If the device is being used in parasite power mode, within 10μs (max) after this command is issued the master must
 // enable a strong pullup on the 1-Wire bus for at least 10ms as described in the Powering the DS18B20 section.
 ds.write(0x48);
 }
 
 //================================================================================================================
 //
 // ARCHIVE
 //
 //================================================================================================================
 
 
 Basic Usage of Onewire Library
 
 
 OneWire myWire(pin)
 Create the OneWire object, using a specific pin. Even though you can connect many 1 wire devices to the same pin, if you have a large number, smaller groups each on their own pin can help isolate wiring problems. You can create multiple OneWire objects, one for each pin.
 
 myWire.search(addrArray)
 Search for the next device. The addrArray is an 8 byte array. If a device is found, addrArray is filled with the device's address and true is returned. If no more devices are found, false is returned.
 
 myWire.reset_search()
 Begin a new search. The next use of search will begin at the first device.
 
 myWire.reset()
 Reset the 1-wire bus. Usually this is needed before communicating with any device.
 
 myWire.select(addrArray)
 Select a device based on its address. After a reset, this is needed to choose which device you will use, and then all communication will be with that device, until another reset.
 
 myWire.skip()
 Skip the device selection. This only works if you have a single device, but you can avoid searching and use this to immediatly access your device.
 
 myWire.write(num);
 Write a byte.
 
 myWire.write(num, 1);
 Write a byte, and leave power applied to the 1 wire bus.
 
 myWire.read()
 Read a byte.
 
 myWire.crc8(dataArray, length)
 Compute a CRC check on an array of data.
 
 
 //================================================================================================================
 
 
 #include "Wire.h"
 #define DS1307_I2C_ADDRESS 0x68
 // Convert normal decimal numbers to binary coded decimal
 byte decToBcd(byte val)
 {
 return ( (val/10*16) + (val%10) );
 }
 // Convert binary coded decimal to normal decimal numbers
 byte bcdToDec(byte val)
 {
 return ( (val/16*10) + (val%16) );
 }
 // Stops the DS1307, but it has the side effect of setting seconds to 0
 // Probably only want to use this for testing
/*void stopDs1307()
 {
 Wire.beginTransmission(DS1307_I2C_ADDRESS);
 Wire.send(0);
 Wire.send(0x80);
 Wire.endTransmission();
 }
 // 1) Sets the date and time on the ds1307
 // 2) Starts the clock
 // 3) Sets hour mode to 24 hour clock
 // Assumes you're passing in valid numbers
 void setDateDs1307(byte second,        // 0-59
 byte minute,        // 0-59
 byte hour,          // 1-23
 byte dayOfWeek,     // 1-7
 byte dayOfMonth,    // 1-28/29/30/31
 byte month,         // 1-12
 byte year)          // 0-99
 {
 Wire.beginTransmission(DS1307_I2C_ADDRESS);
 Wire.send(0);
 Wire.send(decToBcd(second));    // 0 to bit 7 starts the clock
 Wire.send(decToBcd(minute));
 Wire.send(decToBcd(hour));      // If you want 12 hour am/pm you need to set
 // bit 6 (also need to change readDateDs1307)
 Wire.send(decToBcd(dayOfWeek));
 Wire.send(decToBcd(dayOfMonth));
 Wire.send(decToBcd(month));
 Wire.send(decToBcd(year));
 Wire.endTransmission();
 }
 // Gets the date and time from the ds1307
 void getDateDs1307(byte *second,
 byte *minute,
 byte *hour,
 byte *dayOfWeek,
 byte *dayOfMonth,
 byte *month,
 byte *year)
 {
 // Reset the register pointer
 Wire.beginTransmission(DS1307_I2C_ADDRESS);
 Wire.send(0);
 Wire.endTransmission();
 Wire.requestFrom(DS1307_I2C_ADDRESS, 7);
 // A few of these need masks because certain bits are control bits
 *second     = bcdToDec(Wire.receive() & 0x7f);
 *minute     = bcdToDec(Wire.receive());
 *hour       = bcdToDec(Wire.receive() & 0x3f);  // Need to change this if 12 hour am/pm
 *dayOfWeek  = bcdToDec(Wire.receive());
 *dayOfMonth = bcdToDec(Wire.receive());
 *month      = bcdToDec(Wire.receive());
 *year       = bcdToDec(Wire.receive());
 }
 void setup()
 {
 byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
 Wire.begin();
 Serial.begin(9600);
 // Change these values to what you want to set your clock to.
 // You probably only want to set your clock once and then remove
 // the setDateDs1307 call.
 second = 45;
 minute = 3;
 hour = 7;
 dayOfWeek = 5;
 dayOfMonth = 17;
 month = 4;
 year = 8;
 setDateDs1307(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
 }
 void loop()
 {
 byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
 getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
 Serial.print(hour, DEC);
 Serial.print(":");
 Serial.print(minute, DEC);
 Serial.print(":");
 Serial.print(second, DEC);
 Serial.print("  ");
 Serial.print(month, DEC);
 Serial.print("/");
 Serial.print(dayOfMonth, DEC);
 Serial.print("/");
 Serial.print(year, DEC);
 Serial.print("  Day_of_week:");
 Serial.println(dayOfWeek, DEC);
 delay(1000);
 }
 */

//================================================================================================================







