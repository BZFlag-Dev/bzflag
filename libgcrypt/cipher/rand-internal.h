/* rand-internal.h - header to glue the random functions
 *	Copyright (C) 1998, 2002 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifndef G10_RAND_INTERNAL_H
#define G10_RAND_INTERNAL_H


/* Constants used to define the origin of random added to the pool.
   The code is sensitive to the order of the values.  */
enum random_origins 
  {
    RANDOM_ORIGIN_INIT = 0,      /* Used only for initialization. */
    RANDOM_ORIGIN_EXTERNAL = 1,  /* Added from an external source.  */
    RANDOM_ORIGIN_FASTPOLL = 2,  /* Fast random poll function.  */
    RANDOM_ORIGIN_SLOWPOLL = 3,  /* Slow poll function.  */
    RANDOM_ORIGIN_EXTRAPOLL = 4  /* Used to mark an extra pool seed
                                    due to a GCRY_VERY_STRONG_RANDOM
                                    random request.  */
  };




void _gcry_random_progress (const char *what, int printchar,
                            int current, int total);


int _gcry_rndlinux_gather_random (void (*add) (const void *, size_t,
                                               enum random_origins),
                                   enum random_origins origin,
                                  size_t length, int level);
int _gcry_rndunix_gather_random (void (*add) (const void *, size_t,
                                              enum random_origins),
                                 enum random_origins origin,
                                 size_t length, int level);
int _gcry_rndegd_gather_random (void (*add) (const void *, size_t,
                                             enum random_origins),
                                enum random_origins origin,
                                size_t length, int level);
int _gcry_rndegd_connect_socket (int nofail);
int _gcry_rndw32_gather_random (void (*add) (const void *, size_t,
                                             enum random_origins),
                                enum random_origins origin,
                                size_t length, int level);
void _gcry_rndw32_gather_random_fast (void (*add)(const void*, size_t, 
                                                  enum random_origins),
                                      enum random_origins origin );

int _gcry_rndhw_failed_p (void);
void _gcry_rndhw_poll_fast (void (*add)(const void*, size_t,
                                        enum random_origins),
                            enum random_origins origin);
size_t _gcry_rndhw_poll_slow (void (*add)(const void*, size_t,
                                          enum random_origins),
                              enum random_origins origin);



#endif /*G10_RAND_INTERNAL_H*/
