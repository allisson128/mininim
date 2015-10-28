/*
  kid.c -- kid module;

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

#include <stdio.h>
#include <error.h>
#include "kernel/video.h"
#include "kernel/keyboard.h"
#include "anim.h"
#include "physics.h"
#include "level.h"
#include "kid.h"

/* variables */
static ALLEGRO_BITMAP *kid_normal,
  *kid_start_run_01, *kid_start_run_02, *kid_start_run_03, *kid_start_run_04,
  *kid_start_run_05, *kid_start_run_06, *kid_start_run_07,
  *kid_run_08, *kid_run_09, *kid_run_10, *kid_run_11,
  *kid_run_12, *kid_run_13, *kid_run_14,
  *kid_turn_01, *kid_turn_02, *kid_turn_03, *kid_turn_04,
  *kid_stabilize_05, *kid_stabilize_06, *kid_stabilize_07, *kid_stabilize_08,
  *kid_stop_run_01, *kid_stop_run_02, *kid_stop_run_03, *kid_stop_run_04,
  *kid_turn_run_05, *kid_turn_run_06, *kid_turn_run_07, *kid_turn_run_08,
  *kid_turn_run_09, *kid_turn_run_10, *kid_turn_run_11, *kid_turn_run_12,
  *kid_turn_run_13,
  *kid_walk_01, *kid_walk_02, *kid_walk_03, *kid_walk_04, *kid_walk_05,
  *kid_walk_06, *kid_walk_07, *kid_walk_08, *kid_walk_09, *kid_walk_10,
  *kid_walk_11, *kid_walk_12,
  *kid_jump_01, *kid_jump_02, *kid_jump_03, *kid_jump_04, *kid_jump_05, *kid_jump_06,
  *kid_jump_07, *kid_jump_08, *kid_jump_09, *kid_jump_10, *kid_jump_11, *kid_jump_12,
  *kid_jump_13, *kid_jump_14, *kid_jump_15, *kid_jump_16, *kid_jump_17, *kid_jump_18;

struct anim kid; /* kid animation object */

/* functions */
static void draw_kid_normal (void);
static void draw_kid_walk (void);
static void draw_kid_walk_min (void);
static void draw_kid_walk_short (void);
static void draw_kid_walk_long (void);
static void draw_kid_walk_max (void);
static void draw_kid_start_run (void);
static void draw_kid_stop_run (void);
static void draw_kid_run (void);
static void draw_kid_turn (void);
static void draw_kid_turn_run (void);
static void draw_kid_stabilize (void);
static void draw_kid_jump (void);
static void draw_kid_collision (void);
static void draw_kid_hit_wall (void);

