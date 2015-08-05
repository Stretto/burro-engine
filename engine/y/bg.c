#include <stdint.h>
#include <stdbool.h>
#include "../x.h"
#include "bg.h"
#include "eng.h"
#include "tga.h"
#include "tilesheet.h"
#include "vram.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"

struct bg_matrix
{
    bg_size_t size;
    vram_bank_t bank;
    uint32_t *storage;
    uint32_t **data;
};

int bg_matrix_width[9] = {
    [BG_SIZE_16x16] = 16,
    [BG_SIZE_32x16] = 32,
    [BG_SIZE_16x32] = 16,
    [BG_SIZE_32x32] = 32,
    [BG_SIZE_128x128] = 128,
    [BG_SIZE_256x256] = 256,
    [BG_SIZE_512x256] = 512,
    [BG_SIZE_256x512] = 256,
    [BG_SIZE_512x512] = 512,
};

int bg_matrix_height[9] = {
    [BG_SIZE_16x16] = 16,
    [BG_SIZE_32x16] = 16,
    [BG_SIZE_16x32] = 32,
    [BG_SIZE_32x32] = 32,
    [BG_SIZE_128x128] = 128,
    [BG_SIZE_256x256] = 256,
    [BG_SIZE_512x256] = 256,
    [BG_SIZE_256x512] = 512,
    [BG_SIZE_512x512] = 512,
};

int bg_matrix_size[9] = {
    [BG_SIZE_16x16] = 16*16,
    [BG_SIZE_32x16] = 32*16,
    [BG_SIZE_16x32] = 16*32,
    [BG_SIZE_32x32] = 32*32,
    [BG_SIZE_128x128] = 128*128,
    [BG_SIZE_256x256] = 256*256,
    [BG_SIZE_512x256] = 512*256,
    [BG_SIZE_256x512] = 256*512,
    [BG_SIZE_512x512] = 512*512,
};

static int
bg_matrix_get_height (bg_size_t size)
{
    return bg_matrix_height[size];
}

static int
bg_matrix_get_width (bg_size_t size)
{
    return bg_matrix_width[size];
}

static int
bg_matrix_get_u32_size (bg_size_t size)
{
    return bg_matrix_size[size];
} 

static void
bg_matrix_allocate (struct bg_matrix *x, bg_size_t size, vram_bank_t bank)
{
    g_assert_cmpint (bg_matrix_get_u32_size(size), <=,  vram_get_u32_size(bank));

    vram_zero_bank(bank);
    x->bank = bank;
    x->size = size;
    x->storage = vram_get_u32_ptr(bank);

    g_free (x->data);
    x->data = g_new0(uint32_t *, (size_t) bg_matrix_get_height(size));
    for (int i = 0; i < bg_matrix_height[size]; i ++)
        x->data[i] = x->storage + i * bg_matrix_width[size];
}

typedef struct bg_entry
{
    /* BG is displayed when true */
    bool enable;

    /** tile and map, palette bmp, or true color bmp */
    bg_type_t type;

    /* z-level: 0 is foreground, 3 is background */
    int priority;

    /** the "user" or screen location of the rotation center of the
     * background */
    double scroll_x, scroll_y;

    /** the "device" location of the rotation center of the background*/
    double rotation_center_x, rotation_center_y;

    /** the expansion factor of the background: 1.0 = 1 pixel per pixel */
    double expansion;

    /** the rotation angle of the background about its rotation
     * center, in radians */
    double rotation;

    /** The width, height, and data of either the map or the bitmap.
     *  If this is a map, data contains indices.  If this is a bitmap,
     *  data contains colorrefs.
     */
    struct bg_matrix matrix;

} bg_entry_t;

typedef struct bg_tag
{
    /* The RGBA color displayed below all backgrounds and sprites */
    bool colorswap;
  
    double brightness;

    bg_entry_t bg[BG_MAIN_BACKGROUNDS_COUNT + BG_SUB_BACKGROUNDS_COUNT];
    cairo_surface_t *surf[BG_MAIN_BACKGROUNDS_COUNT + BG_SUB_BACKGROUNDS_COUNT];
} bg_t;

bg_t bg;

////////////////////////////////////////////////////////////////

static cairo_surface_t *
bg_render_to_cairo_surface (bg_index_t id);
static cairo_surface_t *
bg_render_map_to_cairo_surface (bg_index_t id);
static cairo_surface_t *
bg_render_bmp_to_cairo_surface (bg_index_t id);

