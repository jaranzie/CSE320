;
; VT52 Simulator Program
;	Also includes insert/delete line/char functions of a
;	Teleray 1061
;

; For two bits of blank between each character, instead of one,
; uncomment the next line

;#define	WIDTH7	0

call	=	170756
ret	=	171013
ret0	=	171010
ret2	=	171000
load	=	171215

SIVEC0	=	3004
SOVEC0	=	3010
SIVEC1	=	3014
SOVEC1	=	3020
LDADDR	=	150000

; Interrupt Vectors

.	=	SIVEC0

	mov	r7, @sp+
	b	intr

.	=	SOVEC0

	mov	r7, @sp+
	b	intr

.	=	SIVEC1

	mov	r7, @sp+
	b	intr

.	=	SOVEC1

	mov	r7, @sp+
	b	intr

.	=	LDADDR

	b	start

intr:	mov	r1, @sp+
	gswd	r1
	mov	r1, @sp+
	dis
	tci
	clrc
	mov	r2, @sp+
	mov	sp, r1
	sub	4, r1
	mov	@r1, r2			; get back old pc
	mov	r0, @r1			; clobber the intru vec info
	sub	SIVEC0 + 1, r2
	sar	2, r2
	add	itable, r2
	mov	r3, @sp+
	mov	r4, @sp+
	mov	r5, @sp+
	mov	@r2, pc			; branch to service routine

iret:	mov	@-sp, r5
	mov	@-sp, r4
	mov	@-sp, r3
	mov	@-sp, r2
	mov	@-sp, r1
	rswd	r1
	mov	@-sp, r1
	mov	@-sp, r0
	eis
	mov	@-sp, pc

itable:	siint0
	soint0
	siint1
	soint1

;
; Serial Interface Driver Routines - with interrupts
;

B9600	=	0
B4800	=	40
B2400	=	100
B1200	=	140
B600	=	200
B300	=	240
B150	=	300
B134.5	=	340

NDB1	=	1
NDB2	=	2
NSB	=	4
NPB	=	10
POE	=	20

TBMT	=	1
ODA	=	2
ROR	=	4
RFE	=	10
RPE	=	20
IEN	=	400

STAT0	=	177762
DATA0	=	177763
STAT1	=	177764
DATA1	=	177765

init0:	mov	STAT0, r1
	mov	r0, @r1
	jmp	ret

init1:	mov	STAT1, r1
	mov	r0, @r1
	jmp	ret

pchr0:	mov	sobuf0, r1
	mov	STAT0, r2
	b	pchr

pchr1:	mov	sobuf1, r1
	mov	STAT1, r2

pchr:	dis
	jsr	r5, call
	putbuf
	jsr	r4, sostrt
	eis
	b	ret

ischr0:	mov	sibuf0, r1
	b	ischr

ischr1:	mov	sibuf1, r1

ischr:	dis
	jsr	r5, call
	size
	eis
	b	ret0

gchr0:	mov	sibuf0, r1
	b	gchr

gchr1:	mov	sibuf1, r1

gchr:	dis
	jsr	r5, call
	size
	tst	r0
	bnze	gch.1
	eis
	b	ret
gch.1:	jsr	r5, call
	getbuf
	eis
	b	ret0

siint0:	mov	sibuf0, r1
	mov	STAT0, r2
	b	siint

siint1:	mov	sibuf1, r1
	mov	STAT1, r2

siint:	mov	@r2, r0			; get status register
	and	ODA, r0			; is there a character?
	bze	iret			; no, return
	inc	r2			; address of data register
	mov	@r2, r0			; get character
	jsr	r5, call
	putbuf				; store character
	bze	iret			; and return

soint0:	mov	sobuf0, r1
	mov	STAT0, r2
	jsr	r4, sostrt
	b	iret

soint1:	mov	sobuf1, r1
	mov	STAT1, r2
	jsr	r4, sostrt
	b	iret

sostrt:	jsr	r5, call
	size				; anything in queue
	tst	r0
	bze	sos.1			; no return
	mov	@r2, r0
	and	TBMT, r0		; UART ready?
	bze	sos.1			; no, return
	jsr	r5, call
	getbuf
	inc	r2			; get data register address
	mov	r0, @r2			; send data
sos.1:	mov	r4, pc			; and return

.incld	buf.16

sibuf0:	sibuf0 + 3
	sibuf0 + 3
	8.
	. = .+8.

