/* $Id$ */

/* Copyright 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#include "setup.h"

#if defined(WIN32) && !defined(WATT32)
#include "nameser.h"

#else
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h> /* <netinet/tcp.h> may need it */
#endif
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h> /* for TCP_NODELAY */
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_ARPA_NAMESER_H
#include <arpa/nameser.h>
#endif
#ifdef HAVE_ARPA_NAMESER_COMPAT_H
#include <arpa/nameser_compat.h>
#endif
#endif /* WIN32 && !WATT32 */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef NETWARE
#include <sys/filio.h>
#endif
#ifdef HAVE_XTI_H
#include <xti.h>
#endif

#ifndef TCP_NODELAY
#define TCP_NODELAY 1
#endif

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include "ares.h"
#include "ares_dns.h"
#include "ares_private.h"


static int try_again(int errnum);
static void write_tcp_data(ares_channel channel, fd_set *write_fds,
                           ares_socket_t write_fd, time_t now);
static void read_tcp_data(ares_channel channel, fd_set *read_fds,
                          ares_socket_t read_fd, time_t now);
static void read_udp_packets(ares_channel channel, fd_set *read_fds,
                             ares_socket_t read_fd, time_t now);
static void advance_tcp_send_queue(ares_channel channel, int whichserver,
                                   ssize_t num_bytes);
static void process_timeouts(ares_channel channel, time_t now);
static void process_broken_connections(ares_channel channel, time_t now);
static void process_answer(ares_channel channel, unsigned char *abuf,
                           int alen, int whichserver, int tcp, time_t now);
static void handle_error(ares_channel channel, int whichserver, time_t now);
static void skip_server(ares_channel channel, struct query *query,
                        int whichserver);
static void next_server(ares_channel channel, struct query *query, time_t now);
static int configure_socket(int s, ares_channel channel);
static int open_tcp_socket(ares_channel channel, struct server_state *server);
static int open_udp_socket(ares_channel channel, struct server_state *server);
static int same_questions(const unsigned char *qbuf, int qlen,
                          const unsigned char *abuf, int alen);
static void end_query(ares_channel channel, struct query *query, int status,
                      unsigned char *abuf, int alen);

/* Something interesting happened on the wire, or there was a timeout.
 * See what's up and respond accordingly.
 */
void ares_process(ares_channel channel, fd_set *read_fds, fd_set *write_fds)
{
  time_t now;

  time(&now);
  write_tcp_data(channel, write_fds, ARES_SOCKET_BAD, now);
  read_tcp_data(channel, read_fds, ARES_SOCKET_BAD, now);
  read_udp_packets(channel, read_fds, ARES_SOCKET_BAD, now);
  process_timeouts(channel, now);
  process_broken_connections(channel, now);
}

/* Something interesting happened on the wire, or there was a timeout.
 * See what's up and respond accordingly.
 */
void ares_process_fd(ares_channel channel,
                     ares_socket_t read_fd, /* use ARES_SOCKET_BAD or valid
                                               file descriptors */
                     ares_socket_t write_fd)
{
  time_t now;

  time(&now);
  write_tcp_data(channel, NULL, write_fd, now);
  read_tcp_data(channel, NULL, read_fd, now);
  read_udp_packets(channel, NULL, read_fd, now);
  process_timeouts(channel, now);
}


/* Return 1 if the specified error number describes a readiness error, or 0
 * otherwise. This is mostly for HP-UX, which could return EAGAIN or
 * EWOULDBLOCK. See this man page
 *
 *      http://devrsrc1.external.hp.com/STKS/cgi-bin/man2html?manpage=/usr/share/man/man2.Z/send.2
 */
static int try_again(int errnum)
{
#if !defined EWOULDBLOCK && !defined EAGAIN
#error "Neither EWOULDBLOCK nor EAGAIN defined"
#endif
  switch (errnum)
    {
#ifdef EWOULDBLOCK
    case EWOULDBLOCK:
      return 1;
#endif
#if defined EAGAIN && EAGAIN != EWOULDBLOCK
    case EAGAIN:
      return 1;
#endif
    }
  return 0;
}

