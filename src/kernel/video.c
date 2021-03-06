/*
  video.c -- video module;

  Copyright (C) 2015, 2016 Bruno Félix Rezende Ribeiro <oitofelix@gnu.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mininim.h"

bool force_full_redraw;
ALLEGRO_DISPLAY *display;
ALLEGRO_BITMAP *uscreen, *iscreen;
ALLEGRO_TIMER *video_timer;
uint64_t bottom_text_timer;
int screen_flags = 0;
bool hgc;
bool is_display_focused = true;
ALLEGRO_BITMAP *effect_buffer;
ALLEGRO_BITMAP *black_screen;
struct video_effect video_effect = {.type = VIDEO_NO_EFFECT};
static ALLEGRO_FONT *builtin_font;
int display_width = DISPLAY_WIDTH, display_height = DISPLAY_HEIGHT;
ALLEGRO_BITMAP *icon;
int effect_counter;
void (*load_callback) (void);
int display_mode = -1;

static struct palette_cache {
  ALLEGRO_BITMAP *ib, *ob;
  palette pal;
} *palette_cache = NULL;
static size_t palette_cache_nmemb = 0;

void
init_video (void)
{
  if (! al_init_image_addon ())
    error (-1, 0, "%s (void): failed to initialize image addon",
            __func__);

  al_set_new_display_flags (al_get_new_display_flags ()
                            | (display_mode < 0 ? ALLEGRO_WINDOWED : ALLEGRO_FULLSCREEN)
                            | ALLEGRO_RESIZABLE
                            | ALLEGRO_GENERATE_EXPOSE_EVENTS);

  if (display_mode >= 0) {
    ALLEGRO_DISPLAY_MODE d;
    get_display_mode (display_mode, &d);
    display_width = d.width;
    display_height = d.height;
    al_set_new_display_refresh_rate (d.refresh_rate);
    al_set_new_display_flags (al_get_new_display_flags ()
                              & ~ALLEGRO_FULLSCREEN_WINDOW);
  }

  al_set_new_display_option (ALLEGRO_SINGLE_BUFFER, 1, ALLEGRO_SUGGEST);

  display = al_create_display (display_width, display_height);
  if (! display) error (-1, 0, "%s (void): failed to initialize display", __func__);

  set_target_backbuffer (display);
  al_set_new_bitmap_flags (ALLEGRO_VIDEO_BITMAP);

  al_set_window_title (display, WINDOW_TITLE);
  icon = load_bitmap (ICON);
  al_set_display_icon (display, icon);

  cutscene = true;
  if (mr.fit_w == 0 && mr.fit_h == 0) {
    mr.fit_w = 2;
    mr.fit_h = 2;
  }
  set_multi_room (1, 1);
  effect_buffer = create_bitmap (ORIGINAL_WIDTH, ORIGINAL_HEIGHT);
  black_screen = create_bitmap (ORIGINAL_WIDTH, ORIGINAL_HEIGHT);
  uscreen = create_bitmap (ORIGINAL_WIDTH, ORIGINAL_HEIGHT);
  iscreen = create_bitmap (display_width, display_height);
  clear_bitmap (uscreen, TRANSPARENT_COLOR);

  video_timer = create_timer (1.0 / EFFECT_HZ);

  al_init_font_addon ();
  builtin_font = al_create_builtin_font ();
  if (! builtin_font)
    error (-1, 0, "%s (void): cannot create builtin font", __func__);

  if (! al_init_primitives_addon ())
    error (-1, 0, "%s (void): failed to initialize primitives addon",
           __func__);
}

void
finalize_video (void)
{
  destroy_bitmap (icon);
  destroy_bitmap (uscreen);
  destroy_bitmap (effect_buffer);
  destroy_bitmap (black_screen);
  al_destroy_font (builtin_font);
  al_destroy_timer (video_timer);
  al_destroy_display (display);
  al_shutdown_image_addon ();
  al_shutdown_font_addon ();
  al_shutdown_primitives_addon ();
}

ALLEGRO_EVENT_SOURCE *
get_display_event_source (ALLEGRO_DISPLAY *display)
{
  ALLEGRO_EVENT_SOURCE *event_source = al_get_display_event_source (display);
  if (! event_source)
    error (-1, 0, "%s: failed to get display event source (%p)",
           __func__,  display);
  return event_source;
}

ALLEGRO_BITMAP *
create_bitmap (int w, int h)
{
  set_target_backbuffer (display);
  ALLEGRO_BITMAP *bitmap = al_create_bitmap (w, h);
  if (! bitmap) {
    error (-1, 0, "%s (%i, %i): cannot create bitmap", __func__, w, h);
    return NULL;
  }
  validate_bitmap_for_mingw (bitmap);
  return bitmap;
}

void
destroy_bitmap (ALLEGRO_BITMAP *bitmap)
{
  al_destroy_bitmap (bitmap);
}

ALLEGRO_BITMAP *
clone_bitmap (ALLEGRO_BITMAP *bitmap)
{
  ALLEGRO_BITMAP *new_bitmap = al_clone_bitmap (bitmap);
  if (! new_bitmap) error (-1, 0, "%s (%p): cannot clone bitmap", __func__, bitmap);
  return new_bitmap;
}

void
set_target_bitmap (ALLEGRO_BITMAP *bitmap)
{
  al_set_target_bitmap (bitmap);
}

void
set_target_backbuffer (ALLEGRO_DISPLAY *display)
{
  set_target_bitmap (al_get_backbuffer (display));
}

void
clear_bitmap (ALLEGRO_BITMAP *bitmap, ALLEGRO_COLOR color)
{
  set_target_bitmap (bitmap);
  al_clear_to_color (color);
}

ALLEGRO_BITMAP *
get_cached_palette (ALLEGRO_BITMAP *bitmap, palette p)
{
  struct palette_cache pc, *rpc;
  pc.ib = bitmap;
  pc.pal = p;

  rpc = bsearch (&pc, palette_cache, palette_cache_nmemb, sizeof (pc),
                 compare_palette_caches);

  if (rpc) return rpc->ob;
  else return NULL;
}

int
compare_palette_caches (const void *pc0, const void *pc1)
{
  struct palette_cache *p0 = (struct palette_cache *) pc0;
  struct palette_cache *p1 = (struct palette_cache *) pc1;

  if (p0->ib < p1->ib) return -1;
  else if (p0->ib > p1->ib) return 1;
  else if (p0->pal < p1->pal) return -1;
  else if (p0->pal > p1->pal) return 1;
  else return 0;
}

ALLEGRO_BITMAP *
apply_palette (ALLEGRO_BITMAP *bitmap, palette p)
{
  if (! bitmap) return NULL;

  ALLEGRO_BITMAP *cached = get_cached_palette (bitmap, p);
  if (cached) return cached;

  int x, y;
  ALLEGRO_BITMAP *rbitmap = clone_bitmap (bitmap);
  int w = al_get_bitmap_width (bitmap);
  int h = al_get_bitmap_height (bitmap);
  al_lock_bitmap (rbitmap, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE);
  set_target_bitmap (rbitmap);
  for (y = 0; y < h; y++)
    for (x = 0; x < w; x++)
      al_put_pixel (x, y, p (al_get_pixel (rbitmap, x, y)));
  al_unlock_bitmap (rbitmap);

  struct palette_cache pc;
  pc.ib = bitmap;
  pc.pal = p;
  pc.ob = rbitmap;

  palette_cache =
    add_to_array (&pc, 1, palette_cache, &palette_cache_nmemb,
                  palette_cache_nmemb, sizeof (pc));

  qsort (palette_cache, palette_cache_nmemb, sizeof (pc),
         compare_palette_caches);

  return rbitmap;
}

bool
color_eq (ALLEGRO_COLOR c0, ALLEGRO_COLOR c1)
{
  unsigned char r0, g0, b0, r1, g1, b1;
  al_unmap_rgb (c0, &r0, &g0, &b0);
  al_unmap_rgb (c1, &r1, &g1, &b1);
  return r0 == r1 && g0 == g1 && b0 == b1;
}

void
convert_mask_to_alpha (ALLEGRO_BITMAP *bitmap, ALLEGRO_COLOR mask_color)
{
  al_convert_mask_to_alpha (bitmap, mask_color);
}

void
draw_bitmap (ALLEGRO_BITMAP *from, ALLEGRO_BITMAP *to, float dx, float dy, int flags)
{
  set_target_bitmap (to);
  al_draw_bitmap (from, dx, dy, flags);
}

void
draw_bitmap_region (ALLEGRO_BITMAP *from, ALLEGRO_BITMAP *to, float sx, float sy,
                    float sw, float sh, float dx, float dy, int flags)
{
  set_target_bitmap (to);
  al_draw_bitmap_region (from, sx, sy, sw, sh, dx, dy, flags);
}

void
draw_rectangle (ALLEGRO_BITMAP *to, float x1, float y1,
                float x2, float y2, ALLEGRO_COLOR color,
                float thickness)
{
  set_target_bitmap (to);
  al_draw_rectangle (x1 + 1, y1, x2 + 1, y2, color, thickness);
}

void
draw_filled_rectangle (ALLEGRO_BITMAP *to, float x1, float y1,
                       float x2, float y2, ALLEGRO_COLOR color)
{
  set_target_bitmap (to);
  al_draw_filled_rectangle (x1, y1, x2 + 1, y2 + 1, color);
}

void
draw_text (ALLEGRO_BITMAP *bitmap, char const *text, float x, float y, int flags)
{
  set_target_bitmap (bitmap);
  al_draw_text (builtin_font, WHITE, x, y, flags, text);
}

void
draw_bottom_text (ALLEGRO_BITMAP *bitmap, char *text, int priority)
{
  static char *current_text = NULL;
  static int cur_priority = INT_MIN;

  if (bitmap == NULL && priority < cur_priority
      && bottom_text_timer < BOTTOM_TEXT_DURATION)
    return;

  if (text) {
    if (current_text) al_free (current_text);
    xasprintf (&current_text, "%s", text);
    bottom_text_timer = 1;
    cur_priority = priority;
  } else if (bottom_text_timer > BOTTOM_TEXT_DURATION
             || ! bitmap) {
    bottom_text_timer = 0;
    cur_priority = INT_MIN;
  } else if (bottom_text_timer) {
    ALLEGRO_COLOR bg_color;

    switch (vm) {
    case CGA: bg_color = C_MSG_LINE_COLOR; break;
    case EGA: bg_color = E_MSG_LINE_COLOR; break;
    case VGA: bg_color = V_MSG_LINE_COLOR; break;
    }

    set_target_bitmap (bitmap);
    al_draw_filled_rectangle (0, ORIGINAL_HEIGHT - 8,
                              ORIGINAL_WIDTH, ORIGINAL_HEIGHT,
                              bg_color);
    draw_text (bitmap, current_text,
               ORIGINAL_WIDTH / 2.0, ORIGINAL_HEIGHT - 7,
               ALLEGRO_ALIGN_CENTRE);
  }
}

ALLEGRO_BITMAP *
load_bitmap (char *filename)
{
  set_target_backbuffer (display);

  ALLEGRO_BITMAP *bitmap =
    load_resource (filename, (load_resource_f) al_load_bitmap);

  if (! bitmap)
    error (-1, 0, "%s: cannot load bitmap file '%s'",
           __func__, filename);

  validate_bitmap_for_mingw (bitmap);

  if (load_callback) load_callback ();

  return bitmap;
}

void
validate_bitmap_for_mingw (ALLEGRO_BITMAP *bitmap)
{
  /* work around a bug (MinGW target), where bitmaps are loaded as
     black/transparent images */
  al_lock_bitmap(bitmap, ALLEGRO_PIXEL_FORMAT_ANY,
		 ALLEGRO_LOCK_READWRITE);
  al_unlock_bitmap(bitmap);
}