sobuf0:	sobuf0 + 3
	sobuf0 + 3
	8.
	. = .+8.

sibuf1:	sibuf1 + 3
	sibuf1 + 3
	1024.
	. = .+1024.

sobuf1:	sobuf1 + 3
	sobuf1 + 3
	8.
	. = .+1024.

;
; Vt52 Simulator Routines
;

; Initialize various devices and execute the simulation loop

start:	dis
	mov	B1200|NDB1|NDB2|NPB|NSB|IEN, r0
	jsr	r5, call
	init0
	mov	B300|NDB1|NDB2|NPB|NSB|IEN, r0
	jsr	r5, call
	init1
	jsr	r5, call
	vinit
	eis
loop:	jsr	r5, call	; main loop
	sim
	b	loop

; Idle routine is called when waiting for characters to process
;	It copies from SI0 -> SO1 and redisplays the cursor

BLKCNT	=	250.

idle:	jsr	r5, call
	ischr0
	tst	r0
	bnze	idl.1
	jsr	r5, call
	vcrdsp
	mov	@blink, r0
	dec	r0
	bpl	idl.2
	mov	BLKCNT, r0
	mov	r0, @blink
	mov	@cnoclr, r0
	tst	r0
	bze	ret
	clr	r0
	mov	r0, @cnoclr
	b	vcdisp
idl.2:	mov	r0, @blink
	cmp	BLKCNT/2, r0
	bneq	ret
	mov	@cnoclr, r0
	tst	r0
	bnze	ret
	mov	1, r0
	mov	r0, @cnoclr
	b	vcdisp
idl.1:	jsr	r5, call
	gchr0
	jsr	r5, call
	pchr1
	b	ret

;
; Simulate the action of a VT52 for one command
;

sim:	jsr	r4, vgchr	; If a printing character,
	and	177, r0		; don't bother to rediplay cursor
	cmp	DEL, r0
	bge	ret
	cmp	SPACE, r0
	blt	sim.1
	jsr	r5, call
	vccrs
	b	vcrt
sim.1:	jsr	r5, call	; We must redisplay cursor
	vcrdsp			; before processing cursor positioning
	cmp	CR, r0		; commands so we know whether
	beq	vcr		; we wipe out the cursor or not
	cmp	LF, r0
	beq	vlf
	cmp	TAB, r0
	beq	vtab
	cmp	BS, r0
	beq	vclf
	cmp	ESC, r0
	beq	vesc
	cmp	BEL, r0
	beq	ret
	cmp	DC1, r0
	beq	ret
	b	ret

vgchr:	jsr	r5, call	; Call this to wait for additional
	ischr1			; characters in multi-character commands
	tst	r0
	bnze	vgc.1
	jsr	r5, call
	idle
	b	vgchr
vgc.1:	jsr	r5, call
	gchr1
	mov	r4, pc

;
; Bitmap Video Display Routines
;

; NOTE: the following constants have been named to make the
;	code easier to read.  This does not automatically mean
;	that they can be changed here without modifying code
;	elsewhere.  It is probably safe to change NLINES,
;	VMEM, VSIZE, and NCPL.  To change NSPL, NWPS, LSIZE,
;	requies changes to vparm and vchbo.

LSIZE	=	NSPL * NWPS	; # of words per char line
NSPL	=	10.		; # of scans per char line
NWPS	=	32.		; # of words per scan
VMEM	=	130000		; start of video memory
VSIZE	=	20000		; size of video memory
NLINES	=	24.		; number of char lines per screen
#ifndef WIDTH7
NCPL	=	85.
#else WIDTH7
NCPL	=	73.
#endif WIDTH7

SCRREG	=	177770
INTLAC	=	1000
INVERT	=	400

CR	=	15
LF	=	12
ESC	=	33
BS	=	10
TAB	=	11
BEL	=	7
DEL	=	177
SPACE	=	40
DC1	=	21

CSLOC	=	172506
#ifdef WIDTH7
msktab	=	172446
#endif WIDTH7

;
; Initialize the video display
;