/* If any TCP sockets select true for writing, write out queued data
 * we have for them.
 */
static void write_tcp_data(ares_channel channel,
                           fd_set *write_fds,
                           ares_socket_t write_fd,
                           time_t now)
{
  struct server_state *server;
  struct send_request *sendreq;
  struct iovec *vec;
  int i;
  ssize_t scount;
  ssize_t wcount;
  size_t n;

  if(!write_fds && (write_fd == ARES_SOCKET_BAD))
    /* no possible action */
    return;

  for (i = 0; i < channel->nservers; i++)
    {
      /* Make sure server has data to send and is selected in write_fds or
         write_fd. */
      server = &channel->servers[i];
      if (!server->qhead || server->tcp_socket == ARES_SOCKET_BAD || server->is_broken)
        continue;

      if(write_fds) {
        if(!FD_ISSET(server->tcp_socket, write_fds))
          continue;
      }
      else {
        if(server->tcp_socket != write_fd)
          continue;
      }

      if(write_fds)
        /* If there's an error and we close this socket, then open
         * another with the same fd to talk to another server, then we
         * don't want to think that it was the new socket that was
         * ready. This is not disastrous, but is likely to result in
         * extra system calls and confusion. */
        FD_CLR(server->tcp_socket, write_fds);

      /* Count the number of send queue items. */
      n = 0;
      for (sendreq = server->qhead; sendreq; sendreq = sendreq->next)
        n++;

      /* Allocate iovecs so we can send all our data at once. */
      vec = malloc(n * sizeof(struct iovec));
      if (vec)
        {
          /* Fill in the iovecs and send. */
          n = 0;
          for (sendreq = server->qhead; sendreq; sendreq = sendreq->next)
            {
              vec[n].iov_base = (char *) sendreq->data;
              vec[n].iov_len = sendreq->len;
              n++;
            }
          wcount = (ssize_t)writev(server->tcp_socket, vec, (int)n);
          free(vec);
          if (wcount < 0)
            {
              if (!try_again(SOCKERRNO))
                  handle_error(channel, i, now);
              continue;
            }

          /* Advance the send queue by as many bytes as we sent. */
          advance_tcp_send_queue(channel, i, wcount);
        }
      else
        {
          /* Can't allocate iovecs; just send the first request. */
          sendreq = server->qhead;

          scount = swrite(server->tcp_socket, sendreq->data, sendreq->len);
          if (scount < 0)
            {
              if (!try_again(SOCKERRNO))
                  handle_error(channel, i, now);
              continue;
            }

          /* Advance the send queue by as many bytes as we sent. */
          advance_tcp_send_queue(channel, i, scount);
        }
    }
}

/* Consume the given number of bytes from the head of the TCP send queue. */
static void advance_tcp_send_queue(ares_channel channel, int whichserver,
                                   ssize_t num_bytes)
{
  struct send_request *sendreq;
  struct server_state *server = &channel->servers[whichserver];
  while (num_bytes > 0)
    {
      sendreq = server->qhead;
      if ((size_t)num_bytes >= sendreq->len)
       {
         num_bytes -= sendreq->len;
         server->qhead = sendreq->next;
         if (server->qhead == NULL)
           {
             SOCK_STATE_CALLBACK(channel, server->tcp_socket, 1, 0);
             server->qtail = NULL;
           }
         if (sendreq->data_storage != NULL)
           free(sendreq->data_storage);
         free(sendreq);
       }
      else
       {
         sendreq->data += num_bytes;
         sendreq->len -= num_bytes;
         num_bytes = 0;
       }
    }
}

/* If any TCP socket selects true for reading, read some data,
 * allocate a buffer if we finish reading the length word, and process
 * a packet if we finish reading one.
 */
