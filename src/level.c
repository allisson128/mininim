/*
  level.c -- level module;

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

/* functions */
static void draw_level (void);
static void compute_level (void);
static void process_keys (void);
static void draw_lives (ALLEGRO_BITMAP *bitmap, struct anim *k, enum vm vm);

struct level *vanilla_level;
struct level level;

struct undo undo;

static char *undo_msg;
static int last_auto_show_time;
static ALLEGRO_TIMER *death_timer;
static bool ignore_level_cutscene;

bool no_room_drawing, game_paused, step_one_cycle;
int room_view;
int retry_level = -1;
int anti_camera_room;
int camera_follow_kid;
int auto_rem_time_1st_cycle = 24;

void
play_level (struct level *lv)
{
  char *text;
  vanilla_level = lv;
  death_timer = create_timer (1.0 / 12);

 start:
  cutscene = false;
  game_paused = false;
  level = *lv;

  if (level_module == LEGACY_LEVEL_MODULE
      || level_module == PLV_LEVEL_MODULE
      || level_module == DAT_LEVEL_MODULE)
    fix_legacy_room_above_zero_with_traversable_at_bottom ();

  register_cons ();
  register_anims ();

  stop_all_samples ();
  if (level.start) level.start ();

  anim_cycle = 0;
  last_auto_show_time = -1;
  current_kid_id = 0;

  al_stop_timer (death_timer);
  al_set_timer_count (death_timer, 0);

  if (retry_level != level.number)
    start_level_time = al_get_timer_count (play_time);

  if (! force_em) em = level.em;
  if (! force_hue) hue = level.hue;

  edit = EDIT_MAIN;
  exit_editor ();

  if (level.nominal_number >= 0) {
    xasprintf (&text, "LEVEL %i", level.nominal_number);
    draw_bottom_text (NULL, text, 0);
    al_free (text);
  }

  play_anim (draw_level, compute_level, 12);

  switch (quit_anim) {
  case NO_QUIT: break;
  case RESTART_LEVEL:
    retry_level = level.number;
    destroy_anims ();
    destroy_cons ();
    draw_bottom_text (NULL, NULL, 0);
   goto start;
  case PREVIOUS_LEVEL:
  case NEXT_LEVEL:
    destroy_anims ();
    destroy_cons ();
    int d = (quit_anim == PREVIOUS_LEVEL) ? -1 : +1;
    if (level.next_level) level.next_level (level.number + d);
    draw_bottom_text (NULL, NULL, 0);
    if (level.cutscene && ! ignore_level_cutscene) {
      cutscene_started = false;
      cutscene = true;
      stop_video_effect ();
      stop_all_samples ();
      play_anim (level.cutscene, NULL, 10);
      stop_video_effect ();
      stop_all_samples ();

      if (quit_anim == RESTART_GAME) goto restart_game;
    }
    ignore_level_cutscene = false;
    goto start;
  case RESTART_GAME:
  restart_game:
    retry_level = -1;
    destroy_anims ();
    destroy_cons ();
    draw_bottom_text (NULL, NULL, 0);
    break;
  case QUIT_GAME: break;
  case OUT_OF_TIME:
    cutscene_started = false;
    cutscene = true;
    stop_video_effect ();
    stop_all_samples ();
    play_anim (cutscene_out_of_time_anim, NULL, 10);
    stop_video_effect ();
    stop_all_samples ();
    goto restart_game;
    break;
  }
}

void
register_con_at_pos (struct pos *p)
{
  switch (con (p)->fg) {
  case LOOSE_FLOOR: register_loose_floor (p); break;
  case OPENER_FLOOR: register_opener_floor (p); break;
  case CLOSER_FLOOR: register_closer_floor (p); break;
  case SPIKES_FLOOR: register_spikes_floor (p); break;
  case DOOR: register_door (p); break;
  case LEVEL_DOOR: register_level_door (p); break;
  case CHOPPER: register_chopper (p); break;
  case MIRROR: register_mirror (p); break;
  default: break;
  }
}

void
register_room (int room)
{
  struct pos p;
  p.room = room;
  for (p.floor = 0; p.floor < FLOORS; p.floor++)
    for (p.place = 0; p.place < PLACES; p.place++)
      register_con_at_pos (&p);
}

