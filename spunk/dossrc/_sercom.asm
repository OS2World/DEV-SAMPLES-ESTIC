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
; interrupts.
; Comments are in german, sorry.



; ***************************************************************************



IDEAL                   ; IDEAL-Modus von TASM einschalten
JUMPS                   ; Sprungoptimierung
LOCALS                  ; Lokale Symbole zulassen
SMART                   ; "Schlauen" Modus einschalten
MODEL LARGE             ; Im LARGE Modell Åbersetzen




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
                        db      ?       ; Fill auf gerade Adressen

        ; Ringpuffer fÅr Senden und Empfang incl Verwaltung

        RXBuf           dd      ?       ; Zeiger auf Empfangspuffer
        TXBuf           dd      ?       ; Zeiger auf Sendepuffer
        RXBufSize       dw      ?       ; Grî·e Empfangspuffer
        TXBufSize       dw      ?       ; Grî·e Sendepuffer
        RXStart         dw      0       ; Ringpufferzeiger
        RXEnd           dw      0       ;
        TXStart         dw      0
        TXEnd           dw      0
        RXCount         dw      0       ; Anzahl Zeichen im Puffer
        TXCount         dw      0       ; Anzahl Zeichen im Puffer

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

        IntHandler      dw      ?       ; Offset des Interrupt-Handlers
        OldVector       dd      ?       ; Alter Interrupt-Vektor

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


EXTRN PASCAL _COMWAIT   : FAR           ; Warte-Prozedur
EXTRN PASCAL _COMERROR  : FAR           ; _ComError-Routine



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

PROC    PASCAL _ComInstall FAR PortHandle: WORD
USES    SI, DI

        mov     si, [PortHandle]                ; Porthandle holen

; si zeigt ab hier auf einen Record vom Typ PortDesc

        test    [(PortDesc si).Installed], 01h  ; Bereits installiert
        jz      @@L3                            ; Nein: Ok

; Fehler, Port war bereits installiert

@@L1:   call    _ComError
        mov     ax, -1                          ; -1 als Handle liefern
        jmp     @@L99

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
; bx enthÑlt noch immer (PortAdresse-1)*2

@@L3:   mov     ax, [(PortDesc si).DataReg]     ; Grundadresse holen
        mov     di, 040h                        ; BIOS-Segment
        mov     es, di
        xor     di, di                          ; BIOS-Offset COM Bereich
        cld
        mov     cx, 4
        repne   scasw                           ; Adresse vorhanden ?
        je      @@L4                            ; Springe wenn Ja

; Fehler, die Adresse konnte nicht gefunden werden. -1 als Handle liefern.

        mov     ax, -1                          ; Handle = -1
        jmp     @@L99                           ; und Ende

; Adresse ist Ok, die gesamten Adressen in den Deskriptor-Record eintragen

@@L4:   push    ds
        pop     es
        lea     di, [(PortDesc ds:si).DataReg]  ; Adresse erstes Register
        mov     cx, 7                           ; 7 Register pro Chip
@@L5:   stosw                                   ; Eintragen
        inc     ax                              ; NÑchste Adresse
        loop    @@L5

; Die FehlerzÑhler nullen

        xor     ax, ax                          ; 0 nach ax
        lea     di, [(PortDesc si).ErrorBlock]  ; Erster ZÑhler
        mov     cx, 6                           ; 6 FehlerzÑhler
        rep     stosw                           ; lîschen

; PrÅfen ob der Port einen 16550A besitzt. Die Anzahl der Bytes die bei einem
; "Transmit-Register leer"-Interrupt geschrieben werden kînnen vermerken. Diese
; Anzahl ist 1 ohne FIFO und bis zu 14 mit FIFO.

        mov     dx, [(PortDesc si).FCR]         ; FIFO control register holen
        mov     al, FIFO_INIT OR FIFO_ENABLED   ; FIFO...
        out     dx, al                          ; ... einschalten
        in      al, dx                          ; Wert wieder holen
        and     al, FIFO_ENABLED                ; FIFO-Bits ausmaskieren
        mov     ah, 1                           ; annehmen, da· kein FIFO da
        cmp     al, FIFO_ENABLED                ; Ist es ein 16550A ?
        jne     @@L6                            ; Springe wenn keiner
        mov     ah, 16                          ; 16 Sendebytes wenn FIFO vorh.
@@L6:   mov     [(PortDesc si).FIFOLen], ah     ; Wert merken

; FIFO (falls vorhanden) erstmal wieder ausschalten

        mov     al, FIFO_CLEAR
        out     dx, al

; Interrupt-Vektor retten und eigenen Vektor setzen

        mov     ah, 35h
        mov     al, [(PortDesc si).IntNr]       ; Nummer des Interrupts
        int     21h                             ; Vektor holen
        mov     [WORD LOW (PortDesc si).OldVector], bx
        mov     [WORD HIGH (PortDesc si).OldVector], es

        push    ds
        mov     ah, 25h
        mov     al, [(PortDesc si).IntNr]       ; Nummer des Interrupts
        mov     dx, [(PortDesc si).IntHandler]  ;
        push    cs
        pop     ds
        int     21h                             ; Vektor setzen
        pop     ds

; Den Port als installiert markieren und das Handle in ax rÅckliefern

        mov     [(PortDesc si).Installed], 01h  ; Bit 0 setzen
        mov     ax, si                          ; Handle nach ax

; Das wars: Funktionsausgang.

@@L99:  ret

ENDP    _ComInstall

; -------------------------------------------------------------------------
;
; _ComDeInstall
;
; Deinstalliert den Port mit dem Åbergebenen Handle.
;

PROC    PASCAL _ComDeInstall FAR  PortHandle: WORD
USES    SI, DI

        mov     si, [PortHandle]                ; Deskriptor holen

