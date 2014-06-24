/*
  libds - something with ds

  Copyright (C) 2011 Someone <someone@example.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _LIBDS_PRIVATE_H_
#define _LIBDS_PRIVATE_H_

#include <stdbool.h>
#include <syslog.h>
#include <cairo.h>

#include "libds.h"

static inline void __attribute__((always_inline, format(printf, 2, 3)))
ds_log_null(struct ds_ctx *ctx, const char *format, ...) {}

#define ds_log_cond(ctx, prio, arg...) \
  do { \
    if (ds_get_log_priority(ctx) >= prio) \
      ds_log(ctx, prio, __FILE__, __LINE__, __FUNCTION__, ## arg); \
  } while (0)

#ifdef ENABLE_LOGGING
#  ifdef ENABLE_DEBUG
#    define dbg(ctx, arg...) ds_log_cond(ctx, LOG_DEBUG, ## arg)
#  else
#    define dbg(ctx, arg...) ds_log_null(ctx, ## arg)
#  endif
#  define info(ctx, arg...) ds_log_cond(ctx, LOG_INFO, ## arg)
#  define err(ctx, arg...) ds_log_cond(ctx, LOG_ERR, ## arg)
#else
#  define dbg(ctx, arg...) ds_log_null(ctx, ## arg)
#  define info(ctx, arg...) ds_log_null(ctx, ## arg)
#  define err(ctx, arg...) ds_log_null(ctx, ## arg)
#endif

#ifndef HAVE_SECURE_GETENV
#  ifdef HAVE___SECURE_GETENV
#    define secure_getenv __secure_getenv
#  else
#    error neither secure_getenv nor __secure_getenv is available
#  endif
#endif

DS_LOCAL void
ds_log(struct ds_ctx *ctx,
       int priority, const char *file, int line, const char *fn,
       const char *format, ...)
  __attribute__((format(printf, 6, 7)));

#include "xcairo.h"
#include "bg.h"
#include "obj.h"
#include "list_bg.h"
#include "list_obj.h"
#endif