vinit:	mov	INVERT, r0	; zero scroll counter, no inverse video
	mov	r0, @SCRREG
	mov	r0, @vidflg
	jsr	r5, call
	vclrsc			; clear screen
	clr	r0
	mov	r0, @scroll	; clear variables
	mov	r0, @coffs
	mov	r0, @ocoffs
	mov	r0, @cchgd
	mov	r0, @cnoclr
	mov	r0, chpos
	mov	VMEM + ((NLINES-1) * LSIZE), r0
	mov	r0, @cbase
	mov	r0, @ocbase
	mov	CSLOC, r0	; get character set location
	mov	r0, @csloc
	b	ret

vcr:	clr	r0		; perform a carriage return
	mov	r0, @coffs
	mov	r0, @chpos
	mov	@cbase, r0
	and	~(NWPS-1), r0
	mov	r0, @cbase
	b	vscchg

vesc:	jsr	r4, vgchr	; process an ESC command
	and	177, r0
	sub	'A, r0
	cmp	ecmdtab-cmdtab, r0
	blge	ret
	add	cmdtab, r0
	mov	r0, r1
	mov	@r1, pc

cmdtab:	vcup		; A
	vcdn		; B
	vcrt		; C
	vclf		; D
	vclr		; E
	ret		; F
	ret		; G
	vhome		; H
	vrlf		; I
	vkeos		; J
	vkeol		; K
	viln		; L
	vdln		; M
	ret		; N
	ret		; O
	vichr		; P
	vdchr		; Q
	ret		; R
	ret		; S
	ret		; T
	ret		; U
	ret		; V
	ret		; W
	ret		; X
	vpos		; Y
	ret		; Z
ecmdtab	=	.

vhome:	clr	r2		; home the cursor
	mov	NLINES-1, r1
	b	vcmov

vclr:	jsr	r5, call	; home and clear
	vhome
	mov	1, r0
	mov	r0, @cnoclr
	b	vclrsc

vpos:	jsr	r4, vgchr	; cursor positioning
	and	177, r0
	sub	SPACE, r0
	neg	r0
	add	NLINES-1, r0
	mov	r0, r1
	jsr	r4, vgchr
	and	177, r0
	sub	SPACE, r0
	mov	r0, r2
	cmp	NLINES, r1
	blge	ret
	cmp	NCPL, r2
	blge	ret
	b	vcmov

vtab:	mov	@chpos, r1	; process a tab
	and	7, r1
	neg	r1
	add	8., r1
vt.1:	jsr	r5, call
	vcrt
	dec	r1
	bnze	vt.1
	b	ret

vkeos:	clr	r0		; clear from cursor to end of screen
	jsr	r5, vpbase	; first figure out physical address
	mov	r0, r4		; of logical bottom of screen, then
	mov	@cbase, r0	; number of words from bottom of
	and	~(NWPS-1), r0	; screen to just before line containing
	jsr	r5, vlbase	; the cursor.  Clear this area, then
	mov	177777, r1	; call vkeol to clear the line
	tst	r0		; containing the cursor
vks.1:	bze	vkeol
	cmp	VMEM + VSIZE, r4
	bllt	vks.2
	sub	VSIZE, r4
vks.2:	mov	r1, @r4+
	dec	r0
	b	vks.1

vkeol:	mov	@coffs, r1	; clear from the cursor to end of line
	add	kmsktb, r1	; figure out which bits to clear in the
	mov	@r1, r0		; word containing the LHS of the cursor,
	mov	177777, r1	; then process each scan line, first
	mov	NWPS-1, r4	; treating the word with the LHS of
	mov	@cbase, r3	; the cursor, and then clearing the
	mov	NSPL, r5	; rest of the words in the line.
vkl.1:	mov	r3, @sp+
	mov	@r3, r2
	com	r2
	and	r0, r2
	com	r2
	mov	r2, @r3
vkl.2:	inc	r3
	mov	r3, @sp+
	and	r4, r3
	bze	vkl.3
	mov	@-sp, r3
	mov	r1, @r3
	b	vkl.2
vkl.3:	mov	@-sp, r3
	mov	@-sp, r3
	add	NWPS, r3
	cmp	VMEM + VSIZE, r3
	bllt	vkl.4
	sub	VSIZE, r3
vkl.4:	dec	r5
	bge	vkl.1
	mov	1, r0
	mov	r0, @cnoclr
	b	vscchg

kmsktb:	.word	000000, 000001, 000003, 000007
	.word	000017, 000037, 000077, 000177
	.word	000377, 000777, 001777, 003777
	.word	007777, 017777, 037777, 077777

;
; move the cursor to horiz (in r2), vert (in r1)
;
;	Bounds checking must be done by the caller.
;

