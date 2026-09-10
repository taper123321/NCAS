// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "uexception.h"
#ifdef MEMMGR
//
// Memory manager: dynamically allocates memory from 
// a fixed pool that is allocated statically at link-time.
// 
// Usage: after calling memmgr_init() in your 
// initialization routine, just use memmgr_alloc() instead
// of malloc() and memmgr_free() instead of free().
// Naturally, you can use the preprocessor to define 
// malloc() and free() as aliases to memmgr_alloc() and 
// memmgr_free(). This way the manager will be a drop-in 
// replacement for the standard C library allocators, and can
// be useful for debugging memory allocation problems and 
// leaks.
//
// Preprocessor flags you can define to customize the 
// memory manager:
//
// DEBUG_MEMMGR_FATAL
//    Allow printing out a message when allocations fail
//
// DEBUG_MEMMGR_SUPPORT_STATS
//    Allow printing out of stats in function 
//    memmgr_print_stats When this is disabled, 
//    memmgr_print_stats does nothing.
//
// Note that in production code on an embedded system 
// you'll probably want to keep those undefined, because
// they cause printf to be called.
//
// POOL_SIZE
//    Size of the pool for new allocations. This is 
//    effectively the heap size of the application, and can 
//    be changed in accordance with the available memory 
//    resources.
//
// MIN_POOL_ALLOC_QUANTAS
//    Internally, the memory manager allocates memory in
//    quantas roughly the size of two ulong objects. To
//    minimize pool fragmentation in case of multiple allocations
//    and deallocations, it is advisable to not allocate
//    blocks that are too small.
//    This flag sets the minimal ammount of quantas for 
//    an allocation. If the size of a ulong is 4 and you
//    set this flag to 16, the minimal size of an allocation
//    will be 4 * 2 * 16 = 128 bytes
//    If you have a lot of small allocations, keep this value
//    low to conserve memory. If you have mostly large 
//    allocations, it is best to make it higher, to avoid 
//    fragmentation.
//
// Notes:
// 1. This memory manager is *not thread safe*. Use it only
//    for single thread/task applications.
// 

//#define DEBUG_MEMMGR_SUPPORT_STATS 1

#define POOL_SIZE 128 * 1024
//#define POOL_SIZE 32 * 1024
#define MIN_POOL_ALLOC_QUANTAS 4

typedef unsigned char byte;
typedef unsigned long ulong;

extern int memmgr_initialized; 
// must be initialized to non 0 otherwise it will not be initialized
// will be initialized to -1 in unew.cc

// Initialize the memory manager. This function should be called
// only once in the beginning of the program. This is done automatically
// if memmge_initialized<0 by memmgr_alloc or memmgr_free
//
void memmgr_init();

// 'malloc' clone
//
void* memmgr_alloc(ulong nbytes);

// 'free' clone
//
void memmgr_free(void* ap);

// Prints statistics about the current state of the memory
// manager
// (disabled)
void memmgr_print_stats();

inline void * tmalloc(size_t n){ return memmgr_alloc(n); }
inline void nfree(void *ptr)  { if (ptr) memmgr_free(ptr); }
void * nrealloc(void *ptr, size_t size);
#else
#include <stdio.h>
/// Just like malloc, but throws on failure.
inline void* tmalloc (size_t n) { return malloc(n); }
/// Just like free, but doesn't crash when given a NULL.
inline void nfree (void* p) noexcept { if (p) free (p); }
void * nrealloc(void *ptr, size_t size);
#endif


#if WITHOUT_LIBSTDCPP

//
// These are replaceable signatures:
//  - normal single new and delete (no arguments, throw @c bad_alloc on error)
//  - normal array new and delete (same)
//  - @c nothrow single new and delete (take a @c nothrow argument, return
//    @c NULL on error)
//  - @c nothrow array new and delete (same)
//
//  Placement new and delete signatures (take a memory address argument,
//  does nothing) may not be replaced by a user's program.
//
inline void* operator new (size_t n) USTL_THROW (std::bad_alloc)	{ return (tmalloc (n)); }
inline void* operator new[] (size_t n) USTL_THROW (std::bad_alloc)	{ return (tmalloc (n)); }
inline void  operator delete (void* p)				{ nfree (p); }
inline void  operator delete[] (void* p)			{ nfree (p); }

// Default placement versions of operator new.
inline void* operator new (size_t, void* p) noexcept	{ return (p); }
inline void* operator new[] (size_t, void* p) noexcept	{ return (p); }

// Default placement versions of operator delete.
inline void  operator delete  (void*, void*) noexcept	{ }
inline void  operator delete[](void*, void*) noexcept	{ }

#else
#include <new>
#endif	// WITHOUT_LIBSTDCPP