static uint32_t
adjust_colorval (uint32_t c32)
{
    uint32_t a, r, g, b;

    a = (((uint32_t) c32 & 0xff000000) >> 24);
    r = (((uint32_t) c32 & 0x00ff0000) >> 16);
    g = (((uint32_t) c32 & 0x0000ff00) >> 8);
    b = ((uint32_t) c32 & 0x000000ff);

    if (bg.colorswap)
    {
        uint32_t temp = r;
        r = b;
        b = temp;
    }
    r = r * bg.brightness;
    g = g * bg.brightness;
    b = b * bg.brightness;
    c32 = (a << 24) + (r << 16) + (g << 8) + b;
    return c32;
}

bool
bg_is_shown (bg_index_t id)
{
    return bg.bg[id].enable;
}

uint32_t *bg_get_data_ptr (bg_index_t id)
{
    return bg.bg[id].matrix.storage;
}

int bg_get_priority (bg_index_t id)
{
    return bg.bg[id].priority;
}

void bg_hide (bg_index_t id)
{
    bg.bg[id].enable = FALSE;
}

void bg_init (bg_index_t id, bg_type_t type, bg_size_t siz, vram_bank_t bank)
{
    bg.bg[id].enable = false;
    bg.bg[id].type = type;
    bg.bg[id].priority = id % 4;
    bg.bg[id].scroll_x = 0.0;
    bg.bg[id].scroll_y = 0.0;
    bg.bg[id].rotation_center_x = 0.0;
    bg.bg[id].rotation_center_y = 0.0;
    bg.bg[id].expansion = 1.0;
    bg.bg[id].rotation = 0.0;
    bg_matrix_allocate (&(bg.bg[id].matrix), siz, bank);
}

void bg_init_all_to_default ()
{
    bg.colorswap = false;
    bg.brightness = 1.0;
    for (int i = 0; i < BG_MAIN_BACKGROUNDS_COUNT + BG_SUB_BACKGROUNDS_COUNT; i ++)
    {
        bg.bg[i].enable = false;
        bg.bg[i].type = BG_TYPE_NONE;
        bg.bg[i].priority = i % 4;
        bg.bg[i].scroll_x = 0.0;
        bg.bg[i].scroll_y = 0.0;
        bg.bg[i].rotation_center_x = 0.0;
        bg.bg[i].rotation_center_y = 0.0;
        bg.bg[i].expansion = 1.0;
        bg.bg[i].rotation = 0.0;
    }
}

void bg_reset (bg_index_t id)
{
    bg.bg[id].type = BG_TYPE_NONE;
    bg.bg[id].scroll_x = 0.0;
    bg.bg[id].scroll_y = 0.0;
    bg.bg[id].rotation_center_x = 0.0;
    bg.bg[id].rotation_center_y = 0.0;
    bg.bg[id].expansion = 1.0;
    bg.bg[id].rotation = 0.0;
    bg.bg[id].priority = id % 4;
}

void bg_rotate (bg_index_t id, double angle)
{
    bg.bg[id].rotation += angle;
}

void bg_scroll (bg_index_t id, double dx, double dy)
{
    bg.bg[id].scroll_x += dx;
    bg.bg[id].scroll_y += dy;
}

void bg_set (bg_index_t id, double rotation, double expansion,
             double scroll_x, double scroll_y,
             double rotation_center_x, double rotation_center_y)
{
    bg.bg[id].rotation = rotation;
    bg.bg[id].expansion = expansion;
    bg.bg[id].scroll_x = scroll_x;
    bg.bg[id].scroll_y = scroll_y;
    bg.bg[id].rotation_center_x = rotation_center_x;
    bg.bg[id].rotation_center_y = rotation_center_y;
}

void bg_set_expansion (bg_index_t id, double expansion)
{
    bg.bg[id].expansion = expansion;
}

void bg_set_priority (bg_index_t id, int priority)
{
    bg.bg[id].priority = priority;
}

void bg_set_rotation (bg_index_t id, double rotation)
{
    bg.bg[id].rotation = rotation;
}

void bg_set_rotation_center (bg_index_t id,
                             double rotation_center_x, double rotation_center_y)
{
    bg.bg[id].rotation_center_x = rotation_center_x;
    bg.bg[id].rotation_center_y = rotation_center_y;
}

void bg_set_rotation_expansion (bg_index_t id, double rotation, double expansion)
{
    bg.bg[id].rotation = rotation;
    bg.bg[id].expansion = expansion;
}

