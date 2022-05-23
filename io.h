/*
 *  various things to IO with
 */

#define	IO(t,x)		((t*)(KSEG1|((ulong)x)))

/* for mt7688 testing on onion Ω 2 + */
#define	SYSCTLBASE	0x10000000
#define TIMERBASE	0x10000100
#define IRQBASE		0x10000200
#define MEMCBASE	0x10000300
#define RBUSBASE	0x10000400
#define	MCNTBASE	0x10000500
#define GPIOBASE	0x10000600
#define	I2CBASE		0x10000900
#define I2SBASE		0x10000A00
#define SPIBASE		0x10000B00
#define UARTLBASE	0x10000C00
#define UART1BASE	0x10000D00
#define UART2BASE	0x10000E00

#define	DMABASE		0x10002800
#define AESBASE		0x10004000	/* crypto engine */

#define ETHBASE		0x10100000
#define SWCHBASE	0x10110000
#define	PCIBASE		0x10140000
#define PCIWIN		0x10150000
#define	WIFIBASE	0x10180000
#define USBBASE		0x101C0000



/*
 *  duarts, frequency and registers
 */
#define DUARTFREQ	40000000  /* mt7688 has a 40MHz clock */	

#define UART_RBR	0x00
#define UART_THR	0x00
#define	UART_IER	0x04
#define UART_IIR	0x08
#define	UART_FCR	0x08
#define	UART_LCR	0x0C
#define UART_MCR	0x10
#define UART_LSR	0x14
#define	UART_MSR	0x18
#define	UART_SCR	0x1C
#define UART_DLL	0x00
#define UART_DLM	0x04



/*
 *  interrupt levels
 */

#define IRQshift	8;

/* for cpu */
enum {
	IRQlow		=	0,	//INTR2
	IRQhigh,
	IRQpci,
	IRQethr,
	IRQwifi,
	IRQtimer,			//INTR7
	IRQsys,				// psuedo numbers for INC
	IRQtimer0,
	IRQwdog,
	IRQillacc,
	IRQpcm,
	IRQuartf,
	IRQgpio,
	IRQdma,
	IRQnand,
	IRQperf,
	IRQi2c,
	IRQspi,
	IRQuartl,
	IRQcrypto,
	IRQsdhc,
	IRQr2p,
	IRQnone,
	IRQethsw,
	IRQusbh,
	IRQusbd,
	IRQmax,
};


/*
 * Interrupts on side controller
 */

#define INC_SYSCTL		0
#define INC_TIMER0		1
#define INC_WDOG		2
#define INC_ILLACC		3
#define INC_PCM			4
#define INC_UARTF		5
#define INC_GPIO		6
#define INC_DMA			7
#define INC_NAND		8
#define INC_PERF		9
#define INC_I2C			10
#define INC_SPI			11
#define INC_UARTL		12
#define INC_CRYPTO		13
#define INC_SDHC		14
#define INC_R2P			15
#define INC_ETHSW		17
#define INC_USBH		18
#define INC_USBD		19
#define INC_GLOBAL		31



/*
 * Interrupt Controller Registers
 */

#define IRQ_STAT		0x9C
#define FIQ_STAT		0xA0
#define IRQ_SEL0		0x00	/* type */
#define INT_PURE		0xA4	/* raw */
#define IRQ_MASK_SET	0x80	/* enable */
#define IRQ_MASK_CLR	0x78	/* disable */


/*
 * timer controls
 */

#define TIME_GLB	0x00

#define CLK0_CTL	0x10
#define CLK0_LOAD	0x14
#define CLK0_TIME	0x18

#define WDOG_CTL	0x20
#define WDOG_LOAD	0x24
#define WDOG_TIME	0x28

#define GLB_T0_IRQ  (1<<0)
#define GLB_WB_IRQ	(1<<1)
#define GLB_T1_IRQ	(1<<2)
#define GLB_T0_RST	(1<<8)
#define	GLB_WD_RST	(1<<9)
#define	GLB_T1_RST	(1<<10)

#define TIMER_EN	(1<<7)  /* used on X_CTL regs */
#define AUTOLOAD	(1<<4)
#define CLK_PRSC(x)	((x)<<16)


/* for MIPS CNT */
#define MCNT_CFG	0x00
#define MCNT_CMP	0x04
#define	MCNT_CNT	0x08

#define MCNT_EN		1	/* for MCNT_CFG */
