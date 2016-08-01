 /*
 ================================================================================
 DCF77 Analyzer / Clock version 2.0
 ================================================================================
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
 ================================================================================

 This C++ code is far from optimized because I myself am an Arduino and C++ novice.
 But even after learning some more now, I want to keep the code simpel and readable.
 That is why I maybe over-documented the code to help understand what's going on.

 Erik de Ruiter
 2014-2016
   
 

 May 2014 First version
 March 2016 - big overhaul...
 July 2016 - Start with building the 2.0 Clock and adapting the sketch

 Version 2.0 - August 2016
 - This sketch is adapted for my 2.0 version of the DCF/Analyzer Clock. It used the Arduino MEGA and the DCF Superfilter
   by default and to drive the many seperate LED's I now use the ports of an Arduino Mega instead of a Maxim 7219 chip. 
   This is because driving LED's with many different Voltage/Current specs is problematic with the Maxim chip. 
   Lighting additional LED's for expample will influence (dim) the LED's already on. As I'm not an electronics engineer
   my only solution was to use the extra ports of the Arduino Mega. Ofcourse you can use transistors or extra chips to
   drive the LED's but for me this was the obvious solution.
 - Removed all the code to use Maxim 72xx chips with Common Anode displays

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            

 Version 1.72 - May 2016
 - Option: Use a cheap Ebay PIR detector to shut off selectable display's when no activity is detected. 
   The switch off delay can be set by the user to prevent the display shutting of if a person
   is not moving but the display should be on.
 - Now the display Night shut-down can be disabled by making both values 'POWERSAVINGOFFTIME'
   and 'POWERSAVINGONTIME' zero. 
 - Fixed temperature display not shutting off at powersave mode.  
 - errorCounter display did not reset every hour so that's fixed

 Version 1.71
 - User option to reset temperature min/max memory at midnight

 Version 1.7:
 - The resolution of the temperature display is improved: from 0.5 to 0.1 degrees Celsius
   Because of the time the DS18B20 sensor needs to convert the temperature and to keep the code clean, 
   the temperature display is updates once per minute.
 - Parity check routine optimized. 
 - More reliable check for bad DCF data, preventing RTC update with invalid data.
 - EoB error now clears inner LED ring as it should.
 - The DCF OK LED now displays the condition of the DCF signal more reliably. Turns off immediately if an error occurs
   and only turns ON when all 3 parity bits are OK.

 Version 1.6:
 - Changed temperature function to only calculate once per minute. Got strange errors before the change because
   I used a delay of 100ms to give the DS18B20 sensor time to calculate the temperature. But the delay function is
   a very bad idea in most c++ code so I finally got rid of it.

 Version 1.5:
 - Complete overhaul of the scanSignal function and the rest of the code! My first attempt worked but could be improved...
 - The rPW and rPT led's did not work as I intended so that is corrected now.
 - The End of Buffer error check routine does work now as it should.
 - I incorporated a Parity check of the incoming DCF signal. In the signal 3 Parity bits are sent so now these are
   checked and only if all three are OK, the received time information is accepted, the display is updated and the RTC synced.
   if desired, you can attach 3 extra dual-color LED's (Common Cathode) to see if each of the 3 Parity bits are OK or Failed.
 - I made wiring (or changing the wiring) much easier I think by putting all the PIN config in one easy to read table
 - As long as you use 1 DS18B20 temp. sensor, I edited the code so you no longer need to figure out the address of the I2C device.
 - Big clean-up of the code...
 - Powersaving by shutting off the displays (the clock remains functioning as normal)
   can now be configured somewhat easier by editing two variables POWERSAVINGONTIME and POWERSAVINGOFFTIME.
 - changed some variable names:
   - Maxim instances 'lc' and 'lc1' are now MaximCC and MaximCA
   - Display description MaximDcfTime is now DisplayTempWeek 
   - DCF77SOUNDPIN is now BUZZERSWITCHPIN
 - LED/Display test after power up now build in
 


  Short description:
   
  Power On:
    After power-on, first a LED test is performed. The LED's and displays lite up sequentially to keep the power consumption low.
    Then the clock starts receiving DCF pulses and when a Minute Mark (2 seconds gap) is detected, the Minute Marker LED is lit
    and the buffer counter is reset. The inner LED ring now will show the incoming DCF pulses which are also stored in the buffer.
    At 3 moments during reception of data the parity DCF bits are checked to see if the data is valid.

  Valid data received:
    When, at the end of the minute, after the Minute Mark is detected (BF (Buffer Full) LED is lit), all three parity bits are OK
    ('DCF OK' LED is lit), the buffer information is used to extract time and date information. 
    Then the RTC clock is updated ('RTC Synced' LED is lit) and the inner LED ring information is copied to the outer LED ring. 
    The time, date and week display, day LED, summer/wintertime and leap year LED information is updated with the new time information.

  No valid data:
    When one or more of the parity bits are not OK because of a noisy signal, receiving of DCF information is continued but
    will not be used to update the RTC, display's and LED's. The outer LED ring, 'RTC synced' and 'DCF OK' LED's will be reset. 
    Time, date, week, day LED, summer/wintertime LED and leap year LED are not affected and keep displaying the last received valid values.
    The 'Period Time' and/or 'Period With' error LED's will indicate the error(s) and the error counter display is updated. 
    Every hour, the error display will bet set to zero. 
    The EoB, End of Buffer LED is lit when more DCF pulses are received before the Minute Mark is detected due to a noisy signal.
    (When a minute Mark is detected we should have no more than 58 bits/pulses) 
    After the detection of the Minute Marker, a new cycle is started.
 
  Temperature:
    At the 30 second mark, the temperature display will show the High and Low values of the past period after the last reset.
  
  Chime:
    At the beginning of each hour, the Chime (if connected) will sound. 
    At night time, a time set by the user in the code itself, the chime is disabled.

  Power saving, two options:
    1. NIGHT SHUT OFF
       At times set by the user, the displays are shutt off at night and turned on in the morning.
       Look at the POWERSAVINGOFFTIME and POWERSAVINGONTIME variables. 
       Check the function <turnDisplaysOff> to select which displays you want to shut off at night.
    2. PIR SENSOR
       Connect a PIR sensor and activate the PIR option POWERSAVE_BY_PIR and the the delay at PIR_DELAY_TIME.
       Every time the PIR detector senses movement, a minute counter is reset but if no movement is detected
       longer than the PIR_DELAY_TIME, the displays are shut off. 
       When movement occurs, the displays immediately switch on. 
    Note: as said before, the clock will function normally while the displays are shut off. 
    The only thing is you can't see it... ;)

  DCF beep:
    With a switch, connected to pin BUZZERSWITCHPIN, you can hear the received DCF bits coming in. 
    The tone duration is equivalent to pulse width of the DCF bits, so either 100 or 200 ms.
  
  Miscelleanous:
    When the RTC battery is empty or a connection fault is detected, the RTC Error LED is lit.




 CREDITS:
 I learned a lot from the work of Matthias Dalheimer and Thijs Elenbaas who made their own DCF77 decoders.
 Without their work I would not have known where to start.
 I ended up writing my own code (using bits and pieces of their ideas) so I could understand what is happening...
 My code is far from efficient or advanced but it does work and I know what is going on.
 
 * A big Thank You to Brett Oliver and Joop Tap for pointing out some errors!

 Interesting websites:

 - Brett Oliver         : http://home.btconnect.com/brettoliver1/
 - Joop Tap             : http://www.jooptap.nl
 - Thijs Ellenbaas      : http://thijs.elenbaas.net/2012/04/arduino-dcf77-radio-clock-receiver-hardware-2/
 - Mathias Dalheimer    : https://github.com/roddi/DCF77-Arduino/blob/master/DCF77Servoclock/DCF77.h
 - DCF77 wikipedia      : https://en.wikipedia.org/wiki/DCF77
 - Much more DCF77 info : http://www.picbasic.nl/indexes_uk.htm

 - My Flickr website    : https://www.flickr.com/photos/edr1924/albums
 - My Github website    : https://github.com/deruiter
 - The Instructables website for this clock: soon!

 */



//----------------------------------------------------------------------------------------------------------
// Libraries
//----------------------------------------------------------------------------------------------------------

// Arduino (new) Time library .................................... http://www.pjrc.com/teensy/td_libs_Time.html
#include <Time.h>

// Enable this line if using Arduino Uno, Mega, etc.
#include <Wire.h>

// a basic DS1307 library that returns time as a time_t .......... http://www.pjrc.com/teensy/td_libs_DS1307RTC.html
#include <DS1307RTC.h>

// Maxim 7219 displays library ................................... http://playground.arduino.cc/Main/LEDMatrix
// !!! NOTE: you must use a special version of the Ledcontrol.h library to get Common Anode support
// because the Maxim chip is normally only suitable for common CATHODE displays!
#include <LedControl.h>

//SPI interface library .......................................... http://arduino.cc/en/Reference/SPI
#include <SPI.h>

// OneWire lets you access 1-wire devices made by Maxim/Dallas,
// such as DS18S20, DS18B20, DS1822 .............................. http://www.pjrc.com/teensy/td_libs_OneWire.html
// The DallasTemperature library can do all this work for you! ... http://milesburton.com/Dallas_Temperature_Control_Library
#include <OneWire.h>


