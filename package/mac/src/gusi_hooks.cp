


#define GUSI_SOURCE
#include <GUSIBasics.h>
#include "mac_funcs.h"

void mySpinHook (bool wait) {

if (wait)
 MacOneEvent ();
}

void MacInitGUSIHook () {

  GUSISetHook(GUSI_SpinHook, (GUSIHook) mySpinHook);
}