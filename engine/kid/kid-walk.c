/*
  kid-walk.c -- kid walk module;

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

struct frameset kid_walk_frameset[KID_WALK_FRAMESET_NMEMB];

static void init_kid_walk_frameset (void);
static bool flow (struct anim *k);
static bool physics_in (struct anim *k);
static void physics_out (struct anim *k);

ALLEGRO_BITMAP *kid_walk_01, *kid_walk_02, *kid_walk_03,
  *kid_walk_04, *kid_walk_05, *kid_walk_06, *kid_walk_07,
  *kid_walk_08, *kid_walk_09, *kid_walk_10, *kid_walk_11,
  *kid_walk_12;

static void
init_kid_walk_frameset (void)
{
  struct frameset frameset[KID_WALK_FRAMESET_NMEMB] =
    {{kid_walk_01,-1,0},{kid_walk_02,-1,0},{kid_walk_03,+0,0},
     {kid_walk_04,-8,0},{kid_walk_05,-7,0},{kid_walk_06,-6,0},
     {kid_walk_07,+3,0},{kid_walk_08,-2,0},{kid_walk_09,-1,0},
     {kid_walk_10,-1,0},{kid_walk_11,-2,0},{kid_walk_12,+0,0}};

  memcpy (&kid_walk_frameset, &frameset,
          KID_WALK_FRAMESET_NMEMB * sizeof (struct frameset));
}

void
load_kid_walk (void)
{
  /* bitmaps */
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

  /* frameset */
  init_kid_walk_frameset ();
}

void
unload_kid_walk (void)
{
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
}

void
kid_walk (struct anim *k)
{
  k->oaction = k->action;
  k->action = kid_walk;
  k->f.flip = (k->f.dir == RIGHT) ? ALLEGRO_FLIP_HORIZONTAL : 0;

  if (! flow (k)) return;
  if (! physics_in (k)) return;
  next_frame (&k->f, &k->f, &k->fo);
  physics_out (k);
}

