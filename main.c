#include	"u.h"
#include	"tos.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"io.h"
#include	"pool.h"
#include	"../ip/ip.h"
#include	"../port/error.h"
#include	"reboot.h"

//#include "rebootcode.i"



/*
 * software tlb simulation
 */
static Softtlb stlb[MAXMACH][STLBSIZE];

Conf	conf;
FPsave initfp;

int normalprint;

static Lock testlock;  /* for debug in machinit */
Mach* machaddr[MAXMACH];


void
checkclock0(void)
{
	print("count=%luX compare=%luX %d\n", rdcount(), rdcompare(), m->speed);
}


static void
prcpuid(void)
{
	ulong cpuid, cfg1;
	char *cpu;

	cpuid = prid();
	if (((cpuid>>16) & MASK(8)) == 0)		/* vendor */
		cpu = "old mips";
	else if (((cpuid>>16) & MASK(8)) == 1)
		switch ((cpuid>>8) & MASK(8)) {		/* processor */
		case 0x93:
			cpu = "mips 24k";
			break;
		case 0x96:
			cpu = "mips 24KEc";
			break;
		default:
			cpu = "mips";
			break;
		}
	else
		cpu = "other mips";
	delay(20);
	print("cpu%d: %ldMHz %s %s v%ld %ld rev %ld, ",
		m->machno, m->hz / Mhz, cpu, getconfig() & (1<<15)? "b": "l",
		(cpuid>>5) & MASK(3), (cpuid>>2) & MASK(3), cpuid & MASK(2));
	delay(200);
	cfg1 = getconfig1();
	print("%s fpu\n", (cfg1 & 1? "has": "no"));
	print("cpu%d: %ld tlb entries, using %dK pages\n", m->machno,
		((cfg1>>25) & MASK(6)) + 1, BY2PG/1024);
	delay(50);
	print("cpu%d: l1 i cache: %d sets 4 ways 32 bytes/line\n", m->machno,
		64 << ((cfg1>>22) & MASK(3)));
	delay(50);
	print("cpu%d: l1 d cache: %d sets 4 ways 32 bytes/line\n", m->machno,
		64 << ((cfg1>>13) & MASK(3)));
	delay(500);
/* i changed this if from 0 to 1 */
	if (1) 
		print("cpu%d: cycle counter res = %ld\n",
			m->machno, gethwreg3());
}


static int
ckpagemask(ulong mask, ulong size)
{
	int s;
	ulong pm;

	s = splhi();
	setpagemask(mask);
	pm = getpagemask();
	splx(s);
	if(pm != mask){
		iprint("page size %ldK not supported on this cpu; "
			"mask %#lux read back as %#lux\n", size/1024, mask, pm);
		return -1;
	}
	return 0;
}


void
exit(int type)
{
	int timer;
	void (*fnp)(void);

	delay(1000);
	lock(&active);
	active.machs[0] &= ~(1<<m->machno);
	active.exiting = 1;
	unlock(&active);
	spllo();

	iprint("cpu %d exiting\n", m->machno);
	timer = 0;
	while(active.machs || consactive()) {
		if(timer++ > 400)
			break;
		delay(10);
	}
	delay(1000);
	splhi();
	USED(type);

	setstatus(BEV);
	coherence();

	iprint("exit: awaiting reset\n");

	delay(1000);			/* await a reset */

	iprint("exit: jumping to rom\n");
	fnp = (void (*)(void))ROM;
	(*fnp)();

	iprint("exit: looping\n");
	for (;;)
		;
}


void
reboot(void *entry, void *code, ulong size)
{
	void (*f)(ulong, ulong, ulong);

//	writeconf();

	/*
	 * the boot processor is cpu0.  execute this function on it
	 * so that the new kernel has the same cpu0.
	 */
	if (m->machno != 0) {
		procwired(up, 0);
		sched();
	}
	if (m->machno != 0)
		print("on cpu%d (not 0)!\n", m->machno);

	cpushutdown();

	/*
	 * should be the only processor running now
	 */
	iprint("reboot: entry %#p code %#p size %ld\n", entry, code, size);
	iprint("code[0] = %#lux\n", *(ulong *)code);

	/* turn off buffered serial console */
	serialoq = nil;
	kprintoq = nil;
	screenputs = nil;

	/* shutdown devices */
	chandevshutdown();

	/* call off the dog */
//	clockshutdown();

	splhi();
	intrshutdown();


	/* setup reboot trampoline function */
	f = (void*)REBOOTADDR;
	memmove(f, rebootcode, sizeof(rebootcode));
	icflush(f, sizeof(rebootcode));

	setstatus(BEV);		/* also, kernel mode, no interrupts */
	coherence();

	/* off we go - never to return */
	(*f)((ulong)entry, (ulong)code, size);

	panic("loaded kernel returned!");
}



