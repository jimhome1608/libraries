// IRheli.h
//
/// \author  Mike McCauley (mikem@open.com.au)
///
// Copyright (C) 2010 Mike McCauley
// $Id: IRheli.h,v 1.1 2010/07/18 21:34:33 mikem Exp mikem $

#ifndef IRheli_h
#define IRheli_h

#include <IRrc.h>

/// Defines the channel codes for use with setChannel
enum
{
    IR_HELI_CHANNEL_A = 0x3,
    IR_HELI_CHANNEL_B = 0x2,
    IR_HELI_CHANNEL_C = 0x1,
};

/// Masks for octet 1
#define IR_HELI_MASK_RUDDER            0xf0
#define IR_HELI_MASK_ELEVATOR          0x0f

/// Masks for octet 2
#define IR_HELI_MASK_RUDDERTRIM        0x1f

/// Flags in octet 2
#define IR_HELI_FLAG_RUDDER_LEFT       0x80
#define IR_HELI_FLAG_ELEVATOR_UP       0x40
#define IR_HELI_FLAG_RUDDERTRIM_LEFT   0x20

/////////////////////////////////////////////////////////////////////
/// \class IRheli IRheli.h <IRheli.h>
/// \brief Control class for Chinsese 3 channel Infra-red Coaxial helicopters etc.
///
/// Subclass of IRrc for typical 3 channel Chinese coaxial helicopters like:
/// \li MicroTwister (as relabelled in Australia)
/// \li Copter V-MAX hypersonic
/// \li etc.
///
/// \par Basic usage
///
/// \li instantiate an instance of IRheli
/// \li Call setCommand when new commands become available
/// \li Call poll as frequently as possible to retransmit the last command at the appropriate 
/// retransmission interval.
///
/// The poll() call retransmits the most recently set command at an appropriate interval. 
/// The appropriate interval depends on the channel in use, and the intervals for each channel have been 
/// chosen to minimise collissions between multiple IRheli transmnitters that might be active in 
/// the same room at the same time.
///
/// \par Protocol
///
/// Protocol as described in http://www.rcgroups.com/forums/showthread.php?t=1231421&page=2
/// as follows:
/// Commands are preceded by the preamble and followed by the postamble
/// 
/// byte,bit explanation. Bits numbered from MSB (transmitted first) to LSB (transmitted last)
/// \li 1,1 fix (always zero)
/// \li 1,2...8 throttle
/// \li 2,1...4 rudder
/// \li 2,5...8 elevator
/// \li 3,1 1=left rudder, 0=right rudder
/// \li 3,2 1=nose up, 0=nose down
/// \li 3,3 1=trim left, 0=trim right
/// \li 3,4...8 trim
/// \li 4,1...2 channel: 11=A, 10=B, 01=C
/// \li 4,3...8 checksum = sum of first 3 octets + 0x0f
///
///
class IRheli : public IRrc
{   
public:
    /// Constructor.
    IRheli();

    /// Specifies the channel the helico-tper listens to, A, B or C. Each helicoipter is hardwired
    /// For a specific channel, which is usually marked on the side of the helicopter.
    /// \param[in] channel Channel to use, IR_HELI_CHANNEL_A, IR_HELI_CHANNEL_C, IR_HELI_CHANNEL_C
    void setChannel(uint8_t channel);

    /// Sets the comand to be sent next time poll() is called. Encodes the settings of throttle,
    /// rudder, elevator and ruddertrim and channel into the complete command, including checksum.
    /// The resulting command will be transmitted at appropriate intervals if poll() is called frequently 
    /// enough.
    /// \param[in] throttle Throttle setting. 0 is min, max is 255
    /// \param[in] rudder Rudder setting. 0 = full left, 127 = neutral 255 = full right
    /// \param[in] elevator Elevator setting. 0 = full nose down, 127 = neutral 255 = full nose up
    /// \param[in] ruddertrim RudderTrim setting. 0 = full left, 127 = neutral 255 = full right
    void setCommand(uint8_t throttle, uint8_t rudder, uint8_t elevator, uint8_t ruddertrim);

    /// Sends immediately a command for the specified control setting. The same command will be sent
    /// thereafer when poll() is called and a retransmission is required.
    /// Caution: blocks until the command is sent.
    /// \param[in] throttle Throttle setting. 0 is min, max is 255
    /// \param[in] rudder Rudder setting. 0 = full left, 127 = neutral 255 = full right
    /// \param[in] elevator Elevator setting. 0 = full nose down, 127 = neutral 255 = full nose up
    /// \param[in] ruddertrim RudderTrim setting. 0 = full left, 127 = neutral 255 = full right
    void sendCommand(uint8_t throttle, uint8_t rudder, uint8_t elevator, uint8_t ruddertrim);

    /// If a command is due to be retransmitted, resends the last set command.
    /// Caution: If a command is to be sent, blocks for about 40ms until the command is sent.
    void poll();

protected:
    /// Sends the most recently set command
    void sendCurrentCommand();

    /// The currently selected channel, as set by setChannel(). Defaults to IR_HELI_CHANNEL_A.
    uint8_t       _channel;

private:

    /// Time the last command was sent in milliseconds
    unsigned long _lastCommandTime;

    /// The most recently set command.
    uint8_t       _commandRaw[4];
};

#endif 