void
register_cons (void)
{
  int room;
  for (room = 0; room < ROOMS; room++)
    register_room (room);
}

void
destroy_con_at_pos (struct pos *p)
{
  switch (con (p)->fg) {
  case LOOSE_FLOOR: remove_loose_floor (loose_floor_at_pos (p)); break;
  case OPENER_FLOOR: remove_opener_floor (opener_floor_at_pos (p)); break;
  case CLOSER_FLOOR: remove_closer_floor (closer_floor_at_pos (p)); break;
  case SPIKES_FLOOR: remove_spikes_floor (spikes_floor_at_pos (p)); break;
  case DOOR: remove_door (door_at_pos (p)); break;
  case LEVEL_DOOR: remove_level_door (level_door_at_pos (p)); break;
  case CHOPPER: remove_chopper (chopper_at_pos (p)); break;
  case MIRROR: remove_mirror (mirror_at_pos (p)); break;
  default: break;
  }
}

void
destroy_room (int room)
{
  struct pos p;
  p.room = room;
  for (p.floor = 0; p.floor < FLOORS; p.floor++)
    for (p.place = 0; p.place < PLACES; p.place++)
      destroy_con_at_pos (&p);
}

void
destroy_cons (void)
{
  destroy_array ((void **) &loose_floor, &loose_floor_nmemb);
  destroy_array ((void **) &opener_floor, &opener_floor_nmemb);
  destroy_array ((void **) &closer_floor, &closer_floor_nmemb);
  destroy_array ((void **) &spikes_floor, &spikes_floor_nmemb);
  destroy_array ((void **) &door, &door_nmemb);
  destroy_array ((void **) &level_door, &level_door_nmemb);
  destroy_array ((void **) &chopper, &chopper_nmemb);
}

void
prepare_con_at_pos (struct pos *p)
{
  if (p->room != room_view) return;

  switch (con (p)->fg) {
  case WALL: update_wall_cache (em, vm); break;
  case MIRROR: create_mirror_bitmaps (room_view, room_view); break;
  default: break;
  }

  switch (con (p)->bg) {
  case BALCONY: compute_stars_position (room_view, room_view); break;
  default: break;
  }
}

void
prepare_room (int room)
{
  if (! is_room_adjacent (room_view, room)) return;
  update_wall_cache (em, vm);
  create_mirror_bitmaps (room_view, room_view);
  compute_stars_position (room_view, room_view);
}

void
register_anims (void)
{
  /* create kid */
  struct pos kid_start_pos;
  if (is_valid_pos (&start_pos))
    kid_start_pos = start_pos;
  else kid_start_pos = level.start_pos;
  int id = create_anim (NULL, KID, &kid_start_pos, level.start_dir);
  struct anim *k = &anima[id];
  k->total_lives = total_lives;
  k->skill = skill;
  k->current_lives = total_lives;
  k->controllable = true;
  k->immortal = immortal_mode;
  k->has_sword = level.has_sword;

  /* create guards */
  int i;
  for (i = 0; i < GUARDS; i++) {
    struct guard *g = &level.guard[i];
    struct anim *a;
    int id;
    switch (g->type) {
    case NO_ANIM: default: continue;
    case KID:
      id = create_anim (NULL, KID, &g->p, g->dir);
      anima[id].shadow = true;
      break;
    case GUARD:
      id = create_anim (NULL, GUARD, &g->p, g->dir);
      break;
    case FAT_GUARD:
      id = create_anim (NULL, FAT_GUARD, &g->p, g->dir);
      break;
    case VIZIER:
      id = create_anim (NULL, VIZIER, &g->p, g->dir);
      break;
    case SKELETON:
      id = create_anim (NULL, SKELETON, &g->p, g->dir);
      break;
    case SHADOW:
      id = create_anim (NULL, SHADOW, &g->p, g->dir);
      break;
    }
    a = &anima[id];
    apply_guard_mode (a, gm);
    a->level_id = i;
    a->has_sword = true;
    a->skill = g->skill;
    a->total_lives = g->total_lives + g->skill.extra_life;
    a->current_lives = g->total_lives;
    a->style = g->style;
  }
}

