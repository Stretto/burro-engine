#include "../x.h"
#include "backdrop.h"
#include "bg.h"
#include "console.h"
#include "draw.h"
#include "eng.h"
#include "obj.h"
#include <math.h>

static cairo_t *main_screen_context;
static cairo_surface_t *main_screen_surface;
static cairo_t *sub_screen_context;
static cairo_surface_t *sub_screen_surface;

static void draw_backdrop_color (void);
static void draw_background_layer (bg_index_t layer);
static void draw_console_layer (void);
static void draw_obj (int id);

cairo_surface_t *draw_get_main_screen_surface (void)
{
    return main_screen_surface;
}

cairo_surface_t *draw_get_sub_screen_surface (void)
{
    return sub_screen_surface;
}

void draw_initialize ()
{
    main_screen_surface = xcairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                                       MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
    main_screen_context = xcairo_create (main_screen_surface);
    xcairo_set_antialias (main_screen_context, CAIRO_ANTIALIAS_NONE);
    
    sub_screen_surface = xcairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                                      SUB_SCREEN_WIDTH, SUB_SCREEN_HEIGHT);
    sub_screen_context = xcairo_create (sub_screen_surface);
    xcairo_set_antialias (sub_screen_context, CAIRO_ANTIALIAS_NONE);
}

void draw_finalize ()
{
    xcairo_destroy (sub_screen_context);
    xcairo_surface_destroy (sub_screen_surface);
    xcairo_destroy (main_screen_context);
    xcairo_surface_destroy (main_screen_surface);
}

void draw ()
{
    draw_backdrop_color ();
    if (eng_is_blank())
        goto end_draw;
    
    for (int priority = PRIORITY_COUNT - 1; priority >= 0; priority --)
    {
        for (int layer = BG_SUB_3; layer >= BG_MAIN_0; layer --)
        {
            if (bg_is_shown (layer) && bg_get_priority (layer) == priority)
                draw_background_layer (layer);
        }
        for (int id = MAIN_OBJ_COUNT + SUB_OBJ_COUNT - 1; id >= 0; id --)
        {
            if (obj_is_shown (id) && obj_get_priority (id) == priority)
                draw_obj (id);
        }
    }
    
 end_draw:

    if (console_is_visible ())
      draw_console_layer ();
    xcairo_surface_mark_dirty (main_screen_surface);
    xcairo_surface_mark_dirty (sub_screen_surface);
}

static void compute_transform (cairo_matrix_t *matrix,
                               double rotation_center_screen_x,
                               double rotation_center_screen_y,
                               int rotation_center_bitmap_row,
                               int rotation_center_bitmap_column,
                               double rotation_angle, double expansion_factor)
{
    double xx, xy, yx, yy, x0, y0;
    double sn, cs;
    if (expansion_factor == 0.0)
        expansion_factor = 1.0;
    sn = sin (rotation_angle);
    cs = cos (rotation_angle);
    xx = expansion_factor * cs;
    xy = expansion_factor * sn;
    yx = -xy;
    yy = xx;
    x0 = (rotation_center_screen_x
          - (xx * (double)rotation_center_bitmap_column
             + xy * (double) rotation_center_bitmap_row));
    y0 = (rotation_center_screen_y
          - (yx * (double)rotation_center_bitmap_column
             + yy * (double) rotation_center_bitmap_row));
    matrix->xx = xx;
    matrix->xy = xy;
    matrix->yx = yx;
    matrix->yy = yy;
    matrix->x0 = x0;
    matrix->y0 = y0;
}


static void draw_backdrop_color ()
{
  double r = 0.0, g = 0.0, b = 0.0;
  
  backdrop_get_color_rgb (BACKDROP_MAIN, &r, &g, &b);  
  xcairo_set_source_rgb (main_screen_context, r, g, b);
  xcairo_paint (main_screen_context);

  backdrop_get_color_rgb (BACKDROP_SUB, &r, &g, &b);  
  xcairo_set_source_rgb (sub_screen_context, r, g, b);
  xcairo_paint (sub_screen_context);
}

static void paint_transformed_image (cairo_t *context,
                                     cairo_matrix_t *matrix,
                                     cairo_surface_t *surface)
{
    /* Set the coordinate transform */
    xcairo_set_matrix (context, matrix);

    /* Now copy it to the screen */
    xcairo_set_source_surface (context, surface, 0, 0);
    xcairo_paint (context);

    /* Restore the coordinate system to normal */
    xcairo_identity_matrix (context);
}

static void draw_background_layer (bg_index_t layer)
{
    cairo_surface_t *surf;
    cairo_matrix_t matrix;
    double scroll_x, scroll_y, rotation_center_x, rotation_center_y;
    double rotation, expansion;
    
    surf = bg_get_cairo_surface (layer);
    xcairo_surface_mark_dirty (surf);
    bg_get_transform (layer, &scroll_x, &scroll_y,
                      &rotation_center_x, &rotation_center_y,
                      &rotation, &expansion);
    compute_transform (&matrix, scroll_x, scroll_y,
                       rotation_center_x, rotation_center_y,
                       rotation, expansion);
    if (layer >= BG_MAIN_0 && layer <= BG_MAIN_3)
        paint_transformed_image (main_screen_context, &matrix, surf);
    else
        paint_transformed_image (sub_screen_context, &matrix, surf);
    // xcairo_surface_destroy (surf);
}

static void draw_obj (int id)
{
    cairo_surface_t *surf;
    cairo_matrix_t matrix;
    double x, y, rotation_center_x, rotation_center_y;
    double rotation, expansion;
    
    surf = obj_render_to_cairo_surface (id);
    xcairo_surface_mark_dirty (surf);
    obj_get_location (id, &x, &y, &rotation_center_x, &rotation_center_y,
                      &rotation, &expansion);
    compute_transform (&matrix, x, y, rotation_center_x, rotation_center_y,
                       rotation, expansion);
    if (id < MAIN_OBJ_COUNT)
        paint_transformed_image (main_screen_context, &matrix, surf);
    else
        paint_transformed_image (sub_screen_context, &matrix, surf);
    xcairo_surface_destroy (surf);
}

static void draw_console_layer ()
{
    cairo_surface_t *surf;
    
    surf = console_render_to_cairo_surface ();
    xcairo_surface_mark_dirty (surf);

    /* Now copy it to the screen */
    xcairo_set_source_surface (sub_screen_context, surf, 0, 0);
    xcairo_paint (sub_screen_context);

    xcairo_surface_destroy (surf);
}

