; *****************************************************************************
; *                                                                           *
; *                               _SERCOM.ASM                                 *
; *                                                                           *
; * (C) 1991-96  Ullrich von Bassewitz                                        *
; *              Zwehrenbuehlstrasse 33                                       *
; *              D-72070 Tuebingen                                            *
; * EMail:       uz@ibb.schwaben.com                                          *
; *                                                                           *
; *****************************************************************************



; $Id$
;
; $Log$
;
;



; This is a simple module for serial communication under DOS based on a module
; called USER. The module supports up to four ports, 16550s and the "high"
; interrupts. DOS4G version.
; Comments are in german, sorry.



; ***************************************************************************



IDEAL                   ; IDEAL-Modus von TASM einschalten
JUMPS                   ; Sprungoptimierung
LOCALS                  ; Lokale Symbole zulassen
SMART                   ; "Schlauen" Modus einschalten
P386                    ; 80386 Protected Mode erlauben
MODEL FLAT              ; Im FLAT Modell Åbersetzen




; -------------------------------------------------------------------
;
; Zeichen fÅr Flow-Control bei XON/XOFF
;
XON             =       11H             ; XON
XOFF            =       13H             ; XOFF

; -------------------------------------------------------------------
;
; Struktur fÅr die Daten eines jeden seriellen Ports. Von diesen Strukturen
; ist fÅr jeden unterstÅtzten Port eine vorhanden.
;

STRUC   PortDesc

        ; Allgemeine Einstellungen

        Baudrate        dd      ?       ; Baudrate
        Connection      db      ?       ; M)odem, D)irect
        Parity          db      ?       ; N)one, O)dd, E)ven, S)pace, M)ark
        StopBits        db      ?       ; 1, 2
        DataBits        db      ?       ; 5..8
        XonXoff         db      ?       ; E)nabled, D)isabled
        ALIGN           4               ; Fill auf gerade Adressen

        ; Ringpuffer fÅr Senden und Empfang incl Verwaltung

        RXBuf           dd      ?       ; Zeiger auf Empfangspuffer
        TXBuf           dd      ?       ; Zeiger auf Sendepuffer
        RXBufSize       dd      ?       ; Grî·e Empfangspuffer
        TXBufSize       dd      ?       ; Grî·e Sendepuffer
        RXStart         dd      0       ; Ringpufferzeiger
        RXEnd           dd      0       ;
        TXStart         dd      0
        TXEnd           dd      0
        RXCount         dd      0       ; Anzahl Zeichen im Puffer
        TXCount         dd      0       ; Anzahl Zeichen im Puffer

        ; FehlerzÑhler

        ERXOverflow     dw      0       ; PufferÅberlauf beim Empfang
        ETXOverflow     dw      0       ; PufferÅberlauf beim Senden
        EOvrRun         dw      0       ; öberlauf beim EmpfÑnger
        EBreak          dw      0       ; Break
        EFrame          dw      0       ; Framing Fehler
        EParity         dw      0       ; Parity Fehler
        ErrorBlock      EQU     ERXOverFlow

        ; Interna

        Installed       db      0       ; 1 wenn installiert
        IntNr           db      ?       ; Interrupt-Nummer
        IC1Mask         db      ?       ; Maske fÅr 8259A #1
        NotIC1Mask      db      ?       ; Komplement
        IC2Mask         db      ?       ; Maske fÅr 8259A #2
        NotIC2Mask      db      ?       ; Komplement
        FIFOLen         db      ?       ; Chunk-Size fÅr Sender-FIFO
        ParityMask      db      ?       ; 7Fh wenn Parity, sonst 0FFh
        ALIGN           4               ; Make it even

        ; Adressen der Chip-Register

        DataReg         dw      ?       ; data register
        DLL             EQU     DataReg ; divisor low latch
        IER             dw      ?       ; interrupt enable register
        DLH             EQU     IER     ; divisor high latch
        IIR             dw      ?       ; interrupt id register (lesen)
        FCR             EQU     IIR     ; FIFO control register (schreiben)
        LCR             dw      ?       ; line control register
        MCR             dw      ?       ; modem control register
        LSR             dw      ?       ; line status register
        MSR             dw      ?       ; modem status register
        ALIGN           4

        IntHandler      dd      ?       ; Offset des Interrupt-Handlers
        OldVecOffs      dd      ?       ; Offset of old interrupt vector
        OldVecSeg       dw      ?       ; Segment of old interrupt vector

        HostOff         db      0       ; Host Xoff'ed (1=ja, 0 = nein)
        PCOff           db      0       ; PC Xoff'ed (1=ja, 0=nein)
        MustSend        db      0       ; Es _mu·_ ein Byte gesendet werden
        DSR_CTS_Ok      db      ?       ; DSR und CTS sind beide On

ENDS    PortDesc


;
; Steuerbits fÅr das FCR des NS16550A.
;
FIFO_ENABLE     =       001H            ; FIFO einschalten
FIFO_CLR_RX     =       002H            ; RX-FIFO lîschen
FIFO_CLR_TX     =       004H            ; TX-FIFO lîschen

FIFO_SZ_1       =       000H            ; Warning level:  1 Bytes
FIFO_SZ_4       =       040H            ; Warning level:  4 Bytes
FIFO_SZ_8       =       080H            ; Warning level:  8 Bytes
FIFO_SZ_14      =       0C0H            ; Warning level: 14 Bytes
FIFO_SZ_MASK    =       0C0H            ; Maske fÅr warning level

;
; FIFO Kommandos. In FIFO_INIT mu· eingetragen werden, nach wievielen Bytes
; ein Interrupt erzeugt wird. Unter DOS sind normalerweise 14 Bytes ok, bei
; langsamen Rechnern (oder langsamen EMM) und hohen Zeichengeschwindigkeiten
; mu· evtl. bereits nach 8 Zeichen ein Interrupt ausgelîst werden.
; Default ist Interrupt nach 8 Zeichen.
;
FIFO_CLEAR      =       FIFO_CLR_RX OR FIFO_CLR_TX
FIFO_INIT       =       FIFO_ENABLE OR FIFO_SZ_8 OR FIFO_CLR_RX OR FIFO_CLR_TX

;
; Sonstiges FIFO-Zeugs.
;

FIFO_ENABLED    =       0C0H            ; Bitmuster wenn FIFO an


;
; Bits im Modem Control Register
;

MCR_DTR         =       00000001B
MCR_RTS         =       00000010B
MCR_OUT2        =       00001000B



; -------------------------------------------------------------------
;
; Externe Prozeduren, die das Programmiersprachenmodul zur VerfÅgung
; stellen mu·.
;
;


EXTRN PASCAL _COMWAIT   : NEAR          ; Warte-Prozedur
EXTRN PASCAL _COMERROR  : NEAR          ; _ComError-Routine



; -------------------------------------------------------------------
;
; Vom Assembler-Modul exportierte Routinen.
;

PUBLIC  _COMINSTALL
PUBLIC  _COMDEINSTALL
PUBLIC  _COMOPEN
PUBLIC  _COMCLOSE
PUBLIC  _COMISINSTALLED
PUBLIC  _COMISOPEN
PUBLIC  _COMDTROFF
PUBLIC  _COMDTRON
PUBLIC  _COMRTSOFF
PUBLIC  _COMRTSON
PUBLIC  _COMRXCOUNT
PUBLIC  _COMRXSIZE
PUBLIC  _COMRXCLEAR
PUBLIC  _COMTXCOUNT
PUBLIC  _COMTXSIZE
PUBLIC  _COMTXFREE
PUBLIC  _COMTXCLEAR
PUBLIC  _COMRECEIVE
PUBLIC  _COMSEND
PUBLIC  _COMBREAK
PUBLIC  _COMMODEMSTATUS