void bg_show (bg_index_t id)
{
    g_assert (bg.bg[id].type != BG_TYPE_NONE);
    
    bg.bg[id].enable = TRUE;
}

#if 0
static void bg_set_map_from_tga (bg_index_t id, targa_image_t *t)
{
    unsigned width, height;
    targa_get_image_dimensions (t, &width, &height);
    bg.bg[id].map.width = width;
    bg.bg[id].map.height = height;
    for (unsigned j = 0; j < height; j ++)
    {
        for (unsigned i = 0; i < width ; i ++)
        {
            bg.bg[id].map.map[j][i] = tga_get_image_data_u16_ptr(t)[j * width + i];
        }
    }
}
#endif

#if 0
static void bg_set_tilesheet_from_tga (bg_index_t id, targa_image_t *t)
{
    unsigned width, height;
    int first = targa_get_color_map_first_index (t);

    targa_get_image_dimensions (t, &width, &height);

    for (unsigned j = 0; j < height; j ++)
    {
        for (unsigned i = 0; i < width ; i ++)
        {
            bg.bg[id].map.tilesheet[j][i] = tga_get_image_data_u8_ptr(t)[j * width + i];
        }
    }

    for (unsigned i = 0; i < targa_get_color_map_length (t) - first; i ++)
        bg.bg[id].map.palette[i] = tga_get_color_map_data_u16_ptr(t)[i + first];
}
#endif

static void set_from_image_file (bg_index_t id, bg_type_t type, const char *filename)
{
    char *path = xg_find_data_file (filename);
    g_return_if_fail (path != NULL);
    GdkPixbuf *pb = xgdk_pixbuf_new_from_file (path);
    g_return_if_fail (pb != NULL);
    if (xgdk_pixbuf_is_argb32 (pb) == false)
    {
        xg_object_unref (pb);
        g_critical ("failed to load %s as an ARGB32 pixbuf", path);
        g_free (path);
    }
    else
    {
        int img_width, img_height, img_stride;
        int bg_width, bg_height;
        int width, height;

        xgdk_pixbuf_get_width_height_stride (pb, &img_width, &img_height, &img_stride);
        uint32_t *c32 = xgdk_pixbuf_get_argb32_pixels (pb);
        bg_width = bg_matrix_width[bg.bg[id].matrix.size];
        bg_height = bg_matrix_height[bg.bg[id].matrix.size];

        width = MIN(img_width, bg_width);
        height = MIN(img_height, bg_height);
        
        for (int j = 0; j < height; j ++)
        {
            for (int i = 0; i < width ; i ++)
            {
                bg.bg[id].matrix.data[j][i] = c32[j * img_stride + i];
            }
        }
        bg.bg[id].type = type;
        g_debug ("loaded pixbuf %s as bg %d", path, id);
        g_free (path);
        g_object_unref (pb);
    }
}

void bg_set_data_from_image_file (bg_index_t id, bg_type_t type, const char *filename)
{
    set_from_image_file (id, type, filename);
}

#if 0
void bg_set_bmp_from_resource (bg_index_t id, const gchar *resource)
{
    targa_image_t *t = tga_load_from_resource (resource);
    bg_set_bmp_from_tga (id, t);
    tga_free (t);
}
#endif

cairo_surface_t *
bg_get_cairo_surface (bg_index_t id)
{
    g_assert (bg.bg[id].type != BG_TYPE_NONE);
    g_assert (bg.surf[id] != NULL);

    return bg.surf[id];
}

static void
bg_update (bg_index_t id)
{
    if (bg.surf[id] != NULL)
        xcairo_surface_destroy (bg.surf[id]);
    bg.surf[id] = bg_render_to_cairo_surface (id);
}

static cairo_surface_t *
bg_render_to_cairo_surface (bg_index_t id)
{
    g_return_if_fail (id >= 0 && id < BG_MAIN_BACKGROUNDS_COUNT + BG_SUB_BACKGROUNDS_COUNT);
    
    switch (bg.bg[id].type)
    {
    case BG_TYPE_NONE:
        return NULL;
        break;
    case BG_TYPE_MAP:
        return bg_render_map_to_cairo_surface (id);
        break;
    case BG_TYPE_BMP:
        return bg_render_bmp_to_cairo_surface (id);
        break;
    }
    g_return_val_if_reached (NULL);
}