void
init0(void)
{
	char buf[128];
	char **sp;

//	up->nerrlab = 0;

	spllo();

//	iprint("made it to init0\n");
//	delay(50);

	chandevinit();


	if(!waserror()){
		ksetenv("cputype", "spim", 0);
		snprint(buf, sizeof buf, "mips %s", conffile);
		ksetenv("terminal", buf, 0);
		if(cpuserver)
			ksetenv("service", "cpu", 0);
		else
			ksetenv("service", "terminal", 0);
		/*
		 * we don't have a good way to read our cfg file in
		 * RouterBOOT, so set the configuration here.
		 */
		ksetenv("bootargs", "tcp", 0);
		ksetenv("console", "0", 0);

		/* no usb */
		ksetenv("usbwait", "0", 0);
		ksetenv("nousbrc", "1", 0);

		poperror();
	}

	kproc("alarm", alarmkproc, 0);

	sp = (char**)(USTKTOP - sizeof(Tos) - 8 - sizeof(sp[0])*4);
	sp[3] = sp[2] = sp[1] = nil;
	strcpy(sp[0] = (char*)&sp[4], "boot");

	i8250console();
	touser(sp);
}


/*
 *  setup MIPS trap vectors
 */
void
vecinit(void)
{
	memmove((ulong*)UTLBMISS, (ulong*)vector0, 0x80);
	memmove((ulong*)XEXCEPTION, (ulong*)vector0, 0x80);
	memmove((ulong*)CACHETRAP, (ulong*)vector100, 0x80);
	memmove((ulong*)EXCEPTION, (ulong*)vector180, 0x80);
	memmove((ulong*)(KSEG0+0x200), (ulong*)vector180, 0x80);
	icflush((ulong*)UTLBMISS, 4*1024);

	setstatus(getstatus() & ~BEV);
}


void
confinit(void)
{
	ulong kpages, ktop;

	/*
	 *  divide memory twixt user pages and kernel.
	 */
	conf.mem[0].base = ktop = PADDR(PGROUND((ulong)end));
	/* fixed memory on routerboard */
	conf.mem[0].npage = MEMSIZE/BY2PG - ktop/BY2PG;
	conf.npage = conf.mem[0].npage;
	conf.nuart = 1;

	kpages = conf.npage - (conf.npage*80)/100;
	if(kpages > (64*MB + conf.npage*sizeof(Page))/BY2PG){
		kpages = (64*MB + conf.npage*sizeof(Page))/BY2PG;
		kpages += (conf.nproc*KSTACK)/BY2PG;
	}
	conf.upages = conf.npage - kpages;
	conf.ialloc = (kpages/2)*BY2PG;

	kpages *= BY2PG;
	kpages -= conf.upages*sizeof(Page)
		+ conf.nproc*sizeof(Proc)
		+ conf.nimage*sizeof(Image)
		+ conf.nswap
		+ conf.nswppo*sizeof(Page);
	mainmem->maxsize = kpages;

	/*
	 *  set up CPU's mach structure
	 *  cpu0's was zeroed in l.s and our stack is in Mach, so don't zero it.
	 */
	m->machno = 0;
	m->speed = 580;			/* for mt7688 */
	m->hz = m->speed * Mhz;
	conf.nmach = 1;

	/* set up other configuration parameters */
	conf.nproc = 2000;
	conf.nswap = 262144;
	conf.nswppo = 4096;
	conf.nimage = 200;

	conf.copymode = 0;		/* copy on write */
}







void
main(void)
{
	static ulong vfy = 0xcafebabe;

	normalprint = 1;

	if (vfy != 0xcafebabe)
		serialputc('d');

	if (m == 0)
		serialputc('m');

	quotefmtinstall();

	confinit();
	savefpregs(&initfp);
	machinit();			/* calls clockinit */


/* print or iprint here seems to break the boot */


	uartinit();
	kmapinit();
	xinit();

	timersinit();
	vecinit();
	printinit();
	intrinit();

	normalprint = 1;
	print("\nPlan 9 Front\n"); 

	prcpuid();
	if (PTECACHABILITY == PTENONCOHERWT)
		print("caches configured as write-through\n");
	checkclock0();
	delay(100);

	print("\0\0\0\0\0\0\0"); /* possible 0c bug sanity check */

	ckpagemask(PGSZ, BY2PG);
	tlbinit();
	machwire();
	pageinit();
	procinit0();
	initseg();
	links();
	chandevreset();

	userinit();
	schedinit();
	panic("schedinit returned");
}



void
machinit(void)
{


	/* Ensure CU1 is off */
	clrfpintr();

	m->machno = 0;
	machaddr[0] = m;

	lock(&testlock);		/* hold this forever */

	if (m == 0) {
		serialputc('?');
		serialputc('m');
		serialputc('0');
	}
	if(machaddr[m->machno] != m) {
		serialputc('?');
		serialputc('m');
		serialputc('m');
	}

	if (canlock(&testlock)) {
		serialputc('?');
		serialputc('l');
		panic("cpu%d: locks don't work", m->machno);
	}

	active.exiting = 0;
	active.machs[0] = 1;

	m->stb = &stlb[m->machno][0];
	m->ticks = 1;
	m->perf.period = 1;
	up = nil;

	clockinit();
}


void
procsetup(Proc *p)
{
	p->fpstate = FPinit;
	memmove(p->fpsave, &initfp, sizeof(FPsave));
}


void
procfork(Proc *p)
{
	int s;

	s = splhi();
	switch(up->fpstate & ~FPillegal){
	case FPactive:
		savefpregs(up->fpsave);
		up->fpstate = FPinactive;
		/* wet floor */
	case FPinactive:
		memmove(p->fpsave, up->fpsave, sizeof(FPsave));
		p->fpstate = FPinactive;
	}
	splx(s);
}


void
setupwatchpts(Proc *, Watchpt *, int n)
{
	if(n > 0)
		error("no watchpoints");
}