; Interrupt-Handler
PUBLIC  _INTCOM1
PUBLIC  _INTCOM2
PUBLIC  _INTCOM3
PUBLIC  _INTCOM4

; ---------------------------------------------------------------------------
; Datensegment
;

DATASEG

EXTRN   C _ComPort1     : PortDesc
EXTRN   C _ComPort2     : PortDesc
EXTRN   C _ComPort3     : PortDesc
EXTRN   C _ComPort4     : PortDesc


; -------------------------------------------------------------------
;
; Beginn Codesegment
;

CODESEG

; -------------------------------------------------------------------------
;
; _ComInstall
;
; Installiert einen Port mit der Åbergebenen Portnummer. Bei Erfolg wird das
; Handle (immer ein Wert <> 0) zurÅckgeliefert, ansonsten der Wert 0 als
; Handle.
;

PROC    PASCAL _ComInstall NEAR PortHandle: DWORD
USES    ESI, EDI

        mov     esi, [PortHandle]               ; Porthandle holen

; esi zeigt ab hier auf einen Record vom Typ PortDesc

        test    [(PortDesc esi).Installed], 01h ; Bereits installiert
        jz      @@L2                            ; Nein: Ok

; Fehler, Port war bereits installiert

@@L1:   call    _ComError
        jmp     @@L3                            ; Fehler, -1 als Handle liefern

; Port ist noch nicht installiert. Port-Adresse aus dem Default-Array holen
; und prÅfen, ob im entsprechenden BIOS-Bereich eine andere Adresse steht.
; Wenn Ja, dann diese andere Adresse Åbernehmen, wenn Nein, oder wenn diese
; andere Adresse eine 0 ist, den Default behalten.
; Dann prÅfen, ob diese Adresse _irgendwo_ im BIOS-Bereich als Adresse
; eingetragen ist. Fehler wenn nein.
; Der Sinn dieses ganzen Hin und Her ist es, auch etwas ungewîhnliche
; Installationen zu unterstÅtzen, bei denen z.B. COM 3 fehlt aber COM 4
; vorhanden ist. Das BIOS packt diese Adressen, so da· der Bereich von COM 4
; leer ist und COM 3 die Adresse von COM 4 enthÑlt.

@@L2:   mov     ax, [(PortDesc esi).DataReg]    ; Grundadresse holen
        mov     edi, 0400h                      ; BIOS-Offset COM Bereich
        cld
        mov     ecx, 4
        repne   scasw                           ; Adresse vorhanden ?
        je      @@L4                            ; Springe wenn Ja

; Fehler, die Adresse konnte nicht gefunden werden. -1 als Handle liefern.

@@L3:   mov     eax, -1                         ; Handle = -1
        jmp     @@L99                           ; und Ende

; Adresse ist Ok, die gesamten Adressen in den Deskriptor-Record eintragen

@@L4:   lea     edi, [(PortDesc esi).DataReg]   ; Adresse erstes Register
        mov     ecx, 7                          ; 7 Register pro Chip
@@L5:   stosw                                   ; Eintragen
        inc     ax                              ; NÑchste Adresse
        loop    @@L5

; Die FehlerzÑhler nullen

        xor     ax, ax                          ; 0 nach ax
        lea     edi, [(PortDesc esi).ErrorBlock]; Erster ZÑhler
        mov     ecx, 6                          ; 6 FehlerzÑhler
        rep     stosw                           ; lîschen

; PrÅfen ob der Port einen 16550A besitzt. Die Anzahl der Bytes die bei einem
; "Transmit-Register leer"-Interrupt geschrieben werden kînnen vermerken. Diese
; Anzahl ist 1 ohne FIFO und bis zu 14 mit FIFO.

        mov     dx, [(PortDesc esi).FCR]        ; FIFO control register holen
        mov     al, FIFO_INIT OR FIFO_ENABLED   ; FIFO...
        out     dx, al                          ; ... einschalten
        in      al, dx                          ; Wert wieder holen
        and     al, FIFO_ENABLED                ; FIFO-Bits ausmaskieren
        mov     ah, 1                           ; annehmen, da· kein FIFO da
        cmp     al, FIFO_ENABLED                ; Ist es ein 16550A ?
        jne     @@L6                            ; Springe wenn keiner
        mov     ah, 16                          ; 16 Sendebytes wenn FIFO vorh.
@@L6:   mov     [(PortDesc esi).FIFOLen], ah    ; Wert merken

; FIFO (falls vorhanden) erstmal wieder ausschalten

        mov     al, FIFO_CLEAR
        out     dx, al

; Interrupt-Vektor retten und eigenen Vektor setzen

        push    es
        mov     ah, 35h
        mov     al, [(PortDesc esi).IntNr]      ; Nummer des Interrupts
        int     21h                             ; Vektor holen
        mov     [(PortDesc esi).OldVecOffs], ebx
        mov     [(PortDesc esi).OldVecSeg], es
        pop     es

        push    ds
        mov     ah, 25h
        mov     al, [(PortDesc esi).IntNr]      ; Nummer des Interrupts
        mov     edx, [(PortDesc esi).IntHandler];
        push    cs
        pop     ds
        int     21h                             ; Vektor setzen
        pop     ds

; Den Port als installiert markieren und das Handle in eax rÅckliefern

        mov     [(PortDesc esi).Installed], 01h ; Bit 0 setzen
        mov     eax, esi                        ; Handle nach eax

; Das wars: Funktionsausgang.

@@L99:  ret

ENDP    _ComInstall

; -------------------------------------------------------------------------
;
; _ComDeInstall
;
; Deinstalliert den Port mit dem Åbergebenen Handle.
;

PROC    PASCAL _ComDeInstall NEAR PortHandle: DWORD
USES    ESI, EDI

        mov     esi, [PortHandle]               ; Deskriptor holen

; PrÅfen ob der Port installiert ist. Fehler wenn nicht.

        test    [(PortDesc esi).Installed], 01h ; Installiert ?
        jnz     @@L1                            ; Springe wenn ja

; Fehler: Port ist nicht installiert

        call    _ComError                       ; Fehler melden
        jmp     @@L99                           ; Und Ende

; Port ist installiert. Falls der Port auch gleichzeitig offen ist, zuerst
; schlie·en

@@L1:   test    [(PortDesc esi).Installed], 02h ; Offen ?
        jz      @@L2                            ; Springe wenn Nein
        push    esi                             ; PortHandle als Parameter
        call    _ComClose                       ; Port schlie·en
        mov     esi, [PortHandle]               ; PortHandle neu laden

; Install-Merker rÅcksetzen

@@L2:   and     [(PortDesc esi).Installed], NOT 01h

; Interrupt-Vektor rÅcksetzen.

        push    ds
        mov     ah, 25h
        mov     al, [(PortDesc esi).IntNr]      ; Nummer des belegten Interrupts
        mov     edx, [(PortDesc esi).OldVecOffs]
        mov     ds, [(PortDesc esi).OldVecSeg]
        int     21h                             ; Vektor rÅcksetzen
        pop     ds