static cairo_surface_t *
bg_render_map_to_cairo_surface (bg_index_t id)
{
    cairo_surface_t *surf;
    uint32_t *data;
    int stride;
    int tile_j, tile_i, delta_tile_i, delta_tile_j;
    int map_index;
    uint32_t c;
    int width, height;
    tilesheet_index_t tilesheet_id;

    if (id >= BG_MAIN_0 && id <= BG_MAIN_3)
        tilesheet_id = TILESHEET_MAIN;
    else
        tilesheet_id = TILESHEET_SUB;
    
    width = bg_matrix_get_width(bg.bg[id].matrix.size);
    height = bg_matrix_get_height(bg.bg[id].matrix.size);

    surf = xcairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                        width * TILE_WIDTH,
                                        height * TILE_HEIGHT);
    data = xcairo_image_surface_get_argb32_data (surf);
    stride = xcairo_image_surface_get_argb32_stride (surf);
    xcairo_surface_flush (surf);

    for (int map_j = 0; map_j < height; map_j ++)
    {
        for (int map_i = 0; map_i < width; map_i ++)
        {
            /* Fill in the tile brush */
            map_index = (int) bg.bg[id].matrix.data[map_j][map_i];
            
            // FIXME -- IS THIS RIGHT??
            // vflip = map_index & (1 << 31);
            // hflip = map_index & (1 << 30);
            // map_index = map_index & 0x0fffffff;
            
            delta_tile_j = (map_index / tilesheet_get_width_in_tiles(tilesheet_id)) * TILE_HEIGHT;
            delta_tile_i = (map_index % tilesheet_get_width_in_tiles(tilesheet_id)) * TILE_WIDTH;
            for (tile_j = 0; tile_j < TILE_HEIGHT; tile_j ++)
            {
                if (false && (bg.brightness == 1.0) && (bg.colorswap == false) /* && hflip == false && vflip == false */)
                {
                    // FAST PATH, use memcpy to copy an entire row
                    // from the tilesheet
                }
                else
                {
                    // SLOW PATH, copy a row pixel-by-pixel, adjusting
                    // brighness, colorswap, (FIXME flipping too)
                    for (tile_i = 0; tile_i < TILE_WIDTH; tile_i ++)
                    {
                        uint32_t c32;
                        c32 = tilesheet_get_u32_data(tilesheet_id)[delta_tile_j + tile_j][delta_tile_i + tile_i];
                        
                        c = adjust_colorval (c32);
                        data[(map_j * TILE_HEIGHT + tile_j) * stride
                             + (map_i * TILE_WIDTH + tile_i)] = c;
                    }
                }
            }
        }
    }
    xcairo_surface_mark_dirty (surf);
    return surf;
}

static cairo_surface_t *
bg_render_bmp_to_cairo_surface (bg_index_t id)
{
    int width, height, stride;
    uint32_t *data;
    uint32_t c32;
    cairo_surface_t *surf;

    width = bg_matrix_get_width(bg.bg[id].matrix.size);
    height = bg_matrix_get_height(bg.bg[id].matrix.size);

    g_return_val_if_fail (width > 0 && height > 0, NULL);

    surf = xcairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    data = xcairo_image_surface_get_argb32_data (surf);
    stride = xcairo_image_surface_get_argb32_stride (surf);
    xcairo_surface_flush (surf);
    if (bg.brightness == 1.0 && bg.colorswap == false /* && hflip == false && vflip == false */)
    {
        // FAST PATH, use memcpy.
        g_assert (stride == width);
        memcpy (data,
                bg.bg[id].matrix.storage,
                bg_matrix_get_u32_size (bg.bg[id].matrix.size) * sizeof (uint32_t));
    }
    else
    {
        for (int j = 0; j < height; j++)
        {
            for (int i = 0; i < width; i++)
            {
                c32 = bg.bg[id].matrix.data[j][i];
                data[j * stride + i] = adjust_colorval (c32);
            }
        }
    }
    xcairo_surface_mark_dirty (surf);
    return surf;
}

void bg_get_transform (bg_index_t id, double *scroll_x, double *scroll_y,
                       double *rotation_center_x, double *rotation_center_y,
                       double *rotation, double *expansion)
{
    *scroll_x = bg.bg[id].scroll_x;
    *scroll_y = bg.bg[id].scroll_y;
    *rotation_center_x = bg.bg[id].rotation_center_x;
    *rotation_center_y = bg.bg[id].rotation_center_y;
    *rotation = bg.bg[id].rotation;
    *expansion = bg.bg[id].expansion;
}

SCM_DEFINE (G_bg_get_priority, "bg-get-priority", 1, 0, 0, (SCM id), "\
return the z-ordering of a given background layer.")
{
    return scm_from_int (bg_get_priority (scm_to_int (id)));
}

