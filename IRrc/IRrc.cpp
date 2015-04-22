// IRrc.cpp
//
// Implementation of IRrc class
/// \author  Mike McCauley (mikem@open.com.au)
///
// Copyright (C) 2010 Mike McCauley
// $Id: IRrc.cpp,v 1.1 2010/07/18 21:34:33 mikem Exp mikem $

#include "IRrc.h"
#if (ARDUINO < 100)
#include "WProgram.h"
#else
#include "Arduino.h"
#endif

// Timer 2 has different output pins on different platforms
#if defined(__AVR_ATmega1280__)
#define IR_OUTPUT_PIN 9
#else
#define IR_OUTPUT_PIN 3
#endif

/////////////////////////////////////////////////////////////////////
IRrc::IRrc()
{
    // Subclasses must define the protocol timing parameters
// For example
//    _carrierFrequency = 38170;
//    _preamble = 3699;
//    _preambleGap = 1004;
//    _zeroOnTime = 814;
//    _zeroOffTime = 242;
//    _oneOnTime = 398;
//    _oneOffTime = 658;
}

/////////////////////////////////////////////////////////////////////
void IRrc::sendMessage(const uint8_t* msg, uint8_t len)
{
    uint8_t i, j;

    enableCarrier();
    sendPreamble();
    for (i = 0; i < len; i++)
    {
	for (j = 0; j < 8; j++)
	{
	    if (msg[i] & _BV(7 - j))
		sendOne();
	    else
		sendZero();
	}
    }
    sendPostamble();
}

/////////////////////////////////////////////////////////////////////
void IRrc::sendPreamble()
{
    carrierOn(_preamble);
    carrierOff(_preambleGap);
}

/////////////////////////////////////////////////////////////////////
void IRrc::sendPostamble()
{
    // Postamble is a single one bit.
    sendOne();
}

/////////////////////////////////////////////////////////////////////
void IRrc::sendZero()
{
    carrierOn(_zeroOnTime);
    carrierOff(_zeroOffTime);
}

/////////////////////////////////////////////////////////////////////
void IRrc::sendOne()
{
    carrierOn(_oneOnTime);
    carrierOff(_oneOffTime);
}

/////////////////////////////////////////////////////////////////////
// This routine is designed for 36-40KHz; if you use it for other values, it's up to you
// to make sure it gives reasonable results.  (Watch out for overflow / underflow / rounding.)
// TIMER2 is used in phase-correct PWM mode, with OCR2A controlling the frequency and OCR2B
// controlling the duty cycle.
// There is no prescaling, so the output frequency is 16MHz / (2 * OCR2A)
// To turn the output on and off, we leave the PWM running, but connect and disconnect the output pin.
// A few hours staring at the ATmega documentation and this will all make sense.
// See Secrets of Arduino PWM at http://arcfn.com/2009/07/secrets-of-arduino-pwm.html for details.
void IRrc::enableCarrier()
{
 // When not sending PWM, we want it low
  pinMode(IR_OUTPUT_PIN, OUTPUT);
  digitalWrite(IR_OUTPUT_PIN, LOW);

  // COM2A = 00: disconnect OC2A
  // COM2B = 00: disconnect OC2B; to send signal set to 10: OC2B non-inverted
  // WGM2 = 101: phase-correct PWM with OCRA as top
  // CS2 = 000: no prescaling
  TCCR2A = _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS20);

  // The top value for the timer.  The modulation frequency will be SYSCLOCK / 2 / OCR2A.
  OCR2A = SYSCLOCK / 2 / _carrierFrequency;
  OCR2B = OCR2A / 2; // 50% duty cycle
}

/////////////////////////////////////////////////////////////////////
void IRrc::disableCarrier()
{
}

/////////////////////////////////////////////////////////////////////
void IRrc::carrierOn(uint16_t time)
{
    // Sends an IR mark for the specified number of microseconds.
    // The mark output is modulated at the PWM frequency.
    TCCR2A |= _BV(COM2B1); // Enable pin 3 PWM output
    delayMicroseconds(time);
}

/////////////////////////////////////////////////////////////////////
void IRrc::carrierOff(uint16_t time)
{
    // Sends an IR space for the specified number of microseconds.
    // A space is no output, so the PWM output is disabled.
    TCCR2A &= ~(_BV(COM2B1)); // Disable pin 3 PWM output
    delayMicroseconds(time);
}

