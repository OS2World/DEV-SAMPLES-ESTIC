# *****************************************************************************
# *                                                                           *
# *                SPUNK MAKEFILE for Borland-C (DOS & OS/2)                  *
# *                                                                           *
# * (C) 1993-96  Ullrich von Bassewitz                                        *
# *              Zwehrenbuehlstrasse 33                                       *
# *              D-72070 Tuebingen                                            *
# * EMail:       uz@ibb.schwaben.com                                          *
# *                                                                           *
# *****************************************************************************



# $Id$
#
# $Log$
#
#



# BEWARE: This makefile is no longer up to date!!!


# ------------------------------------------------------------------------------
# Generelle Einstellungen

.AUTODEPEND
.SUFFIXES       .ASM .C .CC .CPP
.SWAP

# ------------------------------------------------------------------------------
# Allgemeine Definitionen

# Names of executables
CC = BCC
AS = TASM
AR = TLIB
LD = TLINK
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

!if !$d(LIBDIR)
LIBDIR  = $(TARGET)
!endif

LIB     = $(TARGET)\spunk.lib
.PRECIOUS $(LIB)

!if $d(INCDIR)
INCDIR  = .
!endif

!if !$d(CCCFG)
CCCFG   = BCC$(TARGET).CFG
!endif

# ------------------------------------------------------------------------------
# Implicit rules

.c.obj:
  $(CC) +$(CCCFG) -I$(INCDIR) -n$(TARGET) -c {$< }

.cc.obj:
  $(CC) +$(CCCFG) -P -I$(INCDIR) -n$(TARGET) -c {$< }

.asm.obj:
  $(AS) -Mx $*.asm,$*.obj

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
                sercom.obj      \
                serstrm.obj     \
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
                textstrm.obj    \
                thread.obj      \
                winattr.obj     \
                window.obj      \
                winmgr.obj      \
                winsize.obj


#
# Additional target specific modules
#
!if $(TARGET)==DOS
XOBJS   =       _sercom.obj
!endif



.PRECIOUS $(OBJS:.obj=.cc) $(XOBJS:.obj=.asm) $(LIB)



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
# Make the needed directories

libdir:
        @- if not exist $(LIBDIR) mkdir $(LIBDIR)

# ------------------------------------------------------------------------------
# Resource editor

$(TARGET)\resed.exe:  $(LIB) $(RESEDITOBJS)
                $(CC) +$(CCCFG) -l-yx -eresed.exe @&&|
$(TARGET)\resed.obj
$(TARGET)\resedit.obj
$(TARGET)\resfile.obj
$(TARGET)\resitem.obj
$(TARGET)\resprint.obj
$(TARGET)\resutil.obj
$(TARGET)\reswin.obj
$(LIB)
|

# ------------------------------------------------------------------------------
# File-Transfer program

filetran:       $(TARGET)\filetran.exe

$(TARGET)\filetran.exe: lib filetran.obj
                $(CC) +$(CCCFG) -l-yx -efiletran.exe @&&|
$(TARGET)\filetran.obj
$(LIB)
|


# ------------------------------------------------------------------------------
# Library

lib:            $(LIB)

!if $(TARGET)==OS2
$(LIB):         $(OBJS) $(XOBJS)
        @echo Creating librarian job file
        -@if exist lib.job del /Q lib.job
        &@echo +-$? ^& >> lib.job
        @echo *dummy >> lib.job
        $(AR) /P64 $(LIB) @lib.job
        -@del /Q lib.job
!else
$(LIB):         $(OBJS) $(XOBJS)
        @echo Creating librarian job file
        -@if exist lib.job del /Q lib.job
        &@echo +-$? & >> lib.job
        @echo *dummy >> lib.job
        $(AR) /P64 $(LIB) @lib.job
        -@del /Q lib.job
!endif

# ------------------------------------------------------------------------------
# OS specific modules

filesys.obj:    $(TARGET)SRC\filesys.cc
        $(CC) +$(CCCFG) -P -I$(INCDIR) -n$(TARGET) -c $**

kbd.obj:        $(TARGET)SRC\kbd.cc
        $(CC) +$(CCCFG) -P -I$(INCDIR) -n$(TARGET) -c $**

nlsinit.obj:    $(TARGET)SRC\nlsinit.cc
        $(CC) +$(CCCFG) -P -I$(INCDIR) -n$(TARGET) -c $**

screen.obj:     $(TARGET)SRC\screen.cc
        $(CC) +$(CCCFG) -P -I$(INCDIR) -n$(TARGET) -c $**

sercom.obj:     $(TARGET)SRC\sercom.cc
        $(CC) +$(CCCFG) -P -I$(INCDIR) -n$(TARGET) -c $**

_sercom.obj:    $(TARGET)SRC\_sercom.asm
        $(AS) -m3 -mx $**
        @$(MV)  $. $(TARGET)

user.obj:    $(TARGET)SRC\user.cc
        $(CC) +$(CCCFG) -P -I$(INCDIR) -n$(TARGET) -c $**

# ------------------------------------------------------------------------------
# create a ZIP file

zip:
        -del spunk.zip
        -del /S *.bak
        $(ZIP) $(ZIPFILE) *.cc *.h bccdos.cfg bccos2.cfg baseres.res resed.res
        -copy makefile make\borland.mak
        $(ZIP) $(ZIPFILE) copying.txt spunk.chg make\*.*
        $(ZIP) $(ZIPFILE) dossrc\*.cc dossrc\*.asm
        $(ZIP) $(ZIPFILE) dos32src\*.cc dos32src\*.asm djgppsrc\*.cc
        $(ZIP) $(ZIPFILE) linuxsrc\*.cc os2src\*.cc bsdsrc\*.cc unixsrc\*.cc
        $(ZIP) $(ZIPFILE) xsrc\*.cc samples\*.*
        $(ZIP) $(ZIPFILE) doc\*.doc support\*.* data\*.*

# ------------------------------------------------------------------------------
# clean up

clean:
        -del $(TARGET)\*.bak $(TARGET)SRC\*.bak *.bak

zap:    clean
        -del $(TARGET)\*.obj




