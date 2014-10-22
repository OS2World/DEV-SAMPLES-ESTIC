# *****************************************************************************
# *                                                                           *
# *        SPUNK MAKEFILE for DOS, OS/2 and NT using the Watcom Compiler      *
# *                                                                           *
# * (C) 1993-96  Ullrich von Bassewitz                                        *
# *              Wacholderweg 14                                              *
# *              D-70597 Stuttgart                                            *
# * EMail:       uz@ibb.schwaben.com                                          *
# *                                                                           *
# *****************************************************************************



# $Id$
#
# $Log$
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

# Files
ZIPFILE = spunk.zip

# Tools
!if $d(__OS2__)
ZIP = zip
!else
ZIP = pkzip
!endif


!if !$d(TARGET)
!if $d(__OS2__)
TARGET = OS2
!else
TARGET = DOS
!endif
!endif

# target specific macros.
!if $(TARGET)==OS2

# --------------------- OS2 ---------------------
SYSTEM = os2v2
CC = WPP386
CCCFG  = -bm -bt=$(TARGET) -fo=$(TARGET)\ -d$(TARGET) -d1 -onatx -zp4 -5 -fpi87 -zq -w2

!elif $(TARGET)==DOS32

# -------------------- DOS4G --------------------
SYSTEM = dos4g
CC = WPP386
CCCFG  = -bt=$(TARGET) -fo=$(TARGET)\ -d$(TARGET) -d1 -onatx -zp4 -5 -fpi -zq -w2

!elif $(TARGET)==DOS

# --------------------- DOS ---------------------
SYSTEM = dos
CC = WPP
# Optimize for size when running under plain DOS, but use 286 code. Don't
# include ANY debugging code to make as many programs runable under plain DOS
# as possible.
CCCFG  = -bt=$(TARGET) -fo=$(TARGET)\ -d$(TARGET) -dSPUNK_NODEBUG -d1 -oailmns -s -zp2 -zc -2 -fp2 -ml -zq -w2 -zt255

!elif $(TARGET)==NETWARE

# --------------------- NETWARE -------------------
SYSTEM = netware
CC = WPP386
CCCFG  = -bm -bt=$(TARGET) -fo=$(TARGET)\ -d$(TARGET) -d1 -onatx -zp4 -5 -fpi -zq -w2

!elif $(TARGET)==NT

# --------------------- NT ----------------------
SYSTEM = nt
CC = WPP386
CCCFG  = -bm -bt=$(TARGET) -fo=$(TARGET)\ -d$(TARGET) -d1 -onatx -zp4 -5 -fpi87 -zq -w2

!else
!error
!endif

!if !$d(LIBDIR)
LIBDIR  = $(TARGET)
!endif

LIB     = $(LIBDIR)\spunk.lib
.PRECIOUS $(LIB)

# ------------------------------------------------------------------------------
# Implicit rules

.c.obj:
  $(CC) $(CCCFG) $<

.cc.obj:
  $(CC) $(CCCFG) $<

.asm.obj:
  $(AS) -Mx $*.asm,$(TARGET)\$*.obj

.path.obj       = $(TARGET)

# ------------------------------------------------------------------------------
# All SPUNK OBJ files

OBJS    =       bitset.obj      \
                charset.obj     \
                charstrm.obj    \
                chartype.obj    \
                check.obj       \
                coll.obj        \
                cont.obj        \
                cpucvt.obj      \
                crc16.obj       \
                crc32.obj       \
                crcccitt.obj    \
                crcstrm.obj     \
                datetime.obj    \
                delay.obj       \
                environ.obj     \
                errlog.obj      \
                event.obj       \
                filecoll.obj    \
                filepath.obj    \
                filesel.obj     \
                filesys.obj     \
                frame.obj       \
                fviewer.obj     \
                inifile.obj     \
                itemlbl.obj     \
                itemwin.obj     \
                kbd.obj         \
                keydef.obj      \
                keymap.obj      \
                listnode.obj    \
                memcheck.obj    \
                memstrm.obj     \
                menue.obj       \
                menuedit.obj    \
                menuitem.obj    \
                msg.obj         \
                msgcoll.obj     \
                national.obj    \
                nlsinit.obj     \
                nullstrm.obj    \
                object.obj      \
                palette.obj     \
                password.obj    \
                program.obj     \
                progutil.obj    \
                rect.obj        \
                rescoll.obj     \
                resource.obj    \
                rng.obj         \
                screen.obj      \
                serstrm.obj     \
                settings.obj    \
                splitmsg.obj    \
                statline.obj    \
                stdmenue.obj    \
                stdmsg.obj      \
                str.obj         \
                strbox.obj      \
                strcoll.obj     \
                strcvt.obj      \
                stream.obj      \
                strmable.obj    \
                strparse.obj    \
                strpool.obj     \
                syserror.obj    \
                textitem.obj    \
                textstrm.obj    \
                thread.obj      \
                wildargs.obj    \
                winattr.obj     \
                window.obj      \
                winmgr.obj      \
                winsize.obj


#
# Additional target specific modules
#
!if $(TARGET)==DOS
XOBJS   =       sercom.obj      \
                _sercom.obj
!endif

!if $(TARGET)==DOS32
XOBJS   =       sercom.obj      \
                _sercom.obj
!endif

!if $(TARGET)==OS2
XOBJS   =       sema.obj        \
                sercom.obj      \
                task.obj
!endif

!if $(TARGET)==NT
XOBJS   =       sema.obj        \
                sercom.obj      \
                task.obj
!endif


.PRECIOUS $(OBJS:.obj=.cc) $(LIB)



