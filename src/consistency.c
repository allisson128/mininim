/*
  consistency.c -- consistency module;

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

void
fix_level (struct level *l)
{
  struct pos p; new_pos (&p, l, -1, -1, -1);
  for (p.room = 0; p.room < ROOMS; p.room++)
    for (p.floor = 0; p.floor < FLOORS; p.floor++)
      for (p.place = 0; p.place < PLACES; p.place++) {
        fix_door_lacking_opener (&p);
        fix_opener_or_closer_lacking_door (&p);
        fix_single_walls_at_place_0 (&p);
        fix_inaccessible_enclosure (&p);
        fix_loose_enclosure (&p);
        fix_rigid_con_no_floor_top (&p);
        fix_door_adjacent_to_wall_or_door (&p);
        fix_broken_floor_lacking_no_floor_on_top (&p);
        fix_skeleton_or_spikes_floor_with_no_or_loose_floor_at_left (&p);
        fix_adjacent_itens (&p);
        fix_item_on_non_normal_floor (&p);
        fix_sword_at_right_of_wall_or_door (&p);
        fix_confg_which_should_not_have_conbg (&p);
        fix_partial_big_pillar (&p);
      }
}

/* Important: the corrections bellow are all made for two perspectives
   (but just one when symmetrical), because we don't assume levels
   have consistent linking (if room A is at left from room B's
   perspective, it doesn't imply room B is necessarily at right from
   room A's perspective).  If this wasn't the case, we would get an
   inconsistent behavior, because the correction algorithms would only
   work from the perspective of the corrected room, and not
   necessairly from any other adjacent rooms.  Summarizing: without
   consistent linking the space is relative, and we shoud account for
   this. */

/* consistency: wall can't be single at place 0, otherwise the kid
   disappears from view behind it, or him might view room 0, what may
   lead to several problems, room on-demand local linking consistent
   resolution being one example */
void
fix_single_walls_at_place_0 (struct pos *p)
{
  struct con *c = con (p);
  struct con *cl = crel (p, 0, -1);
  struct con *cr = crel (p, 0, +1);

  /* wall's perspective */
  if (p->place == 0
      && c->fg == WALL
      && cl->fg != WALL) cl->fg = WALL;

  /* wall left's perspective */
  if (p->place == 9
      && c->fg != WALL
      && cr->fg == WALL) c->fg = WALL;
}

/* consistency: walls should delimit accessible places or walls */
void
fix_inaccessible_enclosure (struct pos *p)
{
  if (is_enclosure (p, is_inaccessible, LEFT)) fix_enclosure (p, LEFT);
  if (is_enclosure (p, is_inaccessible, RIGHT)) fix_enclosure (p, RIGHT);
}

/* consistency: walls delimiting only loose floors are not valid */
void
fix_loose_enclosure (struct pos *p)
{
  if (is_enclosure (p, is_loose, LEFT)) fix_enclosure (p, LEFT);
  if (is_enclosure (p, is_loose, RIGHT)) fix_enclosure (p, RIGHT);
}

/* consistency: rigid constructions (pillar, big pillar, wall, door, chopper) must
   have something non-traversable lying on it */
void
fix_rigid_con_no_floor_top (struct pos *p)
{
  struct pos pa; prel (p, &pa, -1, 0);
  struct con *ca = con (&pa);

  /* rigid construction's perspective */
  if (is_rigid_con (p) && is_traversable (&pa)) ca->fg = FLOOR;

  /* traversable's perspective */
  struct pos pb; prel (p, &pb, +1, 0);
  if (is_traversable (p)
      && is_rigid_con (&pb)) con (p)->fg = FLOOR;
}

/* consistency: doors should not face a wall nor another door */
void
fix_door_adjacent_to_wall_or_door (struct pos *p)
{
  struct con *c = con (p);
  struct con *cl = crel (p, 0, -1);
  struct con *cr = crel (p, 0, +1);

  /* DOOR's perspective*/
  if (c->fg == DOOR &&
      (cr->fg == WALL || cr->fg == DOOR
       || cl->fg == WALL || cl->fg == DOOR)) c->fg = FLOOR;

  /* WALL's perspective */
  if (c->fg == WALL
      && (cr->fg == DOOR || cl->fg == DOOR)) c->fg = FLOOR;
}