void
load_level (void)
{
  load_room ();
  load_fire ();
  load_potion ();
  load_sword ();
  load_kid ();
  load_guard ();
  load_mouse ();
}

void
unload_level (void)
{
  unload_room ();
  unload_fire ();
  unload_potion ();
  unload_sword ();
  unload_kid ();
  unload_guard ();
  unload_mouse ();
}

static void
compute_level (void)
{
  size_t i;

  process_keys ();

  if (game_paused) return;

  /* this condition is necessary to honor any floor press the start
     level function might have */
  if (anim_cycle > 0) {
    unpress_opener_floors ();
    unpress_closer_floors ();
    uncross_mirrors ();
  }

  struct anim *current_kid = get_anim_by_id (current_kid_id);

  int prev_room = current_kid->f.c.room;

  for (i = 0; i < anima_nmemb; i++) {
    struct anim *a = &anima[i];
    a->splash = false;
    a->xf.b = NULL;
  }

  compute_loose_floors ();

  get_gamepad_state (&current_kid->key);

  for (i = 0; i < anima_nmemb; i++) enter_fight_logic (&anima[i]);
  for (i = 0; i < anima_nmemb; i++) leave_fight_logic (&anima[i]);
  for (i = 0; i < anima_nmemb; i++) fight_ai (&anima[i]);
  for (i = 0; i < anima_nmemb; i++) anima[i].action (&anima[i]);
  for (i = 0; i < anima_nmemb; i++) fight_mechanics (&anima[i]);

  fight_turn_controllable (current_kid);

  clear_anims_keyboard_state ();

  if (! anim_cycle) mr_center_room (room_view);

  if (current_kid->f.c.room != prev_room
      && current_kid->f.c.room != 0
      && current_kid->f.c.room != anti_camera_room
      && camera_follow_kid == current_kid->id)  {
    room_view = current_kid->f.c.room;

    if (! is_kid_visible ()) {
      mr_coord (prev_room, current_kid->f.c.xd, &mr.x, &mr.y);
      mr.room = room_view;
    }
  }

  if (level.special_events) level.special_events ();

  compute_closer_floors ();
  compute_opener_floors ();
  compute_spikes_floors ();
  compute_doors ();
  compute_level_doors ();
  compute_choppers ();
}

