# *****************************************************************************
# *									      *
# *		     SPUNK MAKEFILE for DJGPP and GNU make		      *
# *									      *
# * (C) 1993-96  Ullrich von Bassewitz					      *
# *		 Zwehrenbuehlstrasse 33					      *
# *		 D-72070 Tuebingen					      *
# * EMail:	 uz@ibb.schwaben.com					      *
# *									      *
# *****************************************************************************



# $Id$
#
# $Log$
#
#



# ------------------------------------------------------------------------------
# Definitions

# Names of executables
AS = gas
AR = ar
LD = ld
ZIP = pkzip
CC = gcc

# Flags for the gnu compiler (use the second one for gcc >= 2.6.0)
#CFLAGS  = -DDOS32 -I. -g -O2 -Wall -x c++
CFLAGS	= -DDOS32 -I. -g -O2 -Wall -x c++ -fno-implicit-templates -DEXPLICIT_TEMPLATES

LIB	= spunk.a
ZIPFILE = spunk.zip

# ------------------------------------------------------------------------------
# Implicit rules

.c.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CC) $(CFLAGS) -c $<

# ------------------------------------------------------------------------------
# All SPUNK OBJ files

OBJS	=	bitset.o	\
		charset.o	\
		charstrm.o	\
		chartype.o	\
		check.o		\
		coll.o		\
		cont.o		\
		cpucvt.o	\
		crc16.o		\
		crc32.o		\
		crcccitt.o	\
		crcstrm.o	\
		datetime.o	\
		delay.o		\
		environ.o	\
		errlog.o	\
		event.o		\
		filecoll.o	\
		filepath.o	\
		filesel.o	\
		filesys.o	\
		frame.o		\
		fviewer.o	\
		inifile.o	\
		itemlbl.o	\
		itemwin.o	\
		kbd.o		\
		keydef.o	\
		keymap.o	\
		listnode.o	\
		memcheck.o	\
		memstrm.o	\
		menue.o		\
		menuedit.o	\
		menuitem.o	\
		msg.o		\
		msgcoll.o	\
		national.o	\
		nlsinit.o	\
		nullstrm.o	\
		object.o	\
		palette.o	\
		password.o	\
		program.o	\
		progutil.o	\
		rect.o		\
		rescoll.o	\
		resource.o	\
		rng.o		\
		screen.o	\
		serstrm.o	\
		settings.o	\
		splitmsg.o	\
		statline.o	\
		stdmenue.o	\
		stdmsg.o	\
		str.o		\
		strbox.o	\
		strcoll.o	\
		strcvt.o	\
		stream.o	\
		strmable.o	\
		strparse.o	\
		strpool.o	\
		syserror.o	\
		textstrm.o	\
		thread.o	\
		winattr.o	\
		window.o	\
		winmgr.o	\
		winsize.o

# ------------------------------------------------------------------------------
# All SPUNK header files

HDRS	=	bitset.h	\
		statdef.h	\
		charset.h	\
		chartype.h	\
		check.h		\
		circbuf.h	\
		coll.h		\
		cont.h		\
		crc.h		\
		crcstrm.h	\
		datetime.h	\
		errlog.h	\
		event.h		\
		filecoll.h	\
		filepath.h	\
		filesel.h	\
		filesys.h	\
		fviewer.h	\
		inifile.h	\
		itemwin.h	\
		kbd.h		\
		keydef.h	\
		keymap.h	\
		listbox.h	\
		listnode.h	\
		machine.h	\
		mempool.h	\
		memstrm.h	\
		menue.h		\
		menuedit.h	\
		menuitem.h	\
		msg.h		\
		msgcoll.h	\
		msgid.h		\
		national.h	\
		nullstrm.h	\
		object.h	\
		palette.h	\
		password.h	\
		program.h	\
		progutil.h	\
		rect.h		\
		rescoll.h	\
		resed.h		\
		resource.h	\
		screen.h	\
		sercom.h	\
		settings.h	\
		splitmsg.h	\
		stack.h		\
		statline.h	\
		stdmenue.h	\
		stdmsg.h	\
		str.h		\
		strcoll.h	\
		strcvt.h	\
		stream.h	\
		streamid.h	\
		strmable.h	\
		strparse.h	\
		strpool.h	\
		textstrm.h	\
		thread.h	\
		winattr.h	\
		window.h	\
		winflags.h	\
		winmgr.h	\
		winsize.h

# ------------------------------------------------------------------------------
# All resedit OBJ files

RESEDITOBJS	=	resed.o		\
			resedit.o	\
			resfile.o	\
			resitem.o	\
			resprint.o	\
			resutil.o	\
			reswin.o

# ------------------------------------------------------------------------------
# Dummy targets

resed:	$(LIB) $(RESEDITOBJS) $(HDRS)
	$(CC) -g -o resed $(RESEDITOBJS) $(LIB) -lg -lgpp -lpc

lib:	$(LIB)

# ------------------------------------------------------------------------------
# Library

$(LIB):		$(OBJS)
	$(AR) rs $(LIB) $?

depend dep:
	@echo "Creating dependency information"
	$(CC) -DDOS32 -MM *.cc > .depend

# ------------------------------------------------------------------------------
# Target specific files

delay.o:	dossrc/delay.cc $(HDRS)
	$(CC) $(CFLAGS) -c dossrc/delay.cc

filesys.o:	dossrc/filesys.cc $(HDRS)
	$(CC) $(CFLAGS) -c dossrc/filesys.cc

kbd.o:		dossrc/kbd.cc $(HDRS)
	$(CC) $(CFLAGS) -c dossrc/kbd.cc

nlsinit.o:	djgppsrc/nlsinit.cc $(HDRS)
	$(CC) $(CFLAGS) -c djgppsrc/nlsinit.cc

screen.o:	dossrc/screen.cc $(HDRS)
	$(CC) $(CFLAGS) -c dossrc/screen.cc

# ------------------------------------------------------------------------------
# Create a ZIP file

zip:
	-del spunk.zip
	-del /S *.bak
	$(ZIP) $(ZIPFILE) *.cc *.h bccdos.cfg bccos2.cfg baseres.res resed.res
	-copy makefile make\djgpp.mak
	$(ZIP) $(ZIPFILE) copying.txt spunk.chg make\*.*
	$(ZIP) $(ZIPFILE) dossrc\*.cc dossrc\*.asm
	$(ZIP) $(ZIPFILE) dos32src\*.cc dos32src\*.asm djgppsrc\*.cc
	$(ZIP) $(ZIPFILE) linuxsrc\*.cc os2src\*.cc bsdsrc\*.cc unixsrc\*.cc
	$(ZIP) $(ZIPFILE) xsrc\*.cc samples\*.*
	$(ZIP) $(ZIPFILE) doc\*.doc support\*.* data\*.*

# ------------------------------------------------------------------------------
# clean up

clean:
	-del /S *.bak

zap:	clean
	-del *.o