void
save_bitmap (char *filename, ALLEGRO_BITMAP *bitmap)
{
  if (! al_save_bitmap (filename, bitmap))
    error (-1, 0, "%s: cannot save bitmap file '%s'",
           __func__, filename);
}

ALLEGRO_COLOR
hgc_palette (ALLEGRO_COLOR c)
{
  if (color_eq (c, al_map_rgb (85, 255, 255)))
    return al_map_rgb (170, 170, 170);
  if (color_eq (c, al_map_rgb (255, 85, 255)))
    return al_map_rgb (85, 85, 85);
  return c;
}

void
draw_mr_select_rect (int x, int y, ALLEGRO_COLOR color)
{
  int w = al_get_display_width (display);
  int h = al_get_display_height (display);
  int tw, th; mr_get_resolution (&tw, &th);

  ALLEGRO_BITMAP *screen = mr.cell[x][y].screen;
  int sw = al_get_bitmap_width (screen);
  int sh = al_get_bitmap_height (screen);
  float dx = ((ORIGINAL_WIDTH * x) * w) / (float) tw;
  float dy = ((ROOM_HEIGHT * y) * h) / (float) th;
  float dw = (sw * w) / (float) tw;
  float dh = (sh * h) / (float) th;
  al_draw_rectangle (dx, dy, dx + dw, dy + dh, color, 2);
}