; PrÅfen ob der Port installiert ist. Fehler wenn nicht.

        test    [(PortDesc si).Installed], 01h  ; Installiert ?
        jnz     @@L1                            ; Springe wenn ja

; Fehler: Port ist nicht installiert

        call    _ComError                       ; Fehler melden
        jmp     @@L99                           ; Und Ende

; Port ist installiert. Falls der Port auch gleichzeitig offen ist, zuerst
; schlie·en

@@L1:   test    [(PortDesc si).Installed], 02h  ; Offen ?
        jz      @@L2                            ; Springe wenn Nein
        push    si                              ; PortHandle als Parameter
        call    _ComClose                       ; Port schlie·en
        mov     si, [PortHandle]                ; PortHandle neu laden

; Install-Merker rÅcksetzen

@@L2:   and     [(PortDesc si).Installed], NOT 01h

; Interrupt-Vektor rÅcksetzen.

        push    ds
        mov     ah, 25h
        mov     al, [(PortDesc si).IntNr]       ; Nummer des belegten Interrupts
        mov     dx, [WORD LOW (PortDesc si).OldVector]
        mov     ds, [WORD HIGH (PortDesc si).OldVector]
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

PROC    PASCAL _ComOpen FAR PortHandle: WORD
USES    SI, DI

; Zeiger auf den Port-Deskriptor nach si

        mov     si, [PortHandle]

; PrÅfen ob der Port installiert ist. Fehler wenn nicht.

        test    [(PortDesc si).Installed], 01h
        jnz     @@L0                            ; Springe wenn installiert
        call    _ComError                       ; Fehler-Routine
        jmp     @@L99

; PrÅfen ob der Port bereits offen ist, wenn ja zuerst schlie·en

@@L0:   test    [(PortDesc si).Installed], 02h  ; Offen ?
        jz      @@L1                            ; Nein: Springe
        push    si                              ; Parameter fÅr ComClose
        call    _ComClose                       ; Schlie·en
        mov     si, [PortHandle]                ; Handle neu laden

; Eine der ParitÑt entsprechende AND-Maske setzen

@@L1:   mov     al, 0FFh                        ; Maske fÅr kein Parity
        cmp     [(PortDesc si).Parity], 'N'     ; Parity ?
        jz      @@L2                            ; Springe wenn Nein
        mov     al, 07Fh                        ; Maske wenn Parity
@@L2:   mov     [(PortDesc si).ParityMask], al  ; Maske merken

; Flow-Control rÅcksetzen

        xor     ax, ax
        mov     [(PortDesc si).HostOff], al     ;
        mov     [(PortDesc si).PCOff], al
        mov     [(PortDesc si).MustSend], al
        mov     [(PortDesc si).DSR_CTS_Ok], al

; Puffer rÅcksetzen

        mov     [(PortDesc si).TXStart], ax
        mov     [(PortDesc si).TXEnd], ax
        mov     [(PortDesc si).TXCount], ax
        mov     [(PortDesc si).RXStart], ax
        mov     [(PortDesc si).RXEnd], ax
        mov     [(PortDesc si).RXCount], ax

; UART initialisieren

        mov     dx, [(PortDesc si).MCR]         ; modem control register
        mov     al, 0                           ; clr dtr, rts, out1, out2 & loopback
        out     dx, al
        cmp     [(PortDesc si).Connection], 'D' ; Direkte Verbindung ?
        jz      @@L3                            ; Springe wenn ja
        mov     dx, [(PortDesc si).MSR]         ; modem status register
        in      al, dx                          ; modem status lesen
        and     al, 30h                         ; DSR, CTS maskieren
        cmp     al, 30h                         ; DSR, CTS prÅfen
        jnz     @@L4                            ; Springe wenn nicht ok
@@L3:   mov     [(PortDesc si).DSR_CTS_Ok], 01h ; Beide da, Ausgang Ok
@@L4:   mov     dx, [(PortDesc si).FCR]         ; FIFO control register
        mov     al, FIFO_CLEAR                  ; FIFO ausschalten
        out     dx, al
        mov     dx, [(PortDesc si).LSR]         ; line status register...
        in      al, dx                          ; ...rÅcksetzen
        mov     dx, [(PortDesc si).DataReg]     ; Datenregister
        in      al, dx                          ; ...rÅcksetzen
        mov     dx, [(PortDesc si).MSR]         ; modem status register...
        in      al, dx                          ; ...rÅcksetzen

; Baudrate in Divisor umrechnen und setzen

        cmp     [WORD HIGH (PortDesc si).Baudrate], 0
        jz      @@L5                            ; Baudrate < 65536
        mov     bx, 1                           ; Divisor fÅr 115200
        jmp     @@L6
@@L5:   mov     ax, 0C200h                      ; 115200 MOD 10000h
        mov     dx, 00001h                      ; 115200 DIV 10000h
        div     [WORD LOW (PortDesc si).Baudrate]
        mov     bx, ax                          ; Ergebnis nach bx

; Baudrate am UART einstellen

@@L6:   mov     dx, [(PortDesc si).LCR]         ; line control register
        mov     al, 80h                         ; DLAB = 1
        out     dx, al
        xchg    ax, bx                          ; Divisor nach ax
        mov     dx, [(PortDesc si).DLL]         ; divisor low
        out     dx, al                          ; lsb ausgeben
        mov     dx, [(PortDesc si).DLH]         ; divisor high
        xchg    ah, al
        out     dx, al                          ; msb ausgeben

; Parity und Anzahl Stop- und Daten-Bits setzen

        mov     al, 00h                         ; Bits in al zusammenbauen
        mov     ah, [(PortDesc si).Parity]
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

@@L9:   cmp     [(PortDesc si).StopBits], 2     ; 2 Stop-Bits ?
        jnz     @@L10
        or      al, 04h