void
load_kid (void)
{
  kid_normal = load_bitmap (KID_NORMAL);
  kid_start_run_01 = load_bitmap (KID_START_RUN_01);
  kid_start_run_02 = load_bitmap (KID_START_RUN_02);
  kid_start_run_03 = load_bitmap (KID_START_RUN_03);
  kid_start_run_04 = load_bitmap (KID_START_RUN_04);
  kid_start_run_05 = load_bitmap (KID_START_RUN_05);
  kid_start_run_06 = load_bitmap (KID_START_RUN_06);
  kid_start_run_07 = load_bitmap (KID_START_RUN_07);
  kid_run_08 = load_bitmap (KID_RUN_08);
  kid_run_09 = load_bitmap (KID_RUN_09);
  kid_run_10 = load_bitmap (KID_RUN_10);
  kid_run_11 = load_bitmap (KID_RUN_11);
  kid_run_12 = load_bitmap (KID_RUN_12);
  kid_run_13 = load_bitmap (KID_RUN_13);
  kid_run_14 = load_bitmap (KID_RUN_14);
  kid_turn_01 = load_bitmap (KID_TURN_01);
  kid_turn_02 = load_bitmap (KID_TURN_02);
  kid_turn_03 = load_bitmap (KID_TURN_03);
  kid_turn_04 = load_bitmap (KID_TURN_04);
  kid_stabilize_05 = load_bitmap (KID_STABILIZE_05);
  kid_stabilize_06 = load_bitmap (KID_STABILIZE_06);
  kid_stabilize_07 = load_bitmap (KID_STABILIZE_07);
  kid_stabilize_08 = load_bitmap (KID_STABILIZE_08);
  kid_stop_run_01 = load_bitmap (KID_STOP_RUN_01);
  kid_stop_run_02 = load_bitmap (KID_STOP_RUN_02);
  kid_stop_run_03 = load_bitmap (KID_STOP_RUN_03);
  kid_stop_run_04 = load_bitmap (KID_STOP_RUN_04);
  kid_turn_run_05 = load_bitmap (KID_TURN_RUN_05);
  kid_turn_run_06 = load_bitmap (KID_TURN_RUN_06);
  kid_turn_run_07 = load_bitmap (KID_TURN_RUN_07);
  kid_turn_run_08 = load_bitmap (KID_TURN_RUN_08);
  kid_turn_run_09 = load_bitmap (KID_TURN_RUN_09);
  kid_turn_run_10 = load_bitmap (KID_TURN_RUN_10);
  kid_turn_run_11 = load_bitmap (KID_TURN_RUN_11);
  kid_turn_run_12 = load_bitmap (KID_TURN_RUN_12);
  kid_turn_run_13 = load_bitmap (KID_TURN_RUN_13);
  kid_walk_01 = load_bitmap (KID_WALK_01);
  kid_walk_02 = load_bitmap (KID_WALK_02);
  kid_walk_03 = load_bitmap (KID_WALK_03);
  kid_walk_04 = load_bitmap (KID_WALK_04);
  kid_walk_05 = load_bitmap (KID_WALK_05);
  kid_walk_06 = load_bitmap (KID_WALK_06);
  kid_walk_07 = load_bitmap (KID_WALK_07);
  kid_walk_08 = load_bitmap (KID_WALK_08);
  kid_walk_09 = load_bitmap (KID_WALK_09);
  kid_walk_10 = load_bitmap (KID_WALK_10);
  kid_walk_11 = load_bitmap (KID_WALK_11);
  kid_walk_12 = load_bitmap (KID_WALK_12);
  kid_jump_01 = load_bitmap (KID_JUMP_01);
  kid_jump_02 = load_bitmap (KID_JUMP_02);
  kid_jump_03 = load_bitmap (KID_JUMP_03);
  kid_jump_04 = load_bitmap (KID_JUMP_04);
  kid_jump_05 = load_bitmap (KID_JUMP_05);
  kid_jump_06 = load_bitmap (KID_JUMP_06);
  kid_jump_07 = load_bitmap (KID_JUMP_07);
  kid_jump_08 = load_bitmap (KID_JUMP_08);
  kid_jump_09 = load_bitmap (KID_JUMP_09);
  kid_jump_10 = load_bitmap (KID_JUMP_10);
  kid_jump_11 = load_bitmap (KID_JUMP_11);
  kid_jump_12 = load_bitmap (KID_JUMP_12);
  kid_jump_13 = load_bitmap (KID_JUMP_13);
  kid_jump_14 = load_bitmap (KID_JUMP_14);
  kid_jump_15 = load_bitmap (KID_JUMP_15);
  kid_jump_16 = load_bitmap (KID_JUMP_16);
  kid_jump_17 = load_bitmap (KID_JUMP_17);
  kid_jump_18 = load_bitmap (KID_JUMP_18);

  kid.room = 1;
  kid.x = 230;
  /* kid.x = 10; */
  kid.y = 15 + 63 * 0;
  kid.frame = kid_normal;
  kid.dir = LEFT;
  kid.collision = draw_kid_collision;

  kid.draw = draw_kid_normal;
}