; Ende

@@L99:  ret

ENDP    _ComDeInstall

; -------------------------------------------------------------------------
;
; _ComOpen
;
; Setzt Baudrate etc., lîscht die Puffer, gibt die Interrupts frei, kurz:
; Setzt den RS232-Betrieb in Gang.
;

PROC    PASCAL _ComOpen NEAR PortHandle: DWORD
USES    ESI, EDI

; Zeiger auf den Port-Deskriptor nach esi

        mov     esi, [PortHandle]

; PrÅfen ob der Port installiert ist. Fehler wenn nicht.

        test    [(PortDesc esi).Installed], 01h
        jnz     @@L0                            ; Springe wenn installiert
        call    _ComError                       ; Fehler-Routine
        jmp     @@L99

; PrÅfen ob der Port bereits offen ist, wenn ja zuerst schlie·en

@@L0:   test    [(PortDesc esi).Installed], 02h ; Offen ?
        jz      @@L1                            ; Nein: Springe
        push    esi                             ; Parameter fÅr ComClose
        call    _ComClose                       ; Schlie·en
        mov     esi, [PortHandle]               ; Handle neu laden

; Eine der ParitÑt entsprechende AND-Maske setzen

@@L1:   mov     al, 0FFh                        ; Maske fÅr kein Parity
        cmp     [(PortDesc esi).Parity], 'N'    ; Parity ?
        jz      @@L2                            ; Springe wenn Nein
        mov     al, 07Fh                        ; Maske wenn Parity
@@L2:   mov     [(PortDesc esi).ParityMask], al ; Maske merken

; Flow-Control rÅcksetzen

        sub     eax, eax
        mov     [(PortDesc esi).HostOff], al     ;
        mov     [(PortDesc esi).PCOff], al
        mov     [(PortDesc esi).MustSend], al
        mov     [(PortDesc esi).DSR_CTS_Ok], al

; Puffer rÅcksetzen

        mov     [(PortDesc esi).TXStart], eax
        mov     [(PortDesc esi).TXEnd], eax
        mov     [(PortDesc esi).TXCount], eax
        mov     [(PortDesc esi).RXStart], eax
        mov     [(PortDesc esi).RXEnd], eax
        mov     [(PortDesc esi).RXCount], eax

; UART initialisieren

        mov     dx, [(PortDesc esi).MCR]        ; modem control register
        mov     al, 0                           ; clr dtr, rts, out1, out2 & loopback
        out     dx, al
        cmp     [(PortDesc esi).Connection], 'D'; Direkte Verbindung ?
        jz      @@L3                            ; Springe wenn ja
        mov     dx, [(PortDesc esi).MSR]        ; modem status register
        in      al, dx                          ; modem status lesen
        and     al, 30h                         ; DSR, CTS maskieren
        cmp     al, 30h                         ; DSR, CTS prÅfen
        jnz     @@L4                            ; Springe wenn nicht ok
@@L3:   mov     [(PortDesc esi).DSR_CTS_Ok], 01h; Beide da, Ausgang Ok
@@L4:   mov     dx, [(PortDesc esi).FCR]        ; FIFO control register
        mov     al, FIFO_CLEAR                  ; FIFO ausschalten
        out     dx, al
        mov     dx, [(PortDesc esi).LSR]        ; line status register...
        in      al, dx                          ; ...rÅcksetzen
        mov     dx, [(PortDesc esi).DataReg]    ; Datenregister
        in      al, dx                          ; ...rÅcksetzen
        mov     dx, [(PortDesc esi).MSR]        ; modem status register...
        in      al, dx                          ; ...rÅcksetzen

; Baudrate in Divisor umrechnen und setzen

        mov     eax, 115200
        sub     edx, edx
        div     [(PortDesc esi).Baudrate]
        mov     ebx, eax                        ; Ergebnis nach ebx

; Baudrate am UART einstellen

@@L6:   mov     dx, [(PortDesc esi).LCR]        ; line control register
        mov     al, 80h                         ; DLAB = 1
        out     dx, al
        mov     ax, bx                          ; Divisor nach ax
        mov     dx, [(PortDesc esi).DLL]        ; divisor low
        out     dx, al                          ; lsb ausgeben
        mov     dx, [(PortDesc esi).DLH]        ; divisor high
        xchg    ah, al
        out     dx, al                          ; msb ausgeben

; Parity und Anzahl Stop- und Daten-Bits setzen

        mov     al, 00h                         ; Bits in al zusammenbauen
        mov     ah, [(PortDesc esi).Parity]
        cmp     ah, 'O'                         ; odd ?
        jne     @@L7
        or      al, 0Ah                         ; odd !
        jmp     @@L9

@@L7:   cmp     ah, 'E'                         ; even ?
        jne     @@L8
        or      al, 1Ah                         ; even !
        jmp     @@L9

@@L8:   cmp     ah, 'M'                         ; mark ?
        jne     @@L9
        or      al, 2Ah                         ; mark !

@@L9:   cmp     [(PortDesc esi).StopBits], 2    ; 2 Stop-Bits ?
        jnz     @@L10
        or      al, 04h

@@L10:  mov     ah, [(PortDesc esi).DataBits]   ; Anzahl Datenbits holen
        sub     ah, 5                           ; 5..8 --> 0..3
        and     ah, 3                           ; maskieren
        or      al, ah                          ; und reinodern

        mov     dx, [(PortDesc esi).LCR]        ; line control register
        out     dx, al                          ; parity mode & DLAB = 0

; FIFO's initalisieren (wird ignoriert wenn nicht 16550A)

        mov     dx, [(PortDesc esi).FCR]        ; FIFO control register
        mov     al, FIFO_INIT
        out     dx, al

; Interrupts freigeben

        cli                                     ; Interrupts aus
        in      al, 0A1h                        ; Int-Controller #2
        and     al, [(PortDesc esi).NotIC2Mask] ; Bit lîschen
        out     0A1h, al                        ; und wieder ausgeben
        in      al, 021h                        ; Int-Controller #1
        and     al, [(PortDesc esi).NotIC1Mask] ; Bit lîschen
        out     021h, al                        ; und wieder ausgeben
        sti                                     ; Interrupts an

        mov     dx, [(PortDesc esi).IER]        ; interrupt enable register
        mov     al, 00001101B                   ; line & modem status, rec...
        out     dx, al                          ; ...freigeben

        mov     dx, [(PortDesc esi).MCR]        ; modem control register
        mov     al, MCR_OUT2                    ; OUT2, kein RTS, kein DTR
        cmp     [(PortDesc esi).Connection], 'M'; Modem connection?
        jne     @@L11                           ; Nein: Skip
        or      al, MCR_RTS                     ; Ja: RTS aktiv setzen
@@L11:  out     dx, al                          ; setzen


; Port als Open markieren.

        or      [(PortDesc esi).Installed], 02h

; Ende

@@L99:  ret

ENDP    _ComOpen


; -------------------------------------------------------------------------
;
; _ComClose
;
; Schlie·t einen Com-Port
;

PROC    PASCAL _ComClose NEAR PortHandle: DWORD

; PrÅfen ob der Port Åberhaupt offen ist

        mov     ebx, [PortHandle]
        test    [(PortDesc ebx).Installed], 02h
        jz      @@Error                         ; Springe wenn Port nicht offen

