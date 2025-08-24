#ifndef __ATOMIC_H__
#define __ATOMIC_H__

extern void spin_lockb(void *p);

#define spin_lock(p) _Generic((p), \
    uint8_t* : spin_lockb, \
    int8_t* : spin_lockb )(p)

#define spin_unlock(p) *(p) = 0

#endif