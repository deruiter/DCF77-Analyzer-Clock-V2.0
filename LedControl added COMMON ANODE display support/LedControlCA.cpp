/*
 *    LedControl.cpp - A library for controling Leds with a MAX7219/MAX7221
 *    Copyright (c) 2007 Eberhard Fahle
 * 
 *    Permission is hereby granted, free of charge, to any person
 *    obtaining a copy of this software and associated documentation
 *    files (the "Software"), to deal in the Software without
 *    restriction, including without limitation the rights to use,
 *    copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following
 *    conditions:
 * 
 *    This permission notice shall be included in all copies or 
 *    substantial portions of the Software.
 * 
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *    OTHER DEALINGS IN THE SOFTWARE.
 */
 

#include "LedControl.h"

//the opcodes for the MAX7221 and MAX7219
#define OP_NOOP   0
#define OP_DIGIT0 1
#define OP_DIGIT1 2
#define OP_DIGIT2 3
#define OP_DIGIT3 4
#define OP_DIGIT4 5
#define OP_DIGIT5 6
#define OP_DIGIT6 7
#define OP_DIGIT7 8
#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

LedControl::LedControl(int dataPin, int clkPin, int csPin, int numDevices, bool anode) {
    SPI_MOSI=dataPin;
    SPI_CLK=clkPin;
    SPI_CS=csPin;
    if(numDevices<=0 || numDevices>8 )
	numDevices=8;
    maxDevices=numDevices;
    anodeMode=anode;
    pinMode(SPI_MOSI,OUTPUT);
    pinMode(SPI_CLK,OUTPUT);
    pinMode(SPI_CS,OUTPUT);
    digitalWrite(SPI_CS,HIGH);
    SPI_MOSI=dataPin;
    for(int i=0;i<64;i++) { 
 	status[i]=0x00;
 	statusTransposed[i]=0x00;
    }
	for(int i=0;i<maxDevices;i++) {
	spiTransfer(i,OP_DISPLAYTEST,0);
	//scanlimit is set to max on startup
	setScanLimit(i,7);
	//decode is done in source
	spiTransfer(i,OP_DECODEMODE,0);
	clearDisplay(i);
	//we go into shutdown-mode on startup
	shutdown(i,true);
    }
}

int LedControl::getDeviceCount() {
    return maxDevices;
}

void LedControl::shutdown(int addr, bool b) {
    if(addr<0 || addr>=maxDevices)
	return;
    if(b)
	spiTransfer(addr, OP_SHUTDOWN,0);
    else
	spiTransfer(addr, OP_SHUTDOWN,1);
}
	
void LedControl::setScanLimit(int addr, int limit) {
    if(addr<0 || addr>=maxDevices)
	return;
    if(limit>=0 || limit<8)
    	spiTransfer(addr, OP_SCANLIMIT,limit);
}

void LedControl::setIntensity(int addr, int intensity) {
    if(addr<0 || addr>=maxDevices)
	return;
    if(intensity>=0 || intensity<16)	
	spiTransfer(addr, OP_INTENSITY,intensity);
    
}

void LedControl::clearDisplay(int addr) {
    int offset;

    if(addr<0 || addr>=maxDevices)
	return;
    offset=addr*8;
    for(int i=0;i<8;i++) {
	status[offset+i]=0;
    }
     if (anodeMode) {
     	transposeData(addr);
     	for(int i=0;i<8;i++) {
 	    spiTransfer(addr, i+1, statusTransposed[offset+i]);
     	}
     } else {
     	for(int i=0;i<8;i++) {
 	    spiTransfer(addr, i+1, status[offset+i]);
     	}
    }
}

void LedControl::setLed(int addr, int row, int column, boolean state) {
    int offset;
    byte val=0x00;

    if(addr<0 || addr>=maxDevices)
	return;
    if(row<0 || row>7 || column<0 || column>7)
	return;
    offset=addr*8;
    val=B10000000 >> column;
    if(state)
	status[offset+row]=status[offset+row]|val;
    else {
	val=~val;
	status[offset+row]=status[offset+row]&val;
    }
    spiTransfer(addr, row+1,status[offset+row]);
}
	
void LedControl::setRow(int addr, int row, byte value) {
    int offset;
    if(addr<0 || addr>=maxDevices)
	return;
    if(row<0 || row>7)
	return;
    offset=addr*8;
    status[offset+row]=value;
    spiTransfer(addr, row+1,status[offset+row]);
}
    
void LedControl::setColumn(int addr, int col, byte value) {
    byte val;

    if(addr<0 || addr>=maxDevices)
	return;
    if(col<0 || col>7) 
	return;
    for(int row=0;row<8;row++) {
	val=value >> (7-row);
	val=val & 0x01;
	setLed(addr,row,col,val);
    }
}

void LedControl::setDigit(int addr, int digit, byte value, boolean dp) {
    int offset;
    byte v;

    if(addr<0 || addr>=maxDevices)
	return;
    if(digit<0 || digit>7 || value>15)
	return;
    offset=addr*8;
    v=charTable[value];
    if(dp)
	v|=B10000000;
    status[offset+digit]=v;
    if (anodeMode) {
     	//transpose the digit matrix
     	transposeData(addr);
     	//send the entire set of digits
     	for(int i=0;i<8;i++) {
 	    spiTransfer(addr, i+1, statusTransposed[offset+i]);
     	}
     } else {
     	spiTransfer(addr, digit+1, v);
     }
}