vcmov:	mov	r2, @chpos
	jsr	r5, call
	vparm
	mov	r1, @coffs
	mov	r2, @cbase
	b	vscchg

;
; display the cursor
;

vcdisp:	mov	SPACE, r3		; cursor = space
	mov	1, r1
	mov	r1, @cflg
	mov	@ocoffs, r1
	mov	@ocbase, r2
	b	vchbo

vcclr:	mov	@cnoclr, r0
	tst	r0
	bnze	ret
	jsr	r5, vcdisp
	mov	1, r0
	mov	r0, @cnoclr
	b	ret

vccrs:	mov	@cchgd, r1		; place a character at the
	tst	r1			; current cursor position
	bnze	vcc.1
	com	r1
	mov	r1, @cnoclr
vcc.1:	mov	r0, r3
	mov	@coffs, r1
	mov	@cbase, r2
	b	vchbo

;
; Move the cursor up, down, left, and right, without scrolling
;

vcup:	mov	@cbase, r0
	mov	r0, r2
	jsr	r5, vlbase
	cmp	LSIZE * (NLINES-1), r0
	blge	ret
	add	LSIZE, r2
	cmp	VMEM + VSIZE, r2
	bllt	vcu.1
	sub	VSIZE, r2
vcu.1:	mov	r2, @cbase
	b	vscchg

vcdn:	mov	@cbase, r0
	mov	r0, r2
	jsr	r5, vlbase
	cmp	LSIZE, r0
	bllt	ret
	sub	LSIZE, r2
	cmp	VMEM, r2
	blge	vcd.1
	add	VSIZE, r2
vcd.1:	mov	r2, @cbase
	b	vscchg

vcrt:	mov	@cbase, r0
	mov	@coffs, r1
	mov	r0, r2
	and	NWPS-1, r2
	cmp	NWPS-1, r2
	blt	vcr.1
#ifndef WIDTH7
	cmp	4, r1
#else WIDTH7
	cmp	2, r1
#endif WIDTH7
	bgt	ret
#ifndef WIDTH7
vcr.1:	add	6, r1
#else WIDTH7
vcr.1:	add	7, r1
#endif WIDTH7
	cmp	16., r1
	blt	vcr.2
	sub	16., r1
	inc	r0
vcr.2:	mov	r0, @cbase
	mov	r1, @coffs
	mov	@chpos, r0
	inc	r0
	mov	r0, @chpos
	b	vscchg

vclf:	mov	@coffs, r0
#ifndef WIDTH7
	sub	6, r0
#else WIDTH7
	sub	7, r0
#endif WIDTH7
	bge	vcl.1
	add	16., r0
	mov	@cbase, r1
	mov	r1, r2
	and	NWPS-1, r2
	bze	ret
	dec	r1
	mov	r1, @cbase
vcl.1:	mov	r0, @coffs
	mov	@chpos, r0
	dec	r0
	mov	r0, @chpos
	b	vscchg

; Perform line feed function, with scrolling, if necessary

vlf:	mov	@cbase, r0
	mov	r0, r1
	sub	LSIZE, r1
	cmp	VMEM, r1
	blge	vlf.1
	add	VSIZE, r1
vlf.1:	mov	r1, @cbase
	jsr	r5, vlbase
	cmp	LSIZE, r0
	blge	vscchg
	jsr	r5, call
	vclrlc
	mov	@scroll, r1
	sub	NSPL, r1
	and	377, r1
	mov	r1, @scroll
	xor	@vidflg, r1
	mov	r1, @SCRREG
	jsr	r5, call
	vclgap
	b	vscchg

; Perform reverse line feed function, with scrolling

vrlf:	mov	@cbase, r0
	mov	r0, r1
	add	LSIZE, r1
	cmp	VMEM + VSIZE, r1
	bllt	vrl.1
	sub	VSIZE, r1
vrl.1:	mov	r1, @cbase
	jsr	r5, vlbase
	cmp	LSIZE * (NLINES-1), r0
	bllt	vscchg
	jsr	r5, call
	vclrlc
	mov	@scroll, r1
	add	NSPL, r1
	and	377, r1
	mov	r1, @scroll
	xor	@vidflg, r1
	mov	r1, @SCRREG
	jsr	r5, call
	vscchg
	b	vclgap

; Clear the "gap" between the first and last lines of the logical
;	screen.

