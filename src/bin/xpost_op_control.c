/* control operators */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif !defined alloca
# ifdef __GNUC__
#  define alloca __builtin_alloca
# elif defined _AIX
#  define alloca __alloca
# elif defined _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# elif !defined HAVE_ALLOCA
#  ifdef  __cplusplus
extern "C"
#  endif
void *alloca (size_t);
# endif
#endif

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef HAVE__BOOL
#  ifdef __cplusplus
typedef bool _Bool;
#  else
#   define _Bool signed char
#  endif
# endif
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1
#endif

#include <assert.h>
#include <stdio.h> /* printf */
#include <stdlib.h> /* NULL */

#include "xpost_memory.h"
#include "xpost_object.h"
#include "xpost_stack.h"
#include "xpost_interpreter.h"
#include "xpost_error.h"
#include "xpost_name.h"
#include "xpost_array.h"
#include "xpost_dict.h"
#include "xpost_operator.h"
#include "xpost_op_control.h"

static
void Aexec (context *ctx,
            Xpost_Object O)
{
    push(ctx->lo, ctx->es, O);
}

static
void BPif (context *ctx,
           Xpost_Object B,
           Xpost_Object P)
{
    if (B.int_.val)
        push(ctx->lo, ctx->es, P);
}

static
void BPPifelse (context *ctx,
                Xpost_Object B,
                Xpost_Object Then,
                Xpost_Object Else)
{
    if (B.int_.val)
        push(ctx->lo, ctx->es, Then);
    else
        push(ctx->lo, ctx->es, Else);
}

static
void IIIPfor (context *ctx,
              Xpost_Object init,
              Xpost_Object incr,
              Xpost_Object lim,
              Xpost_Object P)
{
    integer i = init.int_.val;
    integer j = incr.int_.val;
    integer n = lim.int_.val;
    bool up = j > 0;
    if (up? i > n : i < n) return;
    assert(ctx->gl->base);
    //push(ctx->lo, ctx->es, consoper(ctx, "for", NULL,0,0));
    push(ctx->lo, ctx->es, operfromcode(ctx->opcuts.opfor));
    //push(ctx->lo, ctx->es, consoper(ctx, "cvx", NULL,0,0));
    push(ctx->lo, ctx->es, operfromcode(ctx->opcuts.cvx));
    push(ctx->lo, ctx->es, xpost_object_cvlit(P));
    push(ctx->lo, ctx->es, lim);
    push(ctx->lo, ctx->es, incr);
    push(ctx->lo, ctx->es, xpost_cons_int(i + j));
    push(ctx->lo, ctx->es, P);
    push(ctx->lo, ctx->es, init);
}

static
void RRRPfor (context *ctx,
              Xpost_Object init,
              Xpost_Object incr,
              Xpost_Object lim,
              Xpost_Object P)
{
    real i = init.real_.val;
    real j = incr.real_.val;
    real n = lim.real_.val;
    bool up = j > 0;
    if (up? i > n : i < n) return;
    //push(ctx->lo, ctx->es, consoper(ctx, "for", NULL,0,0));
    push(ctx->lo, ctx->es, operfromcode(ctx->opcuts.opfor));
    //push(ctx->lo, ctx->es, consoper(ctx, "cvx", NULL,0,0));
    push(ctx->lo, ctx->es, operfromcode(ctx->opcuts.cvx));
    push(ctx->lo, ctx->es, xpost_object_cvlit(P));
    push(ctx->lo, ctx->es, lim);
    push(ctx->lo, ctx->es, incr);
    push(ctx->lo, ctx->es, xpost_cons_real(i + j));
    push(ctx->lo, ctx->es, P);
    push(ctx->lo, ctx->es, init);
}

static
void IPrepeat (context *ctx,
               Xpost_Object n,
               Xpost_Object P)
{
    if (n.int_.val <= 0) return;
    //push(ctx->lo, ctx->es, consoper(ctx, "repeat", NULL,0,0));
    push(ctx->lo, ctx->es, operfromcode(ctx->opcuts.repeat));
    //push(ctx->lo, ctx->es, consoper(ctx, "cvx", NULL,0,0));
    push(ctx->lo, ctx->es, operfromcode(ctx->opcuts.cvx));
    push(ctx->lo, ctx->es, xpost_object_cvlit(P));
    push(ctx->lo, ctx->es, xpost_cons_int(n.int_.val - 1));
    push(ctx->lo, ctx->es, P);
}

static
void Ploop (context *ctx,
            Xpost_Object P)
{
    //push(ctx->lo, ctx->es, consoper(ctx, "loop", NULL,0,0));
    push(ctx->lo, ctx->es, operfromcode(ctx->opcuts.loop));
    //push(ctx->lo, ctx->es, consoper(ctx, "cvx", NULL,0,0));
    push(ctx->lo, ctx->es, operfromcode(ctx->opcuts.cvx));
    push(ctx->lo, ctx->es, xpost_object_cvlit(P));
    push(ctx->lo, ctx->es, P);
}

