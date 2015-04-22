// HeliRCRx
//
// Sample RCRx RCOIP receiver
// Receives RCOIP commmands on a WiShield and uses them to control
// a cheap Chinese 3 channel coaxial IR helicopter
// This compiles to about 15000 bytes on Duemilanove, and will not fit in Diecimila.
// Tested on Duemilanove+WiShield 1.0 and Asynclabs YellowJacket
//
// This simple example handles 3 RCOIP receiver channels.
// Requires the IRrc library (which provides the IRheli subclass)
//
// HeliRCRx
//
// Sample RCRx RCOIP receiver
// Receives RCOIP commmands on a WiShield and uses them to control
// a cheap Chinese 3 channel coaxial IR helicopter
// This compiles to about 15000 bytes on Duemilanove, and will not fit in Diecimila.
// Tested on Duemilanove+WiShield 1.0 and Asynclabs YellowJacket
//
// This simple example handles 3 RCOIP receiver channels.
// Requires the IRrc library (which provides the IRheli subclass)
//
// This is the sketch that was used in the video http://www.youtube.com/watch?v=lzRpyqnD6_M
//
// This can be used off the shelf with the RCTx transmitter app for iPhone
// Copyright (C) 2010 Mike McCauley


#include <WiShield.h>
#include <Ethernet.h>
#include <WiShieldTransceiver.h>
#include <RCRx.h>
#include <Servo.h>
#include <AccelStepper.h>
#include <IRheli.h>
#include <SPI.h>

// Our singleton heli controller, defaults to pin 3 for IR output
// and helicopter channel A
IRheli heli;

// Declare the transceiver object, in this case a WiShield WiFi shield for
// Arduino, or Yellowjacket board (an Arduino with WiShield built in)
// If you want to change the default IP address, SSID etc, edit
// WiShieldTtransceiver.cpp
// Note: other type of transceiver are supported by RCRx
WiShieldTransceiver transceiver;

// Here we subclass RCRx in order to get 
// control when new analog values are available. Saves us having
// to define Setters for the heli
class MyRCRx : public RCRx
{
  public:
   virtual void setAnalogOutput(uint8_t channel, int value);
};

// Here are collected the settings received from the transmitter via the subclassed setAnalogOutput
int throttle, rudder, elevator;

// Called when new values are received from the transmitter
// Collect them and when we have them all, use them to set 
// the new command for the heli
void MyRCRx::setAnalogOutput(uint8_t channel, int value)
{
  if (channel == 1)
    throttle = -value;
  else if (channel == 2)
    rudder = value;
  else if (channel == 3)
  {
    // This is the last one in this group, execute it
    elevator = value;
    heli.setCommand(throttle, rudder, elevator, 0);  // No rudder trim signal yet
  }
}

// Our singleton Wi_fi radio control receiver
MyRCRx myrcrx;

void setup()
{
//  Serial.begin(9600);
 
  // Put the heli into safe 0 throttle
  heli.sendCommand(0, 127, 127, 127); 
   
  // Join the transceiver and the RCRx receiver object together
  myrcrx.setTransceiver(&transceiver);
  
  // Initialise the receiver
  myrcrx.init();
}

void loop()
{
  // And do it
  myrcrx.run();
  
  // Check for retransmissions to the heli
  // Blocks for about 40ms if a command is to be sent
  heli.poll();
}