; Interrupts an UART abklemmen, FIFO abschalten

        cli                                     ; Keine Interrupts
        mov     dx, [(PortDesc ebx).MCR]        ; modem control register
        mov     al, 0                           ; Kein RTS, DTR, OUT2
        out     dx, al
        mov     dx, [(PortDesc ebx).IER]        ; interrupt enable register
        out     dx, al                          ; Shut up !
        mov     dx, [(PortDesc ebx).FCR]
        mov     al, FIFO_CLEAR
        out     dx, al                          ; FIFO ausschalten

; Interrupt-Controller abschalten

        in      al, 0A1h                        ; Int-Controller #2
        or      al, [(PortDesc ebx).IC2Mask]
        out     0A1h, al                        ; Interrupt sperren

        in      al, 021h                        ; Int-Controller #1
        or      al, [(PortDesc ebx).IC1Mask]
        out     021h, al                        ; Interrupt sperren

; DTR und RTS lîschen

        mov     dx, [(PortDesc ebx).MCR]        ; modem control register
        in      al, dx                          ; Register lesen
        and     al, NOT (MCR_DTR OR MCR_RTS)    ; DTR lîschen...
        out     dx, al                          ; ...und wieder setzen

; Vermerken, da· der Port nicht mehr offen ist und Ende

        and     [(PortDesc ebx).Installed], NOT 02h
        sti                                     ; Interrupts wieder zulassen

@@L99:  ret

; Fehlereinsprung

@@Error:call    _ComError
        jmp     @@L99

ENDP    _ComClose

; ----------------------------------------------------------------------
;
; _ComDTROff
;
; Schaltet DTR auf off
;

PROC    PASCAL _ComDTROff NEAR PortHandle: DWORD

        mov     ebx, [PortHandle]
        test    [(PortDesc ebx).Installed], 02h
        jz      @@Error

; Port ist offen, Aktion durchfÅhren

        mov     dx, [(PortDesc ebx).MCR]        ; modem control register
        in      al, dx                          ; Register lesen
        and     al, NOT MCR_DTR                 ; DTR lîschen...
        out     dx, al                          ; ...und wieder setzen

; Ende

@@L99:  ret

; Fehlereinsprung

@@Error:call    _ComError
        jmp     @@L99

ENDP    _ComDTROff



; -------------------------------------------------------------------------
;
; _ComDTROn
;
; Schaltet DTR auf aktiv
;

PROC PASCAL _ComDTROn NEAR   PortHandle: DWORD

        mov     ebx, [PortHandle]
        test    [(PortDesc ebx).Installed], 02h ; Port offen ?
        jz      @@Error                         ; Springe wenn Port nicht offen

; Port ist offen, DTR RTS setzen

        mov     dx, [(PortDesc ebx).MCR]        ; modem control register
        in      al, dx                          ; Register lesen
        or      al, MCR_DTR                     ; DTR aktiv...
        out     dx, al                          ; und wieder setzen

; Ende

@@L99:  ret

; Fehlereinsprung

@@Error:call    _ComError
        jmp     @@L99

ENDP    _ComDTROn



; ----------------------------------------------------------------------
;
; _ComRTSOff
;
; Schaltet RTS auf off
;

PROC    PASCAL _ComRTSOff NEAR PortHandle: DWORD

        mov     ebx, [PortHandle]
        test    [(PortDesc ebx).Installed], 02h
        jz      @@Error
        cmp     [(PortDesc ebx).Connection], 'M'; Modem connection?
        je      @@Error                         ; Dann Aufruf nicht erlaubt

; Port ist offen, RTS rÅcksetzen

        mov     dx, [(PortDesc ebx).MCR]        ; modem control register
        in      al, dx                          ; Register lesen
        and     al, NOT MCR_RTS                 ; RTS lîschen...
        out     dx, al                          ; ...und wieder setzen

; Ende

@@L99:  ret

; Fehlereinsprung

@@Error:call    _ComError
        jmp     @@L99

ENDP    _ComRTSOff



; -------------------------------------------------------------------------
;
; _ComRTSOn
;
; Schaltet RTS auf aktiv
;

PROC PASCAL _ComRTSOn NEAR  PortHandle: DWORD

        mov     ebx, [PortHandle]
        test    [(PortDesc ebx).Installed], 02h ; Port offen ?
        jz      @@Error                         ; Springe wenn Port nicht offen
        cmp     [(PortDesc ebx).Connection], 'M'; Modem connection?
        je      @@Error                         ; Dann Aufruf nicht erlaubt

; Port ist offen, RTS setzen

        mov     dx, [(PortDesc ebx).MCR]        ; modem control register
        in      al, dx                          ; Register lesen
        or      al, MCR_RTS                     ; RTS aktiv...
        out     dx, al                          ; und wieder setzen

; Ende

@@L99:  ret

; Fehlereinsprung

@@Error:call    _ComError
        jmp     @@L99

ENDP    _ComRTSOn



; -------------------------------------------------------------------------
;
; _ComIsInstalled
;
; Ergibt einen Wert != 0 wenn der Port installiert ist.
;

PROC    PASCAL _ComIsInstalled NEAR   PortHandle: DWORD

        mov     ebx, [PortHandle]
        mov     bl, [(PortDesc ebx).Installed]
        sub     eax, eax
        test    bl, 01h                         ; Installiert ?
        jz      @@L1                            ; Springe wenn Nein
        inc     eax                             ; Installiert !
@@L1:   ret

ENDP    _ComIsInstalled



; -------------------------------------------------------------------------
;
; _ComIsOpen
;
; Ergibt einen Wert != 0 wenn der Port geîffnet ist.
;

PROC    PASCAL _ComIsOpen NEAR   PortHandle: DWORD

        mov     ebx, [PortHandle]
        mov     bl, [(PortDesc ebx).Installed]
        sub     eax, eax
        test    bl, 02h                         ; Offen ?
        jz      @@L1                            ; Springe wenn Nein
        inc     eax                             ; Offen !
@@L1:   ret

ENDP    _ComIsOpen



; -------------------------------------------------------------------------
;
; _ComReceive
;
; Holt ein Zeichen aus dem Empfangspuffer. Warte bis ein Zeichen da ist. Bei
; Fehlern kommt -1 als Wert zurÅck.
;


PROC    PASCAL _ComReceive NEAR   PortHandle: DWORD
USES    ESI, EDI

        mov     esi, [PortHandle]
        test    [(PortDesc esi).Installed], 02h ; Port offen ?
        jz      @@Error                         ; Springe wenn Nein

; Port ist offen, prÅfen ob Zeichen da

@@L1:   cmp     [(PortDesc esi).RXCount], 0     ; Zeichen da ?
        jnz     @@L2                            ; Abholen wenn ja

; Es ist kein Zeichen da - erstmal warten, dabei die _ComWait Routine
; aufrufen

        push    10                              ; 10ms
        call    _ComWait
        jmp     @@L1

; Zeichen ist da, holen. Da die Interrupt-Routinen nicht auf den
; RXStart-Zeiger zugreift, mÅssen die Interrupts nicht gesperrt
; werden.