static void read_tcp_data(ares_channel channel, fd_set *read_fds,
                          ares_socket_t read_fd, time_t now)
{
  struct server_state *server;
  int i;
  ssize_t count;

  if(!read_fds && (read_fd == ARES_SOCKET_BAD))
    /* no possible action */
    return;

  for (i = 0; i < channel->nservers; i++)
    {
      /* Make sure the server has a socket and is selected in read_fds. */
      server = &channel->servers[i];
      if (server->tcp_socket == ARES_SOCKET_BAD || server->is_broken)
        continue;

      if(read_fds) {
        if(!FD_ISSET(server->tcp_socket, read_fds))
          continue;
      }
      else {
        if(server->tcp_socket != read_fd)
          continue;
      }

      if(read_fds)
        /* If there's an error and we close this socket, then open
         * another with the same fd to talk to another server, then we
         * don't want to think that it was the new socket that was
         * ready. This is not disastrous, but is likely to result in
         * extra system calls and confusion. */
        FD_CLR(server->tcp_socket, read_fds);

      if (server->tcp_lenbuf_pos != 2)
        {
          /* We haven't yet read a length word, so read that (or
           * what's left to read of it).
           */
          count = sread(server->tcp_socket,
                        server->tcp_lenbuf + server->tcp_lenbuf_pos,
                        2 - server->tcp_lenbuf_pos);
          if (count <= 0)
            {
              if (!(count == -1 && try_again(SOCKERRNO)))
                  handle_error(channel, i, now);
              continue;
            }

          server->tcp_lenbuf_pos += (int)count;
          if (server->tcp_lenbuf_pos == 2)
            {
              /* We finished reading the length word.  Decode the
               * length and allocate a buffer for the data.
               */
              server->tcp_length = server->tcp_lenbuf[0] << 8
                | server->tcp_lenbuf[1];
              server->tcp_buffer = malloc(server->tcp_length);
              if (!server->tcp_buffer)
                handle_error(channel, i, now);
              server->tcp_buffer_pos = 0;
            }
        }
      else
        {
          /* Read data into the allocated buffer. */
          count = sread(server->tcp_socket,
                        server->tcp_buffer + server->tcp_buffer_pos,
                        server->tcp_length - server->tcp_buffer_pos);
          if (count <= 0)
            {
              if (!(count == -1 && try_again(SOCKERRNO)))
                  handle_error(channel, i, now);
              continue;
            }

          server->tcp_buffer_pos += (int)count;
          if (server->tcp_buffer_pos == server->tcp_length)
            {
              /* We finished reading this answer; process it and
               * prepare to read another length word.
               */
              process_answer(channel, server->tcp_buffer, server->tcp_length,
                             i, 1, now);
          if (server->tcp_buffer)
                        free(server->tcp_buffer);
              server->tcp_buffer = NULL;
              server->tcp_lenbuf_pos = 0;
              server->tcp_buffer_pos = 0;
            }
        }
    }
}

/* If any UDP sockets select true for reading, process them. */
static void read_udp_packets(ares_channel channel, fd_set *read_fds,
                             ares_socket_t read_fd, time_t now)
{
  struct server_state *server;
  int i;
  ssize_t count;
  unsigned char buf[PACKETSZ + 1];

  if(!read_fds && (read_fd == ARES_SOCKET_BAD))
    /* no possible action */
    return;

  for (i = 0; i < channel->nservers; i++)
    {
      /* Make sure the server has a socket and is selected in read_fds. */
      server = &channel->servers[i];

      if (server->udp_socket == ARES_SOCKET_BAD || server->is_broken)
        continue;

      if(read_fds) {
        if(!FD_ISSET(server->udp_socket, read_fds))
          continue;
      }
      else {
        if(server->udp_socket != read_fd)
          continue;
      }

      if(read_fds)
        /* If there's an error and we close this socket, then open
         * another with the same fd to talk to another server, then we
         * don't want to think that it was the new socket that was
         * ready. This is not disastrous, but is likely to result in
         * extra system calls and confusion. */
        FD_CLR(server->udp_socket, read_fds);

      /* To reduce event loop overhead, read and process as many
       * packets as we can. */
      do {
        count = sread(server->udp_socket, buf, sizeof(buf));
        if (count == -1 && try_again(SOCKERRNO))
          continue;
        else if (count <= 0)
          handle_error(channel, i, now);
        else
          process_answer(channel, buf, (int)count, i, 0, now);
       } while (count > 0);
    }
}