void LedControl::setChar(int addr, int digit, char value, boolean dp) {
    int offset;
    byte index,v;

    if(addr<0 || addr>=maxDevices)
	return;
    if(digit<0 || digit>7)
 	return;
   
    offset=addr*8;
    index=(byte)value;
    if(index >127) {
	//nothing defined we use the space char
	value=32;
    }
    v=charTable[index];
    if(dp)
	v|=B10000000;
    status[offset+digit]=v;
    if (anodeMode) {
	   	//transpose the digit matrix
   		transposeData(addr);
   		//send the entire set of digits
   		for(int i=0;i<8;i++) {
		spiTransfer(addr, i+1, statusTransposed[offset+i]);
   		}
    } else {
   		spiTransfer(addr, digit+1, v);
    }
}

void LedControl::spiTransfer(int addr, volatile byte opcode, volatile byte data) {
    //Create an array with the data to shift out
    int offset=addr*2;
    int maxbytes=maxDevices*2;

    for(int i=0;i<maxbytes;i++)
	spidata[i]=(byte)0;
    //put our device data into the array
    spidata[offset+1]=opcode;
    spidata[offset]=data;
    //enable the line 
    digitalWrite(SPI_CS,LOW);
    //Now shift out the data 
    for(int i=maxbytes;i>0;i--)
 	shiftOut(SPI_MOSI,SPI_CLK,MSBFIRST,spidata[i-1]);
    //latch the data onto the display
    digitalWrite(SPI_CS,HIGH);
}    


void LedControl::transposeData(int addr) {
 int offset=addr*8;
 byte a0, a1, a2, a3, a4, a5, a6, a7,
      b0, b1, b2, b3, b4, b5, b6, b7;
 
 // Perform a bitwise transpose operation on an 8x8 bit matrix, stored as 8-byte array.
 // We have to use the naive method because we're working on a 16-bit microprocessor.

 // Load the array into eight one-byte variables.
 a0 = status[offset];
 a1 = status[offset+1];
 a2 = status[offset+2];
 a3 = status[offset+3];
 a4 = status[offset+4];
 a5 = status[offset+5];
 a6 = status[offset+6];
 a7 = status[offset+7];
 
 // Magic happens. Credit goes to: http://www.hackersdelight.org/HDcode/transpose8.c.txt
 b0 = (a0 & 128)    | (a1 & 128)/2  | (a2 & 128)/4  | (a3 & 128)/8 |
      (a4 & 128)/16 | (a5 & 128)/32 | (a6 & 128)/64 | (a7      )/128;
 b1 = (a0 &  64)*2  | (a1 &  64)    | (a2 &  64)/2  | (a3 &  64)/4 |
      (a4 &  64)/8  | (a5 &  64)/16 | (a6 &  64)/32 | (a7 &  64)/64;
 b2 = (a0 &  32)*4  | (a1 &  32)*2  | (a2 &  32)    | (a3 &  32)/2 |
      (a4 &  32)/4  | (a5 &  32)/8  | (a6 &  32)/16 | (a7 &  32)/32;
 b3 = (a0 &  16)*8  | (a1 &  16)*4  | (a2 &  16)*2  | (a3 &  16)   |
      (a4 &  16)/2  | (a5 &  16)/4  | (a6 &  16)/8  | (a7 &  16)/16;
 b4 = (a0 &   8)*16 | (a1 &   8)*8  | (a2 &   8)*4  | (a3 &   8)*2 |
      (a4 &   8)    | (a5 &   8)/2  | (a6 &   8)/4  | (a7 &   8)/8;
 b5 = (a0 &   4)*32 | (a1 &   4)*16 | (a2 &   4)*8  | (a3 &   4)*4 |
      (a4 &   4)*2  | (a5 &   4)    | (a6 &   4)/2  | (a7 &   4)/4;
 b6 = (a0 &   2)*64 | (a1 &   2)*32 | (a2 &   2)*16 | (a3 &   2)*8 |
      (a4 &   2)*4  | (a5 &   2)*2  | (a6 &   2)    | (a7 &   2)/2;
 b7 = (a0      )*128| (a1 &   1)*64 | (a2 &   1)*32 | (a3 &   1)*16|
      (a4 &   1)*8  | (a5 &   1)*4  | (a6 &   1)*2  | (a7 &   1);

 // Assemble into output array.
 statusTransposed[offset] = b0;
 statusTransposed[offset+1] = b1;
 statusTransposed[offset+2] = b2;
 statusTransposed[offset+3] = b3;
 statusTransposed[offset+4] = b4;
 statusTransposed[offset+5] = b5;
 statusTransposed[offset+6] = b6;
 statusTransposed[offset+7] = b7;

}

void LedControl::setDirectDigit(int addr, int digit, byte value) {
   int offset;

   if(addr<0 || addr>=maxDevices)
	return;
   if(digit<0 || digit>7)
	return;
   offset=addr*8;
   status[offset+digit]=value;
   if (anodeMode) {
   	transposeData(addr);
   	for(int i=0;i<8;i++) {
	    spiTransfer(addr, i+1, statusTransposed[offset+i]);
   	}
   } else {
   	spiTransfer(addr, digit+1, value);
   }
}

