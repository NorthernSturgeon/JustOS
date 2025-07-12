#ifndef __PORT_H__
#define __PORT_H__

#define ReadPort8(val, port) asm volatile("insb \n":"":);
#define ReadPort16(val, port) asm volatile("insw \n"::);
#define ReadPort32(val, port) asm volatile("insl \n"::);

#define WritePort8(port, val) asm volatile("outsb \n"::);
#define WritePort16(port, val) asm volatile("outsw \n"::);
#define WritePort32(port, val) asm volatile("outsl \n"::);

#endif