static void
process_keys (void)
{
  char *text = NULL;

  struct anim *current_kid = get_anim_by_id (current_kid_id);

  /* clear the keyboard buffer at the first cycle, so any key pressed
     on the title doesn't trigger any action */
  if (anim_cycle == 0) {
    memset (&key, 0, sizeof (key));
    button = -1;
  }

  /* CTRL+Z: undo */
  if (was_key_pressed (ALLEGRO_KEY_Z, 0, ALLEGRO_KEYMOD_CTRL, true))
    ui_undo_pass (&undo, -1, NULL);

  /* CTRL+Y: redo */
  if (was_key_pressed (ALLEGRO_KEY_Y, 0, ALLEGRO_KEYMOD_CTRL, true))
    ui_undo_pass (&undo, +1, NULL);

  /* ESC: pause game */
  if (step_one_cycle) {
    step_one_cycle = false;
    game_paused = true;
    al_stop_timer (play_time);
  }

  if (was_key_pressed (ALLEGRO_KEY_ESCAPE, 0, 0, true)
      || was_button_pressed (joystick_pause_button, true)) {
    if (game_paused) {
      step_one_cycle = true;
      game_paused = false;
      al_start_timer (play_time);
    } else pause_game ();
  } else if (game_paused
             && (! active_menu || ! was_menu_key_pressed ())
             && (key.keyboard.keycode || button != -1)
             && ! save_game_dialog_thread)
    unpause_game ();

  if (game_paused) anim_cycle--;

  /* R: resurrect kid */
  if (! active_menu
      && was_key_pressed (ALLEGRO_KEY_R, 0, 0, true)
      && is_kid_dead (&current_kid->f))
    kid_resurrect (current_kid);

  /* HOME: camera on kid */
  if (was_key_pressed (ALLEGRO_KEY_HOME, 0, 0, true))
    room_view = current_kid->f.c.room;

  /* A: alternate between kid and its shadows */
  if (! active_menu
      && was_key_pressed (ALLEGRO_KEY_A, 0, 0, true)) {
    do {
      current_kid = &anima[(current_kid - anima + 1) % anima_nmemb];
    } while (current_kid->type != KID || ! current_kid->controllable);
    current_kid_id = current_kid->id;
    room_view = current_kid->f.c.room;
  }

  /* K: kill enemy */
  if (! active_menu
      && was_key_pressed (ALLEGRO_KEY_K, 0, 0, true)) {
    struct anim *ke = get_anim_by_id (current_kid->enemy_id);
    if (ke) {
      ke->current_lives = 0;
      play_sample (guard_hit_sample, ke->f.c.room);
    }
  }

  /* I: enable/disable immortal mode */
  if (! active_menu
      && was_key_pressed (ALLEGRO_KEY_I, 0, 0, true)) {
    immortal_mode = ! immortal_mode;
    current_kid->immortal = immortal_mode;
    xasprintf (&text, "%s MODE", immortal_mode
               ? "IMMORTAL" : "MORTAL");
    draw_bottom_text (NULL, text, 0);
    al_free (text);
  }

  /* SHIFT+S: incremet kid current lives */
  if (was_key_pressed (ALLEGRO_KEY_S, 0, ALLEGRO_KEYMOD_SHIFT, true))
    increase_kid_current_lives (current_kid);

  /* SHIFT+T: incremet kid total lives */
  if (was_key_pressed (ALLEGRO_KEY_T, 0, ALLEGRO_KEYMOD_SHIFT, true))
    increase_kid_total_lives (current_kid);

  /* SHIFT+W: float kid */
  if (was_key_pressed (ALLEGRO_KEY_W, 0, ALLEGRO_KEYMOD_SHIFT, true))
    float_kid (current_kid);

  /* CTRL+A: restart level */
  if (was_key_pressed (ALLEGRO_KEY_A, 0, ALLEGRO_KEYMOD_CTRL, true))
    quit_anim = RESTART_LEVEL;

  /* SHIFT+L: warp to next level */
  if (was_key_pressed (ALLEGRO_KEY_L, 0, ALLEGRO_KEYMOD_SHIFT, true)) {
    ignore_level_cutscene = true;
    quit_anim = NEXT_LEVEL;
  }

  /* SHIFT+M: warp to previous level */
  if (was_key_pressed (ALLEGRO_KEY_M, 0, ALLEGRO_KEYMOD_SHIFT, true)) {
    ignore_level_cutscene = true;
    quit_anim = PREVIOUS_LEVEL;
  }

  /* C: show direct coordinates */
  if (! active_menu
      && was_key_pressed (ALLEGRO_KEY_C, 0, 0, true)) {
    int s = room_view;
    int l = roomd (room_view, LEFT);
    int r = roomd (room_view, RIGHT);
    int a = roomd (room_view, ABOVE);
    int b = roomd (room_view, BELOW);

    xasprintf (&text, "S%i L%i R%i A%i B%i", s, l, r, a, b);
    draw_bottom_text (NULL, text, 0);
    al_free (text);
  }

  /* SHIFT+C: show indirect coordinates */
  if (was_key_pressed (ALLEGRO_KEY_C, 0, ALLEGRO_KEYMOD_SHIFT, true)) {
    int a = roomd (room_view, ABOVE);
    int b = roomd (room_view, BELOW);
    int al = roomd (a, LEFT);
    int ar = roomd (a, RIGHT);
    int bl = roomd (b, LEFT);
    int br = roomd (b, RIGHT);

    xasprintf (&text, "LV%i AL%i AR%i BL%i BR%i",
               level.number, al, ar, bl, br);
    draw_bottom_text (NULL, text, 0);
    al_free (text);
  }

  /* SPACE: display remaining time */
  if (! active_menu
      && (was_key_pressed (ALLEGRO_KEY_SPACE, 0, 0, true)
          || was_button_pressed (joystick_time_button, true)))
    display_remaining_time ();

  /* +: increment and display remaining time */
  if (! active_menu
      && was_key_pressed (ALLEGRO_KEY_EQUALS, 0, ALLEGRO_KEYMOD_SHIFT, true)) {
    int t = time_limit - al_get_timer_count (play_time);
    int d = t > 60 ? -60 : -1;
    al_add_timer_count (play_time, d);
    display_remaining_time ();
  }

  /* -: decrement and display remaining time */
  if (! active_menu
      && was_key_pressed (ALLEGRO_KEY_MINUS, 0, 0, true)) {
    int t = time_limit - al_get_timer_count (play_time);
    int d = t > 60 ? +60 : +1;
    al_add_timer_count (play_time, d);
    display_remaining_time ();
  }

  /* TAB: display skill */
  if (was_key_pressed (ALLEGRO_KEY_TAB, 0, 0, true))
    display_skill (current_kid);

  /* CTRL+=: increment counter attack skill */
  if (was_key_pressed (ALLEGRO_KEY_EQUALS, 0, ALLEGRO_KEYMOD_CTRL, true)) {
    if (current_kid->skill.counter_attack_prob < 99)
      current_kid->skill.counter_attack_prob++;
    display_skill (current_kid);
  }

  /* CTRL+-: decrement counter attack skill */
  if (was_key_pressed (ALLEGRO_KEY_MINUS, 0, ALLEGRO_KEYMOD_CTRL, true)) {
    if (current_kid->skill.counter_attack_prob > -1)
      current_kid->skill.counter_attack_prob--;
    display_skill (current_kid);
  }

  /* ALT+=: increment counter defense skill */
  if (was_key_pressed (ALLEGRO_KEY_EQUALS, 0, ALLEGRO_KEYMOD_ALT, true)) {
    if (current_kid->skill.counter_defense_prob < 99)
      current_kid->skill.counter_defense_prob++;
    display_skill (current_kid);
  }

  /* ALT+-: decrement counter defense skill */
  if (was_key_pressed (ALLEGRO_KEY_MINUS, 0, ALLEGRO_KEYMOD_ALT, true)) {
    if (current_kid->skill.counter_defense_prob > -1)
      current_kid->skill.counter_defense_prob--;
    display_skill (current_kid);
  }

  /* F10: change guard mode */
  if (was_key_pressed (ALLEGRO_KEY_F10, 0, 0, true)) {
    char *gm_str = NULL;

    /* get next guard mode */
    switch (gm) {
    case ORIGINAL_GM: gm = GUARD_GM, gm_str = "GUARD"; break;
    case GUARD_GM: gm = FAT_GUARD_GM, gm_str = "FAT GUARD"; break;
    case FAT_GUARD_GM: gm = VIZIER_GM, gm_str = "VIZIER"; break;
    case VIZIER_GM: gm = SKELETON_GM, gm_str = "SKELETON"; break;
    case SKELETON_GM: gm = SHADOW_GM, gm_str = "SHADOW"; break;
    case SHADOW_GM: gm = ORIGINAL_GM, gm_str = "ORIGINAL"; break;
    }

    /* apply next guard mode */
    int i;
    for (i = 0; i < anima_nmemb; i++) apply_guard_mode (&anima[i], gm);

    xasprintf (&text, "GUARD MODE: %s", gm_str);
    draw_bottom_text (NULL, text, 0);
    al_free (text);
  }

  /* CTRL+G: save game */
  if (was_key_pressed (ALLEGRO_KEY_G, 0, ALLEGRO_KEYMOD_CTRL, true)
      && ! save_game_dialog_thread) {
    save_game_dialog_thread =
      create_thread (save_game_dialog, NULL);
    al_start_thread (save_game_dialog_thread);
    pause_game ();
  }

  /* Restart level after death */
  struct anim *k = get_anim_by_id (0);
  if (k->current_lives <= 0
      && ! game_paused) {
    al_start_timer (death_timer);
    int64_t t = al_get_timer_count (death_timer);

    if (t < 12) k->sample = NULL;

    if (t >= 12 && ! k->sample) {
      ALLEGRO_SAMPLE *sample;
      switch (k->death_reason) {
      case SHADOW_FIGHT_DEATH: sample = success_suspense_sample; break;
      case FIGHT_DEATH: sample = fight_death_sample; break;
      default: sample = death_sample; break;
      }
      k->sample = play_sample (sample, -1);
    }

    if (t < 60) {
      key.keyboard.keycode = 0;
      button = -1;
    }

    if (t >= 60) {
      if ((t < 240 || t % 12 < 8) && ! active_menu) {
        if (t >= 252 && t % 12 == 0)
          play_sample (press_key_sample, -1);
        xasprintf (&text, "Press Button to Continue");
        draw_bottom_text (NULL, text, 0);
        al_free (text);
      } else if (! active_menu) draw_bottom_text (NULL, "", 0);

      if ((key.keyboard.keycode || button != -1)
          && ! was_menu_key_pressed ())
        quit_anim = RESTART_LEVEL;
    }
  } else if (al_get_timer_started (death_timer)
             && ! game_paused) {
    al_stop_timer (death_timer);
    al_set_timer_count (death_timer, 0);
    draw_bottom_text (NULL, NULL, 0);
  }

  camera_follow_kid = (current_kid->f.c.room == room_view)
    ? current_kid->id : -1;
}