/* consistency: broken floors must have no construction above them */
void
fix_broken_floor_lacking_no_floor_on_top (struct pos *p)
{
  struct con *c = con (p);
  struct con *ca = crel (p, -1, 0);
  struct con *cb = crel (p, +1, 0);

  /* broken floor's perspective */
  if (c->fg == BROKEN_FLOOR && ca->fg != NO_FLOOR) c->fg = FLOOR;

  /* non-no-floor's perspective */
  if (c->fg != NO_FLOOR && cb->fg == BROKEN_FLOOR) cb->fg = FLOOR;
}

/* consistency: skeleton and spikes floors can't be a hangable
   construction at left because there is no corner floor sprites to
   render the perspective when the kid is climbing them */
void
fix_skeleton_or_spikes_floor_with_no_or_loose_floor_at_left (struct pos *p)
{
  struct con *c = con (p);
  struct pos ph; prel (p, &ph, +1, -1);
  struct con *cl = crel (p, +0, -1);
  struct con *ca = crel (p, -1, +0);

  /* skeleton and spike's perspective */
  if ((c->fg == SKELETON_FLOOR
       || c->fg == SPIKES_FLOOR)
      && is_hangable_pos (&ph, RIGHT))
    cl->fg = FLOOR;

  /* hangable position perspective  */
  enum confg t = get_hanged_con (p, RIGHT);
  if (is_hangable_pos (p, RIGHT)
      && (t == SKELETON_FLOOR || t == SPIKES_FLOOR))
    ca->fg = FLOOR;
}

/* consistency: itens can't be adjacent */
void
fix_adjacent_itens (struct pos *p)
{
  struct con *c = con (p);
  struct con *cl = crel (p, 0, -1);
  struct con *cr = crel (p, 0, +1);

  /* item's perspective (symmetrical) */
  if (((cl->fg == FLOOR && cl->ext.item != NO_ITEM)
       || (cr->fg == FLOOR && cr->ext.item != NO_ITEM))
      && c->fg != OPENER_FLOOR
      && c->fg != CLOSER_FLOOR) c->ext.item = NO_ITEM;
}

/* consistency: itens can only lay in normal floors */
void
fix_item_on_non_normal_floor (struct pos *p)
{
  struct con *c = con (p);

  /* non-no-floor's perspective (symmetrical) */
  if (c->fg != FLOOR
      && c->fg != OPENER_FLOOR
      && c->fg != CLOSER_FLOOR) c->ext.item = NO_ITEM;
}

/* consistency: swords can't be placed on the right of walls or
   doors */
void
fix_sword_at_right_of_wall_or_door (struct pos *p)
{
  struct con *c = con (p);
  struct con *cl = crel (p, 0, -1);
  struct con *cr = crel (p, 0, +1);

  /* sword's perspective */
  if (c->fg == FLOOR
      && c->ext.item == SWORD
      && (cl->fg == WALL
          || cl->fg == DOOR)) c->ext.item = NO_ITEM;

  /* wall and door's perspective  */
  if ((c->fg == WALL
       || c->fg == DOOR)
      && cr->fg == FLOOR
      && cr->ext.item == SWORD) c->ext.item = NO_ITEM;
}

/* consistency: doors should have an associated event and opener floor */
void
fix_door_lacking_opener (struct pos *p)
{
  int i;

  struct con *c = con (p);
  if (c->fg == DOOR
      || c->fg == LEVEL_DOOR) {
    for (i = 0; i < EVENTS; i++)
      if (peq (&p->l->event[i].p, p)
          && is_there_event_handler (p->l, i)) return;

    /* fprintf (stderr, "%s: replaced %s by %s at pos (%i, %i, %i)\n", */
    /*          __func__, "DOOR", "FLOOR", p->room, p->floor, p->place); */

    c->fg = FLOOR;
    c->bg = NO_BG;
    c->ext.item = NO_ITEM;
  }
}

/* consistency: opener and closer floors should have an associated
   event and door */
void
fix_opener_or_closer_lacking_door (struct pos *p)
{
  struct con *c = con (p);
  if (c->fg == OPENER_FLOOR
      || c->fg == CLOSER_FLOOR) {
    int i = c->ext.event;
    do {
      if (con (&p->l->event[i].p)->fg == DOOR
          || con (&p->l->event[i].p)->fg == LEVEL_DOOR) return;
    } while (p->l->event[i++].next && i < EVENTS);

    /* fprintf (stderr, "%s: replaced %s (event %i) by %s at pos (%i, %i, %i)\n", */
    /*          __func__, c->fg == OPENER_FLOOR ? "OPENER_FLOOR" : "CLOSER_FLOOR", */
    /*          c->ext.event, "FLOOR", p->room, p->floor, p->place); */

    c->fg = FLOOR;
    c->bg = NO_BG;
    c->ext.item = NO_ITEM;
  }
}

