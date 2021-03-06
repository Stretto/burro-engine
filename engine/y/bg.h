/*-----------------------------------------------------------------------------

	background.h -- definitions for DS backgrounds
	Copyright (C) 2013,2014
		Michael L. Gran (spk121)

	GPL3+
-----------------------------------------------------------------------------*/
/** @file bg.h
    @brief background defines and functionality

    These are the public as well as the internal API to control backgrounds.
    There are four backgrounds for both the main and sub screens.  BG0 is 
    the one most in the foreground and BG3 is the one most in the background.

    Each background can be one of 3 different forms:
    - a 32-bit ARGB bitmap
    - an map that points to tiles in an ARGB32 bitmap
*/

#ifndef BURRO_BG_H
#define BURRO_BG_H

#include <stdbool.h>
#include <stdint.h>
#include "../x.h"
#include "matrix.h"
#include "sheet.h"
#include "vram.h"

/** The number of background layers for the main screen */
#define BG_MAIN_BACKGROUNDS_COUNT 4
/** The number of background layers for the sub screen */
#define BG_SUB_BACKGROUNDS_COUNT 4

/** Enumeration of the 8 background layer IDs */
typedef enum {
    BG_MAIN_0 = 0,
    BG_MAIN_1 = 1,
    BG_MAIN_2 = 2,
    BG_MAIN_3 = 3,
    BG_SUB_0 = 4,
    BG_SUB_1 = 5,
    BG_SUB_2 = 6,
    BG_SUB_3 = 7
} bg_index_t;


/** Allowed background types, used in bg_init */
typedef enum {
    BG_TYPE_NONE,
    BG_TYPE_MAP, 
    BG_TYPE_BMP,
} bg_type_t;

bool bg_validate_int_as_bg_index_t (int x);
bool bg_validate_int_as_bg_type_t (int x);
const char *bg_get_index_name (bg_index_t index);

/** Initializes the BG subsystem.
 */
void bg_init (void);

/**  Initializes a memory store for a background layers.
    @param id
        background layer to init. Must be BG_MAIN_0 , 1, 2, 3 or BG_SUB_0, 1, 2, 3
    @param size
        size of the BG, MATRIX_16x16, etc
    @param bank
        storage location for this background: VRAM_A, VRAM_B, etc
*/
void bg_assign_memory (bg_index_t id, matrix_size_t siz, vram_bank_t bank);

/**  Returns the pointer to the map or bmp data
 */
const uint32_t *bg_get_data_ptr (bg_index_t id);

/**  Gets the priority of the background layer.
    @param id
        background layer id. e.g. BG_MAIN_0
    \return
        priority 0, 1, 2, or 3
*/
int bg_get_priority (bg_index_t id);

/**  Hides the current background
    @param id
        background layer ID. e.g. BG_MAIN_0
*/ 
void bg_hide (bg_index_t id);

/**  Performs a cumulative rotation of the background by the specified angle. 
    @param id
        background layer ID
    @param angle
        the angle of counter clockwise rotation in degrees
*/ 
void bg_rotate (bg_index_t id, double angle);

/**  Scrolls the background by the specified relative values.
         Specifically, it is a relative move of the rotational center of the
         background.
    @param id
        background layer ID, e.g. BG_MAIN_0
    @param dx
        the horizontal scroll
    @param dy
        vertical scroll
    \note
        Be mindful of the ordering of the rotation, expansion, and scroll operations
*/ 
void bg_scroll (bg_index_t id, double dx, double dy);

/**  Sets the rotation and scale of the background in one operation.
    @param id
        background layer ID, e.g. BG_MAIN_0
    @param rotation
        the angle of counter clockwise rotation about the rotational center in degrees
    @param expansion
        the expansion about the rotational center of the background, where 1.0 is
        no expansion
    @param scroll_x
        the offset of the rotational center of the background from the screen origin
    @param scroll_y
        the offset of the rotational center of the background from the screen origin
    @param rotation_center_x
        the pixel location of the (unrotated, unexpanded) background that will be its
        rotational center
    @param rotation_center_y
        the pixel location of the (unrotated, unexpanded) background that will be its
        rotational center
*/ 
void bg_set (bg_index_t id, double rotation, double expansion, double scroll_x, double scroll_y,
	     double rotation_center_x, double rotation_center_y);

/**  Sets the center of rotation of a background
    @param id
        background layer ID, e.g. BG_MAIN_0
    @param rotation_center_x
        the pixel location of the (unrotated, unexpanded) background that will be its
        rotational center
    @param rotation_center_y
        the pixel location of the (unrotated, unexpanded) background that will be its
        rotational center
*/
void bg_set_rotation_center (bg_index_t id, double rotation_center_x, double rotation_center_y);

/**  Sets the background priority
    @param id
        background layer id returned from bgInit or bgInitSub
    @param priority
        background priority (0-3) where 0 is most in the background and 3 is most 
        in the foreground
*/ 
void bg_set_priority (bg_index_t id, int priority);

/**  Performs sets the rotation of the background to specified angle. 
        This rotation is about its rotational center.
    @param id
        background layer ID
    @param rotation
        the angle of counter clockwise rotation in degrees
*/ 
void bg_set_rotation (bg_index_t id, double rotation);

/**  Performs sets the rotation and expansion of a background.
        This rotation and expansion is about its rotational center.
    @param id
        background layer ID
    @param rotation
        the angle of counter clockwise rotation in degrees
    @param expansion
        the expansion ratio, where 1.0 is 1:1 expansion
*/ 
void bg_set_rotation_expansion (bg_index_t id, double rotation, double expansion);

/**  Performs sets the expansion of a background.
        This expansion is about its rotational center.
    @param id
        background layer ID
    @param expansion
        the expansion ratio, where 1.0 is 1:1 expansion
*/ 
void bg_set_expansion (bg_index_t id, double expansion);

/**  Shows (makes visible) the specified background layer
    @param id
        background layer ID. e.g. BG_MAIN_0
*/ 
void bg_show (bg_index_t id);

bool bg_is_shown (bg_index_t id);

void bg_reset (bg_index_t id);






/**  Sets the map part of a BG_TYPE_MAP map-and-tile background from a resource.
        Each pixel in the map image will be interpreted as the index to an 8x8 pixel
        block in the tile image, where each block in the tile image is numbered
        row-wise sequentially.
    @param id
        background layer ID, e.g. BG_MAIN_0
    @param
        name of image resource in the GResource bundle
*/
void bg_set_map_from_resource (bg_index_t id, const char *resource);

/**  Sets the tilesheet of a BG_TYPE_MAP map-and-tile background from a resource.
        The tilesheet is interpreted as 8x8 pixel blocks, numbered sequentially rowwise.
    @param id
        background layer ID, e.g. BG_MAIN_0
    @param
        name of image resource in the GResource bundle
*/
void bg_set_tilesheet_from_resource (bg_index_t id, const char *resource);

/**  Sets the a BG_TYPE_BMP background from a resource.
    @param id
        background layer ID, e.g. BG_MAIN_0
    @param
        name of image resource in the GResource bundle
*/
void bg_set_bmp_from_file (bg_index_t id, const char *filename);
void bg_set_bmp_from_resource (bg_index_t id, const char *resource);

void bg_get_transform (bg_index_t id, double *scroll_x, double *scroll_y,
                       double *rotation_center_x, double *rotation_center_y,
                       double *rotation, double *expansion);
		       
cairo_surface_t *bg_get_cairo_surface (bg_index_t id);

void bg_init_guile_procedures (void);

#endif
