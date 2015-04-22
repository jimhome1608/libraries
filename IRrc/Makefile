# Makefile
#
# Makefile for the Arduino IRrc project
#
# Author: Mike McCauley (mikem@open.com.au)
# Copyright (C) 2010 Mike McCauley
# $Id: Makefile,v 1.1 2010/07/18 21:34:33 mikem Exp mikem $

PROJNAME = IRrc
# Dont forget to also change the version at the top of RCRx.h:
DISTFILE = $(PROJNAME)-1.3.zip

all:	doxygen dist upload

doxygen: 
	doxygen project.cfg

ci:
	ci -l `cat MANIFEST`

dist:	
	(cd ..; zip $(PROJNAME)/$(DISTFILE) `cat $(PROJNAME)/MANIFEST`)

upload:
	rsync -avz $(DISTFILE) doc/ server2:/var/www/html/mikem/arduino/$(PROJNAME)

#	scp $(DISTFILE) doc/*.html doc/*.gif doc/*.png doc/*.css doc/*.pdf server2:/var/www/html/mikem/arduino/$(PROJNAME)