void
draw_level_room (ALLEGRO_BITMAP *bitmap, int room)
{
  int room_view_bkp = room_view;
  room_view = room;

  clear_bitmap (bitmap, BLACK);

  struct pos p;
  p.room = room_view;

  run_room_callbacks (p.room);

  for (p.floor = FLOORS; p.floor >= -1; p.floor--)
    for (p.place = -1; p.place < PLACES; p.place++) {
      draw_fire (bitmap, &p, vm);
      draw_balcony_stars (bitmap, &p, vm);
    }

  for (p.floor = FLOORS; p.floor >= 0; p.floor--)
    for (p.place = -1; p.place < PLACES; p.place++)
      draw_no_floor_selection (bitmap, &p);

  if (! no_room_drawing) draw_room (bitmap, room_view, em, vm);

  for (p.floor = FLOORS; p.floor >= -1; p.floor--)
    for (p.place = -1; p.place < PLACES; p.place++) {
      if (con (&p)->fg != MIRROR) continue;
      update_mirror_bitmap (bitmap, &p);
      if (! no_room_drawing) draw_mirror (bitmap, &p, em, vm);
    }

  /* loose_floor_fall_debug (); */

  for (p.floor = FLOORS; p.floor >= -1; p.floor--)
    for (p.place = -1; p.place < PLACES; p.place++) {
      draw_falling_loose_floor (bitmap, &p, em, vm);
    }

  draw_anims (bitmap, em, vm);

  for (p.floor = FLOORS; p.floor >= -1; p.floor--)
    for (p.place = -1; p.place < PLACES; p.place++) {
      draw_potion (bitmap, &p, em, vm);
      if (is_sword (&p)) draw_sword (bitmap, &p, vm);
    }

  room_view = room_view_bkp;
}