//----------------------------------------------------------------------------------------------------------
// Arduino UNO Pin connections in an easy to read table
//
                              // input  - Rx - used for programming/communication with PC
                              // output - Tx - used for programming/communication with PC
#define DCF77PIN            2 // input  - DCF signal from antenna pcb. Pin must an interrupt input!
#define PIRDETECTORPIN      3 // input  - PIR detector: check for activity in the room to activate displays
#define BUZZERSWITCHPIN     4 // input  - SWITCH - turn on/off DCF77 'beep' piezo buzzer / ON = HIGH, OFF = LOW
#define CHIMESWITCHPIN      5 // input  - SWITCH - turn on/off the hourly chime sound / ON = HIGH, OFF = LOW
#define POWERSAVESWITCHPIN  6 // input  - SWITCH - turn on/off the power save feature so display is always on / ON = HIGH, OFF = LOW
#define TEMPSENSORPIN       8 // input  - Dallas One Wire DS18B20 temperature sensor
#define TEMPRESETPIN        9 // input  - PUSH BUTTON - reset temperature min/max memory / HIGH = reset
#define MAXIMCCLD          10 // output - CS/LOAD - pseudo SPI connection to the Maxim 7219 chip - 7 segment displays
#define MAXIMCCCLK         11 // output - CLOCK   - pseudo SPI connection to the Maxim 7219 chip - 7 segment displays
#define MAXIMCCDATA        12 // output - DATA    - pseudo SPI connection to the Maxim 7219 chip - 7 segment displays
// !! Pins 22 through 53 are only to be used for LED's
#define LED_SUNDAY         22 // output - LED - Sunday
#define LED_MONDAY         23 // output - LED - Monday
#define LED_TUESDAY        24 // output - LED - Tuesday
#define LED_WEDNESDAY      25 // output - LED - Wednesday
#define LED_THURSDAY       26 // output - LED - Thursday
#define LED_FRIDAY         27 // output - LED - Friday
#define LED_SATURDAY       28 // output - LED - Saturday
#define LED_CEST           29 // output - LED - Summertime CEST
#define LED_CET            30 // output - LED - Wintertime CET
#define LED_LEAPYEAR       31 // output - LED - Leap year
#define LED_RTCERROR       32 // output - LED - problem reading RTC data (empty battery/connection)
#define LED_RTCSYNC        33 // output - LED - On when RTC is succesfully synced with the DCF time
#define LED_TEMP           34 // output - LED - temperature is displayed
#define LED_OPTION1        35 // output - LED - optional 1 data is displayed
#define LED_OPTION2        36 // output - LED - optional 2 data is displayed
#define LED_ERRORPT        37 // output - LED - DCF Period Time error
#define LED_ERRORPW        38 // output - LED - DCF Period Width error
#define LED_BUFFERFULL     39 // output - LED - Buffer full indicator, next the data will be analized
#define LED_MINUTEMARKER   40 // output - LED - End of DCF data stream detected before buffer is filled, data is corrupt
#define LED_BUFFEROVERFLOW 41 // output - LED - More data received in one minute than expected due to bad signal
#define LED_DCFSTATUS      42 // output - LED - On when we have good DCF data
#define LED_POWERSAVE      43 // output - LED - Power save mode is activated, some displays are off
#define LED_PARITY1PASS    44 // output - LED - Parity 1 bit is OK
#define LED_PARITY1FAIL    45 // output - LED - Parity 1 bit FAILED
#define LED_PARITY2PASS    46 // output - LED - Parity 2 bit is OK
#define LED_PARITY2FAIL    47 // output - LED - Parity 2 bit FAILED
#define LED_PARITY3PASS    48 // output - LED - Parity 3 bit is OK
#define LED_PARITY3FAIL    49 // output - LED - Parity 3 bit FAILED
#define LED_PIRMOTION      50 // output - LED - On when PIR is detecting motion

// Analog pins
#define BUZZER             A7 // Pin A1 - output - Piezo buzzer for DCF77 'beep' (to '+' of the buzzer)
#define SPEAKERVOLPIN      A6 // Pin A2 - output - Sound Board volume - HIGH = volume one notch lower. 
                              //SPEAKERVOLUME determines how many times this output is activated after power on
#define CHIMEPIN           A5 // Pin A3 - output - Chime Activate - OUTPUT HIGH = Activate Chime on Adafruit Soundboard FX
// USED for DS1307 RTC        // I2C DATA  - connect to Real Time Clock pcb
// USED for DS1307 RTC        // I2C CLOCK - connect to Real Time Clock pcb

//----------------------------------------------------------------------------------------------------------
// DS18B20 initialization
//----------------------------------------------------------------------------------------------------------
OneWire  ds(TEMPSENSORPIN); // define Onewire instance DS

//----------------------------------------------------------------------------------------------------------
// Maxim 7219 Matrix Display initialization
//----------------------------------------------------------------------------------------------------------
/*
 clearDisplay(int addr) ............................................. clears the selected display
 MaximCC.shutdown(int addr, boolean) ................................ wake up the MAX72XX from power-saving mode (true = sleep, false = awake)
 MaximCC.setIntensity(int addr, value) .............................. set a medium brightness for the Leds (0=min - 15=max)
 MaximCC.setLed(int addr, int row, int col, boolean state) .......... switch on the led in row, column. remember that indices start at 0!
 MaximCC.setRow(int addr, int row, byte value) ...................... this function takes 3 arguments. example: MaximCC.setRow(0,2,B10110000);
 MaximCC.setColumn(int addr, int col, byte value) ................... this function takes 3 arguments. example: MaximCC.setColumn(0,5,B00001111);
 MaximCC.setDigit(int addr, int digit, byte value, boolean dp) ...... this function takes an argument of type byte and prints the corresponding digit on the specified column.
                                                                      The range of valid values runs from 0..15. All values between 0..9 are printed as digits,
                                                                      values between 10..15 are printed as their hexadecimal equivalent
 MaximCC.setChar(int addr, int digit, char value, boolean dp) ....... will display: 0 1 2 3 4 5 6 7 8 9 A B C D E F H L P; - . , _ <SPACE> (the blank or space char)
 
 ***** Please set the number of devices you have *****
 But the maximum default of 8 MAX72XX wil also work.
 LedConrol(DATAIN, CLOCK, CS/LOAD, NUMBER OF MAXIM CHIPS)
 */

// lc is for the Maxim displays
LedControl MaximCC = LedControl(MAXIMCCDATA, MAXIMCCCLK, MAXIMCCLD, 7, false); // Define pins for Maxim 72xx and how many 72xx we use

//----------------------------------------------------------------------------------------------------------
// User settings, variable and array definitions
//----------------------------------------------------------------------------------------------------------

// The value below is not a PIN number but a value to set how many times the 'Lower volume' input on the sound board is activated
// so that way the volume of the sound board can be lowered after power up, if desired.
#define SPEAKERVOLUME  12            

// Choose if you want a test of all LED's and Displays after a startup
// '1' = Yes, '0' = No
#define PERFORM_LED_TEST 1
// Delay between each 7 segment display in ms
#define LEDTEST_DELAY_DISPLAYS 600
// Delay between each LED in the LED ring and other LED's in ms
#define LEDTEST_DELAY_LED_RING 20

// Choose if you want to configure the DS18B20 temperature sensor ONCE to the highest resolution.
// this is needed after using the sensor for the first time. After running the software
// with this setting ON one time, shut it off.
// '1' = ON, '0' = OFF
#define CONFIGURE_DS18B20 0

// define power saving display OFF and ON time
// values are in 'Hour' format
// ONLY the displays are shut off at power saving time, the clock remains active.
// To disable this feature, make both values zero
#define POWERSAVINGOFFTIME 0  // displays are activated
#define POWERSAVINGONTIME  0 // displays are shutt off

// User option to reset temperature min/max memory at midnight
// '1' = Reset at midnight, '0' = Only manual reset
#define TEMPRESET_MIDNIGHT 1

// User option: activate the displays only when there is activity in the room
// '1' = ON, '0' = OFF
#define POWERSAVE_BY_PIR 1
// delay in MINUTES to wait after no detection before shutting off the displays 
#define PIR_DELAY_TIME 5 

//-------------------------------------------------------------------------------
// define miscellaneous parameters
#define DS1307_I2C_ADDRESS  0x68      // define the RTC I2C address
#define DCF_INTERRUPT       0         // Interrupt number associated with pin

// definition of Maxim 7219 display number wiring sequence
// first Maxim 7219 in wiring 'daisychain' must be '0', next '1' etc.
// COMMON CATHODE DISPLAYS
#define LedRingInner          0
#define LedRingOuter          1
#define DisplayBufferBitError 2
#define DisplayPeriodPulse    3
#define DisplayTempWeek       4
#define DisplayDate           5
#define DisplayTime           6

// definition of display brighness levels
#define BrightnessLedRingOuter           1
#define BrightnessLedRingInner           1
#define BrightnessDisplayTime            1
#define BrightnessDisplayDate            7
#define BrightnessDisplayTempWeek       15
#define BrightnessDisplayPeriodPulse     2
#define BrightnessDisplayBufferBitError 15