void
unload_kid (void)
{
  /* bitmaps */
  al_destroy_bitmap (kid_normal);
  al_destroy_bitmap (kid_start_run_01);
  al_destroy_bitmap (kid_start_run_02);
  al_destroy_bitmap (kid_start_run_03);
  al_destroy_bitmap (kid_start_run_04);
  al_destroy_bitmap (kid_start_run_05);
  al_destroy_bitmap (kid_start_run_06);
  al_destroy_bitmap (kid_start_run_07);
  al_destroy_bitmap (kid_run_08);
  al_destroy_bitmap (kid_run_09);
  al_destroy_bitmap (kid_run_10);
  al_destroy_bitmap (kid_run_11);
  al_destroy_bitmap (kid_run_12);
  al_destroy_bitmap (kid_run_13);
  al_destroy_bitmap (kid_run_14);
  al_destroy_bitmap (kid_turn_01);
  al_destroy_bitmap (kid_turn_02);
  al_destroy_bitmap (kid_turn_03);
  al_destroy_bitmap (kid_turn_04);
  al_destroy_bitmap (kid_stabilize_05);
  al_destroy_bitmap (kid_stabilize_06);
  al_destroy_bitmap (kid_stabilize_07);
  al_destroy_bitmap (kid_stabilize_08);
  al_destroy_bitmap (kid_stop_run_01);
  al_destroy_bitmap (kid_stop_run_02);
  al_destroy_bitmap (kid_stop_run_03);
  al_destroy_bitmap (kid_stop_run_04);
  al_destroy_bitmap (kid_turn_run_05);
  al_destroy_bitmap (kid_turn_run_06);
  al_destroy_bitmap (kid_turn_run_07);
  al_destroy_bitmap (kid_turn_run_08);
  al_destroy_bitmap (kid_turn_run_09);
  al_destroy_bitmap (kid_turn_run_10);
  al_destroy_bitmap (kid_turn_run_11);
  al_destroy_bitmap (kid_turn_run_12);
  al_destroy_bitmap (kid_turn_run_13);
  al_destroy_bitmap (kid_walk_01);
  al_destroy_bitmap (kid_walk_02);
  al_destroy_bitmap (kid_walk_03);
  al_destroy_bitmap (kid_walk_04);
  al_destroy_bitmap (kid_walk_05);
  al_destroy_bitmap (kid_walk_06);
  al_destroy_bitmap (kid_walk_07);
  al_destroy_bitmap (kid_walk_08);
  al_destroy_bitmap (kid_walk_09);
  al_destroy_bitmap (kid_walk_10);
  al_destroy_bitmap (kid_walk_11);
  al_destroy_bitmap (kid_walk_12);
  al_destroy_bitmap (kid_jump_01);
  al_destroy_bitmap (kid_jump_02);
  al_destroy_bitmap (kid_jump_03);
  al_destroy_bitmap (kid_jump_04);
  al_destroy_bitmap (kid_jump_05);
  al_destroy_bitmap (kid_jump_06);
  al_destroy_bitmap (kid_jump_07);
  al_destroy_bitmap (kid_jump_08);
  al_destroy_bitmap (kid_jump_09);
  al_destroy_bitmap (kid_jump_10);
  al_destroy_bitmap (kid_jump_11);
  al_destroy_bitmap (kid_jump_12);
  al_destroy_bitmap (kid_jump_13);
  al_destroy_bitmap (kid_jump_14);
  al_destroy_bitmap (kid_jump_15);
  al_destroy_bitmap (kid_jump_16);
  al_destroy_bitmap (kid_jump_17);
  al_destroy_bitmap (kid_jump_18);
}

static
void draw_kid_normal ()
{
  kid.draw = draw_kid_normal;
  kid.flip = (kid.dir == RIGHT) ? ALLEGRO_FLIP_HORIZONTAL : 0;

  bool turn = ((kid.dir == RIGHT) && left_key) || ((kid.dir == LEFT) && right_key);
  bool run = ((kid.dir == RIGHT) && right_key) || ((kid.dir == LEFT) && left_key);
  bool walk = ((kid.dir == RIGHT) && right_key && shift_key)
    || ((kid.dir == LEFT) && left_key && shift_key);
  bool jump = ((kid.dir == RIGHT) && right_key && up_key)
    || ((kid.dir == LEFT) && left_key && up_key);

  struct level_pos pos = anim_pos (kid);
  enum level_object obj =
    level_obj_rel (pos, 0, (kid.dir == LEFT) ? -1 : 1);
  unsigned int dist = obj_dist (kid);

  if (a_key) kid.x--;
  if (d_key) kid.x++;
  if (w_key) kid.y--;
  if (s_key) kid.y++;

  unsigned int qx = kid.x / 32;
  unsigned int rx = kid.x % 32;
  printf ("floor = %u, place = %u, qx = %u, rx = %u, dist = %u\n",
          pos.floor, pos.place, qx, rx, dist);

  /* comming from stabilize */
  if (kid.frame == kid_stabilize_08) {
    if (turn) draw_kid_turn ();
    else draw_anim (&kid, kid_normal, +2, 0);
  }
  /* comming from walk */
  else if (kid.frame == kid_walk_12)
    draw_anim (&kid, kid_normal, +0, 0);
  /* comming from jump */
  else if (kid.frame == kid_jump_18)
    draw_anim (&kid, kid_normal, -4, 0);
  /* comming from normal */
  else if (kid.frame == kid_normal) {
    if (jump) draw_kid_jump ();
    else if (turn) draw_kid_turn ();
    else if (walk) draw_kid_walk ();
    else if (run)
      if (obj == WALL && dist < 32) draw_kid_walk ();
      else draw_kid_start_run ();
    else draw_anim (&kid, kid_normal, +0, 0);
  } else
    error (-1, 0, "%s: unknown kid frame (%p)", __func__, kid.frame);
}

