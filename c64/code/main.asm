;==========================================================
; 6502 assembly template for VSCode
; created 2019 by Ingo Hinterding // awsm of Mayday
;
; Check github repo for setup instructions & help
; https://github.com/Esshahn/acme-assembly-vscode-template
; If you found this useful, I would be happy to know :)
; 
; https://www.twitter.com/awsm9000
; http://www.awsm.de
;==========================================================
 
;==========================================================
; LABELS
; Comment or uncomment the lines below 
; Depending on your target machine
;==========================================================

; VC20
;BGCOLOR       = $900f
;BORDERCOLOR   = $900f
;BASIC         = $1001
;SCREENRAM     = $1e00

; C16, C116, Plus/4
;BGCOLOR      = $ff15
;BORDERCOLOR  = $ff19
;BASIC        = $1001
;SCREENRAM    = $0c00

; C128
;BGCOLOR       = $d020
;BORDERCOLOR   = $d021
;BASIC         = $1c01
;SCREENRAM     = $0400

; C64
BGCOLOR         = $d020
BORDERCOLOR     = $d021
BASIC           = $0801
SCREENRAM       = $0400
GETIN           = $ffe4

;==========================================================
; BASIC header
;==========================================================

* = BASIC

                !byte $0b, $08
                !byte $E3                     ; BASIC line number:  $E2=2018 $E3=2019 etc       
                !byte $07, $9E
                !byte '0' + entry % 10000 / 1000        
                !byte '0' + entry %  1000 /  100        
                !byte '0' + entry %   100 /   10        
                !byte '0' + entry %    10             
                !byte $00, $00, $00           ; end of basic


;==========================================================
; CODE
;==========================================================

entry:

    lda $d018
    pha

    ; change address of screen ram - switch vic-II bank
    ; dirty way
    ;lda #$2
    ;sta $dd00

    ; more clear way - just touch desired bites
    lda $dd00
    pha

    and #%11111100
    ; 2 - bank1: $4000-$7fff
    ora #%00000010
    sta $dd00

    ; select bitmap block
    lda #120
    sta $d018

    ; set screen ram 1000 bytes

    lda BGCOLOR
    pha
    lda #0
    sta BGCOLOR

    lda #%00010000
set_colors:
    sta $5c00, x
    sta $5d00, x
    sta $5e00, x
    sta $5f00, x
    inx
bne set_colors

    ; enable bitmap mode
    lda $d011
    pha
    ora #32
    sta $d011

    ; wait for key
input_get:
    jsr GETIN
    beq input_get

    ; cleanup

    ; disable bitmap mode and return back to previous mode
    pla
    sta $d011

    pla
    sta BGCOLOR

    ; return bank back
    pla
    sta $dd00

    ; return back bitmap block
    pla
    sta $d018

rts

* = $6000
!binary "image_c64.raw", 8000