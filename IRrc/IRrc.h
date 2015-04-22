// IRrc.h
//
// IRrc class for infra-red control
// 
// Copyright (C) 2010 Mike McCauley
// $Id: IRrc.h,v 1.1 2010/07/18 21:34:33 mikem Exp mikem $

///\mainpage IRrc library for Arduino
///
/// IRrc provides classes for controlling a range of infra-red controlled devices such as 3 channel
/// IR model helicopters and others. Such models all use a similar IR encoding scheme which is 
/// implemented in the IRrc class. Subclasses such as IRheli provide specific implementations and 
/// timings for a specific type of model or other device.
///
/// Videos explaining how it works and what you can do with it can be found at:
/// \li http://www.youtube.com/watch?v=CHZbIvcC5I0
/// \li http://www.youtube.com/watch?v=lzRpyqnD6_M
///
/// The version of the package that this documentation refers to can be downloaded 
/// from http://www.open.com.au/mikem/arduino/IRrc/IRrc-1.3.zip
/// You can find the latest version at http://www.open.com.au/mikem/arduino/IRrc
///
/// Tested on Arduino Duemilanove, Diecimila, Mega and Asynclabs Yellowjacket
/// with arduino-0018 on OpenSuSE 11.1 
/// and avr-libc-1.6.2-5.11,
/// cross-avr-binutils-2.19-9.1 and cross-avr-gcc43-4.3.3_20081022-9.3.
/// 
/// IRrc can also interoperate with the RCKit libbary at http://www.open.com.au/mikem/arduino/RCKit to 
/// create infra-red models that can be controlled from your iPhone with the RCTx app from the Apple App Store 
/// at http://itunes.apple.com/app/rctx/id377833472?mt=8
///
/// The Infra-Red protocol is a 38kHz carrier, modulated with a preamble, a variable number of octets
/// followed by a postamble. Timings are specific for a particular model.
///
/// The IRheli class provides a specific implementation for a Chinese 3 channel co-axial model helicopter.
///
/// \par Basic usage
///
/// You must instantiate a subclass of the IRrc class, such as the IRheli class. By default it will control
/// an infra-red transmitter connected to Arduino digital output pin 3 (pin 9 on Arduino Mega). 
/// You then call sendMessage() 
/// (or a derivative) to send messages to the controlled device. IRrc will then modulate the infra-red 
/// tranmitter on the output pin according to the timings defined by the subclass.
///
/// The carrier output is from PWM timer 2, with 50% duty cycle. It is turned on and off according to the 
/// bit timings and the bits in the message to be transmitted.
///
/// \par Example Sketches
///
/// 2 Example Arduino sketches are provided in the examples directory of the distribution
/// to demonstrate the use of these classes:
///
/// \li HeliDemo
/// Receives simple commands on the Serial port and turns them into Infr-red commands for a
/// coaxial helicopter using the IRheli class. A corresponding program js.pl, a Perl program for Linux, 
/// reads a Logitech or similar
/// joystick and sends complying commands on a Serial port. This allows a 
/// coaxial helicopter to be controlled using a Logitech joystick connected to a Linux computer. 
/// js.pl can be found in the perl directory of the IRrc distribution.
///
/// \li HeliRCRx
/// Implements a RCRx Wi-Fi receiver, that receives RCOIP commands from an RCOIP compliant 
/// controller such as the iPhone RCTx transmitter and controls a
/// coaxial helicopter using the IRheli class. Requires the RCRx library available from 
/// http://www/.open.com.au/mikem/arduino/RCRx and the RCTx iPhone app from 
/// http://itunes.apple.com/app/rctx/id377833472?mt=8
/// This is the sketch that was used in the video http://www.youtube.com/watch?v=lzRpyqnD6_M
///
/// \li USBJoystickHeli
/// Controls a helicopter with a Logitech USB game pad connected to a Sparkfun USB Host shield. 
/// Requires the USB_Host_Shield library. The game pad is 
/// used as a mode 2 controller: throttle on left stick, elevator and rudder on right stick. 
/// Includes elevator and rudder trim buttons with button 1, 2, 3, 4.
/// This is the sketch that was used in the video http://www.youtube.com/watch?v=CHZbIvcC5I0
///
/// \par Electrical
///
/// You can get a short range IR output by connecting an IR LED directly to an Arduino output pin and ground. 
/// This will provide about 25mA of drive for the LED, which is about 1/40th of what a commonly 
/// available IR LED is capable of, but will work for short ranges. 
///
/// A better alternative for greater range is to use a transistor driver to drive a higher current 
/// through several IR LEDs. See  <a href="LED-Output.pdf">LED-Output.pdf</a>  for 
/// sample circuit diagrams for LED connections.
///
/// \par Installation
///
/// Install in the usual way: unzip the distribution zip file to the libraries
/// sub-folder of your sketchbook. 
///
/// \author  Mike McCauley (mikem@open.com.au)
///
/// This software and is Copyright (C) 2010 Mike McCauley. Use is subject to license
/// conditions. The main licensing options available are GPL V2 or Commercial:
/// 
/// \par Open Source Licensing GPL V2
/// This is the appropriate option if you want to share the source code of your
/// application with everyone you distribute it to, and you also want to give them
/// the right to share who uses it. If you wish to use this software under Open
/// Source Licensing, you must contribute all your source code to the open source
/// community in accordance with the GPL Version 2 when your application is
/// distributed. See http://www.gnu.org/copyleft/gpl.html
/// 
/// \par Commercial Licensing
/// This is the appropriate option if you are creating proprietary applications
/// and you are not prepared to distribute and share the source code of your
/// application. Contact info@open.com.au for details.
///
/// \par Revision History
/// \version 1.0 Initial release
/// \version 1.1 Added videos and USBJoystickHeli example
/// \version 1.2 Compiles under Ardiono 1.0
/// \version 1.3 Updated HeliRCRx example for latest version of RCKit.