static
void draw_kid_walk ()
{
  kid.draw = draw_kid_walk;
  kid.flip = (kid.dir == RIGHT) ? ALLEGRO_FLIP_HORIZONTAL : 0;

  struct level_pos pos = anim_pos (kid);
  enum level_object obj =
    level_obj_rel (pos, 0, (kid.dir == LEFT) ? -1 : 1);
  unsigned int dist = obj_dist (kid);

  if (obj == WALL && dist == 0) {
    draw_anim (&kid, kid_normal, +0, 0);
    kid.draw = draw_kid_normal;
    return;
  }

  /* comming from turn */
  if (kid.frame == kid_turn_04)
    draw_anim (&kid, kid_walk_01, +0, 0);
  /* comming from stabilize */
  else if (kid.frame == kid_stabilize_05)
    draw_anim (&kid, kid_walk_01, +2, 0);
  else if (kid.frame == kid_stabilize_06)
    draw_anim (&kid, kid_walk_01, +6, 0);
  else if (kid.frame == kid_stabilize_07)
    draw_anim (&kid, kid_walk_01, +3, 0);
  else if (kid.frame == kid_stabilize_08)
    draw_anim (&kid, kid_walk_01, +0, 0);
  /* comming from normal */
  else if (kid.frame == kid_normal)
    draw_anim (&kid, kid_walk_01, +0, 0);
  /* comming from walk */
  else if (kid.frame == kid_walk_01) {
    if (obj == WALL) {
      if (dist < 8) draw_kid_walk_min ();
      else if (dist < 15) draw_kid_walk_short ();
      else if (dist < 23) draw_kid_walk_long ();
      else draw_kid_walk_max ();
    } else draw_kid_walk_max ();
  } else error (-1, 0, "%s: unknown frame (%p)", __func__, kid.frame);
}

static void
draw_kid_walk_min (void)
{
  kid.draw = draw_kid_walk_min;
  kid.flip = (kid.dir == RIGHT) ?  ALLEGRO_FLIP_HORIZONTAL : 0;

  /* comming from walk */
  if (kid.frame == kid_walk_01)
    draw_anim (&kid, kid_walk_02, -1, 0);
  else if (kid.frame == kid_walk_02)
    draw_anim (&kid, kid_walk_03, -2, 0);
  else if (kid.frame == kid_walk_03) {
    kid.x += (kid.dir == LEFT) ? -32 : +32;
    while (is_colliding (kid)) kid.x += (kid.dir == LEFT) ? +1 : -1;
    draw_anim (&kid, kid_walk_11, 0, 0);
  } else if (kid.frame == kid_walk_11) {
    draw_anim (&kid, kid_walk_12, +0, 0);
    kid.draw = draw_kid_normal;
    kid.collision = draw_kid_collision;
  } else error (-1, 0, "%s: unknown frame (%p)", __func__, kid.frame);
}

static void
draw_kid_walk_short (void)
{
  kid.draw = draw_kid_walk_short;
  kid.flip = (kid.dir == RIGHT) ?  ALLEGRO_FLIP_HORIZONTAL : 0;

  /* comming from walk */
  if (kid.frame == kid_walk_01)
    draw_anim (&kid, kid_walk_02, -1, 0);
  else if (kid.frame == kid_walk_02)
    draw_anim (&kid, kid_walk_03, -2, 0);
  else if (kid.frame == kid_walk_03) {
    kid.x += (kid.dir == LEFT) ? -32 : +32;
    while (is_colliding (kid)) kid.x += (kid.dir == LEFT) ? +1 : -1;
    draw_anim (&kid, kid_walk_04, 0, 0);
  } else if (kid.frame == kid_walk_04)
    draw_anim (&kid, kid_walk_09, +0, 0);
  else if (kid.frame == kid_walk_09)
    draw_anim (&kid, kid_walk_10, +0, 0);
  else if (kid.frame == kid_walk_10)
    draw_anim (&kid, kid_walk_11, +0, 0);
  else if (kid.frame == kid_walk_11) {
    draw_anim (&kid, kid_walk_12, +0, 0);
    kid.draw = draw_kid_normal;
  } else error (-1, 0, "%s: unknown kid frame (%p)", __func__, kid.frame);
}