@@L10:  mov     ah, [(PortDesc si).DataBits]    ; Anzahl Datenbits holen
        sub     ah, 5                           ; 5..8 --> 0..3
        and     ah, 3                           ; maskieren
        or      al, ah                          ; und reinodern

        mov     dx, [(PortDesc si).LCR]         ; line control register
        out     dx, al                          ; parity mode & DLAB = 0

; FIFO's initalisieren (wird ignoriert wenn nicht 16550A)

        mov     dx, [(PortDesc si).FCR]         ; FIFO control register
        mov     al, FIFO_INIT
        out     dx, al

; Interrupts freigeben

        cli                                     ; Interrupts aus
        in      al, 0A1h                        ; Int-Controller #2
        and     al, [(PortDesc si).NotIC2Mask]  ; Bit lîschen
        out     0A1h, al                        ; und wieder ausgeben
        in      al, 021h                        ; Int-Controller #1
        and     al, [(PortDesc si).NotIC1Mask]  ; Bit lîschen
        out     021h, al                        ; und wieder ausgeben
        sti                                     ; Interrupts an

        mov     dx, [(PortDesc si).IER]         ; interrupt enable register
        mov     al, 00001101B                   ; line & modem status, rec...
        out     dx, al                          ; ...freigeben

        mov     dx, [(PortDesc si).MCR]         ; modem control register
        mov     al, MCR_OUT2                    ; OUT2, kein RTS, kein DTR
        cmp     [(PortDesc si).Connection], 'M' ; Modem connection?
        jne     @@L11                           ; Nein: Skip
        or      al, MCR_RTS                     ; Ja: RTS aktiv setzen
@@L11:  out     dx, al                          ; setzen

; Port als Open markieren.

        or      [(PortDesc si).Installed], 02h

; Ende

@@L99:  ret

ENDP    _ComOpen


; -------------------------------------------------------------------------
;
; _ComClose
;
; Schlie·t einen Com-Port
;

PROC    PASCAL _ComClose FAR PortHandle: WORD

; PrÅfen ob der Port Åberhaupt offen ist

        mov     bx, [PortHandle]
        test    [(PortDesc bx).Installed], 02h
        jz      @@Error                         ; Springe wenn Port nicht offen

; Interrupts an UART abklemmen, FIFO abschalten

        cli                                     ; Keine Interrupts
        mov     dx, [(PortDesc bx).MCR]         ; modem control register
        mov     al, 0                           ; Kein RTS, DTR, OUT2
        out     dx, al
        mov     dx, [(PortDesc bx).IER]         ; interrupt enable register
        out     dx, al                          ; Shut up !
        mov     dx, [(PortDesc bx).FCR]
        mov     al, FIFO_CLEAR
        out     dx, al                          ; FIFO ausschalten

; Interrupt-Controller abschalten

        in      al, 0A1h                        ; Int-Controller #2
        or      al, [(PortDesc bx).IC2Mask]
        out     0A1h, al                        ; Interrupt sperren

        in      al, 021h                        ; Int-Controller #1
        or      al, [(PortDesc bx).IC1Mask]
        out     021h, al                        ; Interrupt sperren

; DTR und RTS lîschen

        mov     dx, [(PortDesc bx).MCR]         ; modem control register
        in      al, dx                          ; Register lesen
        and     al, NOT (MCR_DTR OR MCR_RTS)    ; DTR lîschen...
        out     dx, al                          ; ...und wieder setzen

; Vermerken, da· der Port nicht mehr offen ist und Ende

        and     [(PortDesc bx).Installed], NOT 02h
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

PROC    PASCAL _ComDTROff FAR PortHandle: WORD

        mov     bx, [PortHandle]
        test    [(PortDesc bx).Installed], 02h
        jz      @@Error

; Port ist offen, Aktion durchfÅhren

        mov     dx, [(PortDesc bx).MCR]         ; modem control register
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

PROC PASCAL _ComDTROn FAR   PortHandle: WORD

        mov     bx, [PortHandle]
        test    [(PortDesc bx).Installed], 02h  ; Port offen ?
        jz      @@Error                         ; Springe wenn Port nicht offen

; Port ist offen, DTR setzen

        mov     dx, [(PortDesc bx).MCR]         ; modem control register
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

PROC    PASCAL _ComRTSOff FAR PortHandle: WORD

        mov     bx, [PortHandle]
        test    [(PortDesc bx).Installed], 02h
        jz      @@Error
        cmp     [(PortDesc bx).Connection], 'M' ; Modem connection?
        je      @@Error                         ; Dann Aufruf nicht erlaubt

; Port ist offen, RTS rÅcksetzen

        mov     dx, [(PortDesc bx).MCR]         ; modem control register
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

PROC PASCAL _ComRTSOn FAR   PortHandle: WORD

        mov     bx, [PortHandle]
        test    [(PortDesc bx).Installed], 02h  ; Port offen ?
        jz      @@Error                         ; Springe wenn Port nicht offen
        cmp     [(PortDesc bx).Connection], 'M' ; Modem connection?
        je      @@Error                         ; Dann Aufruf nicht erlaubt

; Port ist offen, RTS setzen

        mov     dx, [(PortDesc bx).MCR]         ; modem control register
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

PROC    PASCAL _ComIsInstalled FAR   PortHandle: WORD

        mov     bx, [PortHandle]
        mov     bl, [(PortDesc bx).Installed]
        xor     ax, ax
        test    bl, 01h                         ; Installiert ?
        jz      @@L1                            ; Springe wenn Nein
        inc     ax                              ; Installiert !
@@L1:   ret

ENDP    _ComIsInstalled



; -------------------------------------------------------------------------
;
; _ComIsOpen
;
; Ergibt einen Wert != 0 wenn der Port geîffnet ist.
;