// Pulse flanks
static unsigned long flankTime    = 0;
static unsigned long leadingEdge  = 0;
static unsigned long trailingEdge = 0;
unsigned long previousLeadingEdge = 0;

// used in <Int0handler>
volatile unsigned int DCFSignalState = 0; // interrupt variables ALWAYS need volatile qualifier!!

// used in <loop>
int previousSecond      = 0;
int previousSignalState = 0;

// DCF Buffers and indicators
static int DCFbitBuffer[59]; // here, the received DCFbits are stored
const int bitValue[] = {1, 2, 4, 8, 10, 20, 40, 80}; // these are the decimal values of the received DCFbits

// only after start on a new minute, display received bits on inner LED ring
boolean MinuteMarkerFlag = false;
int bufferPosition       = 0;
int previousMinute       = 0;
int previousHour         = 0;

// variables to check if DCF bits are vald
bool dcfValidSignal  = false;
int dcfP1counter     = 0;
int dcfP2counter     = 0;
int dcfP3counter     = 0;
int dcfParityCheckP1 = 0;
int dcfParityCheckP2 = 0;
int dcfParityCheckP3 = 0;

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
int dayNumber;
int weekNumber;

// error counter variable
int errorCounter        = 0;
boolean errorCondition  = false;

// miscelleanous variables
boolean daytimeChange   = true;
boolean dayTime         = false;        
int dcf77SoundSwitch    = 0;

// temperature variables
byte present            = 0;
byte DS18B20Data[12];
int maxTemp             = 0;
int minTemp             = 0;
int lowByte             = 0;
int highByte            = 0;
float tempReading       = 0;
int tempCelsius         = 0;
boolean tempResetButton = false;

// PIR detector variables
int pirActivity               = 0;
int pirDisplaysState          = 1;
unsigned int pirTimer         = 0;
unsigned long previousTimePIR = 0;        


//==============================================================================
// SETUP
//==============================================================================
void setup()
{
  // initialize Serial communication
  //Serial.begin(115200);

  // initialize PIN connections
  pinMode(DCF77PIN,           INPUT);
  pinMode(TEMPRESETPIN,       INPUT);
  pinMode(BUZZERSWITCHPIN,    INPUT);
  pinMode(CHIMESWITCHPIN,     INPUT);
  pinMode(POWERSAVESWITCHPIN, INPUT);
  pinMode(PIRDETECTORPIN,     INPUT);
  pinMode(CHIMEPIN,           OUTPUT);
  pinMode(SPEAKERVOLPIN,      OUTPUT);
  // initialize LED pins 22 - 49
  for (int i1 = 22; i1 <= 53; i1 ++ )
  {
    pinMode(i1, OUTPUT);
  }

  // Initialize variables, LED displays and LED's
  initialize();

  // Initialize DCF77 pulse interrupt on pin DCF_INTERRUPT, looking for a change of the signal,
  // so either rising or falling edge pulses will trigger the interrupt handler and
  // execute the int0handler function.
  attachInterrupt(DCF_INTERRUPT, int0handler, CHANGE);

  // Initialize RTC and set as SyncProvider.
  // Later RTC will be synced with DCF time
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  // check if RTC has set the system time
  if (timeStatus() != timeSet)
  { // Unable to sync with the RTC - activate RTCError LED
    digitalWrite(LED_RTCERROR, HIGH);
  } 
  else {
    // RTC has set the system time - dim RTCError LED
    digitalWrite(LED_RTCERROR, LOW);
  }

  // After power on, set the speaker volume of the Adafruit Audio Board
  // initialize both pins to LOW which is the default output state
  digitalWrite(SPEAKERVOLPIN, LOW);
  digitalWrite(CHIMEPIN, LOW);
  // lower volume with 'SPEAKERVOLUME' steps
  for(int i = 0; i <= SPEAKERVOLUME; i++)
  {
      digitalWrite(SPEAKERVOLPIN, HIGH);
      delay(100);
      digitalWrite(SPEAKERVOLPIN, LOW);
      delay(100);
  }

  // The following function should run only once.
  // It is used to configure the temperature resolution of the DS18B20 sensor 
  if(CONFIGURE_DS18B20 == 1)
  {
    configureDS18B20();
  }

  // use for test purposes and/or setting the RTC time manually
  // setTime(23, 59, 40, 31, 12, 13);
  // RTC.set(now());

  // Request the temperature conversion
  calculateTemp();
 
  // check if a LED test is needed 
  if(PERFORM_LED_TEST == 1)
  {
    // do a LED test
    ledTest();
  }
  else
  {
    // if not doing a LED test, we need to wait a bit for the DS18B20 sensor to get ready
    delay(750);
  }

  // Now get the temperature from the sensor and display it
  displayTemp();

  // activate errorCounter display after LED test
  ledDisplay(DisplayBufferBitError, "R", 0);

}


//==============================================================================
// LOOP
//==============================================================================
void loop()
{
  // check first if pulse direction is changed (rising or falling)
  // else we would keep evaluating the same pulse
  if (DCFSignalState != previousSignalState)
  {
    // 'reset' state of variable
    previousSignalState = DCFSignalState;

    // evaluate incoming pulse
    scanSignal();
  }

  // check if switches are changed and act upon it
  checkSwitches();

  // check for PIR movement
  checkPIR();
  
  // execute tasks that must happen only once every second, minute or hour
  //----------------------------------------------------------------------------
  tasksEverySecond();
  tasksEveryMinute();
  tasksEveryHour();
}






//================================================================================================================
//
// Function name : processDcfBit
// called from   : <scanSignal>
//
// Purpose       : Evaluates the signal as it is received. Decides whether we received a "1" or a "0"
//                 and perform checks to see if the pulse timing is within limits
// Parameters    : none
// Return value  : none
//
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
       1000 2100           2000    2200        3000 3100         NO PULSE              5000 5100           6000    6200         << example millis() value
                                                                 = end of Minute indication
       ^                   ^                   ^                                       ^                   ^
       DCFbit# 56          DCFbit# 57          DCFbit# 58                               DCFbit# 0           DCFbit# 1  etc...   << DCF bit received
       
       ^                   ^        ^
       previous            leading  trailing
       leading edge        edge     edge
       
       ^   ^
       flanktime (rising or falling)
 
 */

void scanSignal()
{
  //--------------------------------------------------------------------
  // Check for Rising-Edge signal and perform checks
  //--------------------------------------------------------------------
  if (DCFSignalState == 1) 
  {
    // store Rising-Edge Time to check later if the time between two pulses is valid
    leadingEdge = millis();
    // not much to do now so exit.
    return;
  }

  //--------------------------------------------------------------------
  // Check for Falling-Edge signal and perform checks
  //--------------------------------------------------------------------
  
  if (DCFSignalState == 0)
  {
    // store Trailing-Edge Time to check later if the Pulse Width is valid
    trailingEdge = millis();

    // display period width time on "L"eft side of the 8 digit Maxim 72xx LED display
    ledDisplay(DisplayPeriodPulse, "L", (leadingEdge - previousLeadingEdge));
    // display pulse width time on the "R"ight side of the 8 digit Maxim 72xx LED display
    ledDisplay(DisplayPeriodPulse, "R", (trailingEdge - leadingEdge));

    //--------------------------------------------------------------------------------
    // Check PERIOD TIME
    //--------------------------------------------------------------------------------
    // If this flank UP is detected quickly after previous flank UP this is an incorrect
    // Period Time (should be 1000ms -or 2000ms after second 58-) that we shall reject
    if ((leadingEdge - previousLeadingEdge) < 900)
    {
      // rPW - ERROR: Periode Time (rising flank to rising flank) time is too short -> REJECTED
      error(LED_ERRORPW);
      errorCondition = true;
    }
    //--------------------------------------------------------------------------------
    // CHECK PULSE TIME
    //--------------------------------------------------------------------------------
    // If the detected pulse is too short it will be an incorrect pulse that we shall reject
    // should be 100 and 200 ms ideally
    if (((trailingEdge - leadingEdge) < 70) || ((trailingEdge - leadingEdge) > 230))
    {
      //rPT - ERROR: Pulse Width too short or too long -> REJECTED
      error(LED_ERRORPT);
      errorCondition = true;
    }

    // if we had an error return and start over
    if (errorCondition == true)
    {
      errorCondition = false;
      // although we have an error, store current rising edge time to compare at the next Rising-Edge.
      previousLeadingEdge = leadingEdge;
      return;
    }


    //--------------------------------------------------------------------
    // no errors found so now we can continue
    //--------------------------------------------------------------------
   
    // first we turn any error Led's OFF
    digitalWrite(LED_ERRORPW, LOW);         
    digitalWrite(LED_ERRORPT, LOW);
    digitalWrite(LED_BUFFERFULL, LOW);      // previous BF
    digitalWrite(LED_BUFFEROVERFLOW, LOW);  // previous EoB
    digitalWrite(LED_MINUTEMARKER, LOW);    // previous EoM
    
    // END OF MINUTE check, looking for a gap of approx. 2000ms
    if ( leadingEdge - previousLeadingEdge > 1900 && leadingEdge - previousLeadingEdge < 2100)
    {
     // end of minute detected:
      finalizeBuffer();
    }

    // refresh previousLeadingEdge time with the new leading edge time
    previousLeadingEdge = leadingEdge;

    //--------------------------------------------------------------------------------
    // process DCF bits
    //--------------------------------------------------------------------------------
    // distinguish between long and short pulses
    if (trailingEdge - leadingEdge < 170)
    {
      // call processDcfBit function and sent it the value '0'
      processDcfBit(0);
      // if switch is HIGH, the DCF pulses are audible
      if (dcf77SoundSwitch == 1) buzzer(100);
    }
    else
    {
      // call processDcfBit function and sent it the value '1'
      processDcfBit(1);
      // if switch is HIGH, the DCF pulses are audible
      if (dcf77SoundSwitch == 1) buzzer(200);
    }
  } // if (DCFSignalState == 0)
} // void scanSignal();

