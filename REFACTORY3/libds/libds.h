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

#ifndef LIBDS_H
#define LIBDS_H

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
# define BUILDING_DLL
#endif

#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_DLL
    #ifdef __GNUC__
      #define DS_EXPORT __attribute__ ((dllexport)
    #else
      #define DS_EXPORT __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define DS_EXPORT __attribute__ ((dllimport))
    #else
      #define DS_EXPORT __declspec(dllimport)
    #endif
  #endif
  #define DS_LOCAL
#else
  #if __GNUC__ >= 4
    #define DS_EXPORT __attribute__ ((visibility ("default")))
    #define DS_LOCAL __attribute__ ((visibility ("hidden")))
  #else
    #define DS_EXPORT
    #define DS_LOCAL
  #endif
#endif

  typedef enum ds_error_tag {
    DS_OK = 0,
    DS_ERROR_SURFACE_CREATION_FAILURE = -1,
    DS_ERROR_CONTEXT_CREATION_FAILURE = -2,
    DS_ERROR_PAINT_FAILURE = -3,
    DS_ERROR_SURFACE_INVALID = -4,
    DS_ERROR_SURFACE_FORMAT_INCORRECT = -5,
    DS_ERROR_STRIDE_INVALID = -6,
    DS_ERROR_OUT_OF_MEMORY = -7,
    DS_ERROR_CONTEXT_INVALID = -8,
    DS_ERROR_ANTIALIAS_OUT_OF_RANGE = -9,
    DS_ERROR_ANTIALIAS_FAILURE = -10,
    DS_ERROR_OUT_OF_RANGE = -11,
    DS_ERROR_NOT_FOUND = -12,
  } ds_error_t;

  /*
   * ds_ctx
   *
   * library user context - reads the config and system
   * environment, user variables, allows custom logging
   */
  typedef struct ds_ctx ds_ctx_t;
  struct ds_ctx;


DS_EXPORT ds_ctx_t *ds_ref(ds_ctx_t *ctx);
DS_EXPORT ds_ctx_t *ds_unref(ds_ctx_t *ctx);
DS_EXPORT ds_error_t ds_new(ds_ctx_t **ctx, int width, int height);
DS_EXPORT void ds_set_log_fn(ds_ctx_t *ctx,
                  void (*log_fn)(ds_ctx_t *ctx,
                                 int priority, const char *file, int line, const char *fn,
                                 const char *format, va_list args));
DS_EXPORT int ds_get_log_priority(ds_ctx_t *ctx);
DS_EXPORT void ds_set_log_priority(ds_ctx_t *ctx, int priority);
DS_EXPORT void *ds_get_userdata(ds_ctx_t *ctx);
DS_EXPORT void ds_set_userdata(ds_ctx_t *ctx, void *userdata);

DS_EXPORT void ds_set_backdrop(ds_ctx_t *ctx, uint32_t color);
DS_EXPORT uint32_t ds_get_backdrop(ds_ctx_t *ctx);
DS_EXPORT ds_error_t ds_render(ds_ctx_t *ctx, uint32_t **data, int *width, int *height,
	    int *stride);

/*
 * ds_list
 *
 * access to ds generated lists
 */
struct ds_list_entry;
struct ds_list_entry *ds_list_entry_get_next(struct ds_list_entry *list_entry);
const char *ds_list_entry_get_name(struct ds_list_entry *list_entry);
const char *ds_list_entry_get_value(struct ds_list_entry *list_entry);
#define ds_list_entry_foreach(list_entry, first_entry) \
        for (list_entry = first_entry; \
             list_entry != NULL; \
             list_entry = ds_list_entry_get_next(list_entry))

/*
 * ds_thing
 *
 * access to things of ds
 */
struct ds_thing;
struct ds_thing *ds_thing_ref(struct ds_thing *thing);
struct ds_thing *ds_thing_unref(struct ds_thing *thing);
ds_ctx_t *ds_thing_get_ctx(struct ds_thing *thing);
int ds_thing_new_from_string(ds_ctx_t *ctx, const char *string, struct ds_thing **thing);
struct ds_list_entry *ds_thing_get_some_list_entry(struct ds_thing *thing);

#define DS_PACK_RGB24(r,g,b)			\
  ((((uint32_t)(0xff)) << 24)			\
   | (((uint32_t)(r)) << 16)			\
   | (((uint32_t)(g)) << 8)			\
   | ((uint32_t)(b)))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
