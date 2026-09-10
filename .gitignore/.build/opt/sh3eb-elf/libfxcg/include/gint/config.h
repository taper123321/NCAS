//---
//	config - Compile-time generate configuration
//---

#ifndef GINT_CONFIG
#define GINT_CONFIG

#include <gint/defs/types.h>


/* GINT_USER_VRAM: Selects whether to store VRAMs in the user stack (occupying
   350k/512k of the area) or in the system stack (the default). (fx-CG 50) */
/* #define GINT_USER_VRAM */

/* GINT_STATIC_GRAY: Selects whether additional gray VRAMs are allocated
   statically or in the system heap (fx-9860G) */
/* #define GINT_STATIC_GRAY */

/* GINT_KMALLOC_DEBUG: Selects whether kmalloc debug functions are enabled
   (these are mainly data structure integrity checks and information that make
   sense for a developer). This is independent from statistics, which can be
   enabled or disabled at runtime. */
/* #define GINT_KMALLOC_DEBUG */

#endif /* GINT_CONFIG */