//================================================================================================================
//
// Function name : processDcfBit
// called from   : <scanSignal>
//
// Purpose       : after reception of one good DCF bit, do some checks and save it in the DCFbitBuffer array
// Parameters    : none
// Return value  : none
//
//================================================================================================================

void processDcfBit(int dcfBit)
{
  //--------------------------------------------------------------------
  // display values on the 7 segment displays
  //--------------------------------------------------------------------
  // display bufferPosition, digits 7,6
  MaximCC.setChar(DisplayBufferBitError, 7, bufferPosition / 10, false);
  MaximCC.setChar(DisplayBufferBitError, 6, bufferPosition % 10, false);
  
  // display received DCFbit, digit 4
  MaximCC.setChar(DisplayBufferBitError, 4, dcfBit, false);
  
  //--------------------------------------------------------------------
  // display incoming DCF bits on inner LED ring
  //--------------------------------------------------------------------
  // only if we have valid DCF data or after an Minute Mark (EoM) signal 
  // activate the inner LED ring and diplay incoming data
  if (dcfValidSignal == true || MinuteMarkerFlag == true) 
  {
    // display received bits on inner LED ring
    MaximCC.setLed(LedRingInner, bufferPosition / 8, bufferPosition % 8, dcfBit);
  }

  //--------------------------------------------------------------------
  //   // Fill DCFbitBuffer array with DCFbit 
  //--------------------------------------------------------------------
  DCFbitBuffer[bufferPosition] = dcfBit;
 
  //--------------------------------------------------------------------
  // Parity check
  //--------------------------------------------------------------------
  // DURING reception of the DCF bits, calculate and display the results of the DCF parity check.
  //
  // There is a Parity bit for the minutes, the hours and for the date.
  // DCF77 works with EVEN parity, this works as follows:
  // The hours for example have 6 bits plus a paritybit. The bits with value 1 are add up including the paritybit,
  // the result must be an even number. If there is a bit wrong received, a 0 is as 1, or a 1 is as 0 received, 
  // then the result is uneven.  source: http://www.picbasic.nl/frameload_uk.htm?http://www.picbasic.nl/info_dcf77_uk.htm

  if (bufferPosition == 0)
  {
    // reset the parity LED's
    digitalWrite(LED_PARITY1PASS, LOW);
    digitalWrite(LED_PARITY1FAIL, LOW);
    digitalWrite(LED_PARITY2PASS, LOW);
    digitalWrite(LED_PARITY2FAIL, LOW);
    digitalWrite(LED_PARITY3PASS, LOW);
    digitalWrite(LED_PARITY3FAIL, LOW);
    // reset variables
    dcfP1counter = 0;
    dcfP2counter = 0;
    dcfP3counter = 0;
    dcfParityCheckP1 = 0;
    dcfParityCheckP2 = 0;
    dcfParityCheckP3 = 0;
  }

  // ----------------------------------------
  // First parity check: minute bits
  // ----------------------------------------
  if (bufferPosition == 28)
  {
    for(int i = 21; i <= 27; i++)
    {
      // count the number of bits with the value '1'
      dcfP1counter += DCFbitBuffer[i];
    }

    // perform P1 parity check. Parity is OK if the sum is an EVEN value
    if((DCFbitBuffer[28] + dcfP1counter) % 2 == 0)
    {
      // Parity1 PASS LED ON
      digitalWrite(LED_PARITY1PASS, HIGH);
      // Parity P1 PASS
      dcfParityCheckP1 = 1;
    }
    else 
    {
      // Parity1 FAIL LED ON
      digitalWrite(LED_PARITY1FAIL, HIGH);
      // we have no valid data!
      dcfValidSignal = false;
      // Turn DCF OK LED OFF
      digitalWrite(LED_DCFSTATUS, LOW);
    }
  }

  // ----------------------------------------
  // Second parity check: hour bits
  // ----------------------------------------
  if (bufferPosition == 35)
  {
    for(int i = 29; i <= 34; i++)
    {
      dcfP2counter += DCFbitBuffer[i];
    }

    // perform P2 parity check. Parity is OK if the sum is an EVEN value
    if((DCFbitBuffer[35] + dcfP2counter) % 2 == 0)
    {
      // Parity2 PASS LED ON
      digitalWrite(LED_PARITY2PASS, HIGH);
      // Parity P2 PASS
      dcfParityCheckP2 = 1;
     }
    else 
    {
      // Parity2 FAIL LED ON
      digitalWrite(LED_PARITY2FAIL, HIGH);
      // we have no valid data!
      dcfValidSignal = false;
      // Turn DCF OK LED OFF
      digitalWrite(LED_DCFSTATUS, LOW);
    }
  }

  // ----------------------------------------
  // Third parity check: date bits
  // ----------------------------------------
  if (bufferPosition == 58)
  {
    for(int i = 36; i <= 57; i++)
    {
      dcfP3counter += DCFbitBuffer[i];
    }
    // perform P3 parity check. Parity is OK if the sum is an EVEN value
    (DCFbitBuffer[58] + dcfP3counter) % 2 == 0 ? dcfParityCheckP3 = 1 : dcfParityCheckP3 = 0;

      // Turn Parity2 'PASS' or 'FAIL' LED ON
    if(dcfParityCheckP3 == 1)
    {
      // Parity2 PASS LED ON
      digitalWrite(LED_PARITY3PASS, HIGH);
      // Parity P3 PASS
      dcfParityCheckP3 = 1;
    }
    else 
    {
      // Parity2 FAIL LED ON
      digitalWrite(LED_PARITY3FAIL, HIGH);
      // we have no valid data!
      dcfValidSignal = false;
      // Turn DCF OK LED OFF
      digitalWrite(LED_DCFSTATUS, LOW);
    }
  
    // ----------------------------------------
    // finally, check all Parity bits
    // ----------------------------------------
    dcfParityCheckP1 + dcfParityCheckP2 + dcfParityCheckP3 == 3 ? dcfValidSignal = true : dcfValidSignal = false;
  }

  //--------------------------------------------------------------------
  // before continuing with the next bit, increment counter
  //--------------------------------------------------------------------
  bufferPosition++;


  //--------------------------------------------------------------------
  // check if we have not received too many pulses?
  //--------------------------------------------------------------------
  if (bufferPosition > 59)
  {
    // Buffer Overflow ERROR - we have received more pulses before reaching
    // the 2 second 'gap' signalling the end of the minute. 
    //This error may be due to a noisy signal giving addition peaks/dcfBits
    // So clear both DCFbit displays and start again.

    // Reset buffer counter
    bufferPosition = 0;
    // clear inner LED ring
    MaximCC.clearDisplay(LedRingInner);
    // turn Buffer Overflow Error LED ON
    error(LED_BUFFEROVERFLOW);
    // exit
    return;
  }

  //--------------------------------------------------------------------
  // everything OK so we wait for next incoming DCFbit
  //--------------------------------------------------------------------
}

//================================================================================================================
//
// Function name : finalizeBuffer
// called from   : <scanSignal>
//
// Purpose       : Process the succesfully received DCF data of one minute
// Parameters    : none
// Return value  : none
//
//================================================================================================================