@@L2:   mov     ebx, [(PortDesc esi).RXStart]   ; Ringpufferzeiger
        mov     edi, [(PortDesc esi).RXBuf]     ; Zeiger auf Pufferbereich
        mov     al, [edi+ebx]                   ; Zeichen holen
        inc     ebx                             ; Zeiger erhîhen
        movzx   eax, al                         ; Zeichen nach DWORD wandeln
        push    eax                             ; ...und auf den Stack
        cmp     ebx, [(PortDesc esi).RXBufSize] ; Warp-Around ?
        jb      @@L3                            ; Springe wenn Nein
        sub     ebx, ebx                        ; Ja, Ringpufferzeiger wird 0
@@L3:   mov     [(PortDesc esi).RXStart], ebx   ; Zeiger rÅckschreiben
        dec     [(PortDesc esi).RXCount]        ; Ein Zeichen weniger...

; Flow-Control prÅfen. Zuerst Hardware (RTS/CTS), dann Software

        mov     ebx, [(PortDesc esi).RXBufSize] ; Grî·e Puffer nach ebx
        shr     ebx, 2                          ; Grî·e/4 in ebx
        cmp     [(PortDesc esi).RXCount], ebx   ; Puffer fast leer ?
        jae     @@L99                           ; Ende wenn nicht

        cli                                     ; Interrupts aus
        cmp     [(PortDesc esi).Connection], 'M'; Modem connection?
        jne     @@L4                            ; Springe wenn nein
        mov     dx, [(PortDesc esi).MCR]
        in      al, dx                          ; MCR lesen
        or      al, MCR_RTS                     ; RTS setzen (Freigabe)
        out     dx, al

; Jetzt prÅfen ob XON/XOFF Flow-Control an-, und der Sender abgeschaltet wurde

@@L4:   cmp     [(PortDesc esi).XonXoff], 'E'   ; Flow-Control an ?
        jne     @@L7                            ; Ende wenn nicht
        cmp     [(PortDesc esi).HostOff], 00h   ; XOFF-Status ?
        jz      @@L7                            ; Ende wenn nicht

; Der Host ist gestoppt, der Puffer aber wieder leer genug. XON senden.

@@L5:   cmp     [(PortDesc esi).MustSend], 00h  ; Zeichen im Puffer ?
        je      @@L6                            ; Springe wenn Nein

; Es ist noch ein Steuerzeichen im Puffer. Warten bis es weg ist.

        sti                                     ; Interrupts freigeben
        push    10                              ; 10 ms
        call    _ComWait                        ; Warteroutine aufrufen
        mov     esi, [PortHandle]               ; Handle neu laden
        cli                                     ; Interrupts wieder aus
        jmp     @@L5                            ; und neu prÅfen...

; Das Kontrollzeichen kann ausgegeben werden

@@L6:   mov     al, XON
        call    SendII                          ; XON senden
@@L7:   sti                                     ; Interrupts wieder freigeben

; Zeichen vom Stack und Ende

        pop     eax
@@L99:  ret

; Fehlereinsprung wenn Port nicht offen

@@Error:call    _ComError                       ; Fehler melden
        mov     eax, -1                         ; Returncode -1
        jmp     @@L99


ENDP    _ComReceive



; -------------------------------------------------------------------------
;
; _ComRXCount
;
; Liefert die Anzahl Bytes im Empfangspuffer.
;

PROC    PASCAL _ComRXCount NEAR PortHandle: DWORD

        mov     ebx, [PortHandle]
        test    [(PortDesc ebx).Installed], 02h
        jz      @@Error

; Alles klar, Anzahl liefern und Ende

        mov     eax, [(PortDesc ebx).RXCount]
@@L99:  ret

; Fehlereinsprung wenn Port nicht offen

@@Error:call    _ComError
        jmp     @@L99

ENDP    _ComRXCount


; -------------------------------------------------------------------------
;
; _ComRXSize
;
; Liefert die Grî·e des Empfangspuffers.
;

PROC PASCAL _ComRXSize NEAR   PortHandle: DWORD

        mov     ebx, [PortHandle]
        mov     eax, [(PortDesc ebx).RXBufSize]
        ret

ENDP    _ComRXSize


; -------------------------------------------------------------------------
;
; _ComRXClear
;
; Lîscht den kompletten Empfangspuffer. Der Port mu· offen sein.
;

PROC    PASCAL _ComRXClear  NEAR     PortHandle: DWORD

        mov     ebx, [PortHandle]
        test    [(PortDesc ebx).Installed], 02h
        jz      @@Error

; Alles klar, Port ist offen

        sub     eax, eax
        cli                                     ; Interrupts sperren
        mov     [(PortDesc ebx).RXStart], eax
        mov     [(PortDesc ebx).RXEnd], eax
        mov     [(PortDesc ebx).RXCount], eax
        sti                                     ; Interrupts freigeben

@@L99:  ret

; Fehlereinsprung wenn Port nicht offen

@@Error:call    _ComError
        jmp     @@L99

ENDP    _ComRXClear



; -------------------------------------------------------------------------
;
; _ComTXCount
;
; Liefert die Anzahl belegter Bytes im Sendepuffer
;

PROC    PASCAL _ComTXCount NEAR   PortHandle: DWORD

        mov     ebx, [PortHandle]
        test    [(PortDesc ebx).Installed], 02h
        jz      @@Error                         ; Springe wenn nicht offen

; Port ist offen. Anzahl holen und Ende

        mov     eax, [(PortDesc ebx).TXCount]
@@L99:  ret

; Fehlereinsprung wenn Port ist nicht offen.

@@Error:call    _ComError
        jmp     @@L99

ENDP    _ComTXCount


; -------------------------------------------------------------------------
;
; _ComTXFree
;
; Liefert die Anzahl freier Bytes im Sendepuffer
;

PROC    PASCAL _ComTXFree NEAR   PortHandle: DWORD

        mov     ebx, [PortHandle]
        test    [(PortDesc ebx).Installed], 02h
        jz      @@Error                         ; Springe wenn nicht offen

; Port ist offen. Grî·e - Belegte Bytes rechnen.
; VORSICHT: Dieser Wert kann negativ werden, da sich evtl. zusÑtzliche
; Kontrollzeichen im Puffer befinden kînnen. In einem solchen Fall
; einfach 0 liefern.

        mov     eax, [(PortDesc ebx).TXBufSize] ; Gesamtgrî·e
        dec     eax                             ; Maximale Grî·e fÅr Benutzer
        sub     eax, [(PortDesc ebx).TXCount]
        jnc     @@L99                           ; Springe wenn >= 0
        sub     eax, eax                        ; if (eax < 0) then eax := 0;
@@L99:  ret

; Fehlereinsprung wenn Port ist nicht offen.

@@Error:call    _ComError
        jmp     @@L99

ENDP    _ComTXFree



; -------------------------------------------------------------------------
;
; _ComTXSize
;
; Liefert die Grî·e des Sendepuffers.
;

PROC    PASCAL _ComTXSize NEAR   PortHandle: DWORD

        mov     ebx, [PortHandle]
        mov     eax, [(PortDesc ebx).TXBufSize]
        ret

ENDP    _ComTXSize


; -------------------------------------------------------------------------
;
; _ComTXClear
;
; Lîscht den kompletten Sendespuffer. Der Port mu· offen sein.
; ACHTUNG: Im Gegensatz zu ComRXClear kînnen sich hier Steuerzeichen im Puffer
; befinden, deren Lîschung fatal wÑre. Falls sich ein Steuerzeichen im Puffer
; befindet ist es jedoch immer (!) das erste Zeichen im Puffer.