PROC    PASCAL _ComIsOpen FAR   PortHandle: WORD

        mov     bx, [PortHandle]
        mov     bl, [(PortDesc bx).Installed]
        xor     ax, ax
        test    bl, 02h                         ; Offen ?
        jz      @@L1                            ; Springe wenn Nein
        inc     ax                              ; Offen !
@@L1:   ret

ENDP    _ComIsOpen



; -------------------------------------------------------------------------
;
; _ComReceive
;
; Holt ein Zeichen aus dem Empfangspuffer. Warte bis ein Zeichen da ist. Bei
; Fehlern kommt -1 als Wert zurÅck.
;


PROC    PASCAL _ComReceive FAR   PortHandle: WORD
USES    SI, DI

        mov     si, [PortHandle]
        test    [(PortDesc si).Installed], 02h  ; Port offen ?
        jz      @@Error                         ; Springe wenn Nein

; Port ist offen, prÅfen ob Zeichen da

@@L1:   cmp     [(PortDesc si).RXCount], 0      ; Zeichen da ?
        jnz     @@L2                            ; Abholen wenn ja

; Es ist kein Zeichen da - erstmal warten, dabei die _ComWait Routine
; aufrufen

        mov     ax, 10                          ; 10ms
        push    ax
        call    _ComWait
        jmp     @@L1

; Zeichen ist da, holen. Da die Interrupt-Routinen nicht auf den
; RXStart-Zeiger zugreift, mÅssen die Interrupts nicht gesperrt
; werden.

@@L2:   mov     bx, [(PortDesc si).RXStart]     ; Ringpufferzeiger
        les     di, [(PortDesc si).RXBuf]       ; Zeiger auf Pufferbereich
        mov     al, [es:di+bx]                  ; Zeichen holen
        inc     bx                              ; Zeiger erhîhen
        mov     ah, 0                           ; Zeichen nach WORD wandeln
        push    ax                              ; ... und auf den Stack
        cmp     bx, [(PortDesc si).RXBufSize]   ; Warp-Around ?
        jb      @@L3                            ; Springe wenn Nein
        xor     bx, bx                          ; Ja, Ringpufferzeiger wird 0
@@L3:   mov     [(PortDesc si).RXStart], bx     ; Zeiger rÅckschreiben
        dec     [(PortDesc si).RXCount]         ; Ein Zeichen weniger...

; Flow-Control prÅfen. Zuerst Hardware (RTS/CTS), dann Software

        mov     bx, [(PortDesc si).RXBufSize]   ; Grî·e Puffer nach bx
        shr     bx, 1
        shr     bx, 1                           ; Grî·e/4 in bx
        cmp     [(PortDesc si).RXCount], bx     ; Puffer fast leer ?
        jae     @@L99                           ; Ende wenn nicht

        cli                                     ; Interrupts aus
        cmp     [(PortDesc si).Connection], 'M' ; Modem connection?
        jne     @@L4                            ; Springe wenn nein
        mov     dx, [(PortDesc si).MCR]
        in      al, dx                          ; MCR lesen
        or      al, MCR_RTS                     ; RTS setzen (Freigabe)
        out     dx, al

; Jetzt prÅfen ob XON/XOFF Flow-Control an-, und der Sender abgeschaltet wurde

@@L4:   cmp     [(PortDesc si).XonXoff], 'E'    ; Flow-Control an ?
        jne     @@L7                            ; Ende wenn nicht
        cmp     [(PortDesc si).HostOff], 00h    ; XOFF-Status ?
        jz      @@L7                            ; Ende wenn nicht
        mov     bx, [(PortDesc si).RXBufSize]   ; Grî·e Puffer nach bx

; Der Host ist gestoppt, der Puffer aber wieder leer genug. XON senden.

@@L5:   cmp     [(PortDesc si).MustSend], 00h   ; Zeichen im Puffer ?
        je      @@L6                            ; Springe wenn Nein

; Es ist noch ein Steuerzeichen im Puffer. Warten bis es weg ist.

        sti                                     ; Interrupts freigeben
        mov     ax, 10                          ; 10 ms
        push    ax
        call    _ComWait                        ; Warteroutine aufrufen
        mov     si, [PortHandle]                ; Handle neu laden
        cli                                     ; Interrupts wieder aus
        jmp     @@L5                            ; und neu prÅfen...

; Das Kontrollzeichen kann ausgegeben werden

@@L6:   mov     al, XON
        call    SendII                          ; XON senden
@@L7:   sti                                     ; Interrupts wieder freigeben

; Zeichen vom Stack und Ende

@@L99:  pop     ax
        ret

; Fehlereinsprung wenn Port nicht offen

@@Error:call    _ComError                       ; Fehler melden
        mov     ax, -1                          ; Returncode -1
        ret


ENDP    _ComReceive



; -------------------------------------------------------------------------
;
; _ComRXCount
;
; Liefert die Anzahl Bytes im Empfangspuffer.
;

PROC    PASCAL _ComRXCount FAR PortHandle: WORD

        mov     bx, [PortHandle]
        test    [(PortDesc bx).Installed], 02h
        jz      @@Error

; Alles klar, Anzahl liefern und Ende

        mov     ax, [(PortDesc bx).RXCount]
@@L99:  ret

; Fehlereinsprung wenn Port nicht offen

@@Error:call    _ComError
        ret

ENDP    _ComRXCount


; -------------------------------------------------------------------------
;
; _ComRXSize
;
; Liefert die Grî·e des Empfangspuffers.
;

PROC PASCAL _ComRXSize FAR   PortHandle: WORD

        mov     bx, [PortHandle]
        mov     ax, [(PortDesc bx).RXBufSize]
        ret

ENDP    _ComRXSize


; -------------------------------------------------------------------------
;
; _ComRXClear
;
; Lîscht den kompletten Empfangspuffer. Der Port mu· offen sein.
;

