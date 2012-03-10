#ifndef __FSM_H
#define __FSM_H

#include <stdint.h>

#define FSM_DECLARE(name, initial) \
typedef enum { \
    initial##__ST = 0

#define FSM_DECLARE_END(name) \
 , name##__ST_FINAL \
} name##__fsm__t ; \
static name##__fsm__t name##__fsm__var = 0; \

#define FSM_STATE_DECL(name) ,name##__ST

#define FSM_BEGIN(name, n) { \
    name##__fsm__t fsmlocal = name##__fsm__var; \
    const  name##__fsm__t fsmfinal = name##__ST_FINAL; \
    static name##__fsm__t fsmstack[n] = { 0 }; \
    static name##__fsm__t *fsmstackp = &fsmstack[0]; \
    switch(fsmlocal) { \

#define FSM_STACK_PUSH(n) *fsmstackp++ = (n)
#define FSM_STACK_POP()   *(--fsmstackp)

#define FSM_STATE_BEGIN(name) \
    case name##__ST: { \

#define FSM_NEXT_STATE (fsmlocal+1)

#define FSM_CURRENT_STATE (fsmlocal)

#define FSM_FINAL_STATE (fsmfinal)

#define FSM_RESET(n) n##__fsm__var = 0;

#define FSM_STATE_ENDS(nxt) \
    fsmlocal = nxt##__ST; \
    break; \
    } \

#define FSM_STATE_END(nxt) \
    fsmlocal = (nxt); \
    break; \
    } \

#define FSM_NEXT() fsmlocal++;

#define FSM_S(n) n##__ST

#define FSM_TRANS(v) \
    fsmlocal = v; \

#define FSM_END(name) \
    } \
    name##__fsm__var = fsmlocal; \
    } \

#endif