static void
draw_kid_walk_long (void)
{
  kid.draw = draw_kid_walk_long;
  kid.flip = (kid.dir == RIGHT) ?  ALLEGRO_FLIP_HORIZONTAL : 0;

  /* comming from walk */
  if (kid.frame == kid_walk_01)
    draw_anim (&kid, kid_walk_02, -1, 0);
  else if (kid.frame == kid_walk_02)
    draw_anim (&kid, kid_walk_03, -2, 0);
  else if (kid.frame == kid_walk_03)
    draw_anim (&kid, kid_walk_04, -8, 0);
  else if (kid.frame == kid_walk_04) {
    kid.x += (kid.dir == LEFT) ? -32 : +32;
    while (is_colliding (kid)) kid.x += (kid.dir == LEFT) ? +1 : -1;
    draw_anim (&kid, kid_walk_05, 0, 0);
  } else if (kid.frame == kid_walk_05)
    draw_anim (&kid, kid_walk_09, +0, 0);
  else if (kid.frame == kid_walk_09)
    draw_anim (&kid, kid_walk_10, +0, 0);
  else if (kid.frame == kid_walk_10)
    draw_anim (&kid, kid_walk_11, +0, 0);
  else if (kid.frame == kid_walk_11) {
    draw_anim (&kid, kid_walk_12, +0, 0);
    kid.draw = draw_kid_normal;
  } else error (-1, 0, "%s: unknown kid frame (%p)", __func__, kid.frame);
}

static
void draw_kid_walk_max ()
{
  kid.draw = draw_kid_walk_max;
  kid.flip = (kid.dir == RIGHT) ? ALLEGRO_FLIP_HORIZONTAL : 0;

  struct level_pos pos = anim_pos (kid);
  enum level_object obj =
    level_obj_rel (pos, 0, (kid.dir == LEFT) ? -1 : 1);
  unsigned int dist = obj_dist (kid);

  /* comming from walk */
  if (kid.frame == kid_walk_01)
    draw_anim (&kid, kid_walk_02, -1, 0);
  else if (kid.frame == kid_walk_02)
    draw_anim (&kid, kid_walk_03, -2, 0);
  else if (kid.frame == kid_walk_03)
    draw_anim (&kid, kid_walk_04, -8, 0);
  else if (kid.frame == kid_walk_04)
    draw_anim (&kid, kid_walk_05, -9, 0);
  else if (kid.frame == kid_walk_05)
    draw_anim (&kid, kid_walk_06, -6, 0);
  else if (kid.frame == kid_walk_06)
    draw_anim (&kid, kid_walk_07, +1, 0);
  else if (kid.frame == kid_walk_07)
    draw_anim (&kid, kid_walk_08, +1, 0);
  else if (kid.frame == kid_walk_08)
    draw_anim (&kid, kid_walk_09, -1, 0);
  else if (kid.frame == kid_walk_09)
    draw_anim (&kid, kid_walk_10, +0, 0);
  else if (kid.frame == kid_walk_10)
    draw_anim (&kid, kid_walk_11, +0, 0);
  else if (kid.frame == kid_walk_11) {
    if (obj == WALL  && dist < 3)
      kid.x += (kid.dir == LEFT) ? -dist : dist;
    draw_anim (&kid, kid_walk_12, +0, 0);
    kid.draw = draw_kid_normal;
  }
  else error (-1, 0, "%s: unknown kid frame (%p)", __func__, kid.frame);
}

static
void draw_kid_start_run ()
{
  kid.draw = draw_kid_start_run;
  kid.flip = (kid.dir == RIGHT) ? ALLEGRO_FLIP_HORIZONTAL : 0;

  bool run = (kid.dir == RIGHT) ? right_key : left_key;
  bool turn_run = (kid.dir == RIGHT) ? left_key : right_key;

  /* comming from normal */
  if (kid.frame == kid_normal)
    draw_anim (&kid, kid_start_run_01, -1, 0);
  /* comming from turn */
  else if (kid.frame == kid_turn_04)
    draw_anim (&kid, kid_start_run_01, +0, 0);
  /* comming from stabilize */
  else if (kid.frame == kid_stabilize_05)
    draw_anim (&kid, kid_start_run_01, +2, 0);
  else if (kid.frame == kid_stabilize_06)
    draw_anim (&kid, kid_start_run_01, +6, 0);
  else if (kid.frame == kid_stabilize_07)
    draw_anim (&kid, kid_start_run_01, +4, 0);
  else if (kid.frame == kid_stabilize_08)
    draw_anim (&kid, kid_start_run_01, +0, 0);
  /* comming from start run */
  else if (kid.frame == kid_start_run_01)
    draw_anim (&kid, kid_start_run_02, -1, 0);
  else if (kid.frame == kid_start_run_02)
    draw_anim (&kid, kid_start_run_03, -4, 0);
  else if (kid.frame == kid_start_run_03)
    draw_anim (&kid, kid_start_run_04, -3, 0);
  else if (kid.frame == kid_start_run_04) {
    draw_anim (&kid, kid_start_run_05, -8, 0);
    if (turn_run) kid.draw = draw_kid_stop_run;
  } else if (kid.frame == kid_start_run_05) {
    draw_anim (&kid, kid_start_run_06, -8, 0);
    if (turn_run) kid.draw = draw_kid_stop_run;
  } else if (kid.frame == kid_start_run_06) {
    draw_anim (&kid, kid_start_run_07, -7, 0);
    kid.draw = run ? draw_kid_run : draw_kid_stop_run;
  } else error (-1, 0, "%s: unknown kid frame (%p)", __func__, kid.frame);
}