PROC    PASCAL _ComRXClear  FAR     PortHandle: WORD

        mov     bx, [PortHandle]
        test    [(PortDesc bx).Installed], 02h
        jz      @@Error

; Alles klar, Port ist offen

        xor     ax, ax
        cli                             ; Interrupts sperren
        mov     [(PortDesc bx).RXStart], ax
        mov     [(PortDesc bx).RXEnd], ax
        mov     [(PortDesc bx).RXCount], ax
        sti                             ; Interrupts freigeben

@@L99:  ret

; Fehlereinsprung wenn Port nicht offen

@@Error:call    _ComError
        ret

ENDP    _ComRXClear



; -------------------------------------------------------------------------
;
; _ComTXCount
;
; Liefert die Anzahl belegter Bytes im Sendepuffer
;

PROC    PASCAL _ComTXCount FAR   PortHandle: WORD

        mov     bx, [PortHandle]
        test    [(PortDesc bx).Installed], 02h
        jz      @@Error                         ; Springe wenn nicht offen

; Port ist offen. Anzahl holen und Ende

        mov     ax, [(PortDesc bx).TXCount]
@@L99:  ret

; Fehlereinsprung wenn Port ist nicht offen.

@@Error:call    _ComError
        ret

ENDP    _ComTXCount


; -------------------------------------------------------------------------
;
; _ComTXFree
;
; Liefert die Anzahl freier Bytes im Sendepuffer
;

PROC    PASCAL _ComTXFree FAR   PortHandle: WORD

        mov     bx, [PortHandle]
        test    [(PortDesc bx).Installed], 02h
        jz      @@Error                         ; Springe wenn nicht offen

; Port ist offen. Grî·e - Belegte Bytes rechnen.
; VORSICHT: Dieser Wert kann negativ werden, da sich evtl. zusÑtzliche
; Kontrollzeichen im Puffer befinden kînnen. In einem solchen Fall
; einfach 0 liefern.

        mov     ax, [(PortDesc bx).TXBufSize]   ; Gesamtgrî·e
        dec     ax                              ; Maximale Grî·e fÅr Benutzer
        sub     ax, [(PortDesc bx).TXCount]
        jnc     @@L99                           ; Springe wenn >= 0
        xor     ax, ax                          ; if (ax < 0) then ax := 0;
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

PROC    PASCAL _ComTXSize FAR   PortHandle: WORD

        mov     bx, [PortHandle]
        mov     ax, [(PortDesc bx).TXBufSize]
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

PROC    PASCAL _ComTXClear  FAR     PortHandle: WORD

        mov     bx, [PortHandle]
        test    [(PortDesc bx).Installed], 02h
        jz      @@Error

; Alles klar, Port ist offen

        xor     cx, cx                          ; Neue Anzahl Zeichen
        pushf                                   ; I-Flag retten
        cli                                     ; Interrupts sperren
        mov     ax, [(PortDesc bx).TXEnd]
        cmp     [(PortDesc bx).MustSend], 00h   ; Steuerzeichen im Puffer ?
        jz      @@L2                            ; Springe wenn Nein

; Ein Zeichen im Puffer lassen

        inc     cx                              ; Anzahl = 1 nach Clear
        or      ax, ax                          ; Index schon 0?
        jnz     @@L1                            ; Springe wenn nein
        mov     ax, [(PortDesc bx).TXBufSize]   ; Grî·e laden
@@L1:   dec     ax                              ; - 1

; Puffer lîschen

@@L2:   mov     [(PortDesc bx).TXStart], ax
        mov     [(PortDesc bx).TXCount], cx     ; Neue Anzahl
        popf                                    ; Altes I-Flag

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

PROC    PASCAL _ComSend FAR   PortHandle: WORD, C: BYTE
USES    SI, DI

        mov     si, [PortHandle]
        test    [(PortDesc si).Installed], 02h  ; Port offen ?
        jz      @@Error                         ; Springe wenn nein

; Port ist offen, prÅfen ob Platz im Puffer

        mov     dx, [(PortDesc si).TXBufSize]   ; Grî·e des Puffers nach dx
        cmp     [(PortDesc si).TXCount], dx     ; noch Platz ?
        jae     @@L4                            ; Springe wenn Nein

; Es ist genug Platz. Zeichen schreiben.

        mov     al, [C]                         ; Zeichen holen
        les     di, [(PortDesc si).TXBuf]       ; Zeiger auf Puffer holen
        cli                                     ; Keine Interrupts
        mov     bx, [(PortDesc si).TXEnd]       ; Pufferzeiger holen
        mov     [es:di+bx], al                  ; Zeichen in Puffer schreiben
        inc     bx                              ; Pufferzeiger erhîhen
        cmp     bx, dx                          ; Wrap-Around ?
        jb      @@L1                            ; Springe wenn Nein
        xor     bx, bx                          ; Ringpufferzeiger auf 0 wenn ja
@@L1:   mov     [(PortDesc si).TXEnd], bx       ; Pufferzeiger rÅckschreiben
        inc     [(PortDesc si).TXCount]         ; Ein Zeichen mehr

; Wenn notwendig die Sende-Interrupts freischalten

        mov     dx, [(PortDesc si).IER]         ; Interrupt enable register
        in      al, dx                          ; ... lesen
        test    al, 02h                         ; Interrupts frei ?
        jnz     @@L2                            ; Ja, alles klar
        cmp     [(PortDesc si).PCOff], 00h      ; XOFF-Status ?
        jnz     @@L2                            ; Ja, nix unternehmen
        cmp     [(PortDesc si).DSR_CTS_Ok], 00h ; Statusleitungen ok ?
        jz      @@L2                            ; Nein, nix unternehmen
        mov     al, 00001111B                   ;
        out     dx, al                          ; TX-Ints freischalten
