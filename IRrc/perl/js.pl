#!/usr/bin/perl
#
# js.pl
# reads a Logitech (or similar) joystick and sends the data to a Arduino
# controlled helicopter via serial port. The Arduino converts the commands into 
# IR signals for the heli
# Caution: expects the serial port to be on stdout, with
# the baud rate already set.
# Left joy is collective (up only)
# Right joy left/right is yaw (rudder)
# Right joy up down is pitch (elevator)
# Buttons 1/3 are yaw trim (rudder trim)
# Buttons 2/4 are pitch trim (elevator trim)
#
# Run like this:
# ./js.pl >/dev/ttyUSB0
# where /dev/ttyUSB0 is the prt where your Arduino is connected
# Works with the HeliDemo.pde Arduino program and the IRrc library.
#
# Copyright (C) 2010 Mike McCauley
# $Id: $

require "newgetopt.pl";
use Fcntl;
use Time::HiRes;
use strict;

my @options = 
    (
     'h',                   # Help, show usage
     'v',                   # Print version number
     'j=s',                 # Joystick device
     'p=n',                 # Polltime in microseconds
    );

&NGetOpt(@options) || &usage;
&usage if $main::opt_h;
&version if $main::opt_v;

my $js_device = $main::opt_j || '/dev/input/js0';
my $polltime = $main::opt_p || 13000;

# Note non-blocking
sysopen(JS, $js_device, O_NONBLOCK) || die("Could not open $js_device: $!");

my $js_event_size = 8;
my $ruddertrim = 128;
my $elevatortrim = 0;
my @axis;


while (1)
{
    # Read a joystick event
    my $js_event;
    my $bytes;
    my $got_event;
    # Absorb all the event from the queue, non-blocking
    while (($bytes = read(JS, $js_event, $js_event_size)) == $js_event_size)
    {
	# Got a valid event
	my ($jse_time, $jse_value, $jse_type, $jse_number) = unpack('IsCC', $js_event);
#	print "its $jse_time, $jse_value, $jse_type, $jse_number\n";
	$got_event++;
	if ($jse_type == 1)
	{
	    # Button
	    $ruddertrim -= 0x8 if $jse_number == 0 && $jse_value == 1;
	    $ruddertrim += 0x8 if $jse_number == 2 && $jse_value == 1;
	    $elevatortrim += 1 if $jse_number == 1 && $jse_value == 1;
	    $elevatortrim -= 1 if $jse_number == 3 && $jse_value == 1;
	}
	elsif ($jse_type == 2)
	{
	    # Axis
	    $axis[$jse_number] = $jse_value;
	}
    }

    # If we got any events, send a command with the new settings
    if ($got_event)
    {
	# Throttle output is 0 to 255. Stick centre position is 0
	my $throttle = -$axis[1];
	$throttle = 0 if $throttle < 0;
	$throttle >>= 7;
	
	my $rudder = $axis[2];
	$rudder = int($rudder / 0x100);
	$rudder += 128;
	
	my $elevator = $axis[3];
	$elevator = int($elevator / 0x100);
	$elevator += 128;
	$elevator +=  $elevatortrim * 0x10;
	
	$ruddertrim = 0 if $ruddertrim < 0;
	$ruddertrim = 255 if $ruddertrim > 255;
	$elevator = 0 if $elevator < 0;
	$elevator = 255 if $elevator > 255;
	print "$throttle $rudder $elevator $ruddertrim\n";
    }
    # Wait a bit for any command to get transmitted
    # Dont want to overrun the receiver or to poll too hard
    # 13 chars at 9600baud = 13ms
    Time::HiRes::usleep($polltime);
}

sub usage
{
    print "$0 [-hv] [-j joystickdevice] [-p polltime]\n";
    exit;
}

sub version
{
    print "Version 1.0\n";
    exit;
}