void finalizeBuffer(void) 
{
  //--------------------------------------------------------------------
  // We are here because of the detected 2 second 'gap'.
  // Now check if it correspondends with the buffer counter
  // 'bufferPosition' which should be value 59
  //--------------------------------------------------------------------
  if (bufferPosition == 59 && dcfValidSignal == true)
  {
    // bufferPosition == 59 so turn Buffer Full LED ON
    digitalWrite(LED_BUFFERFULL, HIGH);

    // Turn DCF OK LED ON
    digitalWrite(LED_DCFSTATUS, HIGH);

    // Reset inner LED ring (incoming time information)
    MaximCC.clearDisplay(LedRingInner);

    // copy 'contents' of inner LED ring to the outer LED ring (current time information)
    for (int i = 0; i < 59; i++) 
    {
      MaximCC.setLed(LedRingOuter, i / 8, i % 8, DCFbitBuffer[i]);
    }

    // process buffer and extract data sync the time with the RTC
    decodeBufferContents();
  
    // set Arduino time and after that set RTC time
    setTime(dcfHour, dcfMinute, 0, dcfDay, dcfMonth, dcfYear);
    RTC.set(now());
  
    // activate Synced LED
    digitalWrite(LED_RTCSYNC, HIGH);

    // Reset running buffer
    bufferPosition   = 0;

    // Reset DCFbitBuffer array, positions 0-58 (=59 bits)
    for (int i = 0; i < 59; i++) {
      DCFbitBuffer[i] = 0;
    }

    // reset flag
    MinuteMarkerFlag = false;
    
  } // if (bufferPosition == 59)


  //--------------------------------------------------------------------
  // The buffer is not yet filled although the 2 second 'gap' was detected.
  // Can be result of a noisy signal, starting in middle of receiving data etc.
  // Turn 'Minute Mark' LED ON
  //--------------------------------------------------------------------
  else
  {
    digitalWrite(LED_MINUTEMARKER, HIGH);

    // Clear displays
    MaximCC.clearDisplay(LedRingInner);
    MaximCC.clearDisplay(LedRingOuter);

    // Reset running buffer and start afresh. Now we are in sync with the incoming data
    bufferPosition   = 0;

    // Reset DCFbitBuffer array, positions 0-58 (=59 bits)
    for (int i = 0; i < 59; i++) 
    {
      DCFbitBuffer[i] = 0;
    }

    // set flag so we can display incoming pulsed on the inner LED ring.
    MinuteMarkerFlag = true;
  }
}

//================================================================================================================
//
// Function name : decodeBufferContents
// called from   : <finalizeBuffer>
//
// Purpose       : Evaluates the information stored in the buffer. 
//                 This is where the DCF77 signal is decoded to time and date information
// Parameters    : none
// Return value  : none
//
//================================================================================================================
 
void decodeBufferContents(void) 
{
  // Buffer is full and ready to be decoded
  dcfMinute  = bitDecode(21, 27);
  dcfHour    = bitDecode(29, 34);
  dcfDay     = bitDecode(36, 41);
  dcfWeekDay = bitDecode(42, 44);
  dcfMonth   = bitDecode(45, 49);
  dcfYear    = bitDecode(50, 57);

  //call function to calculate day of year and weeknumber
  dayWeekNumber(dcfYear, dcfMonth, dcfDay, dcfWeekDay);

  // Get value of Summertime DCFbit. '1' = Summertime, '0' = wintertime
  dcfDST     = bitDecode(17, 17);

  // determine Leap Year
  leapYear   = calculateLeapYear(dcfYear);
}

//================================================================================================================
//
// bitDecode
//
// called from <processBuffer>
//================================================================================================================
int bitDecode (int bitStart, int bitEnd) 
{
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
// Function name : tasksEverySecond
// called from   : <loop>
//
// Purpose       : perform tasks that must happen once every SECOND
// Parameters    : none
// Return value  : none
//
//================================================================================================================
void tasksEverySecond()
{
  // check if time is changed
  if (second() != previousSecond)
  {
    // 'reset' variable state
    previousSecond = second();

    //display the Real Time Clock Time
    displayRtcTime();

    // display 'HI' and 'LO' temperature on specific moments
    switch (second())
    {
    case 0:
      // hourly chime output: ACTIVATE
      if(dayTime == 1 && minute() == 0 && digitalRead(CHIMESWITCHPIN) == HIGH)
      {
        digitalWrite(CHIMEPIN, HIGH);
      }

      // reset temperature min/max memory at midnight
      //
      // I did put this in the 'tasks every second' section so
      // that this code is executed at one specific second only...
      // Else we would need an extra variable to prevent this code to run
      // every cycle during the whole '00' hour. 
      if(TEMPRESET_MIDNIGHT == 1 && (hour() == 00 && minute() == 00) )
      {
        minTemp = tempCelsius;
        maxTemp = tempCelsius;
      }
      break;
    case 2:
      // hourly chime output: DEACTIVATE
      digitalWrite(CHIMEPIN, LOW);
      break;
    case 30:
      // display 'HI' on display for Hi temperature
      MaximCC.setChar(DisplayTempWeek, 5, ' ', false); // Clear part of the display
      MaximCC.setChar(DisplayTempWeek, 4, ' ', false); // Clear part of the display 
      MaximCC.setChar(DisplayTempWeek, 7, 'H', false); // Display 'H' character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
      MaximCC.setRow(DisplayTempWeek,  6, B00010000); // Display 'i' character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
      break;
    case 31:
    case 32:
      // display Max temperature on DCF display LED display
      MaximCC.setChar(DisplayTempWeek, 7, (maxTemp / 100), false);
      MaximCC.setChar(DisplayTempWeek, 6, (maxTemp % 100) / 10, true);
      MaximCC.setChar(DisplayTempWeek, 5, (maxTemp % 10), false);
      MaximCC.setRow(DisplayTempWeek,  4, B11001110); // Display Degrees dot and 'C' (Celcius) character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
      // NOTE: I physically rotated digit 4 so when I activate the decimal dot, this now is the 'degrees dot' in the upper left corner
      break;
    case 33:
      // display 'LO' on display for Low temperature
      MaximCC.setChar(DisplayTempWeek, 5, ' ', false); // Clear part of the display
      MaximCC.setChar(DisplayTempWeek, 4, ' ', false); // Clear part of the display 
      MaximCC.setChar(DisplayTempWeek, 7, 'L', false); // Display 'L' character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
      MaximCC.setRow(DisplayTempWeek,  6, B00011101); // Display 'o' character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
      break;
    case 34:
    case 35:
      // display Min temperature on DCF display LED display
      MaximCC.setChar(DisplayTempWeek, 7, (minTemp / 100), false);
      MaximCC.setChar(DisplayTempWeek, 6, (minTemp % 100) / 10, true);
      MaximCC.setChar(DisplayTempWeek, 5, (minTemp % 10), false);
      MaximCC.setRow(DisplayTempWeek,  4, B11001110); // Display Degrees dot and 'C' (Celcius) character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
      // NOTE: I physically rotated digit 4 so when I activate the decimal dot, this now is the 'degrees dot' in the upper left corner
      break;
    case 36:
      // befor displaying the temperature in the next second,
      // request temperature from DS18B20 sensor, available after delay of minimal 750 ms (at highest resolution)
      calculateTemp();

      // display 'CU' on display for Current temperature
      MaximCC.setChar(DisplayTempWeek, 5, ' ', false); // Clear part of the display
      MaximCC.setChar(DisplayTempWeek, 4, ' ', false); // Clear part of the display 
      MaximCC.setRow(DisplayTempWeek,  7, B01001110); // Display 'C' character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
      MaximCC.setRow(DisplayTempWeek,  6, B00011100); // Display 'u' character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
      break;
    case 37:
      // read temperature, store in min/max memory and display current temperature
      // only once per minute this is done else things go wrong... ;)
      displayTemp();
      break;
    }// switch
  }// (second() != previousSecond)
}// void tasksEverySecond()


//================================================================================================================
//
// Function name : tasksEveryMinute
// called from   : <loop>
//
// Purpose       : perform tasks that must happen once every MINUTE
// Parameters    : none
// Return value  : none
//
//================================================================================================================
void tasksEveryMinute()
{  
  // display date, week LED's, week nr etc. if time is changed
  if (minute() != previousMinute)
  {
    // 'reset' state of variable
    previousMinute = minute();

    // display date, week LED's, week nr etc.
    if (dcfValidSignal == true)
    {
      displayData(); 
    }

    // increase PIR delay counter
    pirTimer++;

  }
}

//================================================================================================================
//
// Function name : tasksEveryHour
// called from   : <loop>
//
// Purpose       : perform tasks that must happen once every HOUR
// Parameters    : none
// Return value  : none
//
//================================================================================================================
void tasksEveryHour()
{ 
  if (hour() != previousHour)
  {
    // 'reset' variable state
    previousHour = hour();

    //--------------------------------------------------------------------
    // reset error counter and display every hour
    //--------------------------------------------------------------------
    errorCounter = 0;
    // update error counter display
    ledDisplay(DisplayBufferBitError, "R", errorCounter);

    //---------------------------------------------------------------------
    // Power saving function, shutting the displays off at night
    //---------------------------------------------------------------------

    // First, check if the night shut-down function is activated by the user
    // simply by adding up both value. If '0', the user has disabled the function
    if(POWERSAVINGOFFTIME != 0 && POWERSAVINGONTIME != 0)
    { 
      // check whether it is Day- or Nighttime
      if (hour() >= POWERSAVINGOFFTIME && hour() <= POWERSAVINGONTIME)
      {
        // ----------------------------------------
        // it's DAYTIME so activate the displays
        // ----------------------------------------
        
        // this is used to chime only if it is daytime...
        dayTime = 1; 

         // test variable because daytime routine is needed only once
        if (daytimeChange == 1)
        {
          // 'reset' variable state
          daytimeChange = 0;

          // activate SELECTED displays and status LED's
          turnDisplaysOn();
        }
      }
      else
      {
        // no chime at night...
        dayTime = 0; 

        // ----------------------------------------
        // it's NIGHTTIME so time to deactivate displays
        // ----------------------------------------
        // test variable because nighttime routine is needed only once
        if (daytimeChange == 0) 
        {
          // 'reset' variable state
          daytimeChange = 1; 

          // deactivate all the displays and status LED's
          turnDisplaysOff();

        }// if (daytimeChange == 0)
      }// else
    }// if(POWERSAVINGOFFTIME + POWERSAVINGONTIME != 0)
  }// if (dcfHour != previousHour)
}// void tasksEveryHour()


//================================================================================================================
//
// Function name : buzzer
// called from   : <scanSignal>
//
// Purpose       : generate 'beep' sound
// Parameters    : duration in ms
// Return value  : none
//
//================================================================================================================

void buzzer(int duration)
{
  tone(BUZZER, 1500, duration);
}

//================================================================================================================
//
// Function name : initialize
// called from   : <Setup>
//
// Purpose       : initialize variables and displays after power-up
// Parameters    : none
// Return value  : none
//
//================================================================================================================

void initialize(void)
{
  //---------------------------------------------------
  // Initialize Variables
  //---------------------------------------------------
  leadingEdge           = 0;
  trailingEdge          = 0;
  previousLeadingEdge   = 0;
  bufferPosition        = 0;

  digitalWrite(CHIMEPIN, LOW); // Set Chimepin to default state

  // Reset DCFbitBuffer array, positions 0-58 (=59 bits)
  for (int i = 0; i < 59; i++) {
    DCFbitBuffer[i] = 0;
  }


  //---------------------------------------------------
  // Initialize Maxim 72xx 7 segment displays
  //---------------------------------------------------
  // Maxim Common Cathode displays
  for (int i = 0; i < 8; i++)
  {
    // display wake up
    MaximCC.shutdown(i, false);
    // clear display
    MaximCC.clearDisplay(i);
  }

  //---------------------------------------------------
  // Set brightness of Maxim 72xx 7 segment displays
  //---------------------------------------------------
  // Maxim Common Cathode displays
  MaximCC.setIntensity(LedRingOuter, BrightnessLedRingOuter);
  MaximCC.setIntensity(LedRingInner, BrightnessLedRingInner);
  MaximCC.setIntensity(DisplayTime, BrightnessDisplayTime);
  MaximCC.setIntensity(DisplayDate, BrightnessDisplayDate);
  MaximCC.setIntensity(DisplayTempWeek, BrightnessDisplayTempWeek);
  MaximCC.setIntensity(DisplayPeriodPulse, BrightnessDisplayPeriodPulse);
  MaximCC.setIntensity(DisplayBufferBitError, BrightnessDisplayBufferBitError);

  //---------------------------------------------------
  // Set all Arduino LED outputs to LOW (Led's OFF)
  //---------------------------------------------------
  for (int i1 = 22; i1 <= 49; i1 ++ )
  {
    digitalWrite(i1, LOW);
  }
}

//================================================================================================================
//
// Function name : int0handler
// called from   : 
//
// Purpose       : when a rising or falling edge is detected on pin 2, this function is called
// Parameters    : none
// Return value  : none
//
//================================================================================================================

void int0handler()
{
  DCFSignalState = digitalRead(DCF77PIN);
}

//================================================================================================================
//
// Function name : turnDisplaysOn
// called from   : <tasksEveryHour> and <checkPIR>
//
// Purpose       : turn ON selected 7 segment displays and LED's
// Parameters    : none
// Return value  : none
//
//================================================================================================================
void turnDisplaysOn()
{
  // activate SELECTED displays and status LED's
  MaximCC.shutdown(LedRingInner, false);
  MaximCC.shutdown(LedRingOuter, false);
  MaximCC.shutdown(DisplayTime, false);
  MaximCC.shutdown(DisplayDate, false);
  MaximCC.shutdown(DisplayTempWeek, false);
  MaximCC.shutdown(DisplayPeriodPulse, false);
  MaximCC.shutdown(DisplayBufferBitError, false);
}

//================================================================================================================
//
// Function name : turnDisplaysOff
// called from   : <tasksEveryHour> and <checkPIR>
//
// Purpose       : turn OFF selected 7 segment displays and LED's
// Parameters    : none
// Return value  : none
//
//================================================================================================================
void turnDisplaysOff()
{
  // below you can select which display's need to be shut down for the night
  // In this case, the time display remains ON

  //MaximCC.shutdown(DisplayTime, true);
  MaximCC.shutdown(LedRingInner, true);
  MaximCC.shutdown(LedRingOuter, true);
  MaximCC.shutdown(DisplayDate, true); 
  MaximCC.shutdown(DisplayTempWeek, true);
  MaximCC.shutdown(DisplayPeriodPulse, true);
  MaximCC.shutdown(DisplayBufferBitError, true);
}

//================================================================================================================
//
// Function name : ledDisplay
// called from   : <processBuffer>
//
// Purpose       : display a value on a selected 7 segment display
// Parameters    : display 'number 'addr', 'value', "L"eft or "R"ight side.none
// Return value  : none
//
//================================================================================================================

// example: ledDisplay( DisplayBufferBitError, "R", errorCounter )
// so display the 'error counter' value on the RIGHT side of the 8 digit
// 7 segment display number 3

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
  MaximCC.setChar(addr, 3 + shift, ((value < 1000) ? ' ' : thousands), false);
  MaximCC.setChar(addr, 2 + shift, ((value < 100) ? ' ' : hundreds), false);
  MaximCC.setChar(addr, 1 + shift, ((value < 10) ? ' ' : tens), false);
  MaximCC.setChar(addr, 0 + shift, ones, false);
}