static
void draw_kid_stop_run ()
{
  kid.draw = draw_kid_stop_run;
  kid.flip = (kid.dir == RIGHT) ? ALLEGRO_FLIP_HORIZONTAL : 0;

  bool turn_run = (kid.dir == RIGHT) ? left_key : right_key;

  struct level_pos pos = anim_pos (kid);
  enum level_object obj =
    level_obj_rel (pos, 0, (kid.dir == LEFT) ? -1 : 1);
  unsigned int dist = obj_dist (kid);

  /* comming from start run or run */
  if (kid.frame == kid_start_run_05 || kid.frame == kid_start_run_06
      || kid.frame == kid_start_run_07
      || kid.frame == kid_run_08 || kid.frame == kid_run_09
      || kid.frame == kid_run_10 || kid.frame == kid_run_11
      || kid.frame == kid_run_12 || kid.frame == kid_run_13
      || kid.frame == kid_run_14)
    draw_anim (&kid, kid_stop_run_01, +0, 0);
  /* comming from stop run */
  else if (kid.frame == kid_stop_run_01)
    draw_anim (&kid, kid_stop_run_02, -6, 0);
  else if (kid.frame == kid_stop_run_02) {
    if (obj == WALL && dist < 16) {
      draw_anim (&kid, kid_stop_run_04, -5, 0);
      kid.draw = turn_run ? draw_kid_turn_run : draw_kid_stabilize;
    } else draw_anim (&kid, kid_stop_run_03, -16, 0);
  } else if (kid.frame == kid_stop_run_03) {
    draw_anim (&kid, kid_stop_run_04, +0, 0);
    kid.draw = turn_run ? draw_kid_turn_run : draw_kid_stabilize;
  } else error (-1, 0, "%s: unknown kid frame (%p)", __func__,  kid.frame);
}

static
void draw_kid_run ()
{
  kid.draw = draw_kid_run;
  kid.flip = (kid.dir == RIGHT) ? ALLEGRO_FLIP_HORIZONTAL : 0;

  bool stop = ! ((kid.dir == RIGHT) ? right_key : left_key);

  /* comming from turn run */
  if (kid.frame == kid_turn_run_13)
    draw_anim (&kid, kid_run_13, -4, 0);
  /* comming from start run or the last frame of run */
  else if (kid.frame == kid_start_run_07)
    draw_anim (&kid, kid_run_08, -10, 0);
  /* comming from run */
  else if (kid.frame == kid_run_08)
    draw_anim (&kid, kid_run_09, -3, 0);
  else if (kid.frame == kid_run_09)
    draw_anim (&kid, kid_run_10, -5, 0);
  else if (kid.frame == kid_run_10)
    draw_anim (&kid, kid_run_11, -9, 0);
  else if (kid.frame == kid_run_11)
    draw_anim (&kid, kid_run_12, -11, 0);
  else if (kid.frame == kid_run_12)
    draw_anim (&kid, kid_run_13, -4, 0);
  else if (kid.frame == kid_run_13)
    draw_anim (&kid, kid_run_14, -8, 0);
  else if (kid.frame == kid_run_14)
    draw_anim (&kid, kid_start_run_07, -9, 0);
  else
    error (-1, 0, "%s: unknown kid frame (%p)", __func__, kid.frame);

  if (stop) kid.draw = draw_kid_stop_run;
}

