/*
 * Machine specific stuff
 */

#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
#include "tos.h"

#include "ureg.h"


void
evenaddr(uintptr va)
{
	if((va & 3) != 0){
		dumpstack();
		postnote(up, 1, "sys: odd address", NDebug);
		error(Ebadarg);
	}
}


void
idlehands(void)
{
	idle();
}


char *
getconf(char *)
{
	return nil;	/* stub */
}


void
procsave(Proc *p)
{
	/* stub */
}


void
procrestore(Proc *p)
{
	/* stub */
}


/* set up user registers before return from exec(), was execregs */
uintptr
execregs(ulong entry, ulong ssize, ulong nargs)
{
	Ureg *ur;
	ulong *sp;

	sp = (ulong*)(USTKTOP - ssize);
	*--sp = nargs;

	ur = (Ureg*)up->dbgreg;
	ur->usp = (ulong)sp;
	ur->pc = entry - 4;		/* syscall advances it */
	up->fpsave->fpstatus = initfp.fpstatus;
	return USTKTOP-sizeof(Tos);	/* address of kernel/user shared data */
}

ulong
userpc(void)
{
	Ureg *ur;

	ur = (Ureg*)up->dbgreg;
	return ur->pc;
}

/*
 * This routine must save the values of registers the user is not
 * permitted to write from devproc and then restore the saved values
 * before returning
 */
void
setregisters(Ureg *xp, char *pureg, char *uva, int n)
{
	ulong status;

	status = xp->status;
	memmove(pureg, uva, n);
	xp->status = status;
}

/*
 * Give enough context in the ureg to produce a kernel stack for
 * a sleeping process
 */
void
setkernur(Ureg *xp, Proc *p)
{
	xp->pc = p->sched.pc;
	xp->sp = p->sched.sp;
	xp->r24 = (ulong)p;		/* up */
	xp->r31 = (ulong)sched;
}

ulong
dbgpc(Proc *p)
{
	Ureg *ur;

	ur = p->dbgreg;
	if(ur == 0)
		return 0;

	return ur->pc;
}
