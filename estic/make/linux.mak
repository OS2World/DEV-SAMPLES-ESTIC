# *****************************************************************************
# *									      *
# *			   ESTIC Makefile for Linux			      *
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

# Flags for the GNU C compiler
CFLAGS	= -DLINUX -g -O2 -Wall -I$(INCDIR) -fno-implicit-templates -DEXPLICIT_TEMPLATES

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
		icshort.o	\
		icver.o		\
		imon.o		\
		istecmsg.o

ACOBJ =		../areacode/areacode.o

# ------------------------------------------------------------------------------
#

ifeq (.depend,$(wildcard .depend))
all:	estic
include .depend
else
all:	depend
endif


estic:	$(LIB) $(OBJS)
	$(CC) -o estic $(OBJS) $(ACOBJ) $(LIB) -lncurses -lg++

# ------------------------------------------------------------------------------
# Create a dependency file

depend dep:
	@echo "Creating dependency information"
	$(CC) -I$(INCDIR) -DLINUX -MM *.cc > .depend

# ------------------------------------------------------------------------------
# Create a ZIP file

strip:
	strip estic


zip:
	-rm -f estic.zip
	-rm -f *~
	cp Makefile linux.mak
	$(ZIP) -9 estic.zip *.cc *.h estic.res estic.ini *.doc *.chg *.mak alias.dat

bin-dist:	estic icload strip
	-rm -f estic.zip
	$(ZIP) -9 estic.zip estic estic.res estic.doc estic.chg estic.ini alias.dat

convert:
	dos2iso -f *.cc *.h *.doc *.dat estic.ini estic.chg make/*.mak


# ------------------------------------------------------------------------------
# clean up

clean:
	-rm *.bak *~

zap:	clean
	-rm *.o
	-rm .depend


