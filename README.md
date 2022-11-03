# 9front-mt7688
9 Front for the mt7688

10/21/2022
The kernel boots on the HLK-7688A from Hi-Link, which is a barebones router board.
https://www.hlktech.net/index.php?id=432
It does support tftp in u-boot, where the stock Onion Omega 2 does not.  Or at least, I haven't figured out how to get the Onion to do it, even with the ethernet addapter.  Either way, I now have the hardware to test the ethernet and switch settings.

10/18/2022
Was too busy to work on this, and when I found the time, I fugured it would be easier to start over, rather than figure out where I left off.  Turns out some of my previous problems stemmed from some mips, spim, and port code that had been understandably neglegted over the past few years.  Patches need to be done to libc and ape, to get libraries properly built, and to fix a bug in strlen in the spim code.  port/sysproc also needs to be patched for segment alignment.

Right now, on the Onion Omega 2+ with the uart-to-usb on the exp dock, using u boot to load the kernel and plan9.ini froma thumb drive, it boots, and "!rc" can be choosen from the ask at bootargs to run rc.

However, the mt7688 does not have a FPU, so any floating point stuff will crash it.  The existing FP emulation requires a patch to tos.h, which is a rather invasive patch to the portable code.

Next will be drivers for the networking, usb, i2c, and sd card reader.


5/22/2022
It boots, gets through main() and into init0(), then throws a panic.
"boot process died"
Unfortunately, the error message is just a colon

The only driver so far is for the uart.

Development is being done on an Onion Omega2+ (Omega 2 plus)

Credit to;

9Front's kernel for the SGI Indy,
Plan9's kernel for the mikrotik rb450g and loongson,
And NetBSD for some clues for what memmory to poke.
