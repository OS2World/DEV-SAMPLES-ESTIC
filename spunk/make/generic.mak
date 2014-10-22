# *****************************************************************************
# *									      *
# *		   SPUNK MAKEFILE for a generic Unix system		      *
# *									      *
# * (C) 1993-96  Ullrich von Bassewitz					      *
# *		 Wacholderweg 14					      *
# *		 D-70597 Stuttgart					      *
# * EMail:	 uz@ibb.schwaben.com					      *
# *									      *
# *****************************************************************************



# $Id$
#
# $Log$
#
#



# This a generic unix makefile. "generic" does not mean that there's nothing
# to change.
# This makefile has no rule for the sercom module, since this module needs
# changes on probably every architecture.



# ------------------------------------------------------------------------------
# Definitions

# Names of executables, assumes gcc is used
AS = gas
AR = ar
LD = ld
ZIP = zip
CC = g++

# Flags for the gnu compiler
CFLAGS	= -DGENERIC_UNIX -g -O2 -Wall -pipe -x c++ -fno-implicit-templates -DEXPLICIT_TEMPLATES

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
		screen2.o	\
		sercom.o	\
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
		textitem.o	\
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
	$(CC) -g -o resed $(RESEDITOBJS) $(LIB) -ltermcap -lg++

lib:	$(LIB)

# ------------------------------------------------------------------------------
# Library

$(LIB):		$(OBJS)
	$(AR) r $(LIB) $?
	ranlib $(LIB)

depend dep:
	@echo "Creating dependency information"
	$(CC) -DGENERIC_UNIX -MM *.cc > .depend

# ------------------------------------------------------------------------------
# Target specific files

delay.o:	unixsrc/delay.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

filesys.o:	unixsrc/filesys.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

kbd.o:		unixsrc/kbd.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

nlsinit.o:	unixsrc/nlsinit.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

screen.o:	unixsrc/screen.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

screen2.o:	unixsrc/screen2.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

# ------------------------------------------------------------------------------
# Create a ZIP file

zip:
	-rm -f spunk.zip
	-rm -f *.bak *~
	$(ZIP) $(ZIPFILE) *.cc *.h bccdos.cfg bccos2.cfg baseres.res resed.res
	-cp Makefile make/generic.mak
	$(ZIP) $(ZIPFILE) copying.txt spunk.chg make/*
	$(ZIP) $(ZIPFILE) dossrc/*.cc dossrc/*.asm
	$(ZIP) $(ZIPFILE) dos32src/*.cc dos32src/*.asm djgppsrc\*.cc
	$(ZIP) $(ZIPFILE) linuxsrc/*.cc os2src/*.cc bsdsrc/*.cc unixsrc/*.cc
	$(ZIP) $(ZIPFILE) xsrc/*.cc samples/*
	$(ZIP) $(ZIPFILE) doc/*.doc support/* data/*

# ------------------------------------------------------------------------------
# clean up

clean:
	-rm -f *~ linuxsrc/*~ bsdsrc/*~ unixsrc/*~

zap:	clean
	-rm -f *.o
	-rm -f .depend




