# *****************************************************************************
# *									      *
# *	     SPUNK MAKEFILE for a Solaris 2.[45] Unix SPARC system	      *
# *									      *
# * (C) 1993-96  Ullrich von Bassewitz					      *
# *		 Wacholderweg 14					      *
# *		 D-70597 Stuttgart					      *
# * EMail:	 uz@ibb.schwaben.com					      *
# *									      *
# *		 Portierung Sparc Solaris 2.5				      *
# *									      *
# *		 Martin Helmling					      *
# *		 Lindenhofstr. 78					      *
# *		 68163	 Mannheim					      *
# *		 email mh@guug.de  oder mh@octogon.de			      *
# *									      *
# *****************************************************************************



# $Id$
#
# $Log$
#
#



# ------------------------------------------------------------------------------
# Definitions

# Names of executables, assumes gcc is used
AS = gas
AR = ar
LD = ld
ZIP = zip
CC = g++

# Flags for the gnu compiler
CFLAGS	= -DSOLARIS -g -O2 -Wall -pipe -I /usr/openwin/include -fno-implicit-templates -DEXPLICIT_TEMPLATES

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
		console.o	\
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
		filecoll.o	\
		filepath.o	\
		filesel.o	\
		filesys.o	\
		frame.o		\
		fviewer.o	\
		inifile.o	\
		itemlbl.o	\
		itemwin.o	\
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
		scrmodes.h	\
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
	$(CC) -g -o resed $(RESEDITOBJS) $(LIB) -lg++ -lX11

lib:	$(LIB)

# ------------------------------------------------------------------------------
# Library

$(LIB):		$(OBJS)
	$(AR) r $(LIB) $?
	ranlib $(LIB)

depend dep:
	@echo "Creating dependency information"
	$(CC) -DSOLARIS -MM *.cc > .depend

# ------------------------------------------------------------------------------
# Target specific files

console.o:    xsrc/console.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

delay.o:	unixsrc/delay.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

filesys.o:	unixsrc/filesys.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

nlsinit.o:	unixsrc/nlsinit.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

screen.o:	xsrc/screen.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<

sercom.o:     unixsrc/sercom.cc $(HDRS)
	$(CC) $(CFLAGS) -c $<


# ------------------------------------------------------------------------------
# Create a ZIP file

zip:
	-rm -f spunk.zip
	-rm -f *.bak *~
	$(ZIP) $(ZIPFILE) *.cc *.h bccdos.cfg bccos2.cfg baseres.res resed.res
	-cp Makefile make/solaris-x.mak
	$(ZIP) $(ZIPFILE) spunk.chg make/*
	$(ZIP) $(ZIPFILE) dossrc/*.cc dossrc/*.asm
	$(ZIP) $(ZIPFILE) dos32src/*.cc dos32src/*.asm djgppsrc\*.cc
	$(ZIP) $(ZIPFILE) linuxsrc/*.cc os2src/*.cc bsdsrc/*.cc unixsrc/*.cc
	$(ZIP) $(ZIPFILE) xsrc/*.cc
	$(ZIP) $(ZIPFILE) doc/*.doc support/* data/*

# ------------------------------------------------------------------------------
# clean up

clean:
	-rm -f *~ linuxsrc/*~ bsdsrc/*~ unixsrc/*~

zap:	clean
	-rm -f *.o
	-rm -f .depend