/* If any queries have timed out, note the timeout and move them on. */
static void process_timeouts(ares_channel channel, time_t now)
{
  time_t t;  /* the time of the timeouts we're processing */
  struct query *query;
  struct list_node* list_head;
  struct list_node* list_node;

  /* Process all the timeouts that have fired since the last time we
   * processed timeouts. If things are going well, then we'll have
   * hundreds/thousands of queries that fall into future buckets, and
   * only a handful of requests that fall into the "now" bucket, so
   * this should be quite quick.
   */
  for (t = channel->last_timeout_processed; t <= now; t++)
    {
      list_head = &(channel->queries_by_timeout[t % ARES_TIMEOUT_TABLE_SIZE]);
      for (list_node = list_head->next; list_node != list_head; )
        {
          query = list_node->data;
          list_node = list_node->next;  /* in case the query gets deleted */
          if (query->timeout != 0 && now >= query->timeout)
            {
              query->error_status = ARES_ETIMEOUT;
              ++query->timeouts;
              next_server(channel, query, now);
            }
        }
     }
  channel->last_timeout_processed = now;
}

/* Handle an answer from a server. */
static void process_answer(ares_channel channel, unsigned char *abuf,
                           int alen, int whichserver, int tcp, time_t now)
{
  int tc, rcode;
  unsigned short id;
  struct query *query;
  struct list_node* list_head;
  struct list_node* list_node;

  /* If there's no room in the answer for a header, we can't do much
   * with it. */
  if (alen < HFIXEDSZ)
    return;

  /* Grab the query ID, truncate bit, and response code from the packet. */
  id = DNS_HEADER_QID(abuf);
  tc = DNS_HEADER_TC(abuf);
  rcode = DNS_HEADER_RCODE(abuf);

  /* Find the query corresponding to this packet. The queries are
   * hashed/bucketed by query id, so this lookup should be quick.
   * Note that both the query id and the questions must be the same;
   * when the query id wraps around we can have multiple outstanding
   * queries with the same query id, so we need to check both the id and
   * question.
   */
  query = NULL;
  list_head = &(channel->queries_by_qid[id % ARES_QID_TABLE_SIZE]);
  for (list_node = list_head->next; list_node != list_head;
       list_node = list_node->next)
    {
      struct query *q = list_node->data;
      if ((q->qid == id) && same_questions(q->qbuf, q->qlen, abuf, alen))
        {
          query = q;
          break;
        }
    }
  if (!query)
    return;

  /* If we got a truncated UDP packet and are not ignoring truncation,
   * don't accept the packet, and switch the query to TCP if we hadn't
   * done so already.
   */
  if ((tc || alen > PACKETSZ) && !tcp && !(channel->flags & ARES_FLAG_IGNTC))
    {
      if (!query->using_tcp)
        {
          query->using_tcp = 1;
          ares__send_query(channel, query, now);
        }
      return;
    }

  /* Limit alen to PACKETSZ if we aren't using TCP (only relevant if we
   * are ignoring truncation.
   */
  if (alen > PACKETSZ && !tcp)
    alen = PACKETSZ;

  /* If we aren't passing through all error packets, discard packets
   * with SERVFAIL, NOTIMP, or REFUSED response codes.
   */
  if (!(channel->flags & ARES_FLAG_NOCHECKRESP))
    {
      if (rcode == SERVFAIL || rcode == NOTIMP || rcode == REFUSED)
        {
          skip_server(channel, query, whichserver);
          if (query->server == whichserver)
            next_server(channel, query, now);
          return;
        }
    }

  end_query(channel, query, ARES_SUCCESS, abuf, alen);
}

/* Close all the connections that are no longer usable. */
static void process_broken_connections(ares_channel channel, time_t now)
{
  int i;
  for (i = 0; i < channel->nservers; i++)
    {
      struct server_state *server = &channel->servers[i];
      if (server->is_broken)
        {
          handle_error(channel, i, now);
        }
    }
}

