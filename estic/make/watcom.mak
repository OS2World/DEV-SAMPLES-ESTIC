# *****************************************************************************
# *                                                                           *
# *                             ESTIC Makefile                                *
# *                                                                           *
# * (C) 1993-96  Ullrich von Bassewitz                                        *
# *              Wacholderweg 14                                              *
# *              D-70597 Stuttgart                                            *
# * EMail:       uz@ibb.schwaben.com                                          *
# *                                                                           *
# *****************************************************************************



# $Id$
#
#  $Log$
#
#



# ------------------------------------------------------------------------------
# Generelle Einstellungen

.AUTODEPEND
.SUFFIXES       .ASM .C .CC .CPP
.SWAP

# ------------------------------------------------------------------------------
# Allgemeine Definitionen

# Names of executables
AS = TASM
AR = WLIB
LD = WLINK
!if $d(__OS2__)
ZIP = zip
MV = c:\os2\4os2\4os2 /C MOVE /Q
!else
ZIP = pkzip
MV = mv
!endif


!if !$d(TARGET)
!if $d(__OS2__)
TARGET = OS2
!else
TARGET = DOS
!endif
!endif

LIBDIR= ..\spunk
INCDIR= ..\spunk


# target specific macros.
!if $(TARGET)==OS2

# --------------------- OS2 ---------------------
SYSTEM = os2v2
CC = WPP386
CCCFG  = -bm -bt=$(TARGET) -d$(TARGET) -i=$(INCDIR) -d2 -onatx -zp4 -5 -fpi87 -zq -w2 -ze

!elif $(TARGET)==DOS32

# -------------------- DOS4G --------------------
SYSTEM = dos4g
CC = WPP386
CCCFG  = -bt=$(TARGET) -d$(TARGET) -i=$(INCDIR) -d1 -onatx -zp4 -5 -fpi -zq -w2 -ze

!elif $(TARGET)==DOS

# --------------------- DOS ---------------------
SYSTEM = dos
CC = WPP
# Optimize for size when running under plain DOS, but use 286 code. Don't
# include ANY debugging code to make as many programs runable under plain DOS
# as possible.
CCCFG  = -bt=$(TARGET) -d$(TARGET) -dSPUNK_NODEBUG -i=$(INCDIR) -d1 -oailmns -s -zp2 -zc -2 -fp2 -ml -zq -w2 -ze -zt255

!elif $(TARGET)==NETWARE

# --------------------- NETWARE -------------------
SYSTEM = netware
CC = WPP386
CCCFG  = -bm -bt=$(TARGET) -d$(TARGET) -i=$(INCDIR) -d1 -onatx -zp4 -5 -fpi -zq -w2 -ze

!elif $(TARGET)==NT

# --------------------- NT ----------------------
SYSTEM = nt
CC = WPP386
CCCFG  = -bm -bt=$(TARGET) -d$(TARGET) -i=$(INCDIR) -d1 -onatx -zp4 -5 -fpi87 -zq -w2 -ze

!else
!error
!endif

LIB     = $(LIBDIR)\$(TARGET)\SPUNK.LIB

# ------------------------------------------------------------------------------
# Implicit rules

.c.obj:
  $(CC) $(CCCFG) $<

.cc.obj:
  $(CC) $(CCCFG) $<

# --------------------------------------------------------------------

all:            exe

exe:            istec

os2:
        $(MAKE) -DTARGET=OS2

nt:
        $(MAKE) -DTARGET=NT

dos32:
        $(MAKE) -DTARGET=DOS32

dos:
        $(MAKE) -DTARGET=DOS

istec:          estic.exe

# --------------------------------------------------------------------
# ESTIC

estic.exe:      callwin.obj     \
                chargwin.obj    \
                cliwin.obj      \
                devstate.obj    \
                estic.obj       \
                icac.obj        \
                icalias.obj     \
                icbaseed.obj    \
                iccli.obj       \
                icconfig.obj    \
                iccom.obj       \
                iccprint.obj    \
                iccron.obj      \
                iccti.obj       \
                icdevs.obj      \
                icdiag.obj      \
                icdlog.obj      \
                icei.obj        \
                icerror.obj     \
                icfile.obj      \
                icident.obj     \
                icintcon.obj    \
                iclog.obj       \
                icmsgwin.obj    \
                icshort.obj     \
                icver.obj       \
                istecmsg.obj
                @copy makefile make\watcom.mak > nul
                $(LD) system $(SYSTEM) @&&|
DEBUG all
NAME estic.exe
OPTION DOSSEG
OPTION STACK=32K
FILE callwin.obj
FILE chargwin.obj
FILE cliwin.obj
FILE devstate.obj
FILE estic.obj
FILE icac.obj
FILE icalias.obj
FILE icbaseed.obj
FILE iccli.obj
FILE icconfig.obj
FILE iccom.obj
FILE iccprint.obj
FILE iccron.obj
FILE iccti.obj
FILE icdevs.obj
FILE icdiag.obj
FILE icdlog.obj
FILE icei.obj
FILE icerror.obj
FILE icfile.obj
FILE icident.obj
FILE icintcon.obj
FILE iclog.obj
FILE icmsgwin.obj
FILE icshort.obj
FILE icver.obj
FILE istecmsg.obj
FILE ..\areacode\areacode.obj
LIBRARY $(LIB)
|

# ------------------------------------------------------------------------------
# ZIP File erzeugen

zip:
        -del estic.zip
        -del *.bak
        copy makefile make\watcom.mak
        $(ZIP) -9 estic.zip *.cc *.h estic.res estic.ini *.doc *.chg make\*.mak alias.dat

strip:
        -65535 wstrip estic.exe
        -65535 wstrip icload.exe

bin-dist:       exe strip
        -del estic.zip
        $(ZIP) -9 estic.zip estic.exe icload.exe estic.res estic.doc estic.chg estic.ini alias.dat

# ------------------------------------------------------------------------------
# Zeilenzahl

linecount:
        -wc -l *.cc *.

# ------------------------------------------------------------------------------
# Aufr umen

clean:
        -del *.bak

zap:    clean
        -del *.obj
        -del *.mbr
        -del *.dbr

