# 9front-mt7688
9 Front for the mt7688

3/8/2023
This kernel has now been added to 9 Front

12/11/2022
Added Ethernet driver and FPU emulation.  The Ethernet driver works okay, but still needs some finishing touches, like clean up if it is shutdown and restarted.  And for now, the switch and ethernet code is combined, but I plan on splitting out switch controls to seperate device files.  FPU emulation is just barely functional.  It is enough for some simple stuff, and it currently allows the system to log into a grid as a cpu server.  It also seems to have some issues with parts of the draw library.  While you can log in to it with drawterm, I've noticed some things like stats, clock, and catclock won't render corectly.  Keep in mind, fpuemu requires a change to tos.h, and that means recompiling everything else that uses tos.h.

To boot it as a CPU server, I included a copy of nvram to be added to the bootdir.  There is an entry for that in the "mt7688" config file.  Then load a plan9.ini that has "nvram=/boot/nvram", "nvroff=0", and "nvrlen=512".  The plan is to later add a driver to access the on board flash, and store nvram there.


11/3/2022
Updated to boot on the new 9Front release 
“THE GOLDEN AGE OF BALLOONING”
http://9front.org/releases/2022/10/31/0/


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