SCM_DEFINE (G_bg_hide, "bg-hide", 1, 0, 0, (SCM id), "\
Set background later ID to not be drawn")
{
    bg_hide (scm_to_int (id));
    return SCM_UNSPECIFIED;
}

SCM_DEFINE (G_bg_reset, "bg-reset", 1, 0, 0,
            (SCM id), "\
Reset a background to hidden with nominal status")
{
    bg_reset (scm_to_int (id));
    return SCM_UNSPECIFIED;
}

SCM_DEFINE (G_bg_rotate, "bg-rotate", 2, 0, 0, (SCM id, SCM angle), "\
Rotate the background about its rotation center")
{
    bg_rotate (scm_to_int (id), scm_to_double (angle));
    return SCM_UNSPECIFIED;
}

SCM_DEFINE (G_bg_scroll, "bg-scroll", 3, 0, 0, (SCM id, SCM dx, SCM dy), "\
Move the background")
{
    bg_scroll (scm_to_int (id), scm_to_double (dx), scm_to_double (dy));
    return SCM_UNSPECIFIED;
}

SCM_DEFINE (G_bg_set, "bg-set", 7, 0, 0,
            (SCM id, SCM rotation, SCM expansion, SCM scroll_x, SCM scroll_y, SCM center_x, SCM center_y), "\
Set the position, rotation, expansion, and rotation center of a background.")
{
    bg_set (scm_to_int(id), scm_to_double(rotation), scm_to_double (expansion), scm_to_double (scroll_x),
            scm_to_double (scroll_y), scm_to_double (center_x), scm_to_double (center_y));
    return SCM_UNSPECIFIED;
}

SCM_DEFINE (G_bg_set_bmp_from_file, "bg-set-bmp-from-file",
            2, 0, 0, (SCM id, SCM filename), "\
Set BG to be a bitmap-type background using the data from FILE in the data\n\
directory")
{
    char *str = scm_to_locale_string (filename);
    bg_set_data_from_image_file (scm_to_int (id), BG_TYPE_BMP, str);
    free (str);
    return SCM_UNSPECIFIED;
}

SCM_DEFINE (G_bg_set_expansion, "bg-set-expansion", 2, 0, 0, (SCM id, SCM expansion), "\
Set BG expansion")
{
    bg_set_expansion (scm_to_int (id), scm_to_double (expansion));
    return SCM_UNSPECIFIED;
}

SCM_DEFINE (G_bg_set_priority, "bg-set-priority", 2, 0, 0, (SCM id, SCM priority), "\
Set BG priority")
{
    bg_set_priority (scm_to_int (id), scm_to_int (priority));
    return SCM_UNSPECIFIED;
}

SCM_DEFINE (G_bg_set_rotation, "bg-set-rotation", 2, 0, 0, (SCM id, SCM rotation), "\
Set BG rotation")
{
    bg_set_rotation (scm_to_int (id), scm_to_int (rotation));
    return SCM_UNSPECIFIED;
}

SCM_DEFINE (G_bg_set_rotation_center, "bg-set-rotation-center", 3, 0, 0, (SCM id, SCM x, SCM y), "\
Move the rotation center of the background")
{
    bg_set_rotation_center (scm_to_int (id), scm_to_double (x), scm_to_double (y));
    return SCM_UNSPECIFIED;
}

SCM_DEFINE (G_bg_set_rotation_expansion, "bg-set-rotation-expansion", 3, 0, 0, (SCM id, SCM r, SCM e), "\
Set the rotation angle and expansion of a BG")
{
    bg_set_rotation_expansion (scm_to_int (id), scm_to_double (r), scm_to_double (e));
    return SCM_UNSPECIFIED;
}

SCM_DEFINE (G_bg_show, "bg-show", 1, 0, 0, (SCM id), "\
Set background later ID to be drawn")
{
    bg_show (scm_to_int (id));
    return SCM_UNSPECIFIED;
}

SCM_DEFINE (G_bg_shown_p, "bg-shown?", 1, 0, 0, (SCM id), "\
Return #t if indicated background layer is visible.")
{
    return scm_from_bool (bg_is_shown (scm_to_int (id)));
}

SCM_DEFINE (G_bg_update, "bg-update", 1, 0, 0, (SCM id), "\
Apply all changes to this background layer since the last call to 'bg-update'")
{
    bg_update (scm_to_int (id));
    return SCM_UNSPECIFIED;
}

