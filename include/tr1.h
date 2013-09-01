#ifndef _tr1_h_
#define _tr1_h_

#define QUOTE(arg) <arg>

// GNU GCC/G++ between 4.6 and 4.8
#if (defined(__GNUC__) || defined(__GNUG__)) && (__GNUC__ == 4) && (__GNUC_MINOR__ >= 6) && (__GNUC_MINOR__ < 8)
#  define TR1IFY(arg) tr1/arg
#else
#  define TR1IFY(arg) arg
#endif

#define TR1INCLUDE(arg) QUOTE(TR1IFY(arg))

#endif