static void
draw_level (void)
{
  draw_multi_rooms ();

  draw_lives (uscreen, get_anim_by_id (current_kid_id), vm);

  /* automatic remaining time display */
  int rem_time = time_limit - al_get_timer_count (play_time);
  if ((rem_time % (5 * 60) == 0
       && last_auto_show_time != rem_time
       && anim_cycle > 720)
      || (auto_rem_time_1st_cycle >= 0
          && last_auto_show_time != rem_time
          && anim_cycle >= auto_rem_time_1st_cycle
          && anim_cycle <= auto_rem_time_1st_cycle + 6)
      || rem_time <= 60) {
    display_remaining_time ();
    if (rem_time <= 60 && rem_time != last_auto_show_time)
      play_sample (press_key_sample, -1);
    last_auto_show_time = rem_time;
  }
  if (rem_time <= 0) quit_anim = OUT_OF_TIME;

  if (game_paused && ! active_menu)
    draw_bottom_text (NULL, "GAME PAUSED", -1);
}

void
display_remaining_time (void)
{
  char *text;
  int t = time_limit - al_get_timer_count (play_time);
  if (t < 0) t = 0;
  int m = t / 60 + ((t % 60) ? 1 : 0);
  if (t > 60) xasprintf (&text, "%i MINUTES LEFT", m);
  else xasprintf (&text, "%i SECONDS LEFT", t);
  draw_bottom_text (NULL, text, 0);
  al_free (text);
}