static void handle_error(ares_channel channel, int whichserver, time_t now)
{
  struct server_state *server;
  struct query *query;
  struct list_node list_head;
  struct list_node* list_node;

  server = &channel->servers[whichserver];

  /* Reset communications with this server. */
  ares__close_sockets(channel, server);

  /* Tell all queries talking to this server to move on and not try
   * this server again. We steal the current list of queries that were
   * in-flight to this server, since when we call next_server this can
   * cause the queries to be re-sent to this server, which will
   * re-insert these queries in that same server->queries_to_server
   * list.
   */
  ares__init_list_head(&list_head);
  ares__swap_lists(&list_head, &(server->queries_to_server));
  for (list_node = list_head.next; list_node != &list_head; )
    {
      query = list_node->data;
      list_node = list_node->next;  /* in case the query gets deleted */
      assert(query->server == whichserver);
      skip_server(channel, query, whichserver);
      next_server(channel, query, now);
    }
  /* Each query should have removed itself from our temporary list as
   * it re-sent itself or finished up...
   */
  assert(ares__is_list_empty(&list_head));
}

static void skip_server(ares_channel channel, struct query *query,
                        int whichserver) {
  /* The given server gave us problems with this query, so if we have
   * the luxury of using other servers, then let's skip the
   * potentially broken server and just use the others. If we only
   * have one server and we need to retry then we should just go ahead
   * and re-use that server, since it's our only hope; perhaps we
   * just got unlucky, and retrying will work (eg, the server timed
   * out our TCP connection just as we were sending another request).
   */
  if (channel->nservers > 1)
    {
      query->server_info[whichserver].skip_server = 1;
    }
}

static void next_server(ares_channel channel, struct query *query, time_t now)
{
  /* Advance to the next server or try. */
  query->server++;
  for (; query->try < channel->tries; query->try++)
    {
      for (; query->server < channel->nservers; query->server++)
        {
          struct server_state *server = &channel->servers[query->server];
          /* We don't want to use this server if (1) we decided this
           * connection is broken, and thus about to be closed, (2)
           * we've decided to skip this server because of earlier
           * errors we encountered, or (3) we already sent this query
           * over this exact connection.
           */
          if (!server->is_broken &&
               !query->server_info[query->server].skip_server &&
               !(query->using_tcp &&
                 (query->server_info[query->server].tcp_connection_generation ==
                  server->tcp_connection_generation)))
            {
               ares__send_query(channel, query, now);
               return;
            }
        }
      query->server = 0;

      /* You might think that with TCP we only need one try. However,
       * even when using TCP, servers can time-out our connection just
       * as we're sending a request, or close our connection because
       * they die, or never send us a reply because they get wedged or
       * tickle a bug that drops our request.
       */
    }
  end_query(channel, query, query->error_status, NULL, 0);
}

void ares__send_query(ares_channel channel, struct query *query, time_t now)
{
  struct send_request *sendreq;
  struct server_state *server;

  server = &channel->servers[query->server];
  if (query->using_tcp)
    {
      /* Make sure the TCP socket for this server is set up and queue
       * a send request.
       */
      if (server->tcp_socket == ARES_SOCKET_BAD)
        {
          if (open_tcp_socket(channel, server) == -1)
            {
              skip_server(channel, query, query->server);
              next_server(channel, query, now);
              return;
            }
        }
      sendreq = calloc(sizeof(struct send_request), 1);
      if (!sendreq)
        {
        end_query(channel, query, ARES_ENOMEM, NULL, 0);
          return;
        }
      /* To make the common case fast, we avoid copies by using the
       * query's tcpbuf for as long as the query is alive. In the rare
       * case where the query ends while it's queued for transmission,
       * then we give the sendreq its own copy of the request packet
       * and put it in sendreq->data_storage.
       */
      sendreq->data_storage = NULL;
      sendreq->data = query->tcpbuf;
      sendreq->len = query->tcplen;
      sendreq->owner_query = query;
      sendreq->next = NULL;
      if (server->qtail)
        server->qtail->next = sendreq;
      else
        {
          SOCK_STATE_CALLBACK(channel, server->tcp_socket, 1, 1);
          server->qhead = sendreq;
        }
      server->qtail = sendreq;
      query->server_info[query->server].tcp_connection_generation =
        server->tcp_connection_generation;
    }
  else
    {
      if (server->udp_socket == ARES_SOCKET_BAD)
        {
          if (open_udp_socket(channel, server) == -1)
            {
              skip_server(channel, query, query->server);
              next_server(channel, query, now);
              return;
            }
        }
      if (swrite(server->udp_socket, query->qbuf, query->qlen) == -1)
        {
          /* FIXME: Handle EAGAIN here since it likely can happen. */
          skip_server(channel, query, query->server);
          next_server(channel, query, now);
          return;
        }
    }
    query->timeout = now
        + ((query->try == 0) ? channel->timeout
           : channel->timeout << query->try / channel->nservers);
    /* Keep track of queries bucketed by timeout, so we can process
     * timeout events quickly.
     */
    ares__remove_from_list(&(query->queries_by_timeout));
    ares__insert_in_list(
        &(query->queries_by_timeout),
        &(channel->queries_by_timeout[query->timeout %
                                      ARES_TIMEOUT_TABLE_SIZE]));

    /* Keep track of queries bucketed by server, so we can process server
     * errors quickly.
     */
    ares__remove_from_list(&(query->queries_to_server));
    ares__insert_in_list(&(query->queries_to_server),
                         &(server->queries_to_server));
}