//================================================================================================================
//
// Function name : checkSwitches
// called from   : <loop>
//
// Purpose       : check if the temperature reset button button is pressed
// Parameters    : none
// Return value  : none
//
//================================================================================================================
void checkSwitches(void)
{
  //-------------------------------------------------------------
  // read state of push button tempResetButton
  tempResetButton = digitalRead(TEMPRESETPIN);
  // reset temperature min/max values when push-button is pressed
  if (tempResetButton == 1)
  {
    maxTemp = tempCelsius;
    minTemp = tempCelsius;
  }

  //-------------------------------------------------------------
  //read state of switch BUZZERSWITCHPIN
  dcf77SoundSwitch = digitalRead(BUZZERSWITCHPIN);
}

//================================================================================================================
//
// Function name : checkPIR
// called from   : <loop>
//
// Purpose       : check for PIR detector activity to shut off or activate the displays to save power 
// Parameters    : none
// Return value  : none
//
//================================================================================================================
void checkPIR()
{
  //-------------------------------------------------------------
  // Read PIR input, check for movement
  // Only check for PIR activity if user option is '1' 
  // AND only every second to prevent waisted processor time.
  if( POWERSAVE_BY_PIR == 1 && (millis() - previousTimePIR > 1000) )
  {
    // reset the 'once-per-second timer
    previousTimePIR = millis();

    // read the PIR detector PIN
    pirActivity = digitalRead(PIRDETECTORPIN);

     // turn displays ON or OFF depending on state of pin 'pirActivity'
    if(pirActivity == 1)
    {
      // PIR activity detected...
      // activated the PIR LED
      digitalWrite(LED_PIRMOTION, HIGH);
      // Reset the PIR timer variable (used in <tasksEveryMinue> function) every time
      // the PIR is activated so effectually resetting the shut-off timer.
      pirTimer = 0;

      // only continue if the display is now OFF 
      if(pirDisplaysState == 0)
      {
        // Toggle variable to prevent executing the following
        // code every time the PIR detects activity
        pirDisplaysState = 1;

        //displays ON
        turnDisplaysOn();
      }      
    }
    else
    {
      // No PIR activity detected...
      // turn the PIR LED off
      digitalWrite(LED_PIRMOTION, LOW);
      // check the pirTimer counter in the <tasksEveryMinue> function). 
      // and if the delaytime has passed, only continue if the displays are now ON 
      // else this code would be executed many times per second 
      if(pirTimer > PIR_DELAY_TIME && pirDisplaysState == 1)
      {
        // Toggle variable to prevent executing the following
        // code every time the PIR detects activity
        pirDisplaysState = 0;
      
        //shut off displays
        turnDisplaysOff();
      }
    }// else
  }// if(POWERSAVE_BY_PIR == 1)
}// void checkPIR()

//================================================================================================================
//
// Function name : error
// called from   : <scanSignal>
//
// Purpose       : turn error LED ON, clear LED ring's and increase error counter display
// Parameters    : error LED to turn on 
// Return value  : none
//
//================================================================================================================

void error(int errorLed)
{
  // no valid data
  dcfValidSignal = false;

  // turn 'dcfValidSignal = false on'
  digitalWrite(errorLed, HIGH);

  // clear Led's/displays because of error condition
  digitalWrite(LED_DCFSTATUS, LOW);
  digitalWrite(LED_RTCSYNC, LOW);
  MaximCC.clearDisplay(LedRingOuter); // clear display

  // increase errorCounter and display errorCount
  errorCounter++;
  ledDisplay(DisplayBufferBitError, "R", errorCounter);
  return;
}