SCM_DEFINE (G_bg_modify, "bg-get-bytevector", 1, 0, 0, (SCM id), "\
Returns a bytevector of data that holds the BG bitmap or map data")
{
    return scm_pointer_to_bytevector(bg.bg[scm_to_int(id)].storage,
                                     scm_from_int (bg_matrix_size[bg.bg[scm_to_int(id)].size]),
                                     scm_from_int (0),
                                     scm_from_locale_symbol("u32"));
}

SCM_DEFINE (G_bg_get_dimensions,"bg-get-dimensions", 1, 0, 0, (SCM id), "")
{
    bg_size_t = bg.bg[scm_to_int(id)].size;
    return scm_list_3 (scm_from_int (bg_matrix_width (siz)),
                       scm_from_int (bg_matrix_height (siz)),
                       scm_from_int (bg_matrix_size(siz)));
}

SCM_VARIABLE_INIT (G_BG_TYPE_BMP, "BG_TYPE_BMP", scm_from_int (BG_TYPE_BMP));
SCM_VARIABLE_INIT (G_BG_TYPE_MAP, "BG_TYPE_MAP", scm_from_int (BG_TYPE_MAP));

SCM_VARIABLE_INIT (G_BG_SIZE_16x16, "BG_SIZE_16x16", scm_from_int (BG_SIZE_16x16));
SCM_VARIABLE_INIT (G_BG_SIZE_16x32, "BG_SIZE_16x32", scm_from_int (BG_SIZE_16x32));
SCM_VARIABLE_INIT (G_BG_SIZE_32x16, "BG_SIZE_32x16", scm_from_int (BG_SIZE_32x16));
SCM_VARIABLE_INIT (G_BG_SIZE_32x32, "BG_SIZE_32x32", scm_from_int (BG_SIZE_32x32));
SCM_VARIABLE_INIT (G_BG_SIZE_128x128, "BG_SIZE_128x128", scm_from_int (BG_SIZE_128x128));
SCM_VARIABLE_INIT (G_BG_SIZE_256x256, "BG_SIZE_256x256", scm_from_int (BG_SIZE_256x256));
SCM_VARIABLE_INIT (G_BG_SIZE_256x512, "BG_SIZE_256x512", scm_from_int (BG_SIZE_256x512));
SCM_VARIABLE_INIT (G_BG_SIZE_512x256, "BG_SIZE_512x256", scm_from_int (BG_SIZE_512x256));
SCM_VARIABLE_INIT (G_BG_SIZE_512x512, "BG_SIZE_512x512", scm_from_int (BG_SIZE_512x512));

SCM_VARIABLE_INIT (G_BG_MAIN_0, "BG_MAIN_0", scm_from_int (BG_MAIN_0));
SCM_VARIABLE_INIT (G_BG_MAIN_1, "BG_MAIN_1", scm_from_int (BG_MAIN_1));
SCM_VARIABLE_INIT (G_BG_MAIN_2, "BG_MAIN_2", scm_from_int (BG_MAIN_2));
SCM_VARIABLE_INIT (G_BG_MAIN_3, "BG_MAIN_3", scm_from_int (BG_MAIN_3));

SCM_VARIABLE_INIT (G_BG_SUB_0, "BG_SUB_0", scm_from_int (BG_SUB_0));
SCM_VARIABLE_INIT (G_BG_SUB_1, "BG_SUB_1", scm_from_int (BG_SUB_1));
SCM_VARIABLE_INIT (G_BG_SUB_2, "BG_SUB_2", scm_from_int (BG_SUB_2));
SCM_VARIABLE_INIT (G_BG_SUB_3, "BG_SUB_3", scm_from_int (BG_SUB_3));

void
bg_init_guile_procedures (void)
{
#include "bg.x"
    scm_c_export ("bg-get-priority",
                  "bg-hide", 
                  "bg-reset",
                  "bg-rotate",
                  "bg-scroll",
                  "bg-set",
                  "bg-set-bmp-from-file",
                  "bg-set-expansion",
                  "bg-set-priority",
                  "bg-set-rotation",
                  "bg-set-rotation-center",
                  "bg-set-rotation-expansion",
                  "bg-show",
                  "bg-shown?",
                  "bg-update",
                  "bg-modify",
                  NULL);
}

#pragma GCC diagnostic pop

/*
  Local Variables:
  mode:C
  c-file-style:"linux"
  tab-width:4
  c-basic-offset: 4
  indent-tabs-mode:nil
  End:
*/
