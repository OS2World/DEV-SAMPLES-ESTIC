# *****************************************************************************
# *									      *
# *		     SPUNK MAKEFILE for FreeBSD using g++		      *
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



# ------------------------------------------------------------------------------
# Definitions

# Names of executables
AS = gas
AR = ar
LD = ld
ZIP = zip
CC = g++

# Flags for the gnu compiler
CFLAGS	= -DFREEBSD -DUSE_OLD_TTY -g -Wall -x c++ -fno-implicit-templates -DEXPLICIT_TEMPLATES

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

resed:	$(LIB) $(RESEDITOBJS)
	$(CC) -g -o resed $(RESEDITOBJS) $(LIB) -ltermcap -lg++

lib:	$(LIB)

# ------------------------------------------------------------------------------
# Library

$(LIB):		$(OBJS)
	$(AR) r $(LIB) $?
	ranlib $(LIB)

depend dep:
	@echo "Creating dependency information"
	$(CC) -DFREEBSD -MM *.cc > .depend

# ------------------------------------------------------------------------------
# Target specific files

delay.o:	unixsrc/delay.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

filesys.o:	unixsrc/filesys.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

kbd.o:		bsdsrc/kbd.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

nlsinit.o:	unixsrc/nlsinit.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

screen.o:	bsdsrc/screen.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

screen2.o:	unixsrc/screen2.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

sercom.o:	unixsrc/sercom.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

# ------------------------------------------------------------------------------
# Create a ZIP file

zip:
	-rm -f spunk.zip
	-rm -f *.bak *~
	$(ZIP) $(ZIPFILE) *.cc *.h bccdos.cfg bccos2.cfg baseres.res resed.res
	-cp Makefile make/freebsd.mak
	$(ZIP) $(ZIPFILE) copying.txt todo.txt spunk.chg make/*
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