vclgap:	mov	NLINES * LSIZE, r0
	jsr	r5, vpbase
	mov	r0, r4
	mov	VSIZE - (NLINES * LSIZE), r1
	mov	177777, r0
	tst	r1
vclg.1:	bze	ret
	cmp	VMEM + VSIZE, r4
	bllt	vclg.2
	sub	VSIZE, r4
vclg.2:	mov	r0, @r4+
	dec	r1
	b	vclg.1

vpbase:	mov	@scroll, r1	; convert logical cursor base to physical
	sll	2, r1
	sll	2, r1
	sll	1, r1
	add	r1, r0
	add	VMEM, r0
	cmp	VMEM + VSIZE, r0
	bllt	vpb.1
	sub	VSIZE, r0
vpb.1:	jmp	r5

vlbase:	mov	@scroll, r1	; convert physical cursor base to logical
	sll	2, r1
	sll	2,  r1
	sll	1, r1
	sub	r1, r0
	cmp	VMEM, r0
	blge	vlb.1
	add	VSIZE, r0
vlb.1:	sub	VMEM, r0
	jmp	r5

vscchg:	mov	@ocbase, r0	; check whether the cursor has changed position
	mov	@ocoffs, r1
	cmp	@cbase, r0
	bneq	vsc.1
	cmp	@coffs, r1
	beq	ret
vsc.1:	mov	1, r0
	mov	r0, @cchgd
	b	ret

;
; redisplay cursor, if necessary
;

vcrdsp:	mov	@cchgd, r0
	tst	r0
	bze	ret
	clr	r0
	mov	r0, @cchgd
	mov	@cnoclr, r0
	tst	r0
	bze	vcrd.1
	clr	r0
	mov	r0, @cnoclr
	b	vcrd.2
vcrd.1:	jsr	r5, call
	vcdisp
vcrd.2:	mov	cbase, r4
	mov	ocbase, r5
	mov	@r4+, r0
	mov	r0, @r5+
	mov	@r4+, r0
	mov	r0, @r5+
	b	vcdisp

;
; given vert in r1, horiz in r2, compute address of
;	leftmost bottom word of character into
;	r2, and number of bits to shift into r1.
;

vparm:	sll	1, r1		; mult r1 by NSPL
	mov	r1, @sp+
	sll	2, r1
	add	@-sp, r1
	sll	1, r1		; mult by 32.
	sll	2, r1
	sll	2, r1
	mov	r1, @sp+	; save for later
#ifdef WIDTH7
	mov	r2, @sp+	; mult r2 by 7
#endif WIDTH7
	sll	1, r2
#ifndef WIDTH7
	mov	r2, @sp+	; mult r2 by 6
#else WIDTH7
	mov	r2, @sp+
#endif WIDTH7
	sll	1, r2
	add	@-sp, r2
#ifdef WIDTH7
	add	@-sp, r2
#endif WIDTH7
	clr	r1		; divide r2by16.
	sarc	2, r2		; put rem in r1
	rrc	2, r1
	sarc	2, r2
	rrc	2, r1
	swap	1, r1
	sar	2, r1
	sar	2, r1
	add	@-sp, r2	; add in rest of
	mov	@scroll, r3	; offset
	sll	2, r3
	sll	2, r3
	sll	1, r3
	add	r3, r2
	cmp	VSIZE, r2
	blt	vp.1
	sub	VSIZE, r2
vp.1:	add	VMEM, r2
	b	ret2

;
; put char in r0 on screen at positin r1, r2
;

vchxy:	mov	r0, r3		; save char code
	jsr	r5, call
	vparm
vchbo:	mov	r3, @sp+
	sll	2, r3
	add	@-sp, r3
	add	@csloc, r3
	mov	r1, @sp+	; save shift count
	mov	r1, r4
	mov	NSPL, r5		; scan line count
;
; the following code has:
;	r0, r1: two word data to be shifted
;	r2: pointer into video mmory
;	r3: pointer into char table
;	r4: shift count
;	r5: scan line count
; IDEA: first clear out old stuff,
; then XOR in new data.  If cflg .ne.0, skip
; the clearing and just XOR.
;
vc.1:	mov	@cflg, r0
	tst	r0
	bnze	vc.7		
	mov	r4, r1
	sll	1, r1
	add	msktab, r1
	mov	@r1, r0
	inc	r1
	mov	@r1, r1
