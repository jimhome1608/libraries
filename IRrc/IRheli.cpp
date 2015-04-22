// IRheli.cpp
//
// IRheli.H for a CPP file
/// \author  Mike McCauley (mikem@open.com.au)
///
// Copyright (C) 2010 Mike McCauley
// $Id: IRheli.cpp,v 1.1 2010/07/18 21:34:33 mikem Exp mikem $

#include "IRheli.h"
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <wiring.h>
#endif
#include <HardwareSerial.h>

/////////////////////////////////////////////////////////////////////
IRheli::IRheli() : IRrc()
{
#if 0
    // These timings from  http://www.rcgroups.com/forums/showthread.php?t=1231421&page=2
    _carrierFrequency = 38170;
    _preamble = 3699;
    _preambleGap = 1004;
    _zeroOnTime = 814;
    _zeroOffTime = 242;
    _oneOnTime = 398;
    _oneOffTime = 658;
#else
    // These measured by mikem
    // Doesnt work as well as the above on some aircraft
    _carrierFrequency = 38340;
    _preamble = 3604;
    _preambleGap = 1024;
    _zeroOnTime = 734;
    _zeroOffTime = 296;
    _oneOnTime = 332;
    _oneOffTime = 708;
#endif
    _channel = IR_HELI_CHANNEL_A;
}

/////////////////////////////////////////////////////////////////////
void IRheli::setChannel(uint8_t channel)
{
    _channel = channel;
}

/////////////////////////////////////////////////////////////////////
// All inputs are 8 bits unsigned
// Throttle: 0 = 0, 255 = max
// Rudder: 0 = full left, 127 = neutral 255 = full right
// Elevator: 0 = full nose down, 127 = neutral 255 = full nose up
// Rudder trim: 0 = full left, 127 = neutral 255 = full right
void IRheli::setCommand(uint8_t throttle, uint8_t rudder, uint8_t elevator, uint8_t ruddertrim)
{
#if 0
    // Canned captured command
    _commandRaw[0] = 0x28;
    _commandRaw[1] = 0x00;
    _commandRaw[2] = 0xcd;
    _commandRaw[3] = 0xc4;
#endif
    // Throttle (only 7 bits worth are sent. Top bit is 0
    _commandRaw[0] = throttle >> 1;
    // Large throttle settings cause some helis to shut down
    // So we only send 0x00 to 0x76
    if (_commandRaw[0] > 0x76)
	_commandRaw[0] = 0x76;

    // Rudder (only 4 bits worth are sent)
    _commandRaw[1] = _commandRaw[2] = 0;
    if (rudder >= 128)
    {
	_commandRaw[1] = (((rudder - 128) << 1) & IR_HELI_MASK_RUDDER);
    }
    else
    {
	_commandRaw[1] = (((127 - rudder) << 1) & IR_HELI_MASK_RUDDER);
	_commandRaw[2] |= IR_HELI_FLAG_RUDDER_LEFT;
    }

    // Elevator (only 4 bits worth are sent)
    if (elevator >= 128)
    {
	_commandRaw[1] |= (((elevator - 128) >> 3) & IR_HELI_MASK_ELEVATOR);
	_commandRaw[2] |= IR_HELI_FLAG_ELEVATOR_UP;
    }
    else
    {
	_commandRaw[1] |= (((127 - elevator) >> 3) & IR_HELI_MASK_ELEVATOR);
    }

    // Rudder trim (only 5 bits worth are sent)
    if (ruddertrim >= 128)
    {
	_commandRaw[2] |= (((ruddertrim - 128) >> 2) & IR_HELI_MASK_RUDDERTRIM);
    }
    else
    {
	_commandRaw[2] |= (((127 - ruddertrim) >> 2) & IR_HELI_MASK_RUDDERTRIM);
	_commandRaw[2] |= IR_HELI_FLAG_RUDDERTRIM_LEFT;
    }

//    Serial.println(_commandRaw[0], HEX);
//    Serial.println(_commandRaw[1], HEX);
//    Serial.println(_commandRaw[2], HEX);
    // Compute the checksum in the low 6 bits and channel in the top 2 bits
    _commandRaw[3] = (_channel << 6) | ((_commandRaw[0] + _commandRaw[1] + _commandRaw[2] + 0x0f) & 0x3f);
}

/////////////////////////////////////////////////////////////////////
// Commands are always 4 octets
void IRheli::sendCurrentCommand()
{
    _lastCommandTime = millis();
    sendMessage(_commandRaw, sizeof(_commandRaw));
}

/////////////////////////////////////////////////////////////////////
void IRheli::sendCommand(uint8_t throttle, uint8_t rudder, uint8_t elevator, uint8_t ruddertrim)
{
    setCommand(throttle, rudder, elevator, ruddertrim);
    sendCurrentCommand();
}

/////////////////////////////////////////////////////////////////////
// See if its time to resend the command.
// Repetition rates depend on the channel to prevent collisions
void IRheli::poll()
{
    // When we are due to send a keepalive
    unsigned long nextCommandTime = _lastCommandTime;
    switch (_channel)
    {
	case IR_HELI_CHANNEL_A:
	    nextCommandTime += 150;
	    break;

	case IR_HELI_CHANNEL_B:
	    nextCommandTime += 120;
	    break;

	case IR_HELI_CHANNEL_C:
	    nextCommandTime += 80;
	    break;
    }
    if (millis() > nextCommandTime)
    {
	// Resend the last command
	// REVISIT: only if throttle is not 0?
	sendCurrentCommand();
    }
}