static
void draw_kid_turn ()
{
  kid.draw = draw_kid_turn;
  kid.flip = (kid.dir == RIGHT) ? 0 : ALLEGRO_FLIP_HORIZONTAL;

  bool run = (kid.dir == RIGHT) ? left_key : right_key;
  bool walk = run && shift_key;
  bool jump = ((kid.dir == RIGHT) && left_key && up_key)
    || ((kid.dir == LEFT) && right_key && up_key);
  static bool turn = false;
  if (! turn) turn = ((kid.dir == RIGHT) && right_key)
                || ((kid.dir == LEFT) && left_key);

  /* comming from normal */
  if (kid.frame == kid_normal)
    draw_anim (&kid, kid_turn_01, +2, 0);
  /* comming from stabilize */
  else if (kid.frame == kid_stabilize_05)
    draw_anim (&kid, kid_turn_01, +6, 0);
  else if (kid.frame == kid_stabilize_06)
    draw_anim (&kid, kid_turn_01, +11, 0);
  else if (kid.frame == kid_stabilize_07)
    draw_anim (&kid, kid_turn_01, +8, 0);
  else if (kid.frame == kid_stabilize_08)
    draw_anim (&kid, kid_turn_01, +4, 0);
  /* comming from turn */
  else if (kid.frame == kid_turn_04)
    draw_anim (&kid, kid_turn_01, +3, 0);
  else if (kid.frame == kid_turn_01)
    draw_anim (&kid, kid_turn_02, +1, 0);
  else if (kid.frame == kid_turn_02)
    draw_anim (&kid, kid_turn_03, -3, 0);
  else if (kid.frame == kid_turn_03) {
    draw_anim (&kid, kid_turn_04, +0, 0);
    kid.dir = (kid.dir == RIGHT) ? LEFT : RIGHT;
    if (turn) {
      kid.draw = draw_kid_turn;
      turn = false;
    }
    else if (jump) kid.draw = draw_kid_jump;
    else if (walk) kid.draw = draw_kid_walk;
    else if (run) kid.draw = draw_kid_start_run;
    else kid.draw = draw_kid_stabilize;
  } else error (-1, 0, "%s: unknown kid frame (%p)", __func__, kid.frame);
}

static
void draw_kid_turn_run ()
{
  kid.draw = draw_kid_turn_run;
  kid.flip = (kid.dir == RIGHT) ?  ALLEGRO_FLIP_HORIZONTAL : 0;

  /* comming from stop run */
  if (kid.frame == kid_stop_run_04)
    draw_anim (&kid, kid_turn_run_05, -18, 0);
  /* comming from turn run */
  else if (kid.frame == kid_turn_run_05)
    draw_anim (&kid, kid_turn_run_06, -6, 0);
  else if (kid.frame == kid_turn_run_06)
    draw_anim (&kid, kid_turn_run_07, -4, 0);
  else if (kid.frame == kid_turn_run_07)
    draw_anim (&kid, kid_turn_run_08, +2, 0);
  else if (kid.frame == kid_turn_run_08)
    draw_anim (&kid, kid_turn_run_09, -6, 0);
  else if (kid.frame == kid_turn_run_09)
    draw_anim (&kid, kid_turn_run_10, +3, 0);
  else if (kid.frame == kid_turn_run_10)
    draw_anim (&kid, kid_turn_run_11, -1, 0);
  else if (kid.frame == kid_turn_run_11)
    draw_anim (&kid, kid_turn_run_12, +0, 0);
  else if (kid.frame == kid_turn_run_12) {
    draw_anim (&kid, kid_turn_run_13, +4, 0);
    kid.dir = (kid.dir == RIGHT) ? LEFT : RIGHT;
    kid.draw = draw_kid_run;
  } else error (-1, 0, "%s: unknown kid frame (%p)", __func__, kid.frame);
}

static
void draw_kid_stabilize ()
{
  kid.draw = draw_kid_stabilize;
  kid.flip = (kid.dir == RIGHT) ?  ALLEGRO_FLIP_HORIZONTAL : 0;

  bool turn = ((kid.dir == RIGHT) && left_key) || ((kid.dir == LEFT) && right_key);
  bool run = ((kid.dir == RIGHT) && right_key) || ((kid.dir == LEFT) && left_key);
  bool walk = ((kid.dir == RIGHT) && right_key && shift_key)
    || ((kid.dir == LEFT) && left_key && shift_key);
  bool jump = ((kid.dir == RIGHT) && right_key && up_key)
    || ((kid.dir == LEFT) && left_key && up_key);

  struct level_pos pos = anim_pos (kid);
  enum level_object obj =
    level_obj_rel (pos, 0, (kid.dir == LEFT) ? -1 : 1);
  unsigned int dist = obj_dist (kid);

  kid.collision = NULL;

  /* comming from stop run */
  if (kid.frame == kid_stop_run_04)
    draw_anim (&kid, kid_stabilize_05, -5, 0);
  /* comming from turn */
  else if (kid.frame == kid_turn_04)
    draw_anim (&kid, kid_stabilize_05, -3, 0);
  /* comming from stabilize */
  else if (kid.frame == kid_stabilize_05)
    draw_anim (&kid, kid_stabilize_06, -5, 0);
  else if (kid.frame == kid_stabilize_06)
    draw_anim (&kid, kid_stabilize_07, +3, 0);
  else if (kid.frame == kid_stabilize_07) {
    draw_anim (&kid, kid_stabilize_08, +4, 0);
    kid.draw = draw_kid_normal;
  } else error (-1, 0, "%s: unknown kid frame (%p)", __func__, kid.frame);

  kid.collision = draw_kid_collision;

  if (jump) kid.draw = draw_kid_jump;
  else if (turn) kid.draw = draw_kid_turn;
  else if (walk) kid.draw = draw_kid_walk;
  else if (run) {
    if (obj == WALL && dist < 32) kid.draw = draw_kid_walk;
    else kid.draw = draw_kid_start_run;
  }

}