PROC    PASCAL _ComTXClear  NEAR     PortHandle: DWORD

        mov     ebx, [PortHandle]
        test    [(PortDesc ebx).Installed], 02h
        jz      @@Error

; Alles klar, Port ist offen

        sub     ecx, ecx
        cli                                     ; Interrupts sperren
        mov     eax, [(PortDesc ebx).TXEnd]
        cmp     [(PortDesc ebx).MustSend], 00h  ; Steuerzeichen im Puffer ?
        jz      @@L2                            ; Springe wenn Nein

; Ein Zeichen im Puffer lassen

        inc     ecx                             ; Anzahl = 1 nach Clear
        or      eax, eax                        ; Index schon 0?
        jnz     @@L1                            ; Springe wenn nein
        mov     eax, [(PortDesc ebx).TXBufSize] ; Grî·e laden
@@L1:   dec     eax

; Puffer lîschen

@@L2:   mov     [(PortDesc ebx).TXStart], eax
        mov     [(PortDesc ebx).TXCount], ecx
        sti                                     ; Interrupts wieder freigeben

@@L99:  ret

; Fehlereinsprung wenn Port nicht offen

@@Error:call    _ComError
        jmp     @@L99

ENDP    _ComTXClear



; -------------------------------------------------------------------------
;
; _ComSend
;
; Abschicken eines Zeichens.
;

PROC    PASCAL _ComSend NEAR   PortHandle: DWORD, C: BYTE:4
USES    ESI, EDI

        mov     esi, [PortHandle]
        test    [(PortDesc esi).Installed], 02h ; Port offen ?
        jz      @@Error                         ; Springe wenn nein

; Port ist offen, prÅfen ob Platz im Puffer

        mov     edx, [(PortDesc esi).TXBufSize] ; Grî·e des Puffers nach edx
        cmp     [(PortDesc esi).TXCount], edx   ; noch Platz ?
        jae     @@L4                            ; Springe wenn Nein

; Es ist genug Platz. Zeichen schreiben.

        mov     al, [C]                         ; Zeichen holen
        mov     edi, [(PortDesc esi).TXBuf]     ; Zeiger auf Puffer holen
        cli                                     ; Keine Interrupts
        mov     ebx, [(PortDesc esi).TXEnd]     ; Pufferzeiger holen
        mov     [edi+ebx], al                   ; Zeichen in Puffer schreiben
        inc     ebx                             ; Pufferzeiger erhîhen
        cmp     ebx, edx                        ; Wrap-Around ?
        jb      @@L1                            ; Springe wenn Nein
        sub     ebx, ebx                        ; Ringpufferzeiger auf 0 wenn ja
@@L1:   mov     [(PortDesc esi).TXEnd], ebx     ; Pufferzeiger rÅckschreiben
        inc     [(PortDesc esi).TXCount]        ; Ein Zeichen mehr

; Wenn notwendig die Sende-Interrupts freischalten

        mov     dx, [(PortDesc esi).IER]        ; Interrupt enable register
        in      al, dx                          ; ... lesen
        test    al, 02h                         ; Interrupts frei ?
        jnz     @@L2                            ; Ja, alles klar
        cmp     [(PortDesc esi).PCOff], 00h     ; XOFF-Status ?
        jnz     @@L2                            ; Ja, nix unternehmen
        cmp     [(PortDesc esi).DSR_CTS_Ok], 00h; Statusleitungen ok ?
        jz      @@L2                            ; Nein, nix unternehmen
        mov     al, 00001111B                   ;
        out     dx, al                          ; TX-Ints freischalten
@@L2:   sti                                     ; Interrupts wieder frei
        movzx   eax, [C]                        ; Zeichen nach eax

; Ende

@@L99:  ret

; Einsprung wenn Puffer voll

@@L4:   inc     [(PortDesc esi).ETXOverFlow]    ; FehlerzÑhler erhîhen
        mov     eax, -1                         ; Fehlerkennung
        jmp     @@L99                           ; Und Ende

; Einsprung wenn Port nicht offen

@@Error:call    _ComError                       ; Fehler melden
        jmp     @@L99                           ; Ende

ENDP    _ComSend



; -------------------------------------------------------------------------
;
; Interne Routine. Wird aufgerufen von ComReceive und vom RX-Interrupthandler
; wenn Flow-Control Zeichen verschickt werden mÅssen.
; esi mu· auf den Port-Deskriptor zeigen, Interrupts mÅssen gesperrt sein.
; al enthÑlt das zu sendende Zeichen.
;

PROC    SENDII  NEAR

        push    ebx
        push    edx                             ; Register werden benîtigt
        push    edi

        mov     ebx, [(PortDesc esi).TXStart]   ; Ringpufferzeiger holen
        mov     edi, [(PortDesc esi).TXBuf]     ; Zeiger auf Pufferbereich
        mov     edx, [(PortDesc esi).TXBufSize] ; Puffergrî·e nach dx
        cmp     [(PortDesc esi).TXCount], edx   ; Puffer voll ?
        jb      @@L2                            ; Springe wenn Nein

; Im Puffer ist kein Platz mehr (obwohl das eigentlich immer gewÑhrleistet
; werden sollte !). Das kann eigentlich nur passieren, wenn direkt
; hintereinander zwei Flow-Control Zeichen geschickt werden...

        mov     [ebx+edi], al                   ; Zerstîrt erstes Zeichen
        inc     [(PortDesc esi).ETXOverFlow]    ; FehlerzÑhler erhîhen
        jmp     @@L2                            ; einfach weiter

; Im Puffer ist Platz, Zeichen an die erste Stelle schreiben

        dec     ebx                             ; Ergibt 0FFFF wenn ebx = 0
        jns     @@L1                            ; Springe wenn kein Wrap
        mov     ebx, edx                        ; Sonst Grî·e-1 nach ebx
        dec     ebx
@@L1:   mov     [ebx+edi], al                   ; Zeichen schreiben
        mov     [(PortDesc esi).TXStart], ebx   ; Zeiger rÅckschreiben
        inc     [(PortDesc esi).TXCount]        ; Ein Zeichen mehr ...

; Flag fÅr extra Zeichen setzen

@@L2:   mov     [(PortDesc esi).MustSend], 01h  ; Flag setzen

; Falls notwendig Interrupts enablen

        mov     dx, [(PortDesc esi).IER]        ; interrupt enable register
        in      al, dx                          ; Wert holen
        test    al, 02h                         ; TX-Ints frei ?
        jnz     @@L99                           ; Ja: Ende
        cmp     [(PortDesc esi).DSR_CTS_Ok], 00h; Statusleitungen ok ?
        jz      @@L99                           ; Nein, Pech ..
        mov     al, 00001111B                   ; modem & line status, rec, xmit
        out     dx, al                          ; ... freigeben

; Fertig !

@@L99:  pop     edi
        pop     edx
        pop     ebx
        ret

ENDP    SendII


; -------------------------------------------------------------------------
;
; _ComBreak
;
; Sendet ein Break-Signal mit variabler LÑnge.
;

PROC    PASCAL _ComBreak NEAR  PortHandle: DWORD, BreakLen: DWORD

        mov     ebx, [PortHandle]
        test    [(PortDesc ebx).Installed], 02h
        jz      @@Error                         ; Springe wenn Port nicht offen

