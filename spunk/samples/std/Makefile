#------------------------------------------------------------------------------
# Makefile for STD
#------------------------------------------------------------------------------
# User defined section
#------------------------------------------------------------------------------

COMP	= gcc
CPP	= $(COMP) -E
LD	= $(COMP)
INCLUDEDIRS= -Ispunk
DEFS	= -DLINUX
CFLAGS  = -g -O -Wall 
LIBS1	= spunk/spunk.a -ltermcap
LDFLAGS	=
#LDFLAGS= -s

#VPATH	= .
VPATH	= $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)  
srcdir	= $(VPATH)

# Common prefix for machine-independent installed files.
prefix	= /usr/local
# Common prefix for machine-dependent installed files.
exec_prefix = $(prefix)

instname = std
# Directory to install EARS in.
bindir	= $(exec_prefix)/bin
# Directory in which to install some other stuff if needed.
libdir	= $(exec_prefix)/lib
# Directory to install the Info files in.
infodir	= $(prefix)/info
# Directory to install the man page in.
mandir	= $(prefix)/man/man$(manext)
# Number to put on the man page filename.
manext	= 1

# Program to install EARS.
INSTALL_PROGRAM	= $(INSTALL)
# Program to install the man page.
INSTALL_DATA = $(INSTALL) -m 644
# Generic install program. Note that it is also used to create the destination
# directories. Only tested with GNU `install'.
INSTALL	= install

# Program to format Texinfo source into Info files.
MAKEINFO = makeinfo
# Program to format Texinfo source into DVI files.
TEXI2DVI = texi2dvi

# Programs to make tags files.
ETAGS	= etags
CTAGS	= ctags -tw

#------------------------------------------------------------------------------
# You should not need to change anything below this line.
#------------------------------------------------------------------------------

VERSION	= 0.6
dirname	= std-$(VERSION)

OBJS1	= std.o
SRCS1	= std.cc
HFILES	= std.h
ALLSRCS	= $(SRCS1) $(HFILES)
OTHER	= README* Makefile 

# some more rules that aren't builtin

.c.o:
	$(COMP) -c $(CFLAGS) $(INCLUDEDIRS) $(DEFS) -o $*.o $<

.cc.o:
	$(COMP) -c $(CFLAGS) $(INCLUDEDIRS) $(DEFS) -o $*.o $<

#------------------------------------------------------------------------------
# the main programs
#------------------------------------------------------------------------------

all:	  depend std

std: $(LIBS1) $(OBJS1)
	$(LD) $(LIBPATH) $(LDFLAGS) -o std $(OBJS1) $(LIBS1) -lefence

#-----------------------------------------------------------------------------
# Libraries
#-----------------------------------------------------------------------------
#------------------------------------------------------------------------------
# cleaning up
#------------------------------------------------------------------------------

deltrash:		# delete all these annoying files
	rm -f \#* *\# *~ *.orig *.rej core t a.out DEADJOE tmp .remind
	rm -f doc/*~ lib/*~ util/*~ contrib/*~ gmon.out bb.out

clean: deltrash		# everything except configuration
	rm -f *.o std .depend
	rm -rf $(dirname) 

mostlyclean: clean	# clean, but leave libs that take long to build

distclean: clean	# make it as if you had just unpacked the distribution
	rm -f TAGS tags

realclean: distclean	# delete everything that can't be build by any means

veryclean: realclean	# this is not GNU, but I'm used to it

#------------------------------------------------------------------------------
# TAGS etc.
#------------------------------------------------------------------------------

TAGS: $(ALLSRCS) $(MALLSRCS)
	$(ETAGS) $(ALLSRCS) $(MALLSRCS)

tags: $(ALLSRCS) $(MALLSRCS)
	$(CTAGS) $(ALLSRCS) $(MALLSRCS)

#------------------------------------------------------------------------------
# documentation, version control
#------------------------------------------------------------------------------

info:
	@echo "Not yet implemented."

dvi: info

ci:
	ci -u Makefile *.{cc,h}

#------------------------------------------------------------------------------
# packing a distribution and backup
#------------------------------------------------------------------------------

dist: # not exactly elegant, but it works for now :-)
	-chmod -R 777 $(dirname)
	rm -rf $(dirname)
	mkdir $(dirname)
# we can't rely on symbolic links
	ln $(ALLSRCS) $(OTHER) $(dirname)
	chmod 777 $(dirname)  # some old versions of tar need world writable
	tar czf ../std-$(VERSION).tar.gz $(dirname)
	rm -rf $(dirname)	

bindist: std     # assumes that binaries are built already
	strip std
	rm -f *~
	-chmod -R 777 $(dirname)
	rm -rf $(dirname)
	mkdir $(dirname)
# we can't rely on symbolic links
	ln std README COPYING $(dirname)
	chmod 777 $(dirname)  # some old versions of tar need world writable
	chmod 777 $(dirname)/std
	tar czf ../std-$(VERSION)-bin.tar.gz $(dirname)
	rm -rf $(dirname)	

archive := ../ST$(shell date +%y%m%d).TGZ

bkp: deltrash
	find . -maxdepth 1 -type f ! -perm +100 ! -name ".depend"\
	  ! -name "*.o" ! -name "COPYING" ! -name "*.a" ! -name "*.txt"\
	  ! -name "resed.*" >tmp
	tar cfTz $(archive) tmp 
	mcopy -n $(archive) a:
	mcopy -n $(archive) a:bkp
	rm $(archive) tmp
	mdir a:

rcsbkp:
	tar cfz std-rcs.tgz RCS/*
	mcopy -n std-rcs.tgz a:
	rm std-rcs.tgz
	mdir a:

#------------------------------------------------------------------------------
# dependencies
#------------------------------------------------------------------------------
ifeq (.depend,$(wildcard .depend))
include .depend
depend:
else
depend: $(SRCS1) 
	$(CPP) -M $(INCLUDEDIRS) $(DEFS) $(SRCS1) > .depend
endif
