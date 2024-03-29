CONF=mt7688
CONFLIST=mt7688

objtype=spim
</$objtype/mkfile
p=9
# must match mem.h
KTZERO=0x80020000
UTZERO=0x4020

# CFLAGS=$CFLAGS -DFPEMUDEBUG
# CFLAGS=$CFLAGS -BIGPAGES

DEVS=`{rc ../port/mkdevlist $CONF}

PORT=\
	alarm.$O\
	alloc.$O\
	allocb.$O\
	auth.$O\
	cache.$O\
	chan.$O\
	clock.$O\
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
	bootargs.$O\
	clock.$O\
	faultmips.$O\
	irq.$O\
	main.$O\
	mmu.$O\
	random.$O\
	syscall.$O\
	trap.$O\
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
	$LD -o $target -l -R4 -H6 -T$KTZERO $prereq 

install:V:	$p$CONF
	cp $p$CONF /$objtype/

<../boot/bootmkfile
<../port/portmkfile
<|../port/mkbootrules $CONF

initcode.out: init9.$O initcode.$O /$objtype/lib/libc.a
	$LD -T$UTZERO -R4 -s -o $target $prereq 

l.$O: mips24k.s
fpimips.$O arch.$O faultmips.$O mmu.$O syscall.$O trap.$O irq.$O: /$objtype/include/ureg.h
main.$O:	/$objtype/include/ureg.h errstr.h
main.$O trap.$O syscall.$O fpimips.$O: /sys/include/tos.h
fpi.$O fpimips.$O fpimem.$O: fpi.h

%.clean:V:
	rm -f $stem.c [9bz]$stem [9bz]$stem.gz boot$stem.*
