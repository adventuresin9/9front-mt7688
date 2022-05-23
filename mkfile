CONF=mt7688
CONFLIST=mt7688

objtype=spim
</$objtype/mkfile
p=9
# must match mem.h
# KTZERO=0x80020000
KTZERO=0x80020000
PHYSKTZERO=0x20000
# must match mem.h
UTZERO=0x4020
BY2PG=4096
MAXBY2PG=16384
REBOOTADDR=0x80001000

# CFLAGS=$CFLAGS -DFPEMUDEBUG

DEVS=`{rc ../port/mkdevlist $CONF}

PORT=\
	alarm.$O\
	alloc.$O\
	allocb.$O\
	auth.$O\
	cache.$O\
	chan.$O\
	dev.$O\
	edf.$O\
	fault.$O\
	mul64fract.$O\
	page.$O\
	parse.$O\
	pgrp.$O\
	portclock.$O\
	print.$O\
	proc.$O\
	qio.$O\
	qlock.$O\
	rdb.$O\
	rebootcmd.$O\
	segment.$O\
	syscallfmt.$O\
	sysfile.$O\
	sysproc.$O\
	taslock.$O\
	tod.$O\
	xalloc.$O\
	userinit.$O\

OBJ=\
	l.$O\
	arch.$O\
	faultmips.$O\
	main.$O\
	mmu.$O\
	random.$O\
	trap.$O\
	irq.$O\
	clock.$O\
	fpi.$O\
	fpimem.$O\
	fpimips.$O\
	$CONF.root.$O\
	$CONF.rootc.$O\
	$DEVS\
	$PORT\

LIB=\
	/$objtype/lib/libmemlayer.a\
	/$objtype/lib/libmemdraw.a\
	/$objtype/lib/libdraw.a\
	/$objtype/lib/libauth.a\
	/$objtype/lib/libsec.a\
	/$objtype/lib/libmp.a\
	/$objtype/lib/libip.a\
	/$objtype/lib/libc.a\

$p$CONF:	$OBJ $CONF.$O $LIB
	$LD -o $target -H6 -T$KTZERO -R4 -l $prereq
	#size $target


install:V:	$p$CONF
	cp $p$CONF /$objtype/

<../boot/bootmkfile
<../port/portmkfile
<|../port/mkbootrules $CONF

initcode.out: init9.$O initcode.$O /$objtype/lib/libc.a
	$LD -l -T$UTZERO -R4 -s -o $target $prereq

rebootcode.out: initreboot.$O rebootcode.$O /$objtype/lib/libc.a
	$LD -l -T$REBOOTADDR -R4 -s -o $target $prereq

reboot.h:D:	initreboot.s rebootcode.c mem.h
	$AS initreboot.s
	$CC -FTVw rebootcode.c
	# -lc is only for memmove.  -T arg is REBOOTADDR.
	$LD -l -a -s -T0x80001000 -R4 -o reboot.out initreboot.$O rebootcode.$O -lc >reboot.list
	{echo 'uchar rebootcode[]={'
	 xd -1x reboot.out |
		sed -e '1,2d' -e 's/^[0-9a-f]+ //' -e 's/ ([0-9a-f][0-9a-f])/0x\1,/g'
	 echo '};'} > reboot.h

l.$O: mips24k.s
arch.$O clock.$O fpimips.$O faultmips.$O mmu.$O syscall.$O\
	main.$O trap.$O irq.$O: /$objtype/include/ureg.h
main.$O: errstr.h reboot.h
fpi.$O fpimips.$O fpimem.$O: fpi.h


%.clean:V:
	rm -f $stem.c [9bz]$stem [9bz]$stem.gz boot$stem.*