; Port ist offen, Break senden

        mov     dx, [(PortDesc ebx).LCR]
        cli                                     ; Interrupts off
        in      al, dx                          ; get LCR
        or      al, 40h                         ; break bit on
        out     dx, al                          ; setzen
        sti                                     ; Interrupts on

; Warte-Routine aufrufen

        push    [BreakLen]
        call    _ComWait

; Register sind futsch, neu laden

        mov     ebx, [PortHandle]
        mov     dx, [(PortDesc ebx).LCR]

; Und Break-Bit rÅcksetzen

        cli
        in      al, dx
        and     al, NOT 40h
        out     dx, al
        sti

; Fertig

@@L99:  ret

; Fehlereinsprung wenn Port ist nicht offen.

@@Error:call    _ComError
        jmp     @@L99

ENDP    _ComBreak



; -------------------------------------------------------------------------
;
; ComModemStatus
;
; Gibt den Status der Kontroll-Leitungen zurÅck.
;
; Portbelegung:
;       0x80:   -CD     (Carrier Detect, inverted)
;       0x40:   -RI     (Ring Indicator, inverted)
;       0x20:   -DSR    (Data Set Ready, inverted)
;       0x10:   -CTS    (Clear to Send, inverted)
;       0x08:   Delta Carrier Detect    (CD changed)
;       0x04:   Trailing edge of RI     (RI went OFF)
;       0x02:   Delta DSR               (DSR changed)
;       0x01:   Delta CTS               (CTS changed)
;

PROC    PASCAL _ComModemStatus NEAR  PortHandle: DWORD

        mov     ebx, [PortHandle]
        test    [(PortDesc ebx).Installed], 02h ; Port Offen ?
        jz      @@Error                         ; Springe wenn Nein
        mov     dx, [(PortDesc ebx).MSR]        ; Modem status register
        in      al, dx                          ; Wert holen
        movzx   eax, al                         ; Wert in eax
@@L99:  ret

; Fehlereinsprung wenn Port nicht offen

@@Error:call    _ComError
        jmp     @@L99


ENDP    _ComModemStatus


; -------------------------------------------------------------------------
;
; Interrupt-Handler fÅr COM1
;

PROC    PASCAL _IntCom1 FAR

        push    esi                             ; Register retten
        mov     esi, OFFSET _ComPort1           ; Deskriptor laden
        jmp     SHORT IntCommon                 ; Gemeinsame Routine

ENDP    _IntCom1

; -------------------------------------------------------------------------
;
; Interrupt-Handler fÅr COM2
;

PROC    PASCAL _IntCom2 FAR

        push    esi                             ; Register retten
        mov     esi, OFFSET _ComPort2           ; Deskriptor laden
        jmp     SHORT IntCommon                 ; Gemeinsame Routine

ENDP    _IntCom2

; -------------------------------------------------------------------------
;
; Interrupt-Handler fÅr COM3
;

PROC    PASCAL _IntCom3 FAR

        push    esi                             ; Register retten
        mov     esi, OFFSET _ComPort3           ; Deskriptor laden
        jmp     SHORT IntCommon                 ; Gemeinsame Routine

ENDP    _IntCom3

; -------------------------------------------------------------------------
;
; Interrupt-Handler fÅr COM4
;

PROC    PASCAL _IntCom4 FAR

        push    esi                             ; Register retten
        mov     esi, OFFSET _ComPort4           ; Deskriptor laden
        jmp     SHORT IntCommon                 ; Gemeinsame Routine

ENDP    _IntCom4

; -------------------------------------------------------------------------
;
; Gemeinsame Interrrupt-Routine fÅr alle Ports
;

PROC    IntCommon FAR

        pushad
        push    ds                              ; Register retten

        mov     ax, DGROUP
        mov     ds, ax                          ; Datensegment setzen

; Rausfinden, was es fÅr ein Interrupt war und entsprechend verzweigen

@@RePoll:
        mov     dx, [(PortDesc esi).IIR]        ; interrupt id register
        in      al, dx
        test    al, 01h                         ; "no interrupt present" ?
        jnz     @@L98                           ; Ja: Ende
        movzx   ebx, al                         ; Wert nach ebx
        and     ebx, 000Eh                      ; unerwÅnschte Bits ausmaskieren
        jmp     [cs:@@IntDispatch+ebx*2]        ; Behandlungsroutine

LABEL @@IntDispatch DWORD
        dd      @@MSInt                         ; 0: Modem status int
        dd      @@TXInt                         ; 2: Transmitter int
        dd      @@RXInt                         ; 4: Receiver int
        dd      @@LSInt                         ; 6: Line status int
        dd      @@RePoll                        ; 8: Reserviert
        dd      @@RePoll                        ; A: Reserviert
        dd      @@RXInt                         ; C: FIFO timeout, wie RXInt
        dd      @@RePoll                        ; E: Reserviert

; Interrupt-Controller verstÑndigen. Problemlos mîglich, weil die Interrupts
; sowieso noch gesperrt sind.

@@L98:  mov     dx, [(PortDesc esi).IER]        ; Interrupt enable register
        in      al, dx                          ; Wert lesen
        mov     ah, al                          ; ... und nach ah retten
        xor     al, al                          ; Interrupts disablen
        out     dx, al

        mov     al, 20h                         ; EOI
        cmp     [(PortDesc esi).IC2Mask], 00    ; "Hoher" Interrupt ?
        jz      @@L99                           ; Springe wenn Nein
        out     0A0h, al                        ; EOI an Controller #2
@@L99:  out     20h, al                         ; EOI an Controller #1

        mov     al, ah                          ; Alter IER-Wert
        out     dx, al                          ; ... restaurieren

        pop     ds
        popad
        pop     esi
        iretd

; -------------------------------------------------------------------------
;
; Line status interrupt
;

@@LSInt:mov     dx, [(PortDesc esi).LSR]        ; line status register
        in      al, dx                          ; lesen
        shr     al, 1                           ; Bit wird nicht benîtigt
        sub     bx, bx                          ; bx = 0

        shr     al, 1                           ; overrun error prÅfen
        adc     [(PortDesc esi).EOvrRun], bx    ; ZÑhler evtl. erhîhen
        shr     al, 1                           ; parity error prÅfen
        adc     [(PortDesc esi).EParity], bx    ; ZÑhler evtl. erhîhen
        shr     al, 1                           ; framing error prÅfen
        adc     [(PortDesc esi).EFrame], bx     ; ZÑhler evtl. erhîhen
        shr     al, 1                           ; break error prÅfen
        adc     [(PortDesc esi).EBreak], bx     ; ZÑhler evtl. erhîhen

        jmp     @@RePoll                        ; Und neu abfragen

; -------------------------------------------------------------------------
;
; Modem status interrupt
;

@@MSInt:mov     dx, [(PortDesc esi).MSR]        ; modem status register
        in      al, dx                          ; lesen

        cmp     [(PortDesc esi).Connection], 'D'; direkte Verbindung ?
        je      @@B1                            ; dann immer frei

        and     al, 30h                         ; DSR/CTS maskieren
        cmp     al, 30h                         ; beide da ?
        mov     al, 00h                         ; Flag fÅr Nein
        jne     @@B2                            ; Springe wenn Nein