void
flip_display (ALLEGRO_BITMAP *bitmap)
{
  int w = al_get_display_width (display);
  int h = al_get_display_height (display);

  int uw = al_get_bitmap_width (uscreen);
  int uh = al_get_bitmap_height (uscreen);

  if (bitmap) {
    int bw = al_get_bitmap_width (bitmap);
    int bh = al_get_bitmap_height (bitmap);
    set_target_backbuffer (display);
    al_draw_scaled_bitmap
      (bitmap, 0, 0, bw, bh, 0, 0, w, h, screen_flags);
  } else {
    if (has_mr_view_changed ()
        && ! cutscene
        && ! no_room_drawing) {
      draw_multi_rooms ();
      force_full_redraw = true;
    }

    int iw = al_get_bitmap_width (iscreen);
    int ih = al_get_bitmap_height (iscreen);

    if (iw != w || ih != h) {
      destroy_bitmap (iscreen);
      iscreen = clone_bitmap (al_get_backbuffer (display));
    }

    al_set_target_bitmap (iscreen);

    int x, y;
    int tw, th;
    mr_get_resolution (&tw, &th);

    for (y = mr.h - 1; y >= 0; y--)
      for (x = 0; x < mr.w; x++) {
        ALLEGRO_BITMAP *screen =
          (mr.cell[x][y].room || no_room_drawing || cutscene)
          ? mr.cell[x][y].screen : mr.cell[x][y].cache;
        int sw = al_get_bitmap_width (screen);
        int sh = al_get_bitmap_height (screen);
        float dx = ((ORIGINAL_WIDTH * x) * w) / (float) tw;
        float dy = ((ROOM_HEIGHT * y) * h) / (float) th;
        float dw = (sw * w) / (float) tw;
        float dh = (sh * h) / (float) th;

        if (cutscene
            || mr.cell[x][y].room
            || mr.last.display_width != w
            || mr.last.display_height != h
            || force_full_redraw)
          al_draw_scaled_bitmap
            (screen, 0, 0, sw, sh, dx, dy, dw, dh, 0);
      }

    set_target_backbuffer (display);
    al_draw_bitmap (iscreen, 0, 0, screen_flags);

    if (mr.room_select > 0 && ! cutscene
        && edit != EDIT_NONE)
      for (y = mr.h - 1; y >= 0; y--)
        for (x = 0; x < mr.w; x++)
          if (mr.cell[x][y].room == mr.room_select) {
            int rx = x, ry = y;
            if (screen_flags & ALLEGRO_FLIP_HORIZONTAL)
              rx = (mr.w - 1) - x;
            if (screen_flags & ALLEGRO_FLIP_VERTICAL)
              ry = (mr.h - 1) - y;
            draw_mr_select_rect (rx, ry, GREEN);
          }

    if ((mr.room != mr.last.room
         || mr.x != mr.last.x
         || mr.y != mr.last.y
         || mr.w != mr.last.w
         || mr.h != mr.last.h)
        && ! cutscene)
      mr.select_cycles = SELECT_CYCLES;

    if (mr.select_cycles > 0 && ! cutscene) {
      int rx = mr.x, ry = mr.y;
      if (screen_flags & ALLEGRO_FLIP_HORIZONTAL)
        rx = (mr.w - 1) - mr.x;
      if (screen_flags & ALLEGRO_FLIP_VERTICAL)
        ry = (mr.h - 1) - mr.y;
      draw_mr_select_rect (rx, ry, RED);
      mr.select_cycles--;
    }
  }

  al_draw_scaled_bitmap (uscreen, 0, 0, uw, uh, 0, 0, w, h, 0);
  al_flip_display ();

  force_full_redraw = false;
}