@@L2:   sti                                     ; Interrupts wieder frei
        mov     al, [C]
        mov     ah, 0                           ; Zeichen nach ax

; Ende

@@L99:  ret

; Einsprung wenn Puffer voll

@@L4:   inc     [(PortDesc si).ETXOverFlow]     ; FehlerzÑhler erhîhen
        mov     ax, -1                          ; Fehlerkennung
        ret                                     ; Und Ende

; Einsprung wenn Port nicht offen

@@Error:call    _ComError                       ; Fehler melden
        mov     ax, -1                          ; Fehlerkennung
        ret                                     ; Ende

ENDP    _ComSend



; -------------------------------------------------------------------------
;
; Interne Routine. Wird aufgerufen von ComReceive und vom RX-Interrupthandler
; wenn Flow-Control Zeichen verschickt werden mÅssen.
; si mu· auf den Port-Deskriptor zeigen, Interrupts mÅssen gesperrt sein.
; al enthÑlt das zu sendende Zeichen.
;

PROC    SENDII  NEAR

        push    bx
        push    dx                              ; Register werden benîtigt
        push    di
        push    es

        mov     bx, [(PortDesc si).TXStart]     ; Ringpufferzeiger holen
        les     di, [(PortDesc si).TXBuf]       ; Zeiger auf Pufferbereich
        mov     dx, [(PortDesc si).TXBufSize]   ; Puffergrî·e nach dx
        cmp     [(PortDesc si).TXCount], dx     ; Puffer voll ?
        jb      @@L2                            ; Springe wenn Nein

; Im Puffer ist kein Platz mehr (obwohl das eigentlich immer gewÑhrleistet
; werden sollte !). Das kann eigentlich nur passieren, wenn direkt
; hintereinander zwei Flow-Control Zeichen geschickt werden...

        mov     [es:bx+di], al                  ; Zerstîrt erstes Zeichen
        inc     [(PortDesc si).ETXOverFlow]     ; FehlerzÑhler erhîhen
        jmp     @@L2                            ; einfach weiter

; Im Puffer ist Platz, Zeichen an die erste Stelle schreiben

        dec     bx                              ; Ergibt 0FFFF wenn bx = 0
        jns     @@L1                            ; Springe wenn kein Wrap
        mov     bx, dx                          ; Sonst Grî·e-1 nach bx
        dec     bx
@@L1:   mov     [es:bx+di], al                  ; Zeichen schreiben
        mov     [(PortDesc si).TXStart], bx     ; Zeiger rÅckschreiben
        inc     [(PortDesc si).TXCount]         ; Ein Zeichen mehr ...

; Flag fÅr extra Zeichen setzen

@@L2:   mov     [(PortDesc si).MustSend], 01h   ; Flag setzen

; Falls notwendig Interrupts enablen

        mov     dx, [(PortDesc si).IER]         ; interrupt enable register
        in      al, dx                          ; Wert holen
        test    al, 02h                         ; TX-Ints frei ?
        jnz     @@L99                           ; Ja: Ende
        cmp     [(PortDesc si).DSR_CTS_Ok], 00h ; Statusleitungen ok ?
        jz      @@L99                           ; Nein, Pech ..
        mov     al, 00001111B                   ; modem & line status, rec, xmit
        out     dx, al                          ; ... freigeben

; Fertig !

@@L99:  pop     es
        pop     di
        pop     dx
        pop     bx
        ret

ENDP    SendII


; -------------------------------------------------------------------------
;
; _ComBreak
;
; Sendet ein Break-Signal mit variabler LÑnge.
;

PROC    PASCAL _ComBreak FAR  PortHandle: WORD, BreakLen: WORD

        mov     bx, [PortHandle]
        test    [(PortDesc bx).Installed], 02h
        jz      @@Error                         ; Springe wenn Port nicht offen

; Port ist offen, Break senden

        mov     dx, [(PortDesc bx).LCR]
        pushf                                   ; I-Bit retten
        cli                                     ; Interrupts off
        in      al, dx                          ; get LCR
        or      al, 40h                         ; break bit on
        out     dx, al                          ; setzen
        popf                                    ; Altes I-Bit

; Warte-Routine aufrufen

        push    [BreakLen]
        call    _ComWait

; Register sind futsch, neu laden

        mov     bx, [PortHandle]
        mov     dx, [(PortDesc bx).LCR]

; Und Break-Bit rÅcksetzen

        pushf
        cli
        in      al, dx
        and     al, NOT 40h
        out     dx, al
        popf

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

PROC    PASCAL _ComModemStatus FAR  PortHandle: WORD

        mov     bx, [PortHandle]
        test    [(PortDesc bx).Installed], 02h  ; Port Offen ?
        jz      @@Error                         ; Springe wenn Nein
        mov     dx, [(PortDesc bx).MSR]         ; Modem status register
        in      al, dx                          ; Wert holen
        mov     ah, 0                           ; Wert in ax
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

        push    si                              ; Register retten
        mov     si, OFFSET _ComPort1            ; Deskriptor laden
        jmp     SHORT IntCommon                 ; Gemeinsame Routine

ENDP    _IntCom1

; -------------------------------------------------------------------------
;
; Interrupt-Handler fÅr COM2
;

PROC    PASCAL _IntCom2 FAR

        push    si                              ; Register retten
        mov     si, OFFSET _ComPort2            ; Deskriptor laden
        jmp     SHORT IntCommon                 ; Gemeinsame Routine

ENDP    _IntCom2

; -------------------------------------------------------------------------
;
; Interrupt-Handler fÅr COM3
;

PROC    PASCAL _IntCom3 FAR

        push    si                              ; Register retten
        mov     si, OFFSET _ComPort3            ; Deskriptor laden
        jmp     SHORT IntCommon                 ; Gemeinsame Routine

