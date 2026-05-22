#ifndef __ATOMIC_H__
#define __ATOMIC_H__

extern void spin_lockb(void *p);

#define spin_lock(p) _Generic((p), \
    uint8_t* : spin_lockb, \
    int8_t* : spin_lockb )(p)

#define spin_unlock(p) do{(*(_Atomic uint8_t*)(p))=0;}while(0)

extern void rwlock_inc(uint64_t *p);
//extern void rwlock_dec(uint64_t *p);
#define rwlock_dec(p) do{(*(_Atomic uint64_t*)(p))--;}while(0)

extern void rwlock_lock(uint64_t *p);
//extern void rwlock_unlock(uint64_t *p);
#define rwlock_unlock(p) do{(*(_Atomic uint64_t*)(p))=0;}while(0)

#endif