# ------------------------------------------------------------------------------
# All resedit OBJ files

RESEDITOBJS     =       resed.obj       \
                        resedit.obj     \
                        resfile.obj     \
                        resitem.obj     \
                        resprint.obj    \
                        resutil.obj     \
                        reswin.obj

# ------------------------------------------------------------------------------
# Dummy targets. Beware: resed must be the default target

resed:  $(TARGET)\resed.exe

all:    dos dos32 os2 netware nt

libs:   doslib dos32lib os2lib netwarelib ntlib

lib:    libdir $(LIB)

dos:
        $(MAKE) -DTARGET=DOS

os2:
        $(MAKE) -DTARGET=OS2

dos32:
        $(MAKE) -DTARGET=DOS32

netware:
        $(MAKE) -DTARGET=NETWARE resed.nlm

nt:
        $(MAKE) -DTARGET=NT

doslib:
        $(MAKE) -DTARGET=DOS lib

os2lib:
        $(MAKE) -DTARGET=OS2 lib

dos32lib:
        $(MAKE) -DTARGET=DOS32 lib

netwarelib:
        $(MAKE) -DTARGET=NETWARE lib

ntlib:
        $(MAKE) -DTARGET=NT lib


# ------------------------------------------------------------------------------
# Make the needed directories

libdir:
        @- if not exist $(LIBDIR) mkdir $(LIBDIR) > nul

# ------------------------------------------------------------------------------
# Resource editor

$(TARGET)\resed.exe:    lib $(RESEDITOBJS)
        $(LD) system $(SYSTEM) @&&|
DEBUG all
NAME $(TARGET)\resed.exe
OPTION DOSSEG
OPTION STACK=32K
FILE $(TARGET)\resed.obj
FILE $(TARGET)\resedit.obj
FILE $(TARGET)\resfile.obj
FILE $(TARGET)\resitem.obj
FILE $(TARGET)\resprint.obj
FILE $(TARGET)\resutil.obj
FILE $(TARGET)\reswin.obj
LIBRARY $(LIB)
|


# Netware: Add this one if needed
# MODULE aio
# IMPORT @%WATCOM%\novi\aio.imp

resed.nlm:    lib $(RESEDITOBJS)
        $(LD) system $(SYSTEM) @&&|
DEBUG all
NAME $(TARGET)\resed.nlm
OPTION DOSSEG
OPTION STACK=32K
FILE $(TARGET)\resed.obj
FILE $(TARGET)\resedit.obj
FILE $(TARGET)\resfile.obj
FILE $(TARGET)\resitem.obj
FILE $(TARGET)\resprint.obj
FILE $(TARGET)\resutil.obj
FILE $(TARGET)\reswin.obj
LIBRARY $(LIB)
|


# ------------------------------------------------------------------------------
# File-Transfer program

filetran:       $(TARGET)\filetran.exe

$(TARGET)\filetran.exe: lib filetran.obj
        $(LD) system $(SYSTEM) @&&|
DEBUG all
NAME $(TARGET)\filetran.exe
OPTION DOSSEG
OPTION STACK=32K
FILE $(TARGET)\filetran.obj
LIBRARY $(LIB)
|

# ------------------------------------------------------------------------------
# Library

$(LIB): $(OBJS) $(XOBJS)
        -@copy makefile make\watcom.mak > nul
        @echo Creating library...
        &@$(AR) -q -b -P=128 $(LIB) +-$?
        @echo Done!

# ------------------------------------------------------------------------------
# OS specific modules

delay.obj:      $(TARGET)SRC\delay.cc
        $(CC) $(CCCFG) $**

filesys.obj:    $(TARGET)SRC\filesys.cc
        $(CC) $(CCCFG) $**

kbd.obj:        $(TARGET)SRC\kbd.cc
        $(CC) $(CCCFG) $**

nlsinit.obj:    $(TARGET)SRC\nlsinit.cc
        $(CC) $(CCCFG) $**

screen.obj:     $(TARGET)SRC\screen.cc
        $(CC) $(CCCFG) $**

sema.obj:       $(TARGET)SRC\sema.cc
        $(CC) $(CCCFG) $**

sercom.obj:     $(TARGET)SRC\sercom.cc
        $(CC) $(CCCFG) $**

_sercom.obj:    $(TARGET)SRC\_sercom.asm
        $(AS) -m3 -mx -zi $(TARGET)src\_sercom.asm,$(TARGET)\_sercom.obj

task.obj:       $(TARGET)SRC\task.cc
        $(CC) $(CCCFG) $**

# ------------------------------------------------------------------------------
# create a ZIP file

zip:
        -del spunk.zip
        -del /S *.bak
        $(ZIP) $(ZIPFILE) *.cc *.h bccdos.cfg bccos2.cfg baseres.res resed.res
        -copy makefile make\watcom.mak
        $(ZIP) $(ZIPFILE) copying.txt todo.txt spunk.chg make\*.*
        $(ZIP) $(ZIPFILE) dossrc\*.cc dossrc\*.asm
        $(ZIP) $(ZIPFILE) dos32src\*.cc dos32src\*.asm djgppsrc\*.cc
        $(ZIP) $(ZIPFILE) linuxsrc\*.cc os2src\*.cc bsdsrc\*.cc unixsrc\*.cc
        $(ZIP) -r $(ZIPFILE) ntsrc\*.cc xsrc\*.cc samples\*
        $(ZIP) $(ZIPFILE) doc\*.doc nls\*.*

# ------------------------------------------------------------------------------
# clean up

clean:
        -del /S *.bak

zap:    clean
        -del $(TARGET)\*.obj