/* consistency: wall, pillars, big pillars, doors and level doors shouldn't have
   background */
void
fix_confg_which_should_not_have_conbg (struct pos *p)
{
  struct con *c = con (p);
  if (c->fg == WALL || c->fg == PILLAR
      || c->fg == BIG_PILLAR_TOP || c->fg == BIG_PILLAR_BOTTOM
      || c->fg == DOOR || c->fg == LEVEL_DOOR)
    c->bg = NO_BG;
}

/* consistency: big pillar bottom must have big pillar top over it and
   vice-versa */
void
fix_partial_big_pillar (struct pos *p)
{
  struct con *c = con (p);
  struct con *cb = crel (p, +1, +0);
  struct con *ca = crel (p, -1, +0);

  if ((c->fg == BIG_PILLAR_BOTTOM && ca->fg != BIG_PILLAR_TOP)
      || (c->fg == BIG_PILLAR_TOP && cb->fg != BIG_PILLAR_BOTTOM))
    c->fg = FLOOR;
}

void
make_links_locally_consistent (struct level *l, int prev_room, int current_room)
{
  if (roomd (l, prev_room, LEFT) == current_room)
    l->link[current_room].r = prev_room;
  else if (roomd (l, prev_room, RIGHT) == current_room)
    l->link[current_room].l = prev_room;
  else if (roomd (l, prev_room, ABOVE) == current_room)
    l->link[current_room].b = prev_room;
  else if (roomd (l, prev_room, BELOW) == current_room)
    l->link[current_room].a = prev_room;
}

bool
is_there_event_handler (struct level *l, int e)
{
  int i = 0;
  struct pos p; new_pos (&p, l, -1, -1, -1);
  for (p.room = 1; p.room < ROOMS; p.room++)
    for (p.floor = 0; p.floor < FLOORS; p.floor++)
      for (p.place = 0; p.place < PLACES; p.place++) {
        if (con (&p)->fg == OPENER_FLOOR) {
          i = con (&p)->ext.event;
          do {
            if (i == e) return true;
          } while (p.l->event[i++].next && i < EVENTS);
        }
      }
  return false;
}

void
fix_enclosure (struct pos *p, enum dir dir)
{
  int i;
  int d = (dir == LEFT) ? -1 : +1;
  struct pos q;

  for (i = d, prel (p, &q, 0, i); abs (i) < PLACES; i += d, prel (p, &q, 0, i)) {
    if (con (&q)->fg == WALL) break;
    con (&q)->fg = WALL;
    con (&q)->bg = NO_BG;
    con (&q)->ext.item = NO_ITEM;
  }
}

bool
is_enclosure (struct pos *p, bool (*pred) (struct pos *p), enum dir dir)
{
  int i;
  int d = (dir == LEFT) ? -1 : +1;
  struct pos q;

  if (con (p)->fg == WALL)
    for (i = d, prel (p, &q, 0, i); abs (i) < PLACES; i += d, prel (p, &q, 0, i)) {
      if (con (&q)->fg == WALL) {
        if (abs (i) >= 2) return true;
        else return false;
      }

      if (! pred (&q)) return false;
    }
  return false;
}

bool
is_inaccessible (struct pos *p)
{
  struct pos pa; prel (p, &pa, -1, +0);

  return ! is_traversable (p)
    && ! is_traversable (&pa);
}

bool
is_loose (struct pos *p)
{
  return con (p)->fg == LOOSE_FLOOR;
}

void
fix_legacy_room_above_zero_with_traversable_at_bottom (struct level *l)
{
  /* fix rooms above room 0 that have traversable cons at the
     bottom */
  struct pos p; new_pos (&p, l, -1, 2, -1);
  for (p.room = 1; p.room < LROOMS; p.room++)
    for (p.place = 0; p.place < PLACES; p.place++) {
      if (roomd (p.l, p.room, BELOW) != 0) continue;
      if (is_traversable (&p))
        con (&p)->fg = SPIKES_FLOOR;
    }
}



/****************
 * ROOM LINKING *
 ****************/

