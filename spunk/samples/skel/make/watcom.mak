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

LIBDIR= ..\..
INCDIR= ..\..


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

all:    skel

skel:   skel.exe

os2:
        $(MAKE) -DTARGET=OS2

nt:
        $(MAKE) -DTARGET=NT

dos32:
        $(MAKE) -DTARGET=DOS32

dos:
        $(MAKE) -DTARGET=DOS

# --------------------------------------------------------------------
# ESTIC

skel.exe:       skel.obj
                @copy makefile make\watcom.mak > nul
                $(LD) system $(SYSTEM) @&&|
DEBUG all
NAME skel.exe
OPTION DOSSEG
OPTION STACK=32K
FILE skel.obj
LIBRARY $(LIB)
|

# ------------------------------------------------------------------------------
# Aufr umen

clean:
        -del *.bak

zap:    clean
        -del *.obj
        -del *.mbr
        -del *.dbr