/*
 * setsocknonblock sets the given socket to either blocking or non-blocking mode
 * based on the 'nonblock' boolean argument. This function is highly portable.
 */
static int setsocknonblock(ares_socket_t sockfd,    /* operate on this */
                    int nonblock   /* TRUE or FALSE */)
{
#undef SETBLOCK
#define SETBLOCK 0
#ifdef HAVE_O_NONBLOCK
  /* most recent unix versions */
  int flags;

  flags = fcntl(sockfd, F_GETFL, 0);
  if (FALSE != nonblock)
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
  else
    return fcntl(sockfd, F_SETFL, flags & (~O_NONBLOCK));
#undef SETBLOCK
#define SETBLOCK 1
#endif

#if defined(HAVE_FIONBIO) && (SETBLOCK == 0)
  /* older unix versions */
  int flags;

  flags = nonblock;
  return ioctl(sockfd, FIONBIO, &flags);
#undef SETBLOCK
#define SETBLOCK 2
#endif

#if defined(HAVE_IOCTLSOCKET) && (SETBLOCK == 0)
#ifdef WATT32
  char flags;
#else
  /* Windows? */
  unsigned long flags;
#endif
  flags = nonblock;

  return ioctlsocket(sockfd, FIONBIO, &flags);
#undef SETBLOCK
#define SETBLOCK 3
#endif

#if defined(HAVE_IOCTLSOCKET_CASE) && (SETBLOCK == 0)
  /* presumably for Amiga */
  return IoctlSocket(sockfd, FIONBIO, (long)nonblock);
#undef SETBLOCK
#define SETBLOCK 4
#endif

#if defined(HAVE_SO_NONBLOCK) && (SETBLOCK == 0)
  /* BeOS */
  long b = nonblock ? 1 : 0;
  return setsockopt(sockfd, SOL_SOCKET, SO_NONBLOCK, &b, sizeof(b));
#undef SETBLOCK
#define SETBLOCK 5
#endif

#ifdef HAVE_DISABLED_NONBLOCKING
  return 0; /* returns success */
#undef SETBLOCK
#define SETBLOCK 6
#endif

#if (SETBLOCK == 0)
#error "no non-blocking method was found/used/set"
#endif
}

static int configure_socket(int s, ares_channel channel)
{
  setsocknonblock(s, TRUE);

#if defined(FD_CLOEXEC) && !defined(MSDOS)
  /* Configure the socket fd as close-on-exec. */
  if (fcntl(s, F_SETFD, FD_CLOEXEC) == -1)
    return -1;
#endif

  /* Set the socket's send and receive buffer sizes. */
  if ((channel->socket_send_buffer_size > 0) &&
      setsockopt(s, SOL_SOCKET, SO_SNDBUF,
                 (void *)&channel->socket_send_buffer_size,
                 sizeof(channel->socket_send_buffer_size)) == -1)
    return -1;

  if ((channel->socket_receive_buffer_size > 0) &&
      setsockopt(s, SOL_SOCKET, SO_RCVBUF,
                 (void *)&channel->socket_receive_buffer_size,
                 sizeof(channel->socket_receive_buffer_size)) == -1)
    return -1;

  return 0;
 }

