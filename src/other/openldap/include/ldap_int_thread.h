/* ldap_int_thread.h - ldap internal thread wrappers header file */
/* $OpenLDAP: pkg/ldap/include/ldap_int_thread.h,v 1.13.2.4 2007/01/02 21:43:46 kurt Exp $ */
/* This work is part of OpenLDAP Software <http://www.openldap.org/>.
 * 
 * Copyright 1998-2007 The OpenLDAP Foundation.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted only as authorized by the OpenLDAP
 * Public License.
 *
 * A copy of this license is available in file LICENSE in the
 * top-level directory of the distribution or, alternatively, at
 * <http://www.OpenLDAP.org/license.html>.
 */


LDAP_BEGIN_DECL

/* Can be done twice in libldap_r.  See libldap_r/ldap_thr_debug.h. */
LDAP_F(int) ldap_int_thread_initialize LDAP_P(( void ));
LDAP_F(int) ldap_int_thread_destroy    LDAP_P(( void ));

LDAP_END_DECL


#ifndef _LDAP_INT_THREAD_H
#define _LDAP_INT_THREAD_H

#if defined( HAVE_PTHREADS )
/**********************************
 *                                *
 * definitions for POSIX Threads  *
 *                                *
 **********************************/

#include <pthread.h>
#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

LDAP_BEGIN_DECL

typedef pthread_t		ldap_int_thread_t;
typedef pthread_mutex_t		ldap_int_thread_mutex_t;
typedef pthread_cond_t		ldap_int_thread_cond_t;

#define ldap_int_thread_equal(a, b)	pthread_equal((a), (b))

#if defined( _POSIX_REENTRANT_FUNCTIONS ) || \
	defined( _POSIX_THREAD_SAFE_FUNCTIONS ) || \
	defined( _POSIX_THREADSAFE_FUNCTIONS )
#define HAVE_REENTRANT_FUNCTIONS 1
#endif

#if defined( HAVE_PTHREAD_GETCONCURRENCY ) || \
	defined( HAVE_THR_GETCONCURRENCY )
#define LDAP_THREAD_HAVE_GETCONCURRENCY 1
#endif

#if defined( HAVE_PTHREAD_SETCONCURRENCY ) || \
	defined( HAVE_THR_SETCONCURRENCY )
#define LDAP_THREAD_HAVE_SETCONCURRENCY 1
#endif

#if 0 && defined( HAVE_PTHREAD_RWLOCK_DESTROY )
#define LDAP_THREAD_HAVE_RDWR 1
typedef pthread_rwlock_t ldap_int_thread_rdwr_t;
#endif

LDAP_END_DECL

#elif defined ( HAVE_MACH_CTHREADS )
/**********************************
 *                                *
 * definitions for Mach CThreads  *
 *                                *
 **********************************/

#if defined( HAVE_MACH_CTHREADS_H )
#	include <mach/cthreads.h>
#elif defined( HAVE_CTHREAD_H
#	include <cthreads.h>
#endif

LDAP_BEGIN_DECL

typedef cthread_t		ldap_int_thread_t;
typedef struct mutex		ldap_int_thread_mutex_t;
typedef struct condition	ldap_int_thread_cond_t;

LDAP_END_DECL

#elif defined( HAVE_GNU_PTH )
/***********************************
 *                                 *
 * thread definitions for GNU Pth  *
 *                                 *
 ***********************************/

#define PTH_SYSCALL_SOFT 1
#include <pth.h>

LDAP_BEGIN_DECL

typedef pth_t		ldap_int_thread_t;
typedef pth_mutex_t	ldap_int_thread_mutex_t;
typedef pth_cond_t	ldap_int_thread_cond_t;

#if 0
#define LDAP_THREAD_HAVE_RDWR 1
typedef pth_rwlock_t ldap_int_thread_rdwr_t;
#endif

LDAP_END_DECL

#elif defined( HAVE_THR )
/********************************************
 *                                          *
 * thread definitions for Solaris LWP (THR) *
 *                                          *
 ********************************************/

#include <thread.h>
#include <synch.h>

LDAP_BEGIN_DECL

typedef thread_t		ldap_int_thread_t;
typedef mutex_t			ldap_int_thread_mutex_t;
typedef cond_t			ldap_int_thread_cond_t;