void
make_reciprocal_link (struct level *l, int room0, int room1, enum dir dir)
{
  link_room (l, room0, room1, dir);
  link_room (l, room1, room0, opposite_dir (dir));
}

void
make_link_locally_unique (struct level *l, int room, enum dir dir)
{
  if (dir != LEFT && roomd (l, room, LEFT) == roomd (l, room, dir))
    link_room (l, room, 0, LEFT);
  if (dir != RIGHT && roomd (l, room, RIGHT) == roomd (l, room, dir))
    link_room (l, room, 0, RIGHT);
  if (dir != ABOVE && roomd (l, room, ABOVE) == roomd (l, room, dir))
    link_room (l, room, 0, ABOVE);
  if (dir != BELOW && roomd (l, room, BELOW) == roomd (l, room, dir))
    link_room (l, room, 0, BELOW);
}

void
make_link_globally_unique (struct level *l, int room, enum dir dir)
{
  int i;
  for (i = 1; i < ROOMS; i++) {
    if (room != i && roomd (l, i, dir) == roomd (l, room, dir))
      link_room (l, i, 0, dir);
  }
}

void
make_semi_consistent_link (struct level *l, int room0, int room1, enum dir dir)
{
  make_reciprocal_link (l, room0, room1, dir);

  make_link_locally_unique (l, room0, dir);
  make_link_locally_unique (l, room1, opposite_dir (dir));

  make_link_globally_unique (l, room0, dir);
  make_link_globally_unique (l, room1, opposite_dir (dir));
}

void
make_link_adjacency_bound (struct level *l, int room, enum dir dir)
{
  int r = roomd (l, room, dir);
  int dir0 = perpendicular_dir (dir, 0);
  int dir1 = perpendicular_dir (dir, 1);

  int room0a = roomd (l, room, dir0);
  int room0b = roomd (l, r, dir0);

  int room1a = roomd (l, room, dir1);
  int room1b = roomd (l, r, dir1);

  make_semi_consistent_link (l, room0a, room0b, dir);
  make_semi_consistent_link (l, room1a, room1b, dir);
}

void
exchange_rooms  (struct level *l, int room0, int room1)
{
  int r0l = roomd (l, room0, LEFT);
  int r0r = roomd (l, room0, RIGHT);
  int r0a = roomd (l, room0, ABOVE);
  int r0b = roomd (l, room0, BELOW);

  int r0lr = roomd (l, r0l, RIGHT);
  int r0rl = roomd (l, r0r, LEFT);
  int r0ab = roomd (l, r0a, BELOW);
  int r0ba = roomd (l, r0b, ABOVE);

  int r1l = roomd (l, room1, LEFT);
  int r1r = roomd (l, room1, RIGHT);
  int r1a = roomd (l, room1, ABOVE);
  int r1b = roomd (l, room1, BELOW);

  int r1lr = roomd (l, r1l, RIGHT);
  int r1rl = roomd (l, r1r, LEFT);
  int r1ab = roomd (l, r1a, BELOW);
  int r1ba = roomd (l, r1b, ABOVE);

  link_room (l, room0, r1l == room0 ? room1 : r1l, LEFT);
  if (r1l != room0 && r1lr == room1) link_room (l, r1l, room0, RIGHT);
  link_room (l, room0, r1r == room0 ? room1 : r1r, RIGHT);
  if (r1r != room0 && r1rl == room1) link_room (l, r1r, room0, LEFT);
  link_room (l, room0, r1a == room0 ? room1 : r1a, ABOVE);
  if (r1a != room0 && r1ab == room1) link_room (l, r1a, room0, BELOW);
  link_room (l, room0, r1b == room0 ? room1 : r1b, BELOW);
  if (r1b != room0 && r1ba == room1) link_room (l, r1b, room0, ABOVE);

  link_room (l, room1, r0l == room1 ? room0 : r0l, LEFT);
  if (r0l != room1 && r0lr == room0) link_room (l, r0l, room1, RIGHT);
  link_room (l, room1, r0r == room1 ? room0 : r0r, RIGHT);
  if (r0r != room1 && r0rl == room0) link_room (l, r0r, room1, LEFT);
  link_room (l, room1, r0a == room1 ? room0 : r0a, ABOVE);
  if (r0a != room1 && r0ab == room0) link_room (l, r0a, room1, BELOW);
  link_room (l, room1, r0b == room1 ? room0 : r0b, BELOW);
  if (r0b != room1 && r0ba == room0) link_room (l, r0b, room1, ABOVE);
}