static int open_tcp_socket(ares_channel channel, struct server_state *server)
{
  ares_socket_t s;
  int opt;
  struct sockaddr_in sockin;

  /* Acquire a socket. */
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s == ARES_SOCKET_BAD)
    return -1;

  /* Configure it. */
  if (configure_socket(s, channel) < 0)
    {
       close(s);
       return -1;
    }

  /*
   * Disable the Nagle algorithm (only relevant for TCP sockets, and thus not in
   * configure_socket). In general, in DNS lookups we're pretty much interested
   * in firing off a single request and then waiting for a reply, so batching
   * isn't very interesting in general.
   */
  opt = 1;
  if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY,
                 (void *)&opt, sizeof(opt)) == -1)
    {
       close(s);
       return -1;
    }

  /* Connect to the server. */
  memset(&sockin, 0, sizeof(sockin));
  sockin.sin_family = AF_INET;
  sockin.sin_addr = server->addr;
  sockin.sin_port = (unsigned short)(channel->tcp_port & 0xffff);
  if (connect(s, (struct sockaddr *) &sockin, sizeof(sockin)) == -1) {
    int err = SOCKERRNO;

    if (err != EINPROGRESS && err != EWOULDBLOCK) {
      closesocket(s);
      return -1;
    }
  }

  SOCK_STATE_CALLBACK(channel, s, 1, 0);
  server->tcp_buffer_pos = 0;
  server->tcp_socket = s;
  server->tcp_connection_generation = ++channel->tcp_connection_generation;
  return 0;
}

static int open_udp_socket(ares_channel channel, struct server_state *server)
{
  ares_socket_t s;
  struct sockaddr_in sockin;

  /* Acquire a socket. */
  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s == ARES_SOCKET_BAD)
    return -1;

  /* Set the socket non-blocking. */
  if (configure_socket(s, channel) < 0)
    {
       close(s);
       return -1;
    }

  /* Connect to the server. */
  memset(&sockin, 0, sizeof(sockin));
  sockin.sin_family = AF_INET;
  sockin.sin_addr = server->addr;
  sockin.sin_port = (unsigned short)(channel->udp_port & 0xffff);
  if (connect(s, (struct sockaddr *) &sockin, sizeof(sockin)) == -1)
    {
      closesocket(s);
      return -1;
    }

  SOCK_STATE_CALLBACK(channel, s, 1, 0);

  server->udp_socket = s;
  return 0;
}

static int same_questions(const unsigned char *qbuf, int qlen,
                          const unsigned char *abuf, int alen)
{
  struct {
    const unsigned char *p;
    int qdcount;
    char *name;
    long namelen;
    int type;
    int dnsclass;
  } q, a;
  int i, j;

  if (qlen < HFIXEDSZ || alen < HFIXEDSZ)
    return 0;

  /* Extract qdcount from the request and reply buffers and compare them. */
  q.qdcount = DNS_HEADER_QDCOUNT(qbuf);
  a.qdcount = DNS_HEADER_QDCOUNT(abuf);
  if (q.qdcount != a.qdcount)
    return 0;

  /* For each question in qbuf, find it in abuf. */
  q.p = qbuf + HFIXEDSZ;
  for (i = 0; i < q.qdcount; i++)
    {
      /* Decode the question in the query. */
      if (ares_expand_name(q.p, qbuf, qlen, &q.name, &q.namelen)
          != ARES_SUCCESS)
        return 0;
      q.p += q.namelen;
      if (q.p + QFIXEDSZ > qbuf + qlen)
        {
          free(q.name);
          return 0;
        }
      q.type = DNS_QUESTION_TYPE(q.p);
      q.dnsclass = DNS_QUESTION_CLASS(q.p);
      q.p += QFIXEDSZ;

      /* Search for this question in the answer. */
      a.p = abuf + HFIXEDSZ;
      for (j = 0; j < a.qdcount; j++)
        {
          /* Decode the question in the answer. */
          if (ares_expand_name(a.p, abuf, alen, &a.name, &a.namelen)
              != ARES_SUCCESS)
            {
              free(q.name);
              return 0;
            }
          a.p += a.namelen;
          if (a.p + QFIXEDSZ > abuf + alen)
            {
              free(q.name);
              free(a.name);
              return 0;
            }
          a.type = DNS_QUESTION_TYPE(a.p);
          a.dnsclass = DNS_QUESTION_CLASS(a.p);
          a.p += QFIXEDSZ;

          /* Compare the decoded questions. */
          if (strcasecmp(q.name, a.name) == 0 && q.type == a.type
              && q.dnsclass == a.dnsclass)
            {
              free(a.name);
              break;
            }
          free(a.name);
        }

      free(q.name);
      if (j == a.qdcount)
        return 0;
    }
  return 1;
}