//================================================================================================================
//
// Function name : dayWeekNumber
// called from   : <decodeBufferContents>
//
// Purpose       : calculate the WEEK number according to ISO standard, see comments in the ARCHIVE below
// Parameters    : dcfYear, dcfMonth, dcfDay, dcfWeekDay 
// Return value  : weekNumber
//
//================================================================================================================
//Code from: http://forum.arduino.cc/index.php/topic,44476.0.html

int dayWeekNumber(int y, int m, int d, int w)
{
  // Number of days at the beginning of the month in a normal (not leap) year.
  int days[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334}; 
  
  // Start to calculate the number of days of the first two months
  if (m == 1 || m == 2) 
  {
    // for any type of year we calculate the number of days for January or february
    dayNumber = days[(m - 1)] + d;
  }

  // now calculate for the other months
  // first, check for a leap year
  else if ((y % 4 == 0 && y % 100 != 0) ||  y % 400 == 0) 
  { 
    // we have a leap year, so calculate in the same way but adding one day
    dayNumber = days[(m - 1)] + d + 1;
  }
 
  else
  {
    //no leap year, calculate in the normal way, such as January or February
    dayNumber = days[(m - 1)] + d;
  }
  
  // Now start to calculate Week number
  if (w == 0) 
  {
    //if it is sunday (time library returns 0)
    weekNumber = (dayNumber - 7 + 10) / 7;
  }
  
  else
  {
    // for the other days of week
    weekNumber = (dayNumber - w + 10) / 7;
  }

  // finished! return with the week number as an INT value
  return weekNumber;
}


//================================================================================================================
//
// Function name : calculateLeapYear
// called from   : <decodeBufferContents>
//
// Purpose       : determine if a given year is a leap year
// Parameters    : year - the year to test
// Return value  : '1' if the year is a leap year, '0' otherwise
//
//================================================================================================================

int calculateLeapYear(int year)
{
  if ( (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0) ) 
  {
    return 1;
  } 
  else 
  {
    return 0;
  }
}

//================================================================================================================
//
// Function name : displayData
// called from   : <tasksEveryMinute>
//
// Purpose       : display LED's: day of week; status; Winter/Summer time;
// Parameters    : none
// Return value  : none
//
//================================================================================================================

void displayData(void)
{
  // display Day of Week on LED's
  // first, clear all the 'Day' Led's before displaying new value
  digitalWrite(LED_SUNDAY, LOW);
  digitalWrite(LED_MONDAY, LOW);
  digitalWrite(LED_TUESDAY, LOW);
  digitalWrite(LED_WEDNESDAY, LOW);
  digitalWrite(LED_THURSDAY, LOW);
  digitalWrite(LED_FRIDAY, LOW);
  digitalWrite(LED_SATURDAY, LOW);

  // switch on the Weekday LED
  switch (dcfWeekDay) 
  {
    case 1:
      digitalWrite(LED_MONDAY, HIGH);
      break;
    case 2:
      digitalWrite(LED_TUESDAY, HIGH);
      break;
    case 3:
      digitalWrite(LED_WEDNESDAY, HIGH);
      break;
    case 4:
      digitalWrite(LED_THURSDAY, HIGH);
      break;
    case 5:
      digitalWrite(LED_FRIDAY, HIGH);
      break;
    case 6:
      digitalWrite(LED_SATURDAY, HIGH);
      break;
    case 7:
      digitalWrite(LED_SUNDAY, HIGH);
      break;
  }

  // display Weeknumber
  MaximCC.setChar(DisplayTempWeek, 1, ((weekNumber < 10) ? ' ' : (weekNumber / 10)), false);
  MaximCC.setChar(DisplayTempWeek, 0, weekNumber % 10, false);

  // display Date - with dashes between D-M-Y
  // for example: 03-08-2015, so with leading zero's
  MaximCC.setChar(DisplayDate, 7, dcfDay / 10, false);
  MaximCC.setChar(DisplayDate, 6, dcfDay % 10, false);
  MaximCC.setChar(DisplayDate, 5, '-', false);
  MaximCC.setChar(DisplayDate, 4, dcfMonth / 10, false);
  MaximCC.setChar(DisplayDate, 3, dcfMonth % 10, false);
  MaximCC.setChar(DisplayDate, 2  , '-', false);
  MaximCC.setChar(DisplayDate, 1, dcfYear / 10, false);
  MaximCC.setChar(DisplayDate, 0, dcfYear % 10, false);

/* OTHER OPTIONS TO DISPLAY THE DATE:

  // display Date - with dots between D.M.Y
  // for example: 03.08.15
   MaximCC.setChar(DisplayDate, 7, dcfDay / 10, false);
   MaximCC.setChar(DisplayDate, 6, dcfDay % 10, true);
   MaximCC.setChar(DisplayDate, 5, dcfMonth / 10, false);
   MaximCC.setChar(DisplayDate, 4, dcfMonth % 10, true);
   MaximCC.setChar(DisplayDate, 3, 2, false);
   MaximCC.setChar(DisplayDate, 2, 0, false);
   MaximCC.setChar(DisplayDate, 1, dcfYear / 10, false);
   MaximCC.setChar(DisplayDate, 0, dcfYear % 10, false);
   */

  /*
  // display Date - moved day to right if month is <10 
  // for example: __3.8.15 or _22.7.15 or 24.12.15 (where _ is a blank display)
   MaximCC.setChar(DisplayDate, 7, ((dcfDay < 10) ? ' ' : ((dcfMonth < 10) ? ' ' : (dcfDay / 10))), false);
   MaximCC.setChar(DisplayDate, 6, ((dcfMonth < 10) ? ((dcfDay < 10) ? ' ' : (dcfDay / 10)) : (dcfDay % 10)), ((dcfMonth < 10) ? false : true));
   MaximCC.setChar(DisplayDate, 5, ((dcfMonth < 10) ? (dcfDay % 10) : (dcfMonth / 10)), ((dcfMonth < 10) ? true : false));
   MaximCC.setChar(DisplayDate, 4, dcfMonth % 10, true);
   MaximCC.setChar(DisplayDate, 3, 2, false);
   MaximCC.setChar(DisplayDate, 2, 0, false);
   MaximCC.setChar(DisplayDate, 1, dcfYear / 10, false);
   MaximCC.setChar(DisplayDate, 0, dcfYear % 10, false);
   */

  // display Summer- or Wintertime LED
  if (dcfDST == 1) 
  {
    digitalWrite(LED_CEST, HIGH);
    digitalWrite(LED_CET, LOW);
  } 
  else {
    digitalWrite(LED_CET, HIGH);
    digitalWrite(LED_CEST, LOW);
  }

  // display Leap Year LED
  if (leapYear = 1)  
  {
    digitalWrite(LED_LEAPYEAR, HIGH);
  } 
  else
  {
    digitalWrite(LED_LEAPYEAR, LOW);
  } 
}

//================================================================================================================
//
// Function name : displayRtcTime
// called from   : <tasksEverySecond>
//
// Purpose       : display the Real Time Clock time on the RTC display
// Parameters    : none
// Return value  : none
//
//================================================================================================================

void displayRtcTime() 
{
  MaximCC.setChar(DisplayTime, 7, ((hour() < 10) ? ' ' : (hour() / 10)), false);
  MaximCC.setChar(DisplayTime, 6, (hour() % 10), false);
  // activate only the segments 'a' and 'g' of the 7 segment display, representing the dots between the digits
  // Binary pattern to lite up individual segments in this order is: .ABCDEFG
  MaximCC.setRow(DisplayTime,  5, B00001001);
  MaximCC.setChar(DisplayTime, 4, (minute() / 10), false);
  MaximCC.setChar(DisplayTime, 3, (minute() % 10), false);
  // activate only the segments 'a' and 'g' of the 7 segment display, representing the dots between the digits
  // Binary pattern to lite up individual segments in this order is: .ABCDEFG
  MaximCC.setRow(DisplayTime,  2, B01001001);
  MaximCC.setChar(DisplayTime, 1, (second() / 10), false);
  MaximCC.setChar(DisplayTime, 0, (second() % 10), false);
}

//================================================================================================================
//
// Function name : calculateTemp
// called from   : <loop>
//
// Purpose       : get temperature from DS18B20 sensor and display it
// Parameters    : none
// Return value  : none
//
//================================================================================================================

void calculateTemp()
{
  // The initialization sequence consists of a reset pulse transmitted by the bus master
  // followed by presence pulse(s) transmitted by the slave(s).
  ds.reset();             
  // use this command when only 1 sensor is used to avoid specifying the address!                         
  ds.skip();
  // start conversion
  ds.write(0x44);
  // (!) AFTER THIS we need to wait for a time dependent on resolution
  // CONVERSION TIME:
  //  9 bit resolution, 93.75 ms
  // 10 bit resolution, 187.5 ms
  // 11 bit resolution, 375 ms
  // 12 bit resolution, 750 ms
  //
  // This is done in this sketch by requesting the conversion first and at minimal 1 second later reading it.
}

//================================================================================================================
//
// Function name : displayTemp
// called from   : <loop>
//
// Purpose       : get temperature from DS18B20 sensor and display it
//                 Earlier we requested the temperature from the DS18B20 sensor, 
//                 now conversion is finished so we get the data
// Parameters    : none
// Return value  : none
//
//================================================================================================================