void
get_display_mode (int index, ALLEGRO_DISPLAY_MODE *mode)
{
  if (! al_get_display_mode (index, mode))
    error (-1, 0, "%s (%i, %p): cannot get display mode", __func__, index, mode);
}

void
acknowledge_resize (void)
{
  if (! al_acknowledge_resize (display) && ! is_fullscreen ())
    error (0, 0, "%s: cannot acknowledge display resize (%p)", __func__, display);
}

void
draw_fade (ALLEGRO_BITMAP *from, ALLEGRO_BITMAP *to, float factor)
{
  factor = factor < 0 ? 0 : factor;
  clear_bitmap (black_screen, al_map_rgba_f (0, 0, 0, factor));
  draw_bitmap (from, to, 0, 0, 0);
  draw_bitmap (black_screen, to, 0, 0, 0);
}

void
draw_roll_right (ALLEGRO_BITMAP *from, ALLEGRO_BITMAP *to,
                 int total, int i)
{
  int w = al_get_bitmap_width (from);
  int h = al_get_bitmap_height (from);
  float slice =  w / total;
  draw_bitmap_region (from, to, 0, 0, i * slice, h, 0, 0, 0);
}

void
draw_shutter (ALLEGRO_BITMAP *from, ALLEGRO_BITMAP *to,
              int total, int i)
{
  int sw = al_get_bitmap_width (from);
  int sh = al_get_bitmap_height (from);

  int sy;
  for (sy = 0; sy < sh; sy += total)
    draw_bitmap_region (from, to, 0, sy, sw, i, 0, sy, 0);
}