vc.3:	and	@r2, r0
	mov	r0, @r2
	inc	r2
	and	@r2, r1
	mov	r1, @r2
	dec	r2		; stuff cleared
vc.7:	mov	@r3, r0		; fetch scan line
	mov	r5, r1		; see if odd/even
	sarc	1, r1
	bnc	vc.4
	swap	1, r0		; use high half
	inc	r3		; increment pointer
vc.4:	and	177, r0
#ifndef WIDTH7
	sar	1, r0
#endif WIDTH7
	clr	r1		; prepare to shift
	add	2, r4		; adjust shift count
vc.5:	sub	2, r4
	bze	vc.6
	bmi	vc.8
	sllc	2, r0
	rlc	2, r1
	b	vc.5
vc.8:	sarc	1, r1
	rrc	1, r0
vc.6:	xor	@r2, r0		; store new stuff
	mov	r0, @r2
	inc	r2
	xor	@r2, r1
	mov	r1, @r2
	mov	@-sp, r4
	mov	r4, @sp+
	add	37, r2		; new scan line
	cmp	VMEM+VSIZE, r2
	bllt	vc.9
	sub	VSIZE, r2
vc.9:	dec	r5
	bnze	vc.1		; more lines to do
	mov	@-sp, r0	; pop shift count
	clr	r0
	mov	r0, @cflg
	b	ret

;
; fast way to clear entire screen
;

vclrsc:	mov	VMEM, r4
	mov	177777, r0
vcs.1:	mov	r0, @r4+
	cmp	VMEM+VSIZE, r4
	bllt	vcs.1
	b	ret

;
; fast way to clear line containing cursor
;

vclrlc:	mov	@cbase, r4
	and	177740, r4
	mov	LSIZE, r1
	mov	177777, r0
vclc.1:	mov	r0, @r4+
	cmp	VMEM+VSIZE, r4
	bllt	vclc.2
	sub	VSIZE, r4
vclc.2:	dec	r1
	bnze	vclc.1
	b	ret

; Insert/Delete Line Functions

viln:	jsr	r5, call
	vcclr
	mov	@cbase, r0
	and	~(NWPS-1), r0
	jsr	r5, vlbase
	mov	r0, r2
	clr	r0
	jsr	r5, vpbase
	mov	r0, r4
	mov	VMEM + VSIZE, r1
	mov	VSIZE, r3
	add	LSIZE, r0
	cmp	r1, r0
	bllt	vil.1
	sub	r3, r0
vil.1:	mov	r0, r5
	tst	r2
vil.2:	bnze	vil.9
	jsr	r5, call
	vclrlc
	mov	1, r0
	mov	r0, @cnoclr
	b	vscchg
vil.9:	mov	@r5+, r0
	mov	r0, @r4+
	cmp	r1, r5
	bllt	vil.3
	sub	r3, r5
vil.3:	cmp	r1, r4
	bllt	vil.4
	sub	r3, r4
vil.4:	dec	r2
	b	vil.2

vdln:	mov	@cbase, r0
	and	~(NWPS-1), r0
	mov	r0, r2
	jsr	r5, vlbase
	mov	r2, r3
	add	LSIZE-1, r2
	dec	r3
	mov	VMEM, r4
	mov	VSIZE, r5
	cmp	r4, r3
	bge	vdl.1
	add	r5, r3
vdl.1:	tst	r0
vdl.2:	bze	vdl.5
	mov	@r3, r1
	mov	r1, @r2
	dec	r2
	cmp	r4, r2
	blge	vdl.3
	add	r5, r2
vdl.3:	dec	r3
	cmp	r4, r3
	blge	vdl.4
	add	r5, r3
vdl.4:	dec	r0
	b	vdl.2
vdl.5:	mov	@cbase, r2
	clr	r0
	jsr	r5, vpbase
	mov	r0, @cbase
	jsr	r5, call
	vclrlc
	mov	r2, @cbase
	mov	1, r0
	mov	r0, @cnoclr
	b	vscchg

; Insert/Delete Character Functions

vichr:	jsr	r5, call
	vcdisp
	mov	@coffs, r1
	add	kmsktb, r1
	mov	@r1, r5
	mov	NSPL, r3
	mov	@cbase, r2