#define HAVE_REENTRANT_FUNCTIONS 1

#ifdef HAVE_THR_GETCONCURRENCY
#define LDAP_THREAD_HAVE_GETCONCURRENCY 1
#endif
#ifdef HAVE_THR_SETCONCURRENCY
#define LDAP_THREAD_HAVE_SETCONCURRENCY 1
#endif

LDAP_END_DECL

#elif defined( HAVE_LWP )
/*************************************
 *                                   *
 * thread definitions for SunOS LWP  *
 *                                   *
 *************************************/

#include <lwp/lwp.h>
#include <lwp/stackdep.h>
#define LDAP_THREAD_HAVE_SLEEP 1

LDAP_BEGIN_DECL

typedef thread_t		ldap_int_thread_t;
typedef mon_t			ldap_int_thread_mutex_t;
struct ldap_int_thread_lwp_cv {
	int		lcv_created;
	cv_t		lcv_cv;
};
typedef struct ldap_int_thread_lwp_cv ldap_int_thread_cond_t;

#define HAVE_REENTRANT_FUNCTIONS 1

LDAP_END_DECL

#elif defined(HAVE_NT_THREADS)
/*************************************
 *                                   *
 * thread definitions for NT threads *
 *                                   *
 *************************************/

#include <process.h>
#include <windows.h>

LDAP_BEGIN_DECL

typedef unsigned long	ldap_int_thread_t;
typedef HANDLE	ldap_int_thread_mutex_t;
typedef HANDLE	ldap_int_thread_cond_t;

LDAP_END_DECL

#else
/***********************************
 *                                 *
 * thread definitions for no       *
 * underlying library support      *
 *                                 *
 ***********************************/

#ifndef NO_THREADS
#define NO_THREADS 1
#endif

LDAP_BEGIN_DECL

typedef int			ldap_int_thread_t;
typedef int			ldap_int_thread_mutex_t;
typedef int			ldap_int_thread_cond_t;

#define LDAP_THREAD_HAVE_TPOOL 1
typedef int			ldap_int_thread_pool_t;

LDAP_END_DECL

#endif /* no threads support */


LDAP_BEGIN_DECL

#ifndef ldap_int_thread_equal
#define ldap_int_thread_equal(a, b)	((a) == (b))
#endif

#ifndef LDAP_THREAD_HAVE_RDWR
typedef struct ldap_int_thread_rdwr_s * ldap_int_thread_rdwr_t;
#endif

LDAP_F(int) ldap_int_thread_pool_startup ( void );
LDAP_F(int) ldap_int_thread_pool_shutdown ( void );

#ifndef LDAP_THREAD_HAVE_TPOOL
typedef struct ldap_int_thread_pool_s * ldap_int_thread_pool_t;
#endif

LDAP_END_DECL


#if defined(LDAP_THREAD_DEBUG) && !((LDAP_THREAD_DEBUG +0) & 2U)
#define LDAP_THREAD_DEBUG_WRAP 1
#endif

#ifdef LDAP_THREAD_DEBUG_WRAP
/**************************************
 *                                    *
 * definitions for type-wrapped debug *
 *                                    *
 **************************************/

LDAP_BEGIN_DECL

#ifndef LDAP_UINTPTR_T	/* May be configured in CPPFLAGS */
#define LDAP_UINTPTR_T	unsigned long
#endif

typedef union {
	unsigned char			*ptr;
	LDAP_UINTPTR_T			num;
} ldap_debug_usage_info_t;

typedef struct {
	ldap_int_thread_mutex_t	wrapped;
	ldap_debug_usage_info_t	usage;
} ldap_debug_thread_mutex_t;

typedef struct {
	ldap_int_thread_cond_t	wrapped;
	ldap_debug_usage_info_t	usage;
} ldap_debug_thread_cond_t;

typedef struct {
	ldap_int_thread_rdwr_t	wrapped;
	ldap_debug_usage_info_t	usage;
} ldap_debug_thread_rdwr_t;

LDAP_END_DECL

#endif /* LDAP_THREAD_DEBUG_WRAP */

#endif /* _LDAP_INT_THREAD_H */