void
draw_pattern (ALLEGRO_BITMAP *bitmap, int ox, int oy, int w, int h,
              ALLEGRO_COLOR color_0, ALLEGRO_COLOR color_1)
{
  int x, y;
  set_target_bitmap (bitmap);
  al_lock_bitmap (bitmap, ALLEGRO_PIXEL_FORMAT_ANY,
                  ALLEGRO_LOCK_WRITEONLY);
  for (y = oy; y < oy + h; y++)
    for (x = ox; x < ox + w; x++)
      al_put_pixel (x, y, (x % 2 != y % 2) ? color_0 : color_1);
  al_unlock_bitmap (bitmap);
}

void
start_video_effect (enum video_effect_type type, int duration)
{
  ALLEGRO_BITMAP *screen = mr.cell[0][0].screen;

  video_effect.type = type;
  video_effect.duration = duration;
  clear_bitmap (effect_buffer, BLACK);
  clear_bitmap (black_screen, BLACK);
  draw_bitmap (screen, effect_buffer, 0, 0, 0);
  al_start_timer (video_timer);
}

void
stop_video_effect (void)
{
  ALLEGRO_BITMAP *screen = mr.cell[0][0].screen;

  if (! al_get_timer_started (video_timer)) return;
  video_effect.type = VIDEO_NO_EFFECT;
  al_stop_timer (video_timer);
  al_set_timer_count (video_timer, 0);
  drop_all_events_from_source
    (event_queue, get_timer_event_source (video_timer));
  clear_bitmap (screen, BLACK);
  draw_bitmap (effect_buffer, screen, 0, 0, 0);
  effect_counter = 0;
}

