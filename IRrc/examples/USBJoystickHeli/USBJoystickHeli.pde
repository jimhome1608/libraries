//
// Demo code for IRrc library
//
// Controls a typical chinese 3 channel coaxial IR helicopter with commands
// received from a Logitech USB game pad and the Sparkfun USB Host shield
// Tested on Diecimila, Duemilanove and Yellowjacket
//
// This is the sketch that was used in the video http://www.youtube.com/watch?v=CHZbIvcC5I0
//
// Copyright (C) 2010 Mike McCauley
// $Id: $

/* MAX3421E USB Host controller LCD/keyboard demonstration */
#include <Spi.h>
#include <Max3421e.h>
#include <Usb.h>
#include <IRheli.h>

/* keyboard data taken from configuration descriptor */
#define KBD_ADDR        1
#define KBD_EP          1
#define KBD_IF          0
#define EP_MAXPKTSIZE   8
#define EP_POLL         0x0a

// Masks and offsets for contents of USB read for Logitech
// gamepad joystick.
#define  JOY_INDEX_LEFT_X 0
#define  JOY_INDEX_LEFT_Y 1
#define  JOY_INDEX_RIGHT_X 2
#define  JOY_INDEX_RIGHT_Y 3
#define  JOY_INDEX_BUTTONS_1 4
#define  JOY_INDEX_BUTTONS_2 5
#define  JOY_INDEX_BUTTONS_3 6

// Masks for buf index 4
#define JOY_MASK_BUTTON_1 0x10
#define JOY_MASK_BUTTON_2 0x20
#define JOY_MASK_BUTTON_3 0x40
#define JOY_MASK_BUTTON_4 0x80

// Masks for buf index 5
#define JOY_MASK_BUTTON_5 0x1
#define JOY_MASK_BUTTON_6 0x2
#define JOY_MASK_BUTTON_7 0x4
#define JOY_MASK_BUTTON_8 0x8
#define JOY_MASK_BUTTON_9 0x10
#define JOY_MASK_BUTTON_10 0x20
#define JOY_MASK_BUTTON_LEFT_STICK 0x40
#define JOY_MASK_BUTTON_RIGHT_STICK 0x80

// Masks for buf index 6
#define JOY_MASK_BUTTON_MODE 0x8

EP_RECORD ep_record[ 2 ];  //endpoint record structure for the keyboard

uint8_t buf[ 8 ] = { 0 };      // joystick buffer
uint8_t last_poll[8] = { 0 };

void setup();
void loop();

MAX3421E Max;
USB Usb;
// Our singleton heli controller, defaults to pin 3 for IR output (pin 9 on Mega)
// and helicopter channel A
IRheli heli;

void setup() {

  Serial.begin( 9600 );
  Serial.println("Start");
  Max.powerOn();
  heli.sendCommand(0, 127, 127, 127); // Throttle off
  delay( 200 );
}

void loop() 
{
    Max.Task();
    Usb.Task();
    if ( Usb.getUsbTaskState() == USB_STATE_CONFIGURING ) 
    {  //wait for addressing state
        joy_init();
        Usb.setUsbTaskState( USB_STATE_RUNNING );
    }
    if( Usb.getUsbTaskState() == USB_STATE_RUNNING ) 
    {  //poll the joystick
        joy_poll();
    }
    // Refresh the heli with a keepalive transmission of the last command
    // Blocks for about 40ms if a command is to be sent
    heli.poll();
}

void joy_init( void )
{
   byte rcode = 0;  //return code
/**/
    /* Initialize data structures */
    ep_record[ 0 ] = *( Usb.getDevTableEntry( 0,0 ));  //copy endpoint 0 parameters
    ep_record[ 1 ].MaxPktSize = EP_MAXPKTSIZE;
    ep_record[ 1 ].Interval  = EP_POLL;
    ep_record[ 1 ].sndToggle = bmSNDTOG0;
    ep_record[ 1 ].rcvToggle = bmRCVTOG0;
    Usb.setDevTableEntry( 1, ep_record );              //plug kbd.endpoint parameters to devtable
    /* Configure device */
    rcode = Usb.setConf( KBD_ADDR, 0, 1 );                    
    if( rcode ) {
        Serial.print("Error attempting to configure Joystick. Return code :");
        Serial.println( rcode, HEX );
        while(1);  //stop
    }
    /* Set boot protocol */
    rcode = Usb.setProto( KBD_ADDR, 0, 0, 0 );
    if( rcode ) {
        Serial.print("Error attempting to configure boot protocol. Return code :");
        Serial.println( rcode, HEX );
        while( 1 );  //stop
    }

    Serial.println("Joystick initialized");
}


void joy_poll( void)
{
    static int ruddertrim = 127;
    static int elevatortrim = 0;
  
    if ( Usb.inTransfer( KBD_ADDR, KBD_EP, 8, (char*)buf ) == 0)
    {
      // Buttons 4 and 2 adjust elevator trim
      if (     (buf[JOY_INDEX_BUTTONS_1] & JOY_MASK_BUTTON_4)
          && ! (last_poll[JOY_INDEX_BUTTONS_1] & JOY_MASK_BUTTON_4))
          elevatortrim -= 10;
      if (     (buf[JOY_INDEX_BUTTONS_1] & JOY_MASK_BUTTON_2)
          && ! (last_poll[JOY_INDEX_BUTTONS_1] & JOY_MASK_BUTTON_2))
          elevatortrim += 10;
          
      // Buttons 1 and 3 adjust rudder trim
      if (     (buf[JOY_INDEX_BUTTONS_1] & JOY_MASK_BUTTON_1)
          && ! (last_poll[JOY_INDEX_BUTTONS_1] & JOY_MASK_BUTTON_1))
          ruddertrim -= 10;
      if (     (buf[JOY_INDEX_BUTTONS_1] & JOY_MASK_BUTTON_3)
          && ! (last_poll[JOY_INDEX_BUTTONS_1] & JOY_MASK_BUTTON_3))
          ruddertrim += 10;
      
      
      // map 0 to 128 to 255 to 0
      int throttle = buf[JOY_INDEX_LEFT_Y];
      if (throttle > 127)
        throttle = 127;
      throttle = ((255 - throttle) << 1) & 0xff; 

      int rudder = buf[JOY_INDEX_RIGHT_X];
      
      int elevator = buf[JOY_INDEX_RIGHT_Y] + elevatortrim;
      if (elevator < 0)
        elevator = 0;
      if (elevator > 255)
        elevator = 255;
        
      if (ruddertrim < 0)
        ruddertrim = 0;
      if (ruddertrim > 255)
        ruddertrim = 255;
        
      // Set the commands for the next poll cycle
//      Serial.println(ruddertrim, DEC);
      heli.setCommand(throttle, rudder, elevator, ruddertrim);
      
      // Save last poll
      memcpy(last_poll, buf, sizeof(buf));
    }
}




