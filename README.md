 ================================================================================
  DCF77 Analyzer / Clock
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
 
 
 The C++ code is far from optimized because I myself am an Arduino and C++ novice.
 But even after learning some more now, I want to keep the code simpel and readable.
 That is why I maybe over-documented the code to help understand what's going on.

 HELP WANTED: 
 I'm not experienced enough to get to the point where the second-tick
 of the Real Time Clock display is exactly in sync with the DCF pulse.
 Now there's a small time lag.
 If you have a solution, please let me know! 
 

 Erik de Ruiter
 2014-2016
   
 

 August 2014 First version
 March 2016 - big overhaul...

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
 - Changed temperature function to only calculate once per minute. Got strange errors before the change.

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
   - Display description MaximDcfTime is now MaximTemperature 
   - DCF77SOUNDPIN is now BUZZERPIN
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

  Power saving:
  At times set by the user, the displays are shutt off at night and turned on in the morning.
  Look at the POWERSAVINGOFFTIME and POWERSAVINGONTIME variables. 
  Check the function <tasksEveryHour> to select which displays you want to shut off at night.

  DCF beep:
  With a switch, connected to pin BUZZERPIN, you can hear the received DCF bits coming in. 
  The tone duration is equivalent to pulse width of the DCF bits, so either 100 or 200 ms.
  
  Miscelleanous:
  When the RTC battery is empty or a connection fault is detected, the RTC Error LED is lit.