bool
is_video_effect_started (void)
{
  return al_get_timer_started (video_timer);
}

void
show (void)
{
  ALLEGRO_BITMAP *screen = mr.cell[0][0].screen;

  switch (video_effect.type) {
  case VIDEO_NO_EFFECT: flip_display (NULL); return;
  case VIDEO_OFF: return;
  default: break;
  }

  if (++effect_counter >= video_effect.duration + 2) {
    effect_counter = 0;
    stop_video_effect ();
    return;
  }

  switch (video_effect.type) {
  case VIDEO_FLICKERING:
    if (effect_counter % 2 && effect_counter < video_effect.duration) {
      clear_bitmap (effect_buffer, video_effect.color);
      convert_mask_to_alpha (screen, BLACK);
    } else clear_bitmap (effect_buffer, BLACK);
    draw_bitmap (screen, effect_buffer, 0, 0, 0);
    break;
  case VIDEO_FADE_IN:
    switch (vm) {
    case CGA: case EGA:
      draw_shutter (screen, effect_buffer, video_effect.duration / 4, effect_counter);
      if (effect_counter >= video_effect.duration / 4)
        effect_counter += video_effect.duration;
      break;
    case VGA:
      draw_fade (screen, effect_buffer, 1 - (float) effect_counter
                 / (float) video_effect.duration);
      break;
    }
    break;
  case VIDEO_FADE_OUT:
    switch (vm) {
    case CGA: case EGA:
      draw_shutter (black_screen, effect_buffer, video_effect.duration / 4,
                    effect_counter);
      if (effect_counter >= video_effect.duration / 4)
        effect_counter += video_effect.duration;
      break;
    case VGA:
      draw_fade (screen, effect_buffer, (float) effect_counter
                 / (float) video_effect.duration);
      break;
    }
    if (effect_counter + 1 >= video_effect.duration) clear_bitmap (effect_buffer, BLACK);
    break;
  case VIDEO_ROLL_RIGHT:
    draw_roll_right (screen, effect_buffer, video_effect.duration, effect_counter);
    break;
  default:
    error (-1, 0, "%s (void): unknown video effect type (%i)",
           __func__, video_effect.type);
  }

  flip_display (effect_buffer);
}

bool
is_fullscreen (void)
{
  return (al_get_display_flags (display) & ALLEGRO_FULLSCREEN_WINDOW)
    || (al_get_display_flags (display) & ALLEGRO_FULLSCREEN);
}

void
process_display_events (void)
{
  ALLEGRO_EVENT event;
  while (al_get_next_event (event_queue, &event))
    switch (event.type) {
    case ALLEGRO_EVENT_DISPLAY_EXPOSE:
      show ();
      break;
    case ALLEGRO_EVENT_DISPLAY_RESIZE:
      acknowledge_resize ();
      show ();
      break;
    case ALLEGRO_EVENT_DISPLAY_CLOSE:
      quit_game ();
      break;
    }
}
