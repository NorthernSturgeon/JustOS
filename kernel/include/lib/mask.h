#ifndef __MASK_H__
#define __MASK_H__

#define switch_on(val, mask) (val) | (mask)
#define switch_off(val, mask) (val) & ~(mask)

#endif
