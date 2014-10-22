# *****************************************************************************
# *									      *
# *		 ESTIC Makefile for FreeBSD / X Window System		      *
# *									      *
# * (C) 1995-96  Ullrich von Bassewitz					      *
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

LIB	= ../spunk/spunk.a
INCDIR	= ../spunk

CFLAGS	= -DFREEBSD -g -Wall -I$(INCDIR) -x c++ -L /usr/X11R6/lib -fno-implicit-templates -DEXPLICIT_TEMPLATES


# ------------------------------------------------------------------------------
# Implicit rules

.c.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CC) $(CFLAGS) -c $<

# ------------------------------------------------------------------------------
# All OBJ files

OBJS =		callwin.o	\
		chargwin.o	\
		cliwin.o	\
		devstate.o	\
		estic.o		\
		icac.o		\
		icalias.o	\
		icbaseed.o	\
		iccli.o		\
		icconfig.o	\
		iccom.o		\
		iccprint.o	\
		iccron.o	\
		iccti.o		\
		icdevs.o	\
		icdiag.o	\
		icdlog.o	\
		icei.o		\
		icerror.o	\
		icfile.o	\
		icident.o	\
		icintcon.o	\
		iclog.o		\
		icmsgwin.o	\
		icprefix.o	\
		icshort.o	\
		icver.o		\
		istecmsg.o

# ------------------------------------------------------------------------------
#

all:	xestic

xestic: $(LIB) $(OBJS)
	$(CC) -o xestic $(OBJS) $(LIB) -ltermcap -lg++ -lX11


# ------------------------------------------------------------------------------
# Create a dependency file

depend dep:
	@echo "Creating dependency information"
	$(CC) -I$(INCDIR) -DFREEBSD -MM *.cc > .depend

# ------------------------------------------------------------------------------
# Create a ZIP file

strip:
	strip estic


zip:
	-rm -f estic.zip
	-rm -f *~
	cp Makefile make/freebsd-x.mak
	$(ZIP) -9 estic.zip *.cc *.h estic.res estic.ini *.doc *.chg make/*.mak alias.dat

bin-dist:	estic icload strip
	-rm -f estic.zip
	$(ZIP) -9 estic.zip estic estic.res estic.doc estic.chg estic.ini alias.dat

# ------------------------------------------------------------------------------
# clean up

clean:
	-rm -f *.bak *~

zap:	clean
	-rm -f *.o
	-rm -f .depend