void displayTemp()
{
  // The initialization sequence consists of a reset pulse transmitted by the bus master
  // followed by presence pulse(s) transmitted by the slave(s).
  ds.reset();
  // use this command when only 1 sensor is used to avoid specifying the address!
  ds.skip();
  // Read Scratchpad
  ds.write(0xBE);// Read Scratchpad
  // we need 9 bytes
  for ( byte i = 0; i < 9; i++) 
  {
    // Read a Byte a a time
    DS18B20Data[i] = ds.read(); 
  }
 
  // We have the data, next convert to actual temperature.
  lowByte  = DS18B20Data[0];
  highByte = DS18B20Data[1];
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t tempReading = (highByte << 8) + lowByte;
  // tempRaw is 4 digits; 2 integers, 2 fraction. For example: 2115 (= 21.15 degrees Celsius)
  int tempRaw = (6 * tempReading) + tempReading / 4;
  // We need 3 digits: 2 integers, 2 fraction. For example: 216 (= 21.6 degres Celsius)
  tempRaw % 10 >= 5 ? tempCelsius = (tempRaw / 10) + 1 : tempCelsius = tempRaw / 10;

  // store min and max temperatures
  if (minTemp == 0) minTemp = tempCelsius;
  if (tempCelsius > maxTemp) maxTemp = tempCelsius;
  if (tempCelsius < minTemp) minTemp = tempCelsius;

  // Display temperature on the multi purpose display 'TempWeek'
  
  // Also light the 'Temp' LED on the front panel
  digitalWrite(LED_TEMP, HIGH);
  // Display Digit 3: example: temperature value 216 / 100 = 2
  MaximCC.setChar(DisplayTempWeek, 7, (tempCelsius / 100), false);
  // Display Digit 2: example: temperature value 216 % 100 / 10 = 1
  // also activate decimal dot on display 2
  MaximCC.setChar(DisplayTempWeek, 6, (tempCelsius % 100) / 10, true);
  // Display Digit 3: example: temperature value 216 % 10 = 6
  MaximCC.setChar(DisplayTempWeek, 5, (tempCelsius % 10), false);
  // Display Degrees dot and 'C' (Celcius)  character. Binary pattern to lite up individual segments in this order is: .ABCDEFG
  // NOTE: I physically rotated digit 4 7 segement display so when I activate the decimal dot, 
  // this now is the 'degrees dot' in the upper left corner of the display...
  MaximCC.setRow (DisplayTempWeek, 4, B11001110);
}

//================================================================================================================
//
// Function name : ledTest
// called from   : <setup>
//
// Purpose       : after a cold start, do a led test. 
// Parameters    : none
// Return value  : none
//
//================================================================================================================
void ledTest()
{
  // The displays are lit up sequentially because of the current draw. 
  // When all is lit up at the same time, you would need a bigger power supply. 
  
  //---------------------------------------------------------------------
  // Outer LED ring
  for (int i = 0; i < 59; i++) 
  {
    // LED's ON
    MaximCC.setLed(LedRingOuter, i / 8, i % 8, true);
    delay(LEDTEST_DELAY_LED_RING);
  }
  
  for (int i = 58; i >= 0; i--) 
  {
    // LED's OFF
    MaximCC.setLed(LedRingOuter, i / 8, i % 8, false);
    delay(LEDTEST_DELAY_LED_RING);
  }
  //---------------------------------------------------------------------
  // Inner LED ring
  for (int i = 0; i < 59; i++) 
  {
    // LED's ON
    MaximCC.setLed(LedRingInner, i / 8, i % 8, true);
    delay(LEDTEST_DELAY_LED_RING);
  }
  
  for (int i = 58; i >= 0; i--) 
  {
    // LED's OFF
    MaximCC.setLed(LedRingInner, i / 8, i % 8, false);
    delay(LEDTEST_DELAY_LED_RING);
  }
  //---------------------------------------------------------------------
  // LED's
  for (int i = 22; i <= 53; i++) 
  {
    // LED's ON
    digitalWrite(i, HIGH);
    delay(LEDTEST_DELAY_LED_RING);
  }
  // wait before turning the LED's off
  delay(LEDTEST_DELAY_DISPLAYS);
  for (int i = 53; i >= 22; i--) 
  {
    // LED's OFF
    digitalWrite(i, LOW);
  }
  //---------------------------------------------------------------------
  // Real Time Clock display
  for (int i = 0; i < 8; i++) 
  {
    // LED's ON
    MaximCC.setChar(DisplayTime, i, 8, true);
  }
  // wait before turning the LED's off
  delay(LEDTEST_DELAY_DISPLAYS);
  // clear display
  MaximCC.clearDisplay(DisplayTime);
  //---------------------------------------------------------------------
  // Date display
  for (int i = 0; i < 8; i++) 
  {
    // LED's ON
    MaximCC.setChar(DisplayDate, i, 8, true);
  }
  // wait before turning the LED's off
  delay(LEDTEST_DELAY_DISPLAYS);
  // clear display
  MaximCC.clearDisplay(DisplayDate);
  //---------------------------------------------------------------------
  // Temp and Week display
  for (int i = 0; i < 8; i++) 
  {
    // LED's ON
    MaximCC.setChar(DisplayTempWeek, i, 8, true);
  }
  // wait before turning the LED's off
  delay(LEDTEST_DELAY_DISPLAYS);
  // clear display
  MaximCC.clearDisplay(DisplayTempWeek);
  //---------------------------------------------------------------------
  // Period-Pulse display
  for (int i = 0; i < 8; i++) 
  {
    // LED's ON
    MaximCC.setChar(DisplayPeriodPulse, i, 8, true);
  }
  // wait before turning the LED's off
  delay(LEDTEST_DELAY_DISPLAYS);
  // clear display
  MaximCC.clearDisplay(DisplayPeriodPulse);
  //---------------------------------------------------------------------
  // Buffer-DCFbit-Errors display
  for (int i = 0; i < 8; i++) 
  {
    // LED's ON
    MaximCC.setChar(DisplayBufferBitError, i, 8, true);
  }
  // wait before turning the LED's off
  delay(LEDTEST_DELAY_DISPLAYS);
  // clear display
  MaximCC.clearDisplay(DisplayBufferBitError);
}

//================================================================================================================
//
// Function name : configureDS18B20
// called from   : <setup>
//
// Purpose       : ON TIME function to configure the DS18B20 temperature sensor,
//                 setting the desired temperature resolution
// Parameters    : none
// Return value  : none
//
//================================================================================================================

void configureDS18B20()
{
  // This is done ONCE in setup()

  // INITIALIZATION
  // All transactions on the 1-Wire bus begin with an initialization sequence.
  // The initialization sequence consists of a reset pulse transmitted by the bus master followed by presence pulse(s) transmitted by the slave(s).
  // The presence pulse lets the bus master know that slave devices (such as the DS18B20) are on the bus and are ready to operate.
  
  // The initialization sequence consists of a reset pulse transmitted by the bus master followed by presence pulse(s) transmitted by the slave(s).
  ds.reset();      

  // use this command when only 1 sensor is used to avoid specifying the address!                         
  ds.skip();

  // Select a specific DS18x20 device
  //byte addr[8] = {0x28, 0xF0, 0xEC, 0xE2, 0x04, 0x00, 0x00, 0x47};
  //ds.select(addr);

  // WRITE SCRATCHPAD
  // This command allows the master to write 3 bytes of data to the DS18B20’s scratchpad.
  // The first data byte is written into the TH register (byte 2 of the scratchpad), the second byte is written into the TL register (byte 3),
  // and the third byte is written into the configuration register (byte 4).
  // Data must be transmitted least significant bit first. All three bytes MUST be written before the master issues a reset,
  // or the data may be corrupted.
  ds.write(0x4E);

  // write zero into the alarm register HIGH
  ds.write(0);
  // write zero into the alarm register LOW 
  ds.write(0); 
  /* Next, write R1 and R2 into the configuration register to select the precision of the temperature
   value | resolution | conversion time | increments
     0   =   9 bits   |   93,75 ms      | 0,5 Celsius
     1   =  10 bits   |   187,5 ms      | 0,25
     2   =  11 bits   |   375 ms        | 0,125
     3   =  12 bits   |   750 ms        | 0,0625
   CONFIGURATION REGISTER:
   ------ Bit ------
   7  6  5  4  3  2  1  0
   0  R1 R0 1  1  1  1  1
   */
  ds.write(B01111111);
  // WRITE SCRATCHPAD DATA TO EEPROM
  ds.reset();
  // use this command when only 1 sensor is used to avoid specifying the address!                         
  ds.skip();
  // Select a specific DS18x20 device
  //ds.select(addr);

  // COPY SCRATCHPAD [48h]
  // This command copies the contents of the scratchpad TH, TL and configuration registers (bytes 2, 3 and 4) to EEPROM.
  // If the device is being used in parasite power mode, within 10μs (max) after this command is issued the master must
  // enable a strong pullup on the 1-Wire bus for at least 10ms as described in the Powering the DS18B20 section.
  ds.write(0x48);
}






