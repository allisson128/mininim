/*
  undo.h -- undo module;

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

#ifndef MININIM_UNDO_H
#define MININIM_UNDO_H

void register_undo (struct undo *u, void *data, undo_f f, char *desc);
bool can_undo (struct undo *u, int dir);
bool undo_pass (struct undo *u, int dir, char **desc);
void ui_undo_pass (struct undo *u, int dir, char *prefix);

/* CON */
void register_con_undo (struct undo *u, struct pos *p,
                        enum confg fg, enum conbg bg, int ext,
                        bool destroy, bool register, bool prepare,
                        char *desc);
void con_undo (struct con_undo *d, int dir);

/* EXCHANGE POS */
void register_exchange_pos_undo (struct undo *u, struct pos *p0, struct pos *p1,
                                 bool prepare, bool invert_dir, char *desc);
void exchange_pos_undo (struct exchange_pos_undo *d, int dir);

/* ROOM */
void register_room_undo (struct undo *u, int room, struct con c[FLOORS][PLACES],
                         char *desc);
void room_undo (struct room_undo *d, int dir);

#endif	/* MININIM_UNDO_H */
