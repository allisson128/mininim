/*
  kid-keep-sword.c -- kid keep sword module;

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

#include "prince.h"
#include "kernel/video.h"
#include "kernel/keyboard.h"
#include "engine/anim.h"
#include "engine/physics.h"
#include "engine/door.h"
#include "engine/potion.h"
#include "engine/sword.h"
#include "engine/loose-floor.h"
#include "kid.h"

struct frameset kid_keep_sword_frameset[KID_KEEP_SWORD_FRAMESET_NMEMB];

static void init_kid_keep_sword_frameset (void);
static bool flow (struct anim *k);
static bool physics_in (struct anim *k);
static void physics_out (struct anim *k);

ALLEGRO_BITMAP *kid_keep_sword_01, *kid_keep_sword_02,
  *kid_keep_sword_03, *kid_keep_sword_04, *kid_keep_sword_05,
  *kid_keep_sword_06, *kid_keep_sword_07, *kid_keep_sword_08,
  *kid_keep_sword_09, *kid_keep_sword_10;

static void
init_kid_keep_sword_frameset (void)
{
  struct frameset frameset[KID_KEEP_SWORD_FRAMESET_NMEMB] =
    {{kid_keep_sword_01,+2,+0},{kid_keep_sword_02,-2,0},{kid_keep_sword_03,+0,0},
     {kid_keep_sword_04,+1,0},{kid_keep_sword_05,-6,0},{kid_keep_sword_06,+2,0},
     {kid_keep_sword_07,+3,0},{kid_keep_sword_08,+0,0},{kid_keep_sword_09,+3,0},
     {kid_keep_sword_10,+0,0}};

  memcpy (&kid_keep_sword_frameset, &frameset,
          KID_KEEP_SWORD_FRAMESET_NMEMB * sizeof (struct frameset));
}

void
load_kid_keep_sword (void)
{
  /* bitmaps */
  kid_keep_sword_01 = load_bitmap (KID_KEEP_SWORD_01);
  kid_keep_sword_02 = load_bitmap (KID_KEEP_SWORD_02);
  kid_keep_sword_03 = load_bitmap (KID_KEEP_SWORD_03);
  kid_keep_sword_04 = load_bitmap (KID_KEEP_SWORD_04);
  kid_keep_sword_05 = load_bitmap (KID_KEEP_SWORD_05);
  kid_keep_sword_06 = load_bitmap (KID_KEEP_SWORD_06);
  kid_keep_sword_07 = load_bitmap (KID_KEEP_SWORD_07);
  kid_keep_sword_08 = load_bitmap (KID_KEEP_SWORD_08);
  kid_keep_sword_09 = load_bitmap (KID_KEEP_SWORD_09);
  kid_keep_sword_10 = load_bitmap (KID_KEEP_SWORD_10);

  /* frameset */
  init_kid_keep_sword_frameset ();
}

void
unload_kid_keep_sword (void)
{
  al_destroy_bitmap (kid_keep_sword_01);
  al_destroy_bitmap (kid_keep_sword_02);
  al_destroy_bitmap (kid_keep_sword_03);
  al_destroy_bitmap (kid_keep_sword_04);
  al_destroy_bitmap (kid_keep_sword_05);
  al_destroy_bitmap (kid_keep_sword_06);
  al_destroy_bitmap (kid_keep_sword_07);
  al_destroy_bitmap (kid_keep_sword_08);
  al_destroy_bitmap (kid_keep_sword_09);
  al_destroy_bitmap (kid_keep_sword_10);
}

void
kid_keep_sword (struct anim *k)
{
  k->oaction = k->action;
  k->action = kid_keep_sword;
  k->f.flip = (k->f.dir == RIGHT) ?  ALLEGRO_FLIP_HORIZONTAL : 0;

  if (! flow (k)) return;
  if (! physics_in (k)) return;
  next_frame (&k->f, &k->f, &k->fo);
  physics_out (k);
}

static bool
flow (struct anim *k)
{
  if (k->oaction != kid_keep_sword) k->i = -1, k->wait = 1;

  if (k->i < 8) k->i++;
  else if (k->i == 8 && k->wait > 0) k->wait--;
  else if (k->i == 8 && k->wait == 0) k->i++, k->wait = 2;
  else if (k->i == 9 && k->wait > 0) k->wait--;
  else {
    if (k->keep_sword_fast) {
      kid_normal (k);
      k->keep_sword_fast = false;
      return false;
    }
    else {
      /* kid turn will invert kid's direction */
      k->f.dir = (k->f.dir == RIGHT) ? LEFT : RIGHT;
      kid_turn (k);
      return false;
    }
  }

  if (k->keep_sword_fast && k->i % 2 == 0) k->i++, k->wait = 0;

  k->j = 24 + k->i;

  select_frame (k, kid_keep_sword_frameset, k->i);
  if (k->i < 4)
    select_xframe (&k->xf, sword_frameset, k->j);

  if (k->i == 8 && k->wait < 1) k->fo.dx = 0;
  if (k->i == 9 && k->wait < 2) k->fo.dx = 0;
  if (k->keep_sword_fast && k->i % 2)
    k->fo.dx += kid_keep_sword_frameset[k->i - 1].dx;
  if (k->f.b == kid_sword_normal_08) k->fo.dx = +8;

  return true;
}

static bool
physics_in (struct anim *k)
{
  return true;
}

static void
physics_out (struct anim *k)
{
  /* depressible floors */
  keep_depressible_floor (k);
}
