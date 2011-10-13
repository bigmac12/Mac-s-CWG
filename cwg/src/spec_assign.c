/* ************************************************************************
*   File: spec_assign.c                                 Part of CircleMUD *
*  Usage: Functions to assign function pointers to objs/mobs/rooms        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"


/* external globals */
extern int mini_mud;

/* external functions */
SPECIAL(dump);
SPECIAL(pet_shops);
SPECIAL(postmaster);
SPECIAL(receptionist);
SPECIAL(cryogenicist);
SPECIAL(mayor);
SPECIAL(bank);
void assign_kings_castle(void);

/* local functions */
void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);
void ASSIGNROOM(room_vnum room, SPECIAL(fname));
void ASSIGNMOB(mob_vnum mob, SPECIAL(fname));
void ASSIGNOBJ(obj_vnum obj, SPECIAL(fname));

/* functions to perform assignments */

void ASSIGNMOB(mob_vnum mob, SPECIAL(fname))
{
  mob_rnum rnum;

  if ((rnum = real_mobile(mob)) != NOBODY)
    mob_index[rnum].func = fname;
  else if (!mini_mud)
    log("SYSERR: Attempt to assign spec to non-existant mob #%d", mob);
}

void ASSIGNOBJ(obj_vnum obj, SPECIAL(fname))
{
  obj_rnum rnum;

  if ((rnum = real_object(obj)) != NOTHING)
    obj_index[rnum].func = fname;
  else if (!mini_mud)
    log("SYSERR: Attempt to assign spec to non-existant obj #%d", obj);
}

void ASSIGNROOM(room_vnum room, SPECIAL(fname))
{
  room_rnum rnum;

  if ((rnum = real_room(room)) != NOWHERE)
    world[rnum].func = fname;
  else if (!mini_mud)
    log("SYSERR: Attempt to assign spec to non-existant room #%d", room);
}


/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
  assign_kings_castle();

  ASSIGNMOB(3095, cryogenicist);

  ASSIGNMOB(3105, mayor);

  ASSIGNMOB(110, postmaster);
  ASSIGNMOB(1201, postmaster);
  ASSIGNMOB(3010, postmaster);
  ASSIGNMOB(10412, postmaster);
  ASSIGNMOB(10719, postmaster);
  ASSIGNMOB(25710, postmaster);
  ASSIGNMOB(27164, postmaster);
  ASSIGNMOB(30128, postmaster);
  ASSIGNMOB(31510, postmaster);

  ASSIGNMOB(1200, receptionist);
  ASSIGNMOB(3005, receptionist);
  ASSIGNMOB(5404, receptionist);   
}

/* assign special procedures to objects */
void assign_objects(void)
{
  ASSIGNOBJ(115, bank);
  ASSIGNOBJ(334, bank);	        /* atm */
  ASSIGNOBJ(336, bank);	        /* cashcard */
  ASSIGNOBJ(3034, bank);        /* atm */
  ASSIGNOBJ(3036, bank);        /* cashcard */
  ASSIGNOBJ(3907, bank);
  ASSIGNOBJ(10640, bank);
  ASSIGNOBJ(10751, bank);
  ASSIGNOBJ(25758, bank);
}

/* assign special procedures to rooms */
void assign_rooms(void)
{
  room_rnum i;

  ASSIGNROOM(363, pet_shops);
  ASSIGNROOM(3031, pet_shops);
  ASSIGNROOM(10738, pet_shops);
  ASSIGNROOM(23281, pet_shops);
  ASSIGNROOM(25722, pet_shops);
  ASSIGNROOM(27155, pet_shops);
  ASSIGNROOM(27616, pet_shops);
  ASSIGNROOM(31523, pet_shops);
 
  if (CONFIG_DTS_ARE_DUMPS)
    for (i = 0; i <= top_of_world; i++)
      if (ROOM_FLAGGED(i, ROOM_DEATH))
	world[i].func = dump;
}