ENDP    _IntCom3

; -------------------------------------------------------------------------
;
; Interrupt-Handler fÅr COM4
;

PROC    PASCAL _IntCom4 FAR

        push    si                              ; Register retten
        mov     si, OFFSET _ComPort4            ; Deskriptor laden
        jmp     SHORT IntCommon                 ; Gemeinsame Routine

ENDP    _IntCom4

; -------------------------------------------------------------------------
;
; Gemeinsame Interrrupt-Routine fÅr alle Ports
;

PROC    IntCommon FAR

        push    ax
        push    bx
        push    cx
        push    dx
        push    di
        push    bp
        push    es
        push    ds                              ; Register retten

        mov     ax, DGROUP
        mov     ds, ax                          ; Datensegment setzen

; Rausfinden, was es fÅr ein Interrupt war und entsprechend verzweigen

@@RePoll:
        mov     dx, [(PortDesc si).IIR]         ; interrupt id register
        in      al, dx
        test    al, 01h                         ; "no interrupt present" ?
        jnz     @@L98                           ; Ja: Ende
        mov     bl, al                          ; Wert nach bx
        and     bx, 000Eh                       ; unerwÅnschte Bits ausmaskieren
        jmp     [@@IntDispatch+bx]              ; Behandlungsroutine

LABEL @@IntDispatch WORD
        dw      @@MSInt                         ; 0: Modem status int
        dw      @@TXInt                         ; 2: Transmitter int
        dw      @@RXInt                         ; 4: Receiver int
        dw      @@LSInt                         ; 6: Line status int
        dw      @@RePoll                        ; 8: Reserviert
        dw      @@RePoll                        ; A: Reserviert
        dw      @@RXInt                         ; C: FIFO timeout, wie RXInt
        dw      @@RePoll                        ; E: Reserviert

; Interrupt-Controller verstÑndigen. Problemlos mîglich, weil die Interrupts
; sowieso noch gesperrt sind.

@@L98:  mov     dx, [(PortDesc si).IER]         ; Interrupt enable register
        in      al, dx                          ; Wert lesen
        mov     ah, al                          ; ... und nach ah retten
        xor     al, al                          ; Interrupts disablen
        out     dx, al

        mov     al, 20h                         ; EOI
        cmp     [(PortDesc si).IC2Mask], 00     ; "Hoher" Interrupt ?
        jz      @@L99                           ; Springe wenn Nein
        out     0A0h, al                        ; EOI an Controller #2
@@L99:  out     20h, al                         ; EOI an Controller #1

        mov     al, ah                          ; Alter IER-Wert
        out     dx, al                          ; ... restaurieren

        pop     ds
        pop     es
        pop     bp
        pop     di
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        pop     si
        iret

; -------------------------------------------------------------------------
;
; Line status interrupt
;

@@LSInt:mov     dx, [(PortDesc si).LSR]         ; line status register
        in      al, dx                          ; lesen
        shr     al, 1                           ; Bit wird nicht benîtigt
        xor     bx, bx                          ; bx = 0

        shr     al, 1                           ; overrun error prÅfen
        adc     [(PortDesc si).EOvrRun], bx     ; ZÑhler evtl. erhîhen
        shr     al, 1                           ; parity error prÅfen
        adc     [(PortDesc si).EParity], bx     ; ZÑhler evtl. erhîhen
        shr     al, 1                           ; framing error prÅfen
        adc     [(PortDesc si).EFrame], bx      ; ZÑhler evtl. erhîhen
        shr     al, 1                           ; break error prÅfen
        adc     [(PortDesc si).EBreak], bx      ; ZÑhler evtl. erhîhen

        jmp     @@RePoll                        ; Und neu abfragen

; -------------------------------------------------------------------------
;
; Modem status interrupt
;

@@MSInt:mov     dx, [(PortDesc si).MSR]         ; modem status register
        in      al, dx                          ; lesen

        cmp     [(PortDesc si).Connection], 'D' ; direkte Verbindung ?
        je      @@B1                            ; dann immer frei

        and     al, 30h                         ; DSR/CTS maskieren
        cmp     al, 30h                         ; beide da ?
        mov     al, 00h                         ; Flag fÅr Nein
        jne     @@B2                            ; Springe wenn Nein
@@B1:   mov     al, 01h                         ; Flag fÅr Ja
@@B2:   mov     [(PortDesc si).DSR_CTS_Ok], al  ; Flag setzen

; Jetzt auf jeden Fall immer alle Interrupts freigeben. Falls keine Daten
; anliegen werden die Ints in der TXInt-Routine wieder gesperrt.

        mov     dx, [(PortDesc si).IER]         ; interrupt enable register
        mov     al, 00001111B
        out     dx, al
        jmp     @@RePoll                        ; und neu pollen

; -------------------------------------------------------------------------
;
; Transmit interrupt
;

@@TXInt:cmp     [(PortDesc si).DSR_CTS_Ok], 0   ; Hardware Ok ?
        je      @@C5                            ; Nein: Ints abklemmen und Ende

        mov     cx, 1                           ; Anzahl Flow-Control Zeichen
        cmp     [(PortDesc si).MustSend], 00h   ; Flow-Control Zeichen ?
        jnz     @@C2                            ; Ja, immer senden
        cmp     [(PortDesc si).PCOff], 00h      ; Flow-Control ok ?
        jnz     @@C5                            ; Nein: Ints abklemmen und Ende

        mov     cl, [(PortDesc si).FIFOLen]     ; Maximale Anzahl FIFO
        cmp     cx, [(PortDesc si).TXCount]     ; > Anzahl im Puffer ?
        jle     @@C1                            ; Springe wenn kleiner
        mov     cx, [(PortDesc si).TXCount]     ; Sonst Anzahl im Puffer nehmen
