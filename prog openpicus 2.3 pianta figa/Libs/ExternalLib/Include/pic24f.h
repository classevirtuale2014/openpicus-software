#define FCY		32000000UL

#define _DI()		__asm__ volatile("disi #0x3FFF")
#define _EI()		__asm__ volatile("disi #0")
