/* ldap_pvt_thread.h - ldap threads header file */
/* $OpenLDAP: pkg/ldap/include/ldap_pvt_thread.h,v 1.41.2.7 2007/01/02 21:43:46 kurt Exp $ */
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

#ifndef _LDAP_PVT_THREAD_H
#define _LDAP_PVT_THREAD_H /* libldap_r/ldap_thr_debug.h #undefines this */

#include "ldap_cdefs.h"
#include "ldap_int_thread.h"

LDAP_BEGIN_DECL

#ifndef LDAP_PVT_THREAD_H_DONE
typedef ldap_int_thread_t			ldap_pvt_thread_t;
#ifdef LDAP_THREAD_DEBUG_WRAP
typedef ldap_debug_thread_mutex_t	ldap_pvt_thread_mutex_t;
typedef ldap_debug_thread_cond_t	ldap_pvt_thread_cond_t;
typedef ldap_debug_thread_rdwr_t	ldap_pvt_thread_rdwr_t;
#else
typedef ldap_int_thread_mutex_t		ldap_pvt_thread_mutex_t;
typedef ldap_int_thread_cond_t		ldap_pvt_thread_cond_t;
typedef ldap_int_thread_rdwr_t		ldap_pvt_thread_rdwr_t;
#endif
#endif /* !LDAP_PVT_THREAD_H_DONE */

#define ldap_pvt_thread_equal		ldap_int_thread_equal

LDAP_F( int )
ldap_pvt_thread_initialize LDAP_P(( void ));

LDAP_F( int )
ldap_pvt_thread_destroy LDAP_P(( void ));

LDAP_F( unsigned int )
ldap_pvt_thread_sleep LDAP_P(( unsigned int s ));

LDAP_F( int )
ldap_pvt_thread_get_concurrency LDAP_P(( void ));

LDAP_F( int )
ldap_pvt_thread_set_concurrency LDAP_P(( int ));

#define LDAP_PVT_THREAD_CREATE_JOINABLE 0
#define LDAP_PVT_THREAD_CREATE_DETACHED 1

#ifndef LDAP_PVT_THREAD_H_DONE
#define	LDAP_PVT_THREAD_SET_STACK_SIZE
#ifndef LDAP_PVT_THREAD_STACK_SIZE
	/* LARGE stack. Will be twice as large on 64 bit machine. */
#define LDAP_PVT_THREAD_STACK_SIZE	( 1 * 1024 * 1024 * sizeof(void *) )
/* May be explicitly defined to zero to disable it */
#elif LDAP_PVT_THREAD_STACK_SIZE == 0
#undef LDAP_PVT_THREAD_SET_STACK_SIZE
#endif
#endif /* !LDAP_PVT_THREAD_H_DONE */

LDAP_F( int )
ldap_pvt_thread_create LDAP_P((
	ldap_pvt_thread_t * thread,
	int	detach,
	void *(*start_routine)( void * ),
	void *arg));

LDAP_F( void )
ldap_pvt_thread_exit LDAP_P(( void *retval ));

LDAP_F( int )
ldap_pvt_thread_join LDAP_P(( ldap_pvt_thread_t thread, void **status ));

LDAP_F( int )
ldap_pvt_thread_kill LDAP_P(( ldap_pvt_thread_t thread, int signo ));

LDAP_F( int )
ldap_pvt_thread_yield LDAP_P(( void ));

LDAP_F( int )
ldap_pvt_thread_cond_init LDAP_P(( ldap_pvt_thread_cond_t *cond ));

LDAP_F( int )
ldap_pvt_thread_cond_destroy LDAP_P(( ldap_pvt_thread_cond_t *cond ));

LDAP_F( int )
ldap_pvt_thread_cond_signal LDAP_P(( ldap_pvt_thread_cond_t *cond ));

LDAP_F( int )
ldap_pvt_thread_cond_broadcast LDAP_P(( ldap_pvt_thread_cond_t *cond ));

LDAP_F( int )
ldap_pvt_thread_cond_wait LDAP_P((
	ldap_pvt_thread_cond_t *cond,
	ldap_pvt_thread_mutex_t *mutex ));

LDAP_F( int )
ldap_pvt_thread_mutex_init LDAP_P(( ldap_pvt_thread_mutex_t *mutex ));

LDAP_F( int )
ldap_pvt_thread_mutex_destroy LDAP_P(( ldap_pvt_thread_mutex_t *mutex ));

LDAP_F( int )
ldap_pvt_thread_mutex_lock LDAP_P(( ldap_pvt_thread_mutex_t *mutex ));

LDAP_F( int )
ldap_pvt_thread_mutex_trylock LDAP_P(( ldap_pvt_thread_mutex_t *mutex ));