#ifndef IRrc_h
#define IRrc_h
#include <inttypes.h>

#define SYSCLOCK 16000000  // main Arduino clock

/////////////////////////////////////////////////////////////////////
/// \class IRrc IRrc.h <IRrc.h>
/// \brief Low level infa-red output modulator, compatible with a number of infra-red controlled toys
/// This is an abstract class that must be subclassed.
/// Subclasses customise timings etc for particular toys and protocols.
///
class IRrc
{   
public:
    /// Constructor.
    IRrc();

    /// Sends the message pointed to by msg, with length len octets. Message is precended by the preamble 
    /// with sendPreamble() and followed by the postamble with sendPostamble(). 
    /// Each bit in the message is sent with the carrier enabled for the on time and disabled for the off time.
    /// Each octet is sent most significant bit first.
    /// Caution: this call blocks until the postamble has been sent
    /// \param[in] msg Pointer to the octets to be sent
    /// \param[in] len Message length in octets
    void      sendMessage(const uint8_t* msg, uint8_t len);

    /// Sends the preamble, which consists of the carrier on for the _preamble time, followed by carrier off
    /// for the _preambleGap time.
    void      sendPreamble();

    /// Send the postamble which consists by default of a single One bit sent with sendOne()
    void      sendPostamble();

    /// Encodes and sends a Zero bit, with the carrier on for _zeroOnTime microseconds, followed by
    /// off for _zeroOffTime microseconds.
    void      sendZero();

    /// Encodes and sends a One bit, with the carrier on for _oneOnTime microseconds, followed by
    /// off for _oneOffTime microseconds.
    void      sendOne();

    /// Enables IR output.  The _carrierFrequency value controls the carrier frequency in Hertz.
    /// The IR output will be on pin 3 (OC2B) on most platforms, pin 9 on Arduino Mega.
    void      enableCarrier();

    /// Disables the IR output, if necessary.
    /// The default implmentation does nothing.
    void      disableCarrier();

    /// Turns the carrier on for the specified time.
    /// Blocks until complete.
    /// \param[in] time Time on in microseconds
    void      carrierOn(uint16_t time);

    /// Turns the carrier off for the specified time.
    /// Blocks until complete.
    /// \param[in] time
    void      carrierOff(uint16_t time);

protected:
    // Subclasses have to set these in their constructor

    /// IR Carrier frequence in Hertz
    uint16_t  _carrierFrequency;

    /// Length of the preamble carrier on time in microseconds
    uint16_t  _preamble;

    /// Length of the preamble gap off time in microseconds
    uint16_t  _preambleGap;

    /// Time that the carrier is on at the start of a Zero bit in microseconds
    uint16_t  _zeroOnTime;

    /// Time that the carrier is off at the end of a Zero bit in microseconds
    uint16_t  _zeroOffTime;

    /// Time that the carrier is on at the start of a One bit in microseconds
    uint16_t  _oneOnTime;

    /// Time that the carrier is off at the end of a One bit in microseconds
    uint16_t  _oneOffTime;

private:
};

#endif 