static
void Zexit (context *ctx)
{
    //Xpost_Object opfor = consoper(ctx, "for", NULL,0,0);
    Xpost_Object opfor = operfromcode(ctx->opcuts.opfor);
    //Xpost_Object oprepeat = consoper(ctx, "repeat", NULL,0,0);
    Xpost_Object oprepeat = operfromcode(ctx->opcuts.repeat);
    //Xpost_Object oploop = consoper(ctx, "loop", NULL,0,0);
    Xpost_Object oploop = operfromcode(ctx->opcuts.loop);
    //Xpost_Object opforall = consoper(ctx, "forall", NULL,0,0);
    Xpost_Object opforall = operfromcode(ctx->opcuts.forall);
    Xpost_Object x;

#if 0
    printf("\nexit\n");
    xpost_object_dump(opfor);
    xpost_object_dump(oprepeat);
    xpost_object_dump(oploop);
    xpost_object_dump(opforall);

    dumpstack(ctx->lo, ctx->os);
    dumpstack(ctx->lo, ctx->es);
    printf("\n");
#endif

    while (1) {
        x = pop(ctx->lo, ctx->es);
        //xpost_object_dump(x);
        if ( (objcmp(ctx, x, opfor)    == 0)
          || (objcmp(ctx, x, oprepeat) == 0)
          || (objcmp(ctx, x, oploop)   == 0)
          || (objcmp(ctx, x, opforall) == 0)
           ) {
            break;
        }
    }

#if 0
    printf("result:");
    dumpstack(ctx->lo, ctx->es);
#endif
}

/* The stopped context is a boolean 'false' on the exec stack,
   so normal execution simply falls through and pushes the 
   false onto the operand stack. 'stop' then merely has to 
   search for 'false' and push a 'true'.  */

static
void Zstop(context *ctx)
{
    Xpost_Object f = xpost_cons_bool(false);
    int c = count(ctx->lo, ctx->es);
    Xpost_Object x;
    while (c--) {
        x = pop(ctx->lo, ctx->es);
        if(objcmp(ctx, f, x) == 0) {
            push(ctx->lo, ctx->os, xpost_cons_bool(true));
            return;
        }
    }
    error(unregistered, "no stopped context in 'stop'");
}

static
void Astopped(context *ctx,
              Xpost_Object o)
{
    push(ctx->lo, ctx->es, xpost_cons_bool(false));
    push(ctx->lo, ctx->es, o);
}

static
void Zcountexecstack(context *ctx)
{
    push(ctx->lo, ctx->os, xpost_cons_int(count(ctx->lo, ctx->es)));
}

static
void Aexecstack(context *ctx,
                Xpost_Object A)
{
    int z = count(ctx->lo, ctx->es);
    int i;
    for (i=0; i < z; i++)
        barput(ctx, A, i, bot(ctx->lo, ctx->es, i));
    push(ctx->lo, ctx->os, arrgetinterval(A, 0, z));
}

//TODO start

static
void Zquit(context *ctx)
{
    ctx->quit = 1;
}

void initopc (context *ctx,
              Xpost_Object sd)
{
    oper *optab;
    Xpost_Object n,op;
    assert(ctx->gl->base);
    optab = (void *)(ctx->gl->base + adrent(ctx->gl, OPTAB));

    op = consoper(ctx, "exec", Aexec, 0, 1, anytype); INSTALL;
    op = consoper(ctx, "if", BPif, 0, 2, booleantype, proctype); INSTALL;
    op = consoper(ctx, "ifelse", BPPifelse, 0, 3, booleantype, proctype, proctype); INSTALL;
    op = consoper(ctx, "for", IIIPfor, 0, 4, \
            integertype, integertype, integertype, proctype); INSTALL;
    op = consoper(ctx, "for", RRRPfor, 0, 4, \
            floattype, floattype, floattype, proctype); INSTALL;
    ctx->opcuts.opfor = op.mark_.padw;
    op = consoper(ctx, "repeat", IPrepeat, 0, 2, integertype, proctype); INSTALL;
    ctx->opcuts.repeat = op.mark_.padw;
    op = consoper(ctx, "loop", Ploop, 0, 1, proctype); INSTALL;
    ctx->opcuts.loop = op.mark_.padw;
    op = consoper(ctx, "exit", Zexit, 0, 0); INSTALL;
    op = consoper(ctx, "stop", Zstop, 0, 0); INSTALL;
    op = consoper(ctx, "stopped", Astopped, 0, 1, anytype); INSTALL;
    op = consoper(ctx, "countexecstack", Zcountexecstack, 1, 0); INSTALL;
    op = consoper(ctx, "execstack", Aexecstack, 1, 1, arraytype); INSTALL;
    op = consoper(ctx, "quit", Zquit, 0, 0); INSTALL;
    /*
    op = consoper(ctx, "eq", Aeq, 1, 2, anytype, anytype); INSTALL;
    //dumpdic(ctx->gl, sd); fflush(NULL);
    bdcput(ctx, sd, consname(ctx, "mark"), mark);
    */

}