static void
draw_kid_jump (void)
{
  kid.draw = draw_kid_jump;
  kid.flip = (kid.dir == RIGHT) ?  ALLEGRO_FLIP_HORIZONTAL : 0;

  /* comming from normal */
  if (kid.frame == kid_normal)
    draw_anim (&kid, kid_jump_01, +2, 0);
  /* comming from turn */
  else if (kid.frame == kid_turn_04)
    draw_anim (&kid, kid_jump_01, +2, 0);
  /* comming from stabilize */
  else if (kid.frame == kid_stabilize_05)
    draw_anim (&kid, kid_jump_01, +2, 0);
  else if (kid.frame == kid_stabilize_06)
    draw_anim (&kid, kid_jump_01, +6, 0);
  else if (kid.frame == kid_stabilize_07)
    draw_anim (&kid, kid_jump_01, +4, 0);
  else if (kid.frame == kid_stabilize_08)
    draw_anim (&kid, kid_jump_01, +0, 0);
  /* comming from jump */
  else if (kid.frame == kid_jump_01)
    draw_anim (&kid, kid_jump_02, -2, 0);
  else if (kid.frame == kid_jump_02)
    draw_anim (&kid, kid_jump_03, -4, 0);
  else if (kid.frame == kid_jump_03)
    draw_anim (&kid, kid_jump_04, -5, 0);
  else if (kid.frame == kid_jump_04)
    draw_anim (&kid, kid_jump_05, -3, 0);
  else if (kid.frame == kid_jump_05)
    draw_anim (&kid, kid_jump_06, -6, 0);
  else if (kid.frame == kid_jump_06)
    draw_anim (&kid, kid_jump_07, -3, 0);
  else if (kid.frame == kid_jump_07)
    draw_anim (&kid, kid_jump_08, -17, 0);
  else if (kid.frame == kid_jump_08)
    draw_anim (&kid, kid_jump_09, -21, 0);
  else if (kid.frame == kid_jump_09)
    draw_anim (&kid, kid_jump_10, -11, -6);
  else if (kid.frame == kid_jump_10)
    draw_anim (&kid, kid_jump_11, -2, +6);
  else if (kid.frame == kid_jump_11)
    draw_anim (&kid, kid_jump_12, -9, 0);
  else if (kid.frame == kid_jump_12)
    draw_anim (&kid, kid_jump_13, +6, 0);
  else if (kid.frame == kid_jump_13)
    draw_anim (&kid, kid_jump_14, -11, 0);
  else if (kid.frame == kid_jump_14)
    draw_anim (&kid, kid_jump_15, +0, 0);
  else if (kid.frame == kid_jump_15)
    draw_anim (&kid, kid_jump_16, +0, 0);
  else if (kid.frame == kid_jump_16)
    draw_anim (&kid, kid_jump_17, +1, 0);
  else if (kid.frame == kid_jump_17) {
    draw_anim (&kid, kid_jump_18, +0, 0);
    kid.draw = draw_kid_normal;
  } else error (-1, 0, "%s: unknown kid frame (%p)", __func__, kid.frame);
}

static void
draw_kid_collision (void)
{
  kid.draw = draw_kid_collision;
  kid.flip = (kid.dir == RIGHT) ?  ALLEGRO_FLIP_HORIZONTAL : 0;

  if (kid.odraw == draw_kid_walk_max)
    draw_kid_walk_max ();
  else if (kid.odraw == draw_kid_turn_run)
    draw_kid_turn_run ();
  else draw_kid_hit_wall ();
}

static void
draw_kid_hit_wall (void)
{
  kid.draw = draw_kid_hit_wall;
  kid.flip = (kid.dir == RIGHT) ?  ALLEGRO_FLIP_HORIZONTAL : 0;

  if (kid.frame == kid_walk_01)
    draw_anim (&kid, kid_stabilize_06, +0, 0);
  else if (kid.frame == kid_stabilize_06)
    draw_anim (&kid, kid_stabilize_07, +3, 0);
  else if (kid.frame == kid_stabilize_07) {
    draw_anim (&kid, kid_stabilize_08, +4, 0);
    kid.draw = draw_kid_normal;
  } else
    draw_anim (&kid, kid_walk_01, +0, 0);

}
