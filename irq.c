/*
 *  Interrupt Handling for the MT7688
 */
#include	"u.h"
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"ureg.h"
#include	"io.h"
#include	"../port/error.h"



/* map the irq number to the interrupt controller */
static const int irq2inc[32] = {
	/* cpu based interrupts */
	[IRQlow]	=	-1,
	[IRQhigh]	=	-1,
	[IRQpci]	=	-1,
	[IRQethr]	=	-1,
	[IRQwifi]	=	-1,
	[IRQtimer]	=	-1,

	/* irqs on the SoC interrupt controller */
	[IRQsys]	=	INC_SYSCTL,
	[IRQtimer0]	=	INC_TIMER0,
	[IRQwdog]	=	INC_WDOG,
	[IRQillacc]	=	INC_ILLACC,
	[IRQpcm]	=	INC_PCM,
	[IRQuartf]	=	INC_UARTF,
	[IRQgpio]	=	INC_GPIO,
	[IRQdma]	=	INC_DMA,
	[IRQnand]	=	INC_NAND,
	[IRQperf]	=	INC_PERF,
	[IRQi2c]	=	INC_I2C,
	[IRQspi]	=	INC_SPI,
	[IRQuartl]	=	INC_UARTL,
	[IRQcrypto]	=	INC_CRYPTO,
	[IRQsdhc]	=	INC_SDHC,
	[IRQr2p]	=	INC_R2P,
	[IRQnone]	=	-1,
	[IRQethsw]	=	INC_ETHSW,
	[IRQusbh]	=	INC_USBH,
	[IRQusbd]	=	INC_USBD,
};


static const int inc2irq[32] = {
	[INC_SYSCTL]	=	IRQsys,
	[INC_TIMER0]	=	IRQtimer0,
	[INC_WDOG]		=	IRQwdog,
	[INC_ILLACC]	=	IRQillacc,
	[INC_PCM]		=	IRQpcm,
	[INC_UARTF]		=	IRQuartf,
	[INC_GPIO]		=	IRQgpio,
	[INC_DMA]		=	IRQdma,
	[INC_NAND]		=	IRQnand,
	[INC_PERF]		=	IRQperf,
	[INC_I2C]		=	IRQi2c,
	[INC_SPI]		=	IRQspi,
	[INC_UARTL]		=	IRQuartl,
	[INC_CRYPTO]	=	IRQcrypto,
	[INC_SDHC]		=	IRQsdhc,
	[INC_R2P]		=	IRQr2p,
	[INC_ETHSW]		=	IRQethsw,
	[INC_USBH]		=	IRQusbh,
	[INC_USBD]		=	IRQusbd,
};




typedef struct Handler Handler;

struct Handler {
	Handler *next;
	void 	(*f)(Ureg*, void *);
	void	*arg;
	int		irq;
};

static Lock intrlock;
static Handler handlers[IRQmax+1];


void incintr(Ureg*, void*);



static u32int
incread(int offset)
{
	return *IO(u32int, (IRQBASE + offset));
}


static void
incwrite(int offset, u32int val)
{
	*IO(u32int, (IRQBASE + offset)) = val;
}

/*
 * called by main(), clears all the irq's
 * sets SoC interrupt controller to relay
 * IRQs through CPU interrupts 2 and 3
 */

void
intrinit(void)
{
	
	incwrite(IRQ_MASK_CLR, ~0);
	incwrite(IRQ_MASK_SET, INC_GLOBAL);
	coherence();

	intrenable(IRQlow, incintr, (void *)0, 0, "inclow");
	intrenable(IRQhigh, incintr, (void *)1, 1, "inchigh");

}


/* called by drivers to setup irq's */
void
intrenable(int irq, void (*f)(Ureg*, void *), void *arg, int priority, char *name)
{
	Handler *hp;
	u32int r;

	if(irq > IRQmax || irq < 0)
		panic("intrenable: %s gave bad irq number of %d", name, irq);

	hp = &handlers[irq];
	ilock(&intrlock);

	if(hp->f != nil) {
		for(; hp->next != nil; hp = hp->next)
			;
		if((hp->next = xalloc(sizeof *hp)) == nil)
			panic("intrenable: out of memory");
		hp = hp->next;
		hp->next = nil;
	}

	hp->f = f;
	hp->arg = arg;
	hp->irq = irq;

	iunlock(&intrlock);

	if(irq > IRQtimer) {
		r = incread(IRQ_SEL0);
		r |= (priority << irq2inc[irq]);
		incwrite(IRQ_SEL0, r);
		coherence();
		r = incread(IRQ_MASK_SET);
		r |= (1 << irq2inc[irq]);
		incwrite(IRQ_MASK_SET, r);
		coherence();
	} else {
		intron(INTR0 << irq);
	}
}



void
intrdisable(int, void (*)(Ureg*, void *), void*, int, char*)
{
	/* disable an irq */
}


/* called by trap to handle requests, returns true if a clock interrupt */
int
intr(Ureg* ur)
{	
	ulong cause, mask;
	int clockintr;
	Handler *hh, *hp;

	m->intr++;
	clockintr = 0;
	/*
	 * ignore interrupts that we have disabled, even if their cause bits
	 * are set.
	 */
	cause = ur->cause & ur->status & INTMASK;
	cause &= ~(INTR1|INTR0);		/* ignore sw interrupts */

	if (cause == 0)
		print("spurious interrupt\n");

	if(cause & INTR7){
		clock(ur);
//		intrcauses[ILclock]++;
		cause &= ~INTR7;
		clockintr = 1;
	}
	hh = &handlers[0];
	for(mask = INTR2; cause != 0 && mask < INTR7; mask <<= 1){
		if(cause & mask){
			for(hp = hh; hp != nil; hp = hp->next){
				if(hp->f != nil){
					hp->f(ur, hp->arg);
					cause &= ~mask;
				}
			}
		}
		hh++;
	}
	if(cause != 0)
		print("unhandled interrupts %lux\n", cause);

	/* preemptive scheduling */
	if(up != nil && !clockintr)
		preempted();
	/* if it was a clockintr, sched will be called at end of trap() */
	return clockintr;
}


/* off to handle requests for the SoC interrupt controller */
/*
 * the interrupts controller on the mt7688 SoC can be mapped to 
 * either CPU interrupt 2 or 3.  So when those are tripped, 
 * this code then checks the secondary interrupt controller 
 * to see which IRQ it has.  The controller defines CPU INTR2 
 * as "low priority" IRQ, and INTR3 as "high priority" FIQ.
 */

void
incintr(Ureg *ureg, void *arg)
{
	int p;
	u32int reg;
	u32int pending;
	u32int mask;
//	int irq;
	Handler *hh, *hp;


	p = (intptr)arg;
	reg = (p == 0) ? IRQ_STAT : FIQ_STAT;
	pending = incread(reg);


	hh = &handlers[6];
	for(mask = 1 ; pending != 0 && mask < 0x80000000; mask <<= 1) {
		if(pending & mask) {
			for(hp = hh; hp != nil; hp = hp->next) {
				if(hp->f != nil) {
					hp->f(ureg, hp->arg);
					pending &= ~mask;
				}
			}
		}
		hh++;
	}

	if(pending != 0)
		print("unhandled interrupts %uX\n", pending);
}


void
intrshutdown(void)
{
	introff(INTMASK);
	incwrite(IRQ_MASK_CLR, ~0);
	incwrite(IRQ_MASK_SET, INC_GLOBAL);
	coherence();

}

