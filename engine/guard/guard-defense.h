/*
  guard-defense.h -- guard defense module;

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

#ifndef MININIM_GUARD_DEFENSE_H
#define MININIM_GUARD_DEFENSE_H

/* guard */
#define GUARD_DEFENSE_01 "data/guard/defense/01.png"
#define GUARD_DEFENSE_COUNTER_ATTACK "data/guard/defense/counter-attack.png"

/* fat guard */
#define FAT_GUARD_DEFENSE_01 "data/fat-guard/defense/01.png"
#define FAT_GUARD_DEFENSE_COUNTER_ATTACK "data/fat-guard/defense/counter-attack.png"

#define GUARD_DEFENSE_FRAMESET_NMEMB 2
extern struct frameset guard_defense_frameset[GUARD_DEFENSE_FRAMESET_NMEMB];
extern struct frameset fat_guard_defense_frameset[GUARD_DEFENSE_FRAMESET_NMEMB];

void load_guard_defense (void);
void unload_guard_defense (void);
void guard_defense (struct anim *g);
struct frameset *get_guard_defense_frameset (enum anim_type t);

/* guard */
extern ALLEGRO_BITMAP *guard_defense_01, *guard_defense_counter_attack;

/* fat guard */
extern ALLEGRO_BITMAP *fat_guard_defense_01, *fat_guard_defense_counter_attack;

#endif	/* MININIM_GUARD_DEFENSE_H */