void
display_skill (struct anim *k)
{
  char *text;
  struct anim *ke = get_anim_by_id (k->enemy_id);
  if (ke) xasprintf (&text, "KCA%i KCD%i A%i CA%i D%i CD%i",
                      k->skill.counter_attack_prob + 1,
                      k->skill.counter_defense_prob + 1,
                      ke->skill.attack_prob + 1,
                      ke->skill.counter_attack_prob + 1,
                      ke->skill.defense_prob + 1,
                      ke->skill.counter_defense_prob + 1);
  else xasprintf (&text, "KCA%i KCD%i",
                  k->skill.counter_attack_prob + 1,
                  k->skill.counter_defense_prob + 1);
  draw_bottom_text (NULL, text, 0);
  al_free (text);
}

static void
draw_lives (ALLEGRO_BITMAP *bitmap, struct anim *k, enum vm vm)
{
  if (al_get_timer_started (bottom_text_timer)
      && al_get_timer_count (bottom_text_timer) < BOTTOM_TEXT_DURATION)
    return;

  if (k->f.c.room == room_view) {
    draw_kid_lives (bitmap, k, vm);
    if (k->enemy_id != -1) {
      struct anim *ke = get_anim_by_id (k->enemy_id);
      if (ke) draw_guard_lives (bitmap, ke, vm);
    }
  }
}

void
pause_game (void)
{
  memset (&key, 0, sizeof (key));
  button = -1;
  game_paused = true;
  al_stop_timer (play_time);
}

void
unpause_game (void)
{
  game_paused = false;
  draw_bottom_text (NULL, NULL, 0);
  memset (&key, 0, sizeof (key));
  button = -1;
  al_start_timer (play_time);
}

/************************************/
/* NOT USED ALTERNATIVE UNDO METHOD */
/************************************/

void
apply_to_diff_pos (struct diff *d, void (*func) (struct pos *p))
{
  size_t base = ((uint8_t *) &level.con[0][0][0]) - ((uint8_t *) &level);
  size_t end = ((uint8_t *) &level.con[ROOMS - 1][FLOORS - 1][PLACES - 1])
    - ((uint8_t *) &level);

  size_t i, j;
  for (i = 0; i < d->count; i++) {
    if (d->line[i].offset < base) continue;
    if (d->line[i].offset > end) continue;

    size_t base_con = d->line[i].offset - base;

    struct pos prev_p = {-1,-1,-1};
    for (j = 0; j < d->line[i].count; j++) {
      struct pos p;
      size_t qr = (base_con + j * d->unit_size)
        / (sizeof (struct con) * PLACES * FLOORS);
      size_t rr = (base_con + j * d->unit_size)
        % (sizeof (struct con) * PLACES * FLOORS);

      size_t qf = rr / (sizeof (struct con) * PLACES);
      size_t rf = rr % (sizeof (struct con) * PLACES);

      size_t qp = rf / sizeof (struct con);

      p.room = qr;
      p.floor = qf;
      p.place = qp;

      if (! peq (&p, &prev_p)) {
        /* printf ("%s: %i,%i,%i\n", */
        /*         (func == destroy_con_at_pos) ? "destroy" : "register", */
        /*         p.room, p.floor, p.place); */
        func (&p);
        prev_p = p;
      }
    }
  }
}

void
level_undo (struct diffset *diffset, int dir, char *prefix)
{
  char *text;
  char *dir_str = (dir >= 0) ? "REDO" : "UNDO";

  bool b = can_apply_diffset (diffset, dir);

  if (undo_msg) al_free (undo_msg);

  if (! b) {
    xasprintf (&undo_msg, "NO FURTHER %s %s", prefix, dir_str);
    editor_msg (undo_msg, 24);
    return;
  }

  size_t i = diffset->current;

  apply_to_diff_pos (&diffset->diff[(dir >= 0) ? i + 1 : i], destroy_con_at_pos);
  apply_diffset_diff (diffset, &level, sizeof (level), dir, &text);
  apply_to_diff_pos (&diffset->diff[(dir >= 0) ? i + 1 : i], register_con_at_pos);
  update_wall_cache (em, vm);
  create_mirror_bitmaps (room_view, room_view);
  compute_stars_position (room_view, room_view);

  xasprintf (&undo_msg, "%s %s: %s", prefix, dir_str, text);
  editor_msg (undo_msg, 24);
}