@@B1:   mov     al, 01h                         ; Flag fÅr Ja
@@B2:   mov     [(PortDesc esi).DSR_CTS_Ok], al ; Flag setzen

; Jetzt auf jeden Fall immer alle Interrupts freigeben. Falls keine Daten
; anliegen werden die Ints in der TXInt-Routine wieder gesperrt.

        mov     dx, [(PortDesc esi).IER]        ; interrupt enable register
        mov     al, 00001111B
        out     dx, al
        jmp     @@RePoll                        ; und neu pollen

; -------------------------------------------------------------------------
;
; Transmit interrupt
;

@@TXInt:cmp     [(PortDesc esi).DSR_CTS_Ok], 0  ; Hardware Ok ?
        je      @@C5                            ; Nein: Ints abklemmen und Ende

        mov     ecx, 1                          ; Anzahl Zeichen
        cmp     [(PortDesc esi).MustSend], 00h  ; Flow-Control Zeichen ?
        jnz     @@C2                            ; Ja, immer senden
        cmp     [(PortDesc esi).PCOff], 00h     ; Flow-Control ok ?
        jnz     @@C5                            ; Nein: Ints abklemmen und Ende

        mov     cl, [(PortDesc esi).FIFOLen]    ; Maximale Anzahl FIFO
        cmp     ecx, [(PortDesc esi).TXCount]   ; > Anzahl im Puffer ?
        jb      @@C1                            ; Springe wenn kleiner
        mov     ecx, [(PortDesc esi).TXCount]   ; Sonst Anzahl im Puffer nehmen
@@C1:   jecxz    @@C5                           ; Nichts zu senden

@@C2:   mov     ebx, [(PortDesc esi).TXStart]   ; Ringpufferzeiger
        mov     edi, [(PortDesc esi).TXBuf]     ; Zeiger auf Sendepuffer
        mov     dx, [(PortDesc esi).DataReg]    ; Datenregister
        sub     [(PortDesc esi).TXCount], ecx   ; Anzahl passend vermindern

@@C3:   mov     al, [ebx+edi]                   ; Zeichen holen
        out     dx, al                          ; und schreiben
        inc     ebx                             ; Pufferzeiger erhîhen
        cmp     ebx, [(PortDesc esi).TXBufSize] ; Wrap ?
        jb      @@C4
        sub     ebx, ebx                        ; Wrap !
@@C4:   loop    @@C3                            ; und nÑchstes Zeichen

        mov     [(PortDesc esi).TXStart], ebx   ; Zeiger rÅckschreiben
        mov     [(PortDesc esi).MustSend], 0    ; Flow-Control ist weg

        jmp     @@RePoll                        ; Fertig !

; Es gibt nichts zu senden, oder es darf nicht gesendet werden:
; TX-Interrupts abklemmen

@@C5:   mov     dx, [(PortDesc esi).IER]        ; interrupt enable register
        mov     al, 00001101B                   ; line & modem status, rec.
        out     dx, al
        jmp     @@RePoll

; -------------------------------------------------------------------------
;
; Receive interrupt
;

@@RXInt:mov     ebx, [(PortDesc esi).RXEnd]     ; Ringpufferzeiger holen
        mov     edi, [(PortDesc esi).RXBuf]     ; Zeiger auf Pufferbereich holen
        mov     ebp, [(PortDesc esi).RXBufSize] ; Puffergrî·e nach bp

@@D1:   mov     dx, [(PortDesc esi).DataReg]    ; Datenregister
        in      al, dx                          ; lesen
        and     al, [(PortDesc esi).ParityMask] ; Parity-Bit ausmaskieren
        cmp     [(PortDesc esi).XonXoff], 'E'   ; Flow Control ?
        jnz     @@D4                            ; Springe wenn Nein

; Flow-Control ist an, Ctrl-S und Ctrl-Q prÅfen

        cmp     al, XOFF                        ; Ctrl-S ?
        jnz     @@D2                            ; Nein: Skip
        mov     [(PortDesc esi).PCOff], 01h     ; Ja: Stop !
        mov     al, 00001101B                   ; TX-Ints sperren
        jmp     @@D3                            ; Zeichen nicht speichern
@@D2:   cmp     al, XON                         ; Ctrl-Q ?
        jnz     @@D4                            ; Nein: Skip
        mov     [(PortDesc esi).PCOff], 00h     ; Ja: Output freigeben
        mov     al, 00001111B
@@D3:   mov     dx, [(PortDesc esi).IER]
        out     dx, al
        jmp     @@D7                            ; Zeichen nicht speichern

; Zeichen speichern

@@D4:   cmp     [(PortDesc esi).RXCount], ebp   ; Noch Platz ?
        jb      @@D5                            ; Springe wenn ja
        inc     [(PortDesc esi).ERXOverflow]    ; FehlerzÑhler erhîhen
        jmp     @@D7                            ; Und nÑchstes Byte

@@D5:   mov     [ebx+edi], al                   ; Zeichen speichern
        inc     ebx                             ; Zeiger erhîhen
        cmp     ebx, ebp                        ; Wrap-Araound ?
        jb      @@D6                            ; Springe wenn Nein
        sub     ebx, ebx
@@D6:   inc     [(PortDesc esi).RXCount]        ; Anzahl im Puffer erhîhen

; PrÅfen, ob noch mehr Zeichen vorliegen (FIFO)

@@D7:   mov     dx, [(PortDesc esi).LSR]        ; line status register
        in      al, dx                          ; ...lesen
        test    al, 01h                         ; noch Zeichen da ?
        jne     @@D1                            ; Ja: NÑchstes Zeichen lesen

; Es liegen keine Zeichen mehr vor. Pufferzeiger rÅckspeichern.

        mov     [(PortDesc esi).RXEnd], ebx

; Jetzt noch prÅfen, ob der Sender gestoppt werden mu·, wenn der Puffer
; zu voll wird.

        mov     eax, [(PortDesc esi).RXBufSize] ; Puffergrî·e nach eax
        shr     eax, 1                          ; / 2
        mov     ebx, eax
        shr     eax, 1                          ; / 4
        add     eax, ebx                        ; 3/4 Grî·e in eax
        cmp     [(PortDesc esi).RXCount], eax   ; Puffer 3/4 voll ?
        jb      @@D99                           ; Nein: Kein Grund fÅr HostOff

; Falls RTS/CTS verwendet wird, RTS abschalten

        cmp     [(PortDesc esi).Connection], 'M'; HW Control?
        jne     @@D8                            ; Springe wenn nein
        mov     dx, [(PortDesc esi).MCR]
        in      al, dx                          ; Sonst MCR lesen...
        and     al, NOT MCR_RTS                 ; ...RTS off...
        out     dx, al                          ; ...und wieder schreiben

; Falls Software Flow Control verwendet wird, XOFF absenden

@@D8:   cmp     [(PortDesc esi).XonXoff], 'E'   ; Enabled ?
        jnz     @@D99                           ; Nein: Fertig
        cmp     [(PortDesc esi).HostOff], 00h   ; Schon XOFF gesandt ?
        jnz     @@D99                           ; Ja: Keins mehr senden
        mov     al, XOFF
        call    SendII                          ; Zeichen abschicken
        mov     [(PortDesc esi).HostOff], 01h   ; und merken...

; Fertig, neu pollen

@@D99:  jmp     @@RePoll


ENDP    IntCommon

END