LDAP_F( int )
ldap_pvt_thread_mutex_unlock LDAP_P(( ldap_pvt_thread_mutex_t *mutex ));

LDAP_F( ldap_pvt_thread_t )
ldap_pvt_thread_self LDAP_P(( void ));

LDAP_F( int )
ldap_pvt_thread_rdwr_init LDAP_P((ldap_pvt_thread_rdwr_t *rdwrp));

LDAP_F( int )
ldap_pvt_thread_rdwr_destroy LDAP_P((ldap_pvt_thread_rdwr_t *rdwrp));

LDAP_F( int )
ldap_pvt_thread_rdwr_rlock LDAP_P((ldap_pvt_thread_rdwr_t *rdwrp));

LDAP_F( int )
ldap_pvt_thread_rdwr_rtrylock LDAP_P((ldap_pvt_thread_rdwr_t *rdwrp));

LDAP_F( int )
ldap_pvt_thread_rdwr_runlock LDAP_P((ldap_pvt_thread_rdwr_t *rdwrp));

LDAP_F( int )
ldap_pvt_thread_rdwr_wlock LDAP_P((ldap_pvt_thread_rdwr_t *rdwrp));

LDAP_F( int )
ldap_pvt_thread_rdwr_wtrylock LDAP_P((ldap_pvt_thread_rdwr_t *rdwrp));

LDAP_F( int )
ldap_pvt_thread_rdwr_wunlock LDAP_P((ldap_pvt_thread_rdwr_t *rdwrp));

#ifdef LDAP_DEBUG
LDAP_F( int )
ldap_pvt_thread_rdwr_readers LDAP_P((ldap_pvt_thread_rdwr_t *rdwrp));

LDAP_F( int )
ldap_pvt_thread_rdwr_writers LDAP_P((ldap_pvt_thread_rdwr_t *rdwrp));

LDAP_F( int )
ldap_pvt_thread_rdwr_active LDAP_P((ldap_pvt_thread_rdwr_t *rdwrp));
#endif /* LDAP_DEBUG */

#define LDAP_PVT_THREAD_EINVAL EINVAL
#define LDAP_PVT_THREAD_EBUSY EINVAL

#ifndef LDAP_PVT_THREAD_H_DONE
typedef ldap_int_thread_pool_t ldap_pvt_thread_pool_t;

typedef void * (ldap_pvt_thread_start_t) LDAP_P((void *ctx, void *arg));
typedef void (ldap_pvt_thread_pool_keyfree_t) LDAP_P((void *key, void *data));
#endif /* !LDAP_PVT_THREAD_H_DONE */

LDAP_F( int )
ldap_pvt_thread_pool_init LDAP_P((
	ldap_pvt_thread_pool_t *pool_out,
	int max_threads,
	int max_pending ));

LDAP_F( int )
ldap_pvt_thread_pool_submit LDAP_P((
	ldap_pvt_thread_pool_t *pool,
	ldap_pvt_thread_start_t *start,
	void *arg ));

LDAP_F( int )
ldap_pvt_thread_pool_maxthreads LDAP_P((
	ldap_pvt_thread_pool_t *pool,
	int max_threads ));

LDAP_F( int )
ldap_pvt_thread_pool_backload LDAP_P((
	ldap_pvt_thread_pool_t *pool ));

LDAP_F( int )
ldap_pvt_thread_pool_pause LDAP_P((
	ldap_pvt_thread_pool_t *pool ));

LDAP_F( int )
ldap_pvt_thread_pool_resume LDAP_P((
	ldap_pvt_thread_pool_t *pool ));

LDAP_F( int )
ldap_pvt_thread_pool_destroy LDAP_P((
	ldap_pvt_thread_pool_t *pool,
	int run_pending ));

LDAP_F( int )
ldap_pvt_thread_pool_getkey LDAP_P((
	void *ctx,
	void *key,
	void **data,
	ldap_pvt_thread_pool_keyfree_t **kfree ));

LDAP_F( int )
ldap_pvt_thread_pool_setkey LDAP_P((
	void *ctx,
	void *key,
	void *data,
	ldap_pvt_thread_pool_keyfree_t *kfree ));

LDAP_F( void )
ldap_pvt_thread_pool_purgekey LDAP_P(( void *key ));

LDAP_F( void *)
ldap_pvt_thread_pool_context LDAP_P(( void ));

LDAP_F( void )
ldap_pvt_thread_pool_context_reset LDAP_P(( void *key ));

LDAP_END_DECL

#define LDAP_PVT_THREAD_H_DONE
#endif /* _LDAP_THREAD_H */