@@C1:   jcxz    @@C5                            ; Nichts zu senden

@@C2:   mov     bx, [(PortDesc si).TXStart]     ; Ringpufferzeiger
        les     di, [(PortDesc si).TXBuf]       ; Zeiger auf Sendepuffer
        mov     dx, [(PortDesc si).DataReg]     ; Datenregister
        sub     [(PortDesc si).TXCount], cx     ; Anzahl passend vermindern

@@C3:   mov     al, [es:bx+di]                  ; Zeichen holen
        out     dx, al                          ; und schreiben
        inc     bx                              ; Pufferzeiger erhîhen
        cmp     bx, [(PortDesc si).TXBufSize]   ; Wrap ?
        jb      @@C4
        xor     bx, bx                          ; Wrap !
@@C4:   loop    @@C3                            ; und nÑchstes Zeichen

        mov     [(PortDesc si).TXStart], bx     ; Zeiger rÅckschreiben
        mov     [(PortDesc si).MustSend], 0     ; Flow-Control ist weg

        jmp     @@RePoll                        ; Fertig !

; Es gibt nichts zu senden, oder es darf nicht gesendet werden:
; TX-Interrupts abklemmen

@@C5:   mov     dx, [(PortDesc si).IER]         ; interrupt enable register
        mov     al, 00001101B                   ; line & modem status, rec.
        out     dx, al
        jmp     @@RePoll

; -------------------------------------------------------------------------
;
; Receive interrupt
;

@@RXInt:mov     bx, [(PortDesc si).RXEnd]       ; Ringpufferzeiger holen
        les     di, [(PortDesc si).RXBuf]       ; Zeiger auf Pufferbereich holen
        mov     bp, [(PortDesc si).RXBufSize]   ; Puffergrî·e nach bp

@@D1:   mov     dx, [(PortDesc si).DataReg]     ; Datenregister
        in      al, dx                          ; lesen
        and     al, [(PortDesc si).ParityMask]  ; Parity-Bit ausmaskieren
        cmp     [(PortDesc si).XonXoff], 'E'    ; Flow Control ?
        jnz     @@D4                            ; Springe wenn Nein

; Flow-Control ist an, Ctrl-S und Ctrl-Q prÅfen

        cmp     al, XOFF                        ; Ctrl-S ?
        jnz     @@D2                            ; Nein: Skip
        mov     [(PortDesc si).PCOff], 01h      ; Ja: Stop !
        mov     al, 00001101B                   ; TX-Ints sperren
        jmp     @@D3                            ; Zeichen nicht speichern
@@D2:   cmp     al, XON                         ; Ctrl-Q ?
        jnz     @@D4                            ; Nein: Skip
        mov     [(PortDesc si).PCOff], 00h      ; Ja: Output freigeben
        mov     al, 00001111B
@@D3:   mov     dx, [(PortDesc si).IER]
        out     dx, al
        jmp     @@D7                            ; Zeichen nicht speichern

; Zeichen speichern

@@D4:   cmp     [(PortDesc si).RXCount], bp     ; Noch Platz ?
        jb      @@D5                            ; Springe wenn ja
        inc     [(PortDesc si).ERXOverflow]     ; FehlerzÑhler erhîhen
        jmp     @@D7                            ; Und nÑchstes Byte

@@D5:   mov     [es:bx+di], al                  ; Zeichen speichern
        inc     bx                              ; Zeiger erhîhen
        cmp     bx, bp                          ; Wrap-Araound ?
        jb      @@D6                            ; Springe wenn Nein
        xor     bx, bx
@@D6:   inc     [(PortDesc si).RXCount]         ; Anzahl im Puffer erhîhen

; PrÅfen, ob noch mehr Zeichen vorliegen (FIFO)

@@D7:   mov     dx, [(PortDesc si).LSR]         ; line status register
        in      al, dx                          ; ...lesen
        test    al, 01h                         ; noch Zeichen da ?
        jne     @@D1                            ; Ja: NÑchstes Zeichen lesen

; Es liegen keine Zeichen mehr vor. Pufferzeiger rÅckspeichern.

        mov     [(PortDesc si).RXEnd], bx

; Jetzt noch prÅfen, ob der Sender gestoppt werden mu·, wenn der Puffer
; zu voll wird.

        mov     ax, [(PortDesc si).RXBufSize]   ; Puffergrî·e nach ax
        shr     ax, 1                           ; / 2
        mov     bx, ax
        shr     ax, 1                           ; / 4
        add     ax, bx                          ; 3/4 Grî·e in ax
        cmp     [(PortDesc si).RXCount], ax     ; Puffer 3/4 voll ?
        jb      @@D99                           ; Nein: Kein Grund zum Abschalten

; Falls RTS/CTS verwendet wird, RTS abschalten

        cmp     [(PortDesc si).Connection], 'M' ; HW Control?
        jne     @@D8                            ; Springe wenn nein
        mov     dx, [(PortDesc si).MCR]
        in      al, dx                          ; Sonst MCR lesen...
        and     al, NOT MCR_RTS                 ; ...RTS off...
        out     dx, al                          ; ...und wieder schreiben

; Falls Software Flow Control verwendet wird, XOFF absenden

@@D8:   cmp     [(PortDesc si).XonXoff], 'E'    ; Enabled ?
        jnz     @@D99                           ; Nein: Fertig
        cmp     [(PortDesc si).HostOff], 00h    ; Schon XOFF gesandt ?
        jnz     @@D99                           ; Ja: Keins mehr senden
        mov     al, XOFF
        call    SendII                          ; Zeichen abschicken
        mov     [(PortDesc si).HostOff], 01h    ; und merken...

; Fertig, neu pollen

@@D99:  jmp     @@RePoll


ENDP    IntCommon

END
