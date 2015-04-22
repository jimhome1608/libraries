// HeliDemo.pde
//
// Demo code for IRrc library
//
// Controls a typical chinese 3 channel coaxial IR helicopter with commands
// received on the serial port. See js.pl for sample Perl program that can send such commands
// from a Logitech joystick.
// Tested on Diecimila, Duemilanove, Mega and Yellowjacket
//
// Copyright (C) 2010 Mike McCauley
// $Id: $

#include <IRheli.h>

// Our singleton heli controller, defaults to pin 3 for IR output (pin 9 on Mega)
// and helicopter channel A
IRheli heli;

void setup()
{
  Serial.begin(9600);
  heli.sendCommand(0, 127, 127, 127); // Throttle off
}

// Read a command from the serial input, parse it and
// send it to the heli
void readCommand()
{
  static char buf[50];
  static uint8_t pos = 0;
  if (Serial.available())
  {
    buf[pos] = Serial.read();
    // Here we accept either eol or : as the end of command to make it easy for interactive
    // testing from the IDE serial port monitor
    if (buf[pos] == '\n' || buf[pos] == ':')
    {
      // End of line, parse the command
      int throttle, rudder, elevator, ruddertrim;
      if (sscanf(buf, "%d %d %d %d\n", &throttle, &rudder, &elevator, &ruddertrim) == 4)
      {
        // Set the commands for the next poll cycle
        heli.setCommand(throttle, rudder, elevator, ruddertrim);
      }
      pos = 0;
    }
    else
    {
      pos++;
      // Prevent buffer overflow
      if (pos >= sizeof(buf))
        pos = sizeof(buf) - 1;
    }
  }
}
void loop()
{
  // Maybe get a new command from the serial port and use it to set
  // the heli command
  readCommand();
  
  // Refresh the heli with a keepalive transmission of the last command
  // Blocks for about 40ms if a command is to be sent
  heli.poll();
}


