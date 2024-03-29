typedef struct Conf	Conf;
typedef struct Confmem	Confmem;
typedef struct FPsave	FPsave;
typedef struct PFPU	PFPU;
typedef struct KMap	KMap;
typedef struct Lance	Lance;
typedef struct Lancemem	Lancemem;
typedef struct Label	Label;
typedef struct Lock	Lock;
typedef struct Mach	Mach;
typedef struct MMU	MMU;
typedef struct PMMU	PMMU;
typedef struct Softtlb	Softtlb;
typedef struct Ureg	Ureg;
typedef struct Proc	Proc;
typedef struct ISAConf	ISAConf;
typedef uvlong		Tval;

#define MAXSYSARG	5	/* for mount(fd, afd, mpt, flag, arg) */

/*
 *  parameters for sysproc.c and rebootcmd.c
 */
//#define AOUT_MAGIC	(P_MAGIC)
#define AOUT_MAGIC	N_MAGIC || magic==P_MAGIC
/* r3k or r4k boot images */
#define BOOT_MAGIC	(0x160<<16) || magic == ((0x160<<16)|3)

/* fron legacy /mips/include/u.h */
#define FPCOND	(1<<23)

/*
 *  machine dependent definitions used by ../port/dat.h
 */

struct Lock
{
	ulong	key;			/* semaphore (non-zero = locked) */
	ulong	sr;
	ulong	pc;
	Proc	*p;
	Mach	*m;
	ushort	isilock;
};

struct Label
{
	uintptr	sp;
	uintptr	pc;
};

struct Confmem
{
	ulong	base;
	ulong	npage;
	ulong	kbase;
	ulong	klimit;
};

struct Conf
{
	ulong	nmach;		/* processors */
	ulong	nproc;		/* processes */
	Confmem	mem[4];
	ulong	npage;		/* total physical pages of memory */
	ulong	upages;		/* user page pool */
	ulong	nimage;		/* number of page cache image headers */
	ulong	nswap;		/* number of swap pages */
	int	nswppo;		/* max # of pageouts per segment pass */
	ulong	copymode;	/* 0 is copy on write, 1 is copy on reference */
	ulong	ialloc;		/* bytes available for interrupt-time allocation */
	ulong	pipeqsize;	/* size in bytes of pipe queues */
	int	nuart;		/* number of uart devices */
	int	monitor;
	int	keyboard;
};

struct ISAConf
{
	char	*type;
	ulong	port;
	int	irq;
	int	nopt;
	char	*opt[1];
};
#define BUSUNKNOWN -1

/*
 * floating point registers
 */
enum {
	Nfpregs		= 32,		/* floats; half as many doubles */
};

/*
 * emulated floating point (mips32r2 with ieee fp regs)
 * fpstate is separate, kept in Proc
 */
struct FPsave
{
	/* /dev/proc expects the registers to be first in FPsave */
	ulong	reg[Nfpregs];		/* the canonical bits */
	union {
		ulong	fpstatus;	/* both are fcr31 */
		ulong	fpcontrol;
	};

	int	fpdelayexec;		/* executing delay slot of branch */
	uintptr	fpdelaypc;		/* pc to resume at after */
	ulong	fpdelaysts;	/* save across user-mode delay-slot execution */

	/* stuck-fault detection */
	uintptr	fppc;			/* addr of last fault */
	int	fpcnt;			/* how many consecutive at that addr */
};

struct PFPU
{
	int	fpstate;
	FPsave	fpsave[1];
};

enum
{
	/* floating point state */
	FPinit,
	FPactive,
	FPinactive,
	FPemu,

	/* bit meaning floating point illegal */
	FPillegal= 0x100,
};

/*
 *  mmu goo in the Proc structure
 */
struct PMMU
{
	int	pidonmach[MAXMACH];
};

#include "../port/portdat.h"

struct Mach
{
	/* the following are all known by l.s and cannot be moved */
	int	machno;			/* physical id of processor */
	Softtlb*stb;
	Proc*	proc;			/* process on this processor */
	uintptr	splpc;			/* pc that called splhi() */
	ulong	tlbfault;

	/* the following is safe to move */
	PMach;

	int	lastpid;		/* last pid allocated on this machine */
	Proc*	pidproc[NTLBPID];	/* proc that owns tlbpid on this mach */

	KMap*	kactive;		/* active on this machine */
	int	knext;
	uchar	ktlbx[NTLB];		/* tlb index used for kmap */
	uchar	ktlbnext;

	/* for per-processor timers */
	uvlong	fastticks;
	ulong	lastcount;

	ulong	hz;
	int	speed;			/* cpu speed */
	ulong	delayloop;		/* for the delay() routine */
	ulong	maxperiod;
	ulong	minperiod;

	int	hashcoll;		/* soft-tlb hash collisions */
	int	paststartup;		/* for putktlb */

	uintptr	stack[1];
};

struct KMap
{
	Ref;
	ulong	virt;
	ulong	phys0;
	ulong	phys1;
	KMap*	next;
	KMap*	konmach[MAXMACH];
	Page*	pg;
	ulong	pc;			/* of caller to kmap() */
};

#define	VA(k)		((k)->virt)
#define PPN(x)		((ulong)(x)>>6)

struct Softtlb
{
	ulong	virt;
	ulong	phys0;
	ulong	phys1;
};

struct
{
	char	machs[MAXMACH];		/* active cpus */
	short	exiting;
}active;

enum {
	Mhz		= 1000*1000,
};


extern register Mach	*m;
extern register Proc	*up;

extern Mach* machaddr[MAXMACH];
#define	MACHP(n)	(machaddr[n])
extern int	normalprint;

#define CONSOLE	0

/*
 *  hardware info about a device
 */
typedef struct {
	ulong	port;
	int	size;
} Devport;

struct DevConf
{
	ulong	intnum;			/* interrupt number */
	char	*type;			/* card type, malloced */
	int	nports;			/* Number of ports */
	Devport	*ports;			/* The ports themselves */
};