static void end_query (ares_channel channel, struct query *query, int status,
                       unsigned char *abuf, int alen)
{
  int i;

  /* First we check to see if this query ended while one of our send
   * queues still has pointers to it.
   */
  for (i = 0; i < channel->nservers; i++)
    {
      struct server_state *server = &channel->servers[i];
      struct send_request *sendreq;
      for (sendreq = server->qhead; sendreq; sendreq = sendreq->next)
        if (sendreq->owner_query == query)
          {
            sendreq->owner_query = NULL;
            assert(sendreq->data_storage == NULL);
            if (status == ARES_SUCCESS)
              {
                /* We got a reply for this query, but this queued
                 * sendreq points into this soon-to-be-gone query's
                 * tcpbuf. Probably this means we timed out and queued
                 * the query for retransmission, then received a
                 * response before actually retransmitting. This is
                 * perfectly fine, so we want to keep the connection
                 * running smoothly if we can. But in the worst case
                 * we may have sent only some prefix of the query,
                 * with some suffix of the query left to send. Also,
                 * the buffer may be queued on multiple queues. To
                 * prevent dangling pointers to the query's tcpbuf and
                 * handle these cases, we just give such sendreqs
                 * their own copy of the query packet.
                 */
               sendreq->data_storage = malloc(sendreq->len);
               if (sendreq->data_storage != NULL)
                 {
                   memcpy(sendreq->data_storage, sendreq->data, sendreq->len);
                   sendreq->data = sendreq->data_storage;
                 }
              }
            if ((status != ARES_SUCCESS) || (sendreq->data_storage == NULL))
              {
                /* We encountered an error (probably a timeout,
                 * suggesting the DNS server we're talking to is
                 * probably unreachable, wedged, or severely
                 * overloaded) or we couldn't copy the request, so
                 * mark the connection as broken. When we get to
                 * process_broken_connections() we'll close the
                 * connection and try to re-send requests to another
                 * server.
                 */
               server->is_broken = 1;
               /* Just to be paranoid, zero out this sendreq... */
               sendreq->data = NULL;
               sendreq->len = 0;
             }
          }
    }

  /* Invoke the callback */
  query->callback(query->arg, status, query->timeouts, abuf, alen);
  ares__free_query(query);

  /* Simple cleanup policy: if no queries are remaining, close all
   * network sockets unless STAYOPEN is set.
   */
  if (!(channel->flags & ARES_FLAG_STAYOPEN) &&
      ares__is_list_empty(&(channel->all_queries)))
    {
      for (i = 0; i < channel->nservers; i++)
        ares__close_sockets(channel, &channel->servers[i]);
    }
}

void ares__free_query(struct query *query)
{
  /* Remove the query from all the lists in which it is linked */
  ares__remove_from_list(&(query->queries_by_qid));
  ares__remove_from_list(&(query->queries_by_timeout));
  ares__remove_from_list(&(query->queries_to_server));
  ares__remove_from_list(&(query->all_queries));
  /* Zero out some important stuff, to help catch bugs */
  query->callback = NULL;
  query->arg = NULL;
  /* Deallocate the memory associated with the query */
  free(query->tcpbuf);
  free(query->server_info);
  free(query);
}
