/* Output of mkerrcodes2.awk.  DO NOT EDIT.  */

/* errnos.h - List of system error values.
   Copyright (C) 2004 g10 Code GmbH
   This file is part of libgpg-error.

   libgpg-error is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   libgpg-error is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with libgpg-error; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

static const int err_code_from_index[] = {
  GPG_ERR_EPERM,
  GPG_ERR_ENOENT,
  GPG_ERR_ESRCH,
  GPG_ERR_EINTR,
  GPG_ERR_EIO,
  GPG_ERR_ENXIO,
  GPG_ERR_E2BIG,
  GPG_ERR_ENOEXEC,
  GPG_ERR_EBADF,
  GPG_ERR_ECHILD,
  GPG_ERR_EAGAIN,
  GPG_ERR_ENOMEM,
  GPG_ERR_EACCES,
  GPG_ERR_EFAULT,
  GPG_ERR_EBUSY,
  GPG_ERR_EEXIST,
  GPG_ERR_EXDEV,
  GPG_ERR_ENODEV,
  GPG_ERR_ENOTDIR,
  GPG_ERR_EISDIR,
  GPG_ERR_EINVAL,
  GPG_ERR_ENFILE,
  GPG_ERR_EMFILE,
  GPG_ERR_ENOTTY,
  GPG_ERR_EFBIG,
  GPG_ERR_ENOSPC,
  GPG_ERR_ESPIPE,
  GPG_ERR_EROFS,
  GPG_ERR_EMLINK,
  GPG_ERR_EPIPE,
  GPG_ERR_EDOM,
  GPG_ERR_ERANGE,
  GPG_ERR_EDEADLK,
  GPG_ERR_EDEADLOCK,
  GPG_ERR_ENAMETOOLONG,
  GPG_ERR_ENOLCK,
  GPG_ERR_ENOSYS,
  GPG_ERR_ENOTEMPTY,
  GPG_ERR_EILSEQ,
};

#define errno_to_idx(code) (0 ? -1 \
  : ((code >= 1) && (code <= 14)) ? (code - 1) \
  : ((code >= 16) && (code <= 25)) ? (code - 2) \
  : ((code >= 27) && (code <= 34)) ? (code - 3) \
  : ((code >= 36) && (code <= 36)) ? (code - 4) \
  : ((code >= 36) && (code <= 36)) ? (code - 3) \
  : ((code >= 38) && (code <= 42)) ? (code - 4) \
  : -1)