static bool
flow (struct anim *k)
{
  struct coord nc; struct pos np, pbf, pmbo;

  if (k->oaction != kid_walk) {
    survey (_bf, pos, &k->f, &nc, &pbf, &np);
    survey (_mbo, pos, &k->f, &nc, &pmbo, &np);
    if (is_traversable (&pbf)
        || con (&pbf)->fg == CLOSER_FLOOR) k->p = pmbo;
    else k->p = pbf;
    k->i = k->walk = -1;

    k->dc = dist_collision (&k->f, false, &k->ci) + 4;
    k->df = dist_fall (&k->f, false);
    k->dl = dist_con (&k->f, _bf, pos, -4, false, LOOSE_FLOOR);
    k->dcl = dist_con (&k->f, _bf, pos, -4, false, CLOSER_FLOOR);
    k->dch = dist_chopper (&k->f, false);

    k->dcd = 0;

    if (k->dc <= PLACE_WIDTH + 4 && k->ci.t != WALL) {
      k->dcd = 9;
      k->dc -= k->dcd;
    }

    if (k->dch < 4 && k->dch <= k->dl && k->dch <= k->df)
      k->misstep = true;
    else if (k->dch <= PLACE_WIDTH) {
      k->dcd = 5;
      /* k->dch -= k->dcd; */
    }

    if (k->dch <= PLACE_WIDTH) k->confg = CHOPPER;
    else if (k->df <= PLACE_WIDTH) k->confg = NO_FLOOR;
    else if (k->dl <= PLACE_WIDTH) k->confg = LOOSE_FLOOR;
    else if (k->dcl <= PLACE_WIDTH) k->confg = CLOSER_FLOOR;
    else if (k->dc <= PLACE_WIDTH + 4) k->confg = k->ci.t;
    else k->confg = FLOOR;
  }

  if (k->i == -1 && con (&k->p)->fg != LOOSE_FLOOR) {
    if (k->dc < 4) {
      kid_normal (k);
      return false;
    }

    if (! k->misstep) {
      if (k->df < 4 || k->dl < 4 || k->dcl < 4) {
        kid_misstep (k);
        return false;
      }

      if (k->dch <= PLACE_WIDTH)
        k->misstep = true;

      int dx = 0;
      if (k->dc < 10 || k->df < 10 || k->dl < 10
          || k->dcl < 10 || k->dch < 10)
        k->walk = 0, dx = 5;
      else if (k->dc < 15 || k->df < 15 || k->dl < 15
               || k->dcl < 15 || k->dch < 15)
        k->walk = 1, dx = 10;
      else if (k->dc < 22 || k->df < 22 || k->dl < 22
               || k->dcl < 22 || k->dch < 22)
        k->walk = 2, dx = 15;
      else if (k->dc < 27 || k->df < 27 || k->dl < 27
               || k->dcl < 27 || k->dch < 27)
        k->walk = 3, dx = 22;

      if (k->walk != -1 )
        place_frame (&k->f, &k->f, kid_normal_00, &k->p,
                     (k->f.dir == LEFT) ? +11 + dx + k->dcd
                     : PLACE_WIDTH + 7 - dx - k->dcd, +15);
    }
  } else if (k->i == 2 && k->walk == 0) k->i = 9;
  else if (k->i == 3 && k->walk == 1) k->i = 8;
  else if (k->i == 4 && k->walk == 2) k->i = 6;
  else if (k->i == 5 && k->walk == 3) k->i = 6;
  else if (k->i == 11){
    if (k->walk != -1) {
      /* printf ("dcd: %i\n", k->dcd); */

      if (k->confg == CHOPPER)
        place_frame (&k->f, &k->f, kid_normal_00, &k->p,
                     (k->f.dir == LEFT) ? +15
                     : PLACE_WIDTH + 3, +15);
      else place_frame (&k->f, &k->f, kid_normal_00, &k->p,
                        (k->f.dir == LEFT) ? +11 + k->dcd
                        : PLACE_WIDTH + 7 - k->dcd, +15);
    }
    kid_normal (k);
    k->misstep = false;
    return false;
  }

  select_frame (k, kid_walk_frameset, k->i + 1);

  if (k->f.b == kid_turn_frameset[3].frame) k->fo.dx = +0;

  if (k->walk == 0) {
    if (k->dc > 4 && k->df > 4 && k->dl > 4 && k->dcl > 4)
      k->fo.dx += -1;
    if (k->i == 10) k->fo.dx = +0;
  }

  if (k->walk == 1) {
    if (k->i == 9) k->fo.dx = +1;
    if (k->i == 10) k->fo.dx = +0;
  }

  if (k->walk == 2 || k->walk == 3) {
    if (k->i == 7) k->fo.dx = +3;
    if (k->i == 10) k->fo.dx = +0;
  }

  return true;
}

static bool
physics_in (struct anim *k)
{
  struct coord nc; struct pos np, pmbo, pbb;

  /* inertia */
  k->inertia = k->cinertia = 0;

  /* fall */
  survey (_mbo, pos, &k->f, &nc, &pmbo, &np);
  survey (_bb, pos, &k->f, &nc, &pbb, &np);
  if (k->walk == -1
      && ((k->i < 6 && is_strictly_traversable (&pbb))
          || (k->i >= 6 && is_strictly_traversable (&pmbo)))) {
    kid_fall (k);
    return false;
  }

  return true;
}

static void
physics_out (struct anim *k)
{
  /* depressible floors */
  if (k->walk == -1) {
    if (k->i == 6) update_depressible_floor (k, -3, -5);
    else if (k->i == 7) update_depressible_floor (k, 0, -6);
    else if (k->i == 10) update_depressible_floor (k, -4, -10);
    else keep_depressible_floor (k);
  } else keep_depressible_floor (k);
}