vic.2:	mov	r2, r4
	inc	r4
	mov	@r2, r0
	mov	r0, r1
	and	r5, r1
	mov	r1, @sp+
	com	r5
	and	r5, r0
	com	r5
	clr	r1
	sllc	2, r0
	rlc	2, r1
	sllc	2, r0
	rlc	2, r1
	sllc	2, r0
	rlc	2, r1
#ifdef WIDTH7
	sllc	1, r0
	rlc	1, r1
#endif WIDTH7
	xor	@-sp, r0
	mov	r0, @r2
	mov	r2, @sp+
vic.1:	mov	r4, r2
	and	NWPS-1, r2
	bze	vic.4
	swap	1, r1
#ifdef WIDTH7
	sll	1, r1
#else WIDTH7
	sll	2, r1
#endif WIDTH7
	clr	r2
	mov	@r4+, r0
	dec	r4
	sllc	2, r1
	rlc	2, r0
	rlc	2, r2
	sllc	2, r1
	rlc	2, r0
	rlc	2, r2
	sllc	2, r1
	rlc	2, r0
	rlc	2, r2
#ifdef WIDTH7
	sllc	1, r1
	rlc	1, r0
	rlc	1, r2
#endif WIDTH7
	mov	r0, @r4+
	mov	r2, r1
	b	vic.1
vic.4:	mov	@-sp, r2
	dec	r3
	bze	vic.3
	add	NWPS, r2
	cmp	VMEM + VSIZE, r2
	bllt	vic.2
	sub	VSIZE, r2
	b	vic.2
vic.3:	mov	1, r0
	mov	r0, @cnoclr
	b	vscchg

vdchr:	mov	@coffs, r1
	add	kmsktb, r1
	mov	@r1, r5
	mov	NSPL, r4
	mov	@cbase, r2
vdc.4:	mov	177777, r1
	mov	r2, r3
	and	~(NWPS-1), r3
	xor	NWPS-1, r3
vdc.2:	cmp	r3, r2
	beq	vdc.1
	mov	r2, @sp+
	mov	@r3, r0
	clr	r2
	sarc	2, r1
	rrc	2, r0
	rrc	2, r2
	sarc	2, r1
	rrc	2, r0
	rrc	2, r2
	sarc	2, r1
	rrc	2, r0
	rrc	2, r2
#ifdef WIDTH7
	sarc	1, r1
	rrc	1, r0
	rrc	1, r2
#endif WIDTH7
	mov	r0, @r3
	swap	1, r2
#ifdef	WIDTH7
	sar	1, r2
#else WIDTH7
	sar	2, r2
#endif WIDTH7
	mov	r2, r1
	dec	r3
	mov	@-sp, r2
	b	vdc.2
vdc.1:	mov	r2, @sp+
	mov	@r3, r0
	mov	r0, r2
	and	r5, r0
	sarc	2, r1
	rrc	2, r2
	sarc	2, r1
	rrc	2, r2
	sarc	2, r1
	rrc	2, r2
#ifdef WIDTH7
	sarc	1, r1
	rrc	1, r2
#endif WIDTH7
	com	r5
	and	r5, r2
	com	r5
	xor	r0, r2
	mov	r2, @r3
	mov	@-sp, r2
	dec	r4
	bze	vdc.3
	add	NWPS, r2
	cmp	VMEM + VSIZE, r2
	bllt	vdc.4
	add	VSIZE, r2
	b	vdc.4
vdc.3:	mov	1, r0
	mov	r0, @cnoclr
	b	vscchg

; global video data

scroll:	.word	0		; current scroll counter
cbase:	.word	0		; cursor base
coffs:	.word	0		; offset in bits
csloc:	.word	0		; loc of cs table
cflg:	.word	0		; clear or not?
ocbase:	.word	0		; old cursor base
ocoffs:	.word	0		; old cursor offset
cchgd:	.word	0		; has cursor position changed?
cnoclr:	.word	0		; need to clear old cursor
vidflg:	.word	0		; video status flags
chpos:	.word	0		; cursor horizontal position
blink:	.word	0		; cursor blink counter
#ifndef WIDTH7

msktab:	.word	177700, 177777, 177601, 177777
	.word	177403, 177777, 177007, 177777
	.word	176017, 177777, 174037, 177777
	.word	170077, 177777, 160177, 177777
	.word	140377, 177777, 100777, 177777
	.word	001777, 177777, 003777, 177776
	.word	007777, 177774, 017777, 177770
	.word	037777, 177760, 077777, 177760
#endif WIDTH7
