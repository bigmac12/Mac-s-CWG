/* ************************************************************************
*   File: act.wizard.c                                  Part of CircleMUD *
*  Usage: Player-level god commands and other goodies                     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "house.h"
#include "screen.h"
#include "constants.h"
#include "oasis.h"
#include "genzon.h"
#include "dg_scripts.h"
#include "assemblies.h"

/*   external vars  */
extern FILE *player_fl;
extern struct attack_hit_type attack_hit_text[];
extern char *class_abbrevs[];
extern char *race_abbrevs[];
extern time_t boot_time;
extern int circle_shutdown, circle_reboot;
extern int circle_restrict;
extern int buf_switches, buf_largecount, buf_overflows;
extern int top_of_p_table;
extern socket_t mother_desc;
extern ush_int port;
extern struct player_index_element *player_table;
struct time_info_data *real_time_passed(time_t t2, time_t t1);

/* for chars */
extern const char *pc_class_types[];
extern const char *pc_race_types[];
extern const char *class_names[];
extern const char *race_names[];

/* extern functions */
int level_exp(int chclass, int level);
void show_shops(struct char_data *ch, char *value);
void hcontrol_list_houses(struct char_data *ch);
void do_start(struct char_data *ch);
void appear(struct char_data *ch);
void reset_zone(zone_rnum zone);
void roll_real_abils(struct char_data *ch);
int parse_class(char arg);
int parse_race(struct char_data *ch, int arg);
void run_autowiz(void);
int save_all(void);
void print_zone(struct char_data *ch, zone_vnum vnum);
struct char_data *find_char(long n);
SPECIAL(shop_keeper);
void Crash_rentsave(struct char_data * ch, int cost);
void show_guild(struct char_data * ch, char *arg);
void list_skills_perct(struct char_data *ch, struct char_data *vict);

/* local functions */
int perform_set(struct char_data *ch, struct char_data *vict, int mode, char *val_arg);
void perform_immort_invis(struct char_data *ch, int level);
ACMD(do_echo);
ACMD(do_send);
room_rnum find_target_room(struct char_data *ch, char *rawroomstr);
ACMD(do_at);
ACMD(do_goto);
ACMD(do_trans);
ACMD(do_teleport);
ACMD(do_vnum);
void do_stat_room(struct char_data *ch);
void do_stat_object(struct char_data *ch, struct obj_data *j);
void do_stat_character(struct char_data *ch, struct char_data *k);
ACMD(do_stat);
ACMD(do_shutdown);
void stop_snooping(struct char_data *ch);
ACMD(do_snoop);
ACMD(do_switch);
ACMD(do_return);
ACMD(do_load);
ACMD(do_vstat);
ACMD(do_vwear);
ACMD(do_purge);
ACMD(do_syslog);
ACMD(do_advance);
ACMD(do_restore);
void perform_immort_vis(struct char_data *ch);
ACMD(do_invis);
ACMD(do_gecho);
ACMD(do_aecho);
ACMD(do_poofset);
ACMD(do_dc);
ACMD(do_wizlock);
ACMD(do_date);
ACMD(do_last);
ACMD(do_force);
ACMD(do_wiznet);
ACMD(do_zreset);
ACMD(do_wizutil);
size_t print_zone_to_buf(char *bufptr, size_t left, zone_rnum zone, int listall);
ACMD(do_show);
ACMD(do_set);
void snoop_check(struct char_data *ch);
ACMD(do_saveall);
ACMD(do_wizupdate);
ACMD(do_chown);
ACMD(do_zpurge);
void mob_checkload(struct char_data *ch, mob_vnum mvnum);
void obj_checkload(struct char_data *ch, obj_vnum ovnum);
void trg_checkload(struct char_data *ch, trig_vnum tvnum);
ACMD(do_zcheck);
ACMD(do_checkloadstatus);

ACMD(do_echo)
{
  skip_spaces(&argument);

  if (!*argument)
    send_to_char(ch, "Yes.. but what?\r\n");
  else {
    char buf[MAX_INPUT_LENGTH + 4];

    if (subcmd == SCMD_EMOTE)
      snprintf(buf, sizeof(buf), "$n %s", argument);
    else
      strlcpy(buf, argument, sizeof(buf));

    act(buf, FALSE, ch, 0, 0, TO_ROOM);

    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(ch, "%s", CONFIG_OK);
    else
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
  }
}


ACMD(do_send)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  struct char_data *vict;

  half_chop(argument, arg, buf);

  if (!*arg) {
    send_to_char(ch, "Send what to who?\r\n");
    return;
  }
  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD))) {
    send_to_char(ch, "%s", CONFIG_NOPERSON);
    return;
  }
  send_to_char(vict, "%s\r\n", buf);
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(ch, "Sent.\r\n");
  else
    send_to_char(ch, "You send '%s' to %s.\r\n", buf, GET_NAME(vict));
}



/* take a string, and return an rnum.. used for goto, at, etc.  -je 4/6/93 */
room_rnum find_target_room(struct char_data *ch, char *rawroomstr)
{
  room_rnum location = NOWHERE;
  char roomstr[MAX_INPUT_LENGTH];
  struct room_data *rm;

  one_argument(rawroomstr, roomstr);

  if (!*roomstr) {
    send_to_char(ch, "You must supply a room number or name.\r\n");
    return (NOWHERE);
  }

  if (isdigit(*roomstr) && !strchr(roomstr, '.')) {
    if ((location = real_room((room_vnum)atoi(roomstr))) == NOWHERE) {
      send_to_char(ch, "No room exists with that number.\r\n");
      return (NOWHERE);
    }
  } else {
    struct char_data *target_mob;
    struct obj_data *target_obj;
    char *mobobjstr = roomstr;
    int num;

    num = get_number(&mobobjstr);
    if ((target_mob = get_char_vis(ch, mobobjstr, &num, FIND_CHAR_WORLD)) != NULL) {
      if ((location = IN_ROOM(target_mob)) == NOWHERE) {
        send_to_char(ch, "That character is currently lost.\r\n");
        return (NOWHERE);
      }
    } else if ((target_obj = get_obj_vis(ch, mobobjstr, &num)) != NULL) {
      if (IN_ROOM(target_obj) != NOWHERE)
        location = IN_ROOM(target_obj);
      else if (target_obj->carried_by && IN_ROOM(target_obj->carried_by) != NOWHERE)
        location = IN_ROOM(target_obj->carried_by);
      else if (target_obj->worn_by && IN_ROOM(target_obj->worn_by) != NOWHERE)
        location = IN_ROOM(target_obj->worn_by);

      if (location == NOWHERE) {
        send_to_char(ch, "That object is currently not in a room.\r\n");
        return (NOWHERE);
      }
    }

    if (location == NOWHERE) {
      send_to_char(ch, "Nothing exists by that name.\r\n");
      return (NOWHERE);
    }
  }

  /* a location has been found -- if you're >= GRGOD, no restrictions. */
  if (GET_LEVEL(ch) >= LVL_GRGOD)
    return (location);

  rm = &world[location];

  if ((!can_edit_zone(ch, rm->zone) && GET_LEVEL(ch) < LVL_GOD)
       && ZONE_FLAGGED(rm->zone, ZONE_QUEST)) {
       send_to_char(ch, "This target is in a quest zone.\r\n");
       return (NOWHERE);
       }

  if ((GET_LEVEL(ch) < LVL_GRGOD) && ZONE_FLAGGED(rm->zone, ZONE_NOIMMORT)){
       send_to_char(ch, "This target is in a zone closed to all.\r\n");
       return (NOWHERE);
      }

  if (ROOM_FLAGGED(location, ROOM_GODROOM))
    send_to_char(ch, "You are not godly enough to use that room!\r\n");
  else if (ROOM_FLAGGED(location, ROOM_PRIVATE) && world[location].people && world[location].people->next_in_room)
    send_to_char(ch, "There's a private conversation going on in that room.\r\n");
  else if (ROOM_FLAGGED(location, ROOM_HOUSE) && !House_can_enter(ch, GET_ROOM_VNUM(location)))
    send_to_char(ch, "That's private property -- no trespassing!\r\n");
  else
    return (location);

  return (NOWHERE);
}



ACMD(do_at)
{
  char command[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  room_rnum location, original_loc;

  half_chop(argument, buf, command);
  if (!*buf) {
    send_to_char(ch, "You must supply a room number or a name.\r\n");
    return;
  }

  if (!*command) {
    send_to_char(ch, "What do you want to do there?\r\n");
    return;
  }

  if ((location = find_target_room(ch, buf)) == NOWHERE)
    return;

  /* a location has been found. */
  original_loc = IN_ROOM(ch);
  char_from_room(ch);
  char_to_room(ch, location);
  command_interpreter(ch, command);

  /* check if the char is still there */
  if (IN_ROOM(ch) == location) {
    char_from_room(ch);
    char_to_room(ch, original_loc);
  }
}


ACMD(do_goto)
{
  char buf[MAX_STRING_LENGTH];
  room_rnum location;

  if ((location = find_target_room(ch, argument)) == NOWHERE)
    return;

  snprintf(buf, sizeof(buf), "$n %s", POOFOUT(ch) ? POOFOUT(ch) : "disappears in a puff of smoke.");
  act(buf, TRUE, ch, 0, 0, TO_ROOM);

  char_from_room(ch);
  char_to_room(ch, location);

  snprintf(buf, sizeof(buf), "$n %s", POOFIN(ch) ? POOFIN(ch) : "appears with an ear-splitting bang.");
  act(buf, TRUE, ch, 0, 0, TO_ROOM);

  look_at_room(IN_ROOM(ch), ch, 0);
  enter_wtrigger(&world[IN_ROOM(ch)], ch, -1);
}



ACMD(do_trans)
{
  char buf[MAX_INPUT_LENGTH];
  struct descriptor_data *i;
  struct char_data *victim;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char(ch, "Whom do you wish to transfer?\r\n");
  else if (str_cmp("all", buf)) {
    if (!(victim = get_char_vis(ch, buf, NULL, FIND_CHAR_WORLD)))
      send_to_char(ch, "%s", CONFIG_NOPERSON);
    else if (victim == ch)
      send_to_char(ch, "That doesn't make much sense, does it?\r\n");
    else {
      if ((GET_LEVEL(ch) < GET_LEVEL(victim)) && !IS_NPC(victim)) {
	send_to_char(ch, "Go transfer someone your own size.\r\n");
	return;
      }
      act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
      char_from_room(victim);
      char_to_room(victim, IN_ROOM(ch));
      act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
      act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
      look_at_room(IN_ROOM(victim), victim, 0);
      enter_wtrigger(&world[IN_ROOM(victim)], victim, -1);
    }
  } else {			/* Trans All */
    if (GET_LEVEL(ch) < LVL_GRGOD) {
      send_to_char(ch, "I think not.\r\n");
      return;
    }

    for (i = descriptor_list; i; i = i->next)
      if (STATE(i) == CON_PLAYING && i->character && i->character != ch) {
	victim = i->character;
	if (GET_LEVEL(victim) >= GET_LEVEL(ch))
	  continue;
	act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
	char_from_room(victim);
	char_to_room(victim, IN_ROOM(ch));
	act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
	act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
        look_at_room(IN_ROOM(victim), victim, 0);
        enter_wtrigger(&world[IN_ROOM(victim)], victim, -1);
      }
    send_to_char(ch, "%s", CONFIG_OK);
  }
}



ACMD(do_teleport)
{
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  struct char_data *victim;
  room_rnum target;

  two_arguments(argument, buf, buf2);

  if (!*buf)
    send_to_char(ch, "Whom do you wish to teleport?\r\n");
  else if (!(victim = get_char_vis(ch, buf, NULL, FIND_CHAR_WORLD)))
    send_to_char(ch, "%s", CONFIG_NOPERSON);
  else if (victim == ch)
    send_to_char(ch, "Use 'goto' to teleport yourself.\r\n");
  else if (GET_LEVEL(victim) >= GET_LEVEL(ch))
    send_to_char(ch, "Maybe you shouldn't do that.\r\n");
  else if (!*buf2)
    send_to_char(ch, "Where do you wish to send this person?\r\n");
  else if ((target = find_target_room(ch, buf2)) != NOWHERE) {
    send_to_char(ch, "%s", CONFIG_OK);
    act("$n disappears in a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, target);
    act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    act("$n has teleported you!", FALSE, ch, 0, (char *) victim, TO_VICT);
    look_at_room(IN_ROOM(victim), victim, 0);
    enter_wtrigger(&world[IN_ROOM(victim)], victim, -1);
  }
}



ACMD(do_vnum)
{
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2 || (!is_abbrev(buf, "mob") && !is_abbrev(buf, "obj"))) {
    send_to_char(ch, "Usage: vnum { obj | mob } <name>\r\n");
    return;
  }
  if (is_abbrev(buf, "mob"))
    if (!vnum_mobile(buf2, ch))
      send_to_char(ch, "No mobiles by that name.\r\n");

  if (is_abbrev(buf, "obj"))
    if (!vnum_object(buf2, ch))
      send_to_char(ch, "No objects by that name.\r\n");
}


#define ZOCMD zone_table[zrnum].cmd[subcmd]
 
void list_zone_commands_room(struct char_data *ch, room_vnum rvnum)
{
  extern struct index_data **trig_index;
  zone_rnum zrnum = real_zone_by_thing(rvnum);
  room_rnum rrnum = real_room(rvnum), cmd_room = NOWHERE;
  int subcmd = 0, count = 0;

  if (zrnum == NOWHERE || rrnum == NOWHERE) {
    send_to_char(ch, "No zone information available.\r\n");
    return;
  }

  get_char_colors(ch);

  send_to_char(ch, "Zone commands in this room:%s\r\n", yel);
  while (ZOCMD.command != 'S') {
    switch (ZOCMD.command) {
      case 'M':
      case 'O':
      case 'T':
      case 'V':
        cmd_room = ZOCMD.arg3;
        break;
      case 'D':
      case 'R':
        cmd_room = ZOCMD.arg1;
        break;
      default:
        break;
    }
    if (cmd_room == rrnum) {
      count++;
      /* start listing */
      switch (ZOCMD.command) {
        case 'M':
          send_to_char(ch, "%sLoad %s [%s%d%s], MaxMud : %d, MaxR : %d, Chance : %d\r\n",
                  ZOCMD.if_flag ? " then " : "",
                  mob_proto[ZOCMD.arg1].player.short_descr, cyn,
                  mob_index[ZOCMD.arg1].vnum, yel, ZOCMD.arg2,
                  ZOCMD.arg4, ZOCMD.arg5
                  );
          break;
        case 'G':
          send_to_char(ch, "%sGive it %s [%s%d%s], Max : %d, Chance : %d\r\n",
    	      ZOCMD.if_flag ? " then " : "",
    	      obj_proto[ZOCMD.arg1].short_description,
    	      cyn, obj_index[ZOCMD.arg1].vnum, yel, ZOCMD.arg2,
    	      ZOCMD.arg5
    	      );
          break;
        case 'O':
          send_to_char(ch, "%sLoad %s [%s%d%s], Max : %d, MaxR : %d, Chance : %d\r\n",
    	      ZOCMD.if_flag ? " then " : "",
    	      obj_proto[ZOCMD.arg1].short_description,
    	      cyn, obj_index[ZOCMD.arg1].vnum, yel, ZOCMD.arg2,
    	      ZOCMD.arg4, ZOCMD.arg5
    	      );
          break;
        case 'E':
          send_to_char(ch, "%sEquip with %s [%s%d%s], %s, Max : %d, Chance : %d\r\n",
    	      ZOCMD.if_flag ? " then " : "",
    	      obj_proto[ZOCMD.arg1].short_description,
    	      cyn, obj_index[ZOCMD.arg1].vnum, yel,
    	      equipment_types[ZOCMD.arg3], ZOCMD.arg2, ZOCMD.arg5
    	      );
          break;
        case 'P':
          send_to_char(ch, "%sPut %s [%s%d%s] in %s [%s%d%s], Max : %d, Chance : %d\r\n",
    	      ZOCMD.if_flag ? " then " : "",
    	      obj_proto[ZOCMD.arg1].short_description,
    	      cyn, obj_index[ZOCMD.arg1].vnum, yel,
    	      obj_proto[ZOCMD.arg3].short_description,
    	      cyn, obj_index[ZOCMD.arg3].vnum, yel,
    	      ZOCMD.arg2, ZOCMD.arg5
    	      );
          break;
        case 'R':
          send_to_char(ch, "%sRemove %s [%s%d%s] from room.\r\n",
    	      ZOCMD.if_flag ? " then " : "",
    	      obj_proto[ZOCMD.arg2].short_description,
    	      cyn, obj_index[ZOCMD.arg2].vnum, yel
    	      );
          break;
        case 'D':
          send_to_char(ch, "%sSet door %s as %s.\r\n",
    	      ZOCMD.if_flag ? " then " : "",
    	      dirs[ZOCMD.arg2],
    	      ZOCMD.arg3 ? ((ZOCMD.arg3 == 1) ? "closed" : "locked") : "open"
    	      );
          break;
        case 'T':
          send_to_char(ch, "%sAttach trigger %s%s%s [%s%d%s] to %s\r\n",
            ZOCMD.if_flag ? " then " : "",
            cyn, trig_index[ZOCMD.arg2]->proto->name, yel, 
            cyn, trig_index[ZOCMD.arg2]->vnum, yel,
            ((ZOCMD.arg1 == MOB_TRIGGER) ? "mobile" :
              ((ZOCMD.arg1 == OBJ_TRIGGER) ? "object" :
                ((ZOCMD.arg1 == WLD_TRIGGER)? "room" : "????"))));
          break;
        case 'V':
          send_to_char(ch, "%sAssign global %s:%d to %s = %s\r\n",
            ZOCMD.if_flag ? " then " : "",
            ZOCMD.sarg1, ZOCMD.arg2,
            ((ZOCMD.arg1 == MOB_TRIGGER) ? "mobile" :
              ((ZOCMD.arg1 == OBJ_TRIGGER) ? "object" :
                ((ZOCMD.arg1 == WLD_TRIGGER)? "room" : "????"))),
            ZOCMD.sarg2);
          break;
        default:
          send_to_char(ch, "<Unknown Command>\r\n");
          break;
      }
    } 
    subcmd++;  
  }
  send_to_char(ch, nrm);
  if (!count) 
    send_to_char(ch, "None!\r\n");

}
#undef ZOCMD

void do_stat_room(struct char_data *ch)
{
  char buf2[MAX_STRING_LENGTH];
  struct extra_descr_data *desc;
  struct room_data *rm = &world[IN_ROOM(ch)];
  int i, found, column;
  struct obj_data *j;
  struct char_data *k;

  send_to_char(ch, "Room name: %s%s%s\r\n", CCCYN(ch, C_NRM), rm->name, CCNRM(ch, C_NRM));

  sprinttype(rm->sector_type, sector_types, buf2, sizeof(buf2));
  send_to_char(ch, "Zone: [%3d], VNum: [%s%5d%s], RNum: [%5d], IDNum: [%5ld], Type: %s\r\n",
	  zone_table[rm->zone].number, CCGRN(ch, C_NRM), rm->number,
	  CCNRM(ch, C_NRM), IN_ROOM(ch), (long) rm->number + ROOM_ID_BASE, buf2);

  sprintbitarray(rm->room_flags, room_bits, RF_ARRAY_MAX, buf2);
  send_to_char(ch, "SpecProc: %s, Flags: %s\r\n", rm->func == NULL ? "None" : "Exists", buf2);

  send_to_char(ch, "Description:\r\n%s", rm->description ? rm->description : "  None.\r\n");

  if (rm->ex_description) {
    send_to_char(ch, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = rm->ex_description; desc; desc = desc->next)
      send_to_char(ch, " [%s]", desc->keyword);
    send_to_char(ch, "%s\r\n", CCNRM(ch, C_NRM));
  }

  send_to_char(ch, "Chars present:%s", CCYEL(ch, C_NRM));
  column = 14;	/* ^^^ strlen ^^^ */
  for (found = FALSE, k = rm->people; k; k = k->next_in_room) {
    if (!CAN_SEE(ch, k))
      continue;

    column += send_to_char(ch, "%s %s(%s)", found++ ? "," : "", GET_NAME(k),
		!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB"));
    if (column >= 62) {
      send_to_char(ch, "%s\r\n", k->next_in_room ? "," : "");
      found = FALSE;
      column = 0;
    }
  }
  send_to_char(ch, "%s", CCNRM(ch, C_NRM));

  if (rm->contents) {
    send_to_char(ch, "Contents:%s", CCGRN(ch, C_NRM));
    column = 9;	/* ^^^ strlen ^^^ */

    for (found = 0, j = rm->contents; j; j = j->next_content) {
      if (!CAN_SEE_OBJ(ch, j))
	continue;

      column += send_to_char(ch, "%s %s", found++ ? "," : "", j->short_description);
      if (column >= 62) {
	send_to_char(ch, "%s\r\n", j->next_content ? "," : "");
	found = FALSE;
        column = 0;
      }
    }
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
  }

  for (i = 0; i < NUM_OF_DIRS; i++) {
    char buf1[128];

    if (!rm->dir_option[i])
      continue;

    if (rm->dir_option[i]->to_room == NOWHERE)
      snprintf(buf1, sizeof(buf1), " %sNONE%s", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
    else
      snprintf(buf1, sizeof(buf1), "%s%5d%s", CCCYN(ch, C_NRM), GET_ROOM_VNUM(rm->dir_option[i]->to_room), CCNRM(ch, C_NRM));

    sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2, sizeof(buf2));

    send_to_char(ch, "Exit %s%-5s%s:  To: [%s], Key: [%5d], Keywrd: %s, Type: %s\r\n%s",
        CCCYN(ch, C_NRM), dirs[i], CCNRM(ch, C_NRM), buf1,
        rm->dir_option[i]->key == NOTHING ? -1 : rm->dir_option[i]->key,
	rm->dir_option[i]->keyword ? rm->dir_option[i]->keyword : "None", buf2,
	rm->dir_option[i]->general_description ? rm->dir_option[i]->general_description : "  No exit description.\r\n");
  }

  /* check the room for a script */
  do_sstat_room(ch);
 
  list_zone_commands_room(ch, rm->number);
}



void do_stat_object(struct char_data *ch, struct obj_data *j)
{
  int i, found;
  obj_vnum vnum;
  struct obj_data *j2;
  struct extra_descr_data *desc;
  char buf[MAX_STRING_LENGTH];

  vnum = GET_OBJ_VNUM(j);
  send_to_char(ch, "Name: '%s%s%s', Aliases: %s\r\n", CCYEL(ch, C_NRM),
	  j->short_description ? j->short_description : "<None>",
	  CCNRM(ch, C_NRM), j->name);

  sprinttype(GET_OBJ_TYPE(j), item_types, buf, sizeof(buf));
  send_to_char(ch, "VNum: [%s%5d%s], RNum: [%5d], Idnum: [%5ld], Type: %s, SpecProc: %s\r\n",
	CCGRN(ch, C_NRM), vnum, CCNRM(ch, C_NRM), GET_OBJ_RNUM(j), GET_ID(j), buf,
	GET_OBJ_SPEC(j) ? "Exists" : "None");

  send_to_char(ch, "Generation time: @g%s@nUnique ID: @g%lld@n\r\n",
    ctime(&j->generation), j->unique_id);

  send_to_char(ch, "Object loaded in room: @y%d@n\r\n",
    OBJ_LOADROOM(j));

  if (j->ex_description) {
    send_to_char(ch, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = j->ex_description; desc; desc = desc->next)
      send_to_char(ch, " [%s]", desc->keyword);
    send_to_char(ch, "%s\r\n", CCNRM(ch, C_NRM));
    send_to_char(ch, "\r\n");
  }

  sprintbitarray(GET_OBJ_WEAR(j), wear_bits, TW_ARRAY_MAX, buf);
  send_to_char(ch, "Can be worn on: %s\r\n", buf);

  sprintbitarray(GET_OBJ_AFFECT(j), affected_bits, AF_ARRAY_MAX, buf);
  send_to_char(ch, "Set char bits : %s\r\n", buf);

  sprintbitarray(GET_OBJ_EXTRA(j), extra_bits, EF_ARRAY_MAX, buf);
  send_to_char(ch, "Extra flags   : %s\r\n", buf);

  send_to_char(ch, "Weight: %d, Value: %d, Cost/day: %d, Timer: %d, Min Level: %d\r\n",
     GET_OBJ_WEIGHT(j), GET_OBJ_COST(j), GET_OBJ_RENT(j), GET_OBJ_TIMER(j), GET_OBJ_LEVEL(j));

  send_to_char(ch, "In room: %d (%s), ", GET_ROOM_VNUM(IN_ROOM(j)),
	IN_ROOM(j) == NOWHERE ? "Nowhere" : world[IN_ROOM(j)].name);

  /*
   * NOTE: In order to make it this far, we must already be able to see the
   *       character holding the object. Therefore, we do not need CAN_SEE().
   */
  send_to_char(ch, "In object: %s, ", j->in_obj ? j->in_obj->short_description : "None");
  send_to_char(ch, "Carried by: %s, ", j->carried_by ? GET_NAME(j->carried_by) : "Nobody");
  send_to_char(ch, "Worn by: %s\r\n", j->worn_by ? GET_NAME(j->worn_by) : "Nobody");

  switch (GET_OBJ_TYPE(j)) {
  case ITEM_LIGHT:
    if (GET_OBJ_VAL(j, VAL_LIGHT_HOURS) == -1)
      send_to_char(ch, "Hours left: Infinite\r\n");
    else
      send_to_char(ch, "Hours left: [%d]\r\n", GET_OBJ_VAL(j, VAL_LIGHT_HOURS));
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    send_to_char(ch, "Spells: (Level %d) %s, %s, %s\r\n", GET_OBJ_VAL(j, VAL_POTION_LEVEL),
          skill_name(GET_OBJ_VAL(j, VAL_POTION_SPELL1)),
            skill_name(GET_OBJ_VAL(j, VAL_POTION_SPELL2)),
          skill_name(GET_OBJ_VAL(j, VAL_POTION_SPELL3)));
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    send_to_char(ch, "Spell: %s at level %d, %d (of %d) charges remaining\r\n",
          skill_name(GET_OBJ_VAL(j, VAL_STAFF_SPELL)), GET_OBJ_VAL(j, VAL_STAFF_LEVEL),
          GET_OBJ_VAL(j, VAL_STAFF_CHARGES), GET_OBJ_VAL(j, VAL_STAFF_MAXCHARGES));
    break;
  case ITEM_WEAPON:
    send_to_char(ch, "Todam: %dd%d, Message type: %d\r\n",
            GET_OBJ_VAL(j, VAL_WEAPON_DAMDICE),
            GET_OBJ_VAL(j, VAL_WEAPON_DAMSIZE),
            GET_OBJ_VAL(j, VAL_WEAPON_DAMTYPE));
    break;
  case ITEM_ARMOR:
    send_to_char(ch, "AC-apply: [%d]\r\n", GET_OBJ_VAL(j, VAL_ARMOR_APPLYAC));
    break;
  case ITEM_TRAP:
    send_to_char(ch, "Spell: %d, - Hitpoints: %d\r\n", GET_OBJ_VAL(j, VAL_TRAP_SPELL),
                 GET_OBJ_VAL(j, VAL_TRAP_HITPOINTS));
    break;
  case ITEM_CONTAINER:
    sprintbit(GET_OBJ_VAL(j, VAL_CONTAINER_FLAGS), container_bits, buf, sizeof(buf));
    send_to_char(ch, "Weight capacity: %d, Lock Type: %s, Key Num: %d, Corpse: %s\r\n",
          GET_OBJ_VAL(j, VAL_CONTAINER_CAPACITY), buf, GET_OBJ_VAL(j, VAL_CONTAINER_KEY),
          YESNO(GET_OBJ_VAL(j, VAL_CONTAINER_CORPSE)));
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    sprinttype(GET_OBJ_VAL(j, 2), drinks, buf, sizeof(buf));
    send_to_char(ch, "Capacity: %d, Contains: %d, Poisoned: %s, Liquid: %s\r\n",
          GET_OBJ_VAL(j, VAL_DRINKCON_CAPACITY), GET_OBJ_VAL(j, VAL_DRINKCON_HOWFULL),
            YESNO(GET_OBJ_VAL(j, VAL_DRINKCON_POISON)), buf);
    break;
  case ITEM_NOTE:
    send_to_char(ch, "Language: %d\r\n", GET_OBJ_VAL(j, VAL_NOTE_LANGUAGE));
    break;
  case ITEM_KEY:
    /* Nothing */
    break;
  case ITEM_FOOD:
    send_to_char(ch, "Makes full: %d, Poisoned: %s\r\n", GET_OBJ_VAL(j, VAL_FOOD_FOODVAL),
                 YESNO(GET_OBJ_VAL(j, VAL_FOOD_POISON)));
    break;
  case ITEM_MONEY:
    send_to_char(ch, "Coins: %d\r\n", GET_OBJ_VAL(j, VAL_MONEY_SIZE));
    break;
  default:
    send_to_char(ch, "Values 0-3: [%d] [%d] [%d] [%d]\r\n",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1),
	    GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3));
    break;
  }

  /*
   * I deleted the "equipment status" code from here because it seemed
   * more or less useless and just takes up valuable screen space.
   */

  if (j->contains) {
    int column;

    send_to_char(ch, "\r\nContents:%s", CCGRN(ch, C_NRM));
    column = 9;	/* ^^^ strlen ^^^ */

    for (found = 0, j2 = j->contains; j2; j2 = j2->next_content) {
      column += send_to_char(ch, "%s %s", found++ ? "," : "", j2->short_description);
      if (column >= 62) {
	send_to_char(ch, "%s\r\n", j2->next_content ? "," : "");
	found = FALSE;
        column = 0;
      }
    }
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
  }

  found = FALSE;
  send_to_char(ch, "Affections:");
  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (j->affected[i].modifier) {
      sprinttype(j->affected[i].location, apply_types, buf, sizeof(buf));
      send_to_char(ch, "%s %+d to %s", found++ ? "," : "", j->affected[i].modifier, buf);
    }
  if (!found)
    send_to_char(ch, " None");

  send_to_char(ch, "\r\n");

  /* check the object for a script */
  do_sstat_object(ch, j);
}


void do_stat_character(struct char_data *ch, struct char_data *k)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int i, i2, column, found = FALSE;
  struct obj_data *j;
  struct follow_type *fol;
  struct affected_type *aff;
  struct affectedv_type *affv;

  sprinttype(GET_SEX(k), genders, buf, sizeof(buf));
  send_to_char(ch, "%s %s '%s'  IDNum: [%5ld], In room [%5d], Loadroom : [%5d]\r\n",
	  buf, (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
	  GET_NAME(k), IS_NPC(k) ? GET_ID(k) : GET_IDNUM(k), GET_ROOM_VNUM(IN_ROOM(k)), IS_NPC(k) ? MOB_LOADROOM(k) : GET_LOADROOM(k));

  if (IS_MOB(k)) {
    if (k->master_id > -1)
      sprintf(buf, ", Master: %s", get_name_by_id(k->master_id));
    else
      buf[0] = 0;
    send_to_char(ch, "Alias: %s, VNum: [%5d], RNum: [%5d]%s\r\n", k->player.name,
                 GET_MOB_VNUM(k), GET_MOB_RNUM(k), buf);
  } else
  send_to_char(ch, "Title: %s\r\n", k->player.title ? k->player.title : "<None>");
  send_to_char(ch, "Alias: %s\r\n", k->player.alias ? k->player.alias : "<None>\r\n");
  send_to_char(ch, "S-Des: %s\r\n", k->player.short_descr ? k->player.short_descr : "<None>\r\n");
  send_to_char(ch, "L-Des: %s\r\n", k->player.long_descr ? k->player.long_descr : "<None>\r\n");
  sprinttype(k->player.chclass, IS_NPC(k) ? npc_class_types : pc_class_types, buf, sizeof(buf));
  sprinttype(k->player.race, pc_race_types, buf2, sizeof(buf2));
  send_to_char(ch, "%sClass: %s, Race: %s, Lev: [%s%2d%s], XP: [%s%7d%s], "
                   "Align: [%4d]\r\n", IS_NPC(k) ? "Monster " : "", buf, buf2,
                   CCYEL(ch, C_NRM), GET_LEVEL(k), CCNRM(ch, C_NRM),
                   CCYEL(ch, C_NRM), GET_EXP(k), CCNRM(ch, C_NRM),
                   GET_ALIGNMENT(k));

  if (!IS_NPC(k)) {
    char buf1[64], buf2[64];

    strlcpy(buf1, asctime(localtime(&(k->player.time.birth))), sizeof(buf1));
    strlcpy(buf2, asctime(localtime(&(k->player.time.logon))), sizeof(buf2));
    buf1[10] = buf2[10] = '\0';

    send_to_char(ch, "Created: [%s], Last Logon: [%s], Played [%dh %dm], Age [%d]\r\n",
	    buf1, buf2, k->player.time.played / 3600,
	    ((k->player.time.played % 3600) / 60), age(k)->year);

    send_to_char(ch, "Hometown: [%d], Speaks: [%d/%d/%d], (STL[%d]/per[%d]/NSTL[%d])",
	 GET_HOME(k), GET_TALK(k, 0), GET_TALK(k, 1), GET_TALK(k, 2),
	    GET_PRACTICES(k), int_app[GET_INT(k)].learn,
	    wis_app[GET_WIS(k)].bonus);
    /*. Display OLC zone for immorts .*/
    if (GET_LEVEL(k) >= LVL_BUILDER) {
      if (GET_OLC_ZONE(k)==AEDIT_PERMISSION)
        send_to_char(ch, ", OLC[%sActions%s]", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
      else if (GET_OLC_ZONE(k)==HEDIT_PERMISSION)
        send_to_char(ch, ", OLC[%sHedit%s]", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
      else if (GET_OLC_ZONE(k)==NOWHERE)
        send_to_char(ch, ", OLC[%sOFF%s]", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
      else
        send_to_char(ch, ", OLC[%s%d%s]", CCCYN(ch, C_NRM), GET_OLC_ZONE(k), CCNRM(ch, C_NRM));
    }
    send_to_char(ch, "\r\n");
  }
  send_to_char(ch, "Str: [%s%d%s]  Int: [%s%d%s]  Wis: [%s%d%s]  "
	  "Dex: [%s%d%s]  Con: [%s%d%s]  Cha: [%s%d%s]\r\n",
	  CCCYN(ch, C_NRM), GET_STR(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_INT(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_WIS(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_DEX(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_CON(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_CHA(k), CCNRM(ch, C_NRM));

  send_to_char(ch, "Hit p.:[%s%d/%d+%d%s]  Mana p.:[%s%d/%d+%d%s]  Move p.:[%s%d/%d+%d%s]\r\n",
	  CCGRN(ch, C_NRM), GET_HIT(k), GET_MAX_HIT(k), hit_gain(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_MANA(k), GET_MAX_MANA(k), mana_gain(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_MOVE(k), GET_MAX_MOVE(k), move_gain(k), CCNRM(ch, C_NRM));

  send_to_char(ch, "Coins: [%9d], Bank: [%9d] (Total: %d)\r\n",
	  GET_GOLD(k), GET_BANK_GOLD(k), GET_GOLD(k) + GET_BANK_GOLD(k));

  send_to_char(ch, "AC: [%d%+d/10], Hitroll: [%2d], Damroll: [%2d], Saving throws: [%d/%d/%d/%d/%d]\r\n",
	  GET_AC(k), dex_app[GET_DEX(k)].defensive, k->points.hitroll,
	  k->points.damroll, GET_SAVE(k, 0), GET_SAVE(k, 1), GET_SAVE(k, 2),
	  GET_SAVE(k, 3), GET_SAVE(k, 4));

  sprinttype(GET_POS(k), position_types, buf, sizeof(buf));
  send_to_char(ch, "Pos: %s, Fighting: %s", buf, FIGHTING(k) ? GET_NAME(FIGHTING(k)) : "Nobody");

  if (IS_NPC(k))
    send_to_char(ch, ", Attack type: %s", attack_hit_text[(int) k->mob_specials.attack_type].singular);

  if (k->desc) {
    sprinttype(STATE(k->desc), connected_types, buf, sizeof(buf));
    send_to_char(ch, ", Connected: %s", buf);
  }

  if (IS_NPC(k)) {
    sprinttype(k->mob_specials.default_pos, position_types, buf, sizeof(buf));
    send_to_char(ch, ", Default position: %s\r\n", buf);
    sprintbitarray(MOB_FLAGS(k), action_bits, PM_ARRAY_MAX, buf);
    send_to_char(ch, "NPC flags: %s%s%s\r\n", CCCYN(ch, C_NRM), buf, CCNRM(ch, C_NRM));
  } else {
    send_to_char(ch, ", Idle Timer (in tics) [%d]\r\n", k->char_specials.timer);

    sprintbitarray(PLR_FLAGS(k), player_bits, PM_ARRAY_MAX, buf);
    send_to_char(ch, "PLR: %s%s%s\r\n", CCCYN(ch, C_NRM), buf, CCNRM(ch, C_NRM));

    sprintbitarray(PRF_FLAGS(k), preference_bits, PR_ARRAY_MAX, buf);
    send_to_char(ch, "PRF: %s%s%s\r\n", CCGRN(ch, C_NRM), buf, CCNRM(ch, C_NRM));
  }

  if (IS_MOB(k))
    send_to_char(ch, "Mob Spec-Proc: %s, NPC Bare Hand Dam: %dd%d\r\n",
	    (mob_index[GET_MOB_RNUM(k)].func ? "Exists" : "None"),
	    k->mob_specials.damnodice, k->mob_specials.damsizedice);

  for (i = 0, j = k->carrying; j; j = j->next_content, i++);
  send_to_char(ch, "Carried: weight: %d, items: %d; Items in: inventory: %d, ", IS_CARRYING_W(k), IS_CARRYING_N(k), i);

  for (i = 0, i2 = 0; i < NUM_WEARS; i++)
    if (GET_EQ(k, i))
      i2++;
  send_to_char(ch, "eq: %d\r\n", i2);

  if (!IS_NPC(k))
    send_to_char(ch, "Hunger: %d, Thirst: %d, Drunk: %d\r\n", GET_COND(k, FULL), GET_COND(k, THIRST), GET_COND(k, DRUNK));

  column = send_to_char(ch, "Master is: %s, Followers are:", k->master ? GET_NAME(k->master) : "<none>");
  if (!k->followers)
    send_to_char(ch, " <none>\r\n");
  else {
    for (fol = k->followers; fol; fol = fol->next) {
      column += send_to_char(ch, "%s %s", found++ ? "," : "", PERS(fol->follower, ch));
      if (column >= 62) {
        send_to_char(ch, "%s\r\n", fol->next ? "," : "");
        found = FALSE;
        column = 0;
      }
    }
    if (column != 0)
      send_to_char(ch, "\r\n");
  }

  /* Showing the bitvector */
  sprintbitarray(AFF_FLAGS(k), affected_bits, AF_ARRAY_MAX, buf);
  send_to_char(ch, "AFF: %s%s%s\r\n", CCYEL(ch, C_NRM), buf, CCNRM(ch, C_NRM));

  /* Routine to show what spells a char is affected by */
  if (k->affected) {
    for (aff = k->affected; aff; aff = aff->next) {
      send_to_char(ch, "SPL: (%3dhr) %s%-21s%s ", aff->duration + 1, CCCYN(ch, C_NRM), skill_name(aff->type), CCNRM(ch, C_NRM));

      if (aff->modifier)
	send_to_char(ch, "%+d to %s", aff->modifier, apply_types[(int) aff->location]);

      if (aff->bitvector) {
	if (aff->modifier)
	  send_to_char(ch, ", ");

	strcpy(buf, affected_bits[aff->bitvector]);
        send_to_char(ch, "sets %s", buf);
      }
      send_to_char(ch, "\r\n");
    }
  }

  /* Routine to show what spells a char is affectedv by */
  if (k->affectedv) {
    for (affv = k->affectedv; affv; affv = affv->next) {
      send_to_char(ch, "SPL: (%3d rounds) %s%-21s%s ", affv->duration + 1, CCCYN(ch, C_NRM), skill_name(affv->type), CCNRM(ch, C_NRM));

      if (affv->modifier)
        send_to_char(ch, "%+d to %s", affv->modifier, apply_types[(int) affv->location]);

      if (affv->bitvector) {
        if (affv->modifier)
          send_to_char(ch, ", ");

        strcpy(buf, affected_bits[affv->bitvector]);
        send_to_char(ch, "sets %s", buf);
      }
      send_to_char(ch, "\r\n");
    }
  }

  /* check mobiles for a script */
  if (IS_NPC(k)) {
    do_sstat_character(ch, k);
    if (SCRIPT_MEM(k)) {
      struct script_memory *mem = SCRIPT_MEM(k);
      send_to_char(ch, "Script memory:\r\n  Remember             Command\r\n");
      while (mem) {
        struct char_data *mc = find_char(mem->id);
        if (!mc)
          send_to_char(ch, "  ** Corrupted!\r\n");
        else {
          if (mem->cmd) 
            send_to_char(ch, "  %-20.20s%s\r\n",GET_NAME(mc),mem->cmd);
          else 
            send_to_char(ch, "  %-20.20s <default>\r\n",GET_NAME(mc));
        }
      mem = mem->next;
      }
    }
  } else {
    /* this is a PC, display their global variables */
    if (k->script && k->script->global_vars) {
      struct trig_var_data *tv;
      char uname[MAX_INPUT_LENGTH];
      void find_uid_name(char *uid, char *name, size_t nlen);

      send_to_char(ch, "Global Variables:\r\n");

      /* currently, variable context for players is always 0, so it is */
      /* not displayed here. in the future, this might change */
      for (tv = k->script->global_vars; tv; tv = tv->next) {
        if (*(tv->value) == UID_CHAR) {
          find_uid_name(tv->value, uname, sizeof(uname));
          send_to_char(ch, "    %10s:  [UID]: %s\r\n", tv->name, uname);
        } else
          send_to_char(ch, "    %10s:  %s\r\n", tv->name, tv->value);
      }
    }
  }
}


ACMD(do_stat)
{
  char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  struct char_data *victim;
  struct obj_data *object;

  half_chop(argument, buf1, buf2);

  if (!*buf1) {
    send_to_char(ch, "Stats on who or what or where?\r\n");
    return;
  } else if (is_abbrev(buf1, "room")) {
    do_stat_room(ch);
  } else if (is_abbrev(buf1, "mob")) {
    if (!*buf2)
      send_to_char(ch, "Stats on which mobile?\r\n");
    else {
      if ((victim = get_char_vis(ch, buf2, NULL, FIND_CHAR_WORLD)) != NULL)
	do_stat_character(ch, victim);
      else
	send_to_char(ch, "No such mobile around.\r\n");
    }
  } else if (is_abbrev(buf1, "player")) {
    if (!*buf2) {
      send_to_char(ch, "Stats on which player?\r\n");
    } else {
      if ((victim = get_player_vis(ch, buf2, NULL, FIND_CHAR_WORLD)) != NULL)
	do_stat_character(ch, victim);
      else
	send_to_char(ch, "No such player around.\r\n");
    }
  } else if (is_abbrev(buf1, "file")) {
    if (!*buf2)
      send_to_char(ch, "Stats on which player?\r\n");
    else if ((victim = get_player_vis(ch, buf2, NULL, FIND_CHAR_WORLD)) != NULL)
	do_stat_character(ch, victim);
    else {
      CREATE(victim, struct char_data, 1);
      clear_char(victim);
      CREATE(victim->player_specials, struct player_special_data, 1);
      if (load_char(buf2, victim) >= 0) {
	char_to_room(victim, 0);
	if (GET_LEVEL(victim) > GET_LEVEL(ch))
	  send_to_char(ch, "Sorry, you can't do that.\r\n");
	else
	  do_stat_character(ch, victim);
	extract_char_final(victim);
      } else {
	send_to_char(ch, "There is no such player.\r\n");
	free_char(victim);
      }
    }
  } else if (is_abbrev(buf1, "object")) {
    if (!*buf2)
      send_to_char(ch, "Stats on which object?\r\n");
    else {
      if ((object = get_obj_vis(ch, buf2, NULL)) != NULL)
	do_stat_object(ch, object);
      else
	send_to_char(ch, "No such object around.\r\n");
    }
  } else if (is_abbrev(buf1, "zone")) {
    if (!*buf2) {
      send_to_char(ch, "Stats on which zone?\r\n");
      return;
    } else {
      print_zone(ch, atoi(buf2));
      return;
    }
  } else {
    char *name = buf1;
    int number = get_number(&name);

    if ((object = get_obj_in_equip_vis(ch, name, &number, ch->equipment)) != NULL)
      do_stat_object(ch, object);
    else if ((object = get_obj_in_list_vis(ch, name, &number, ch->carrying)) != NULL)
      do_stat_object(ch, object);
    else if ((victim = get_char_vis(ch, name, &number, FIND_CHAR_ROOM)) != NULL)
      do_stat_character(ch, victim);
    else if ((object = get_obj_in_list_vis(ch, name, &number, world[IN_ROOM(ch)].contents)) != NULL)
      do_stat_object(ch, object);
    else if ((victim = get_char_vis(ch, name, &number, FIND_CHAR_WORLD)) != NULL)
      do_stat_character(ch, victim);
    else if ((object = get_obj_vis(ch, name, &number)) != NULL)
      do_stat_object(ch, object);
    else
      send_to_char(ch, "Nothing around by that name.\r\n");
  }
}


ACMD(do_shutdown)
{
  char arg[MAX_INPUT_LENGTH];

  if (subcmd != SCMD_SHUTDOWN) {
    send_to_char(ch, "If you want to shut something down, say so!\r\n");
    return;
  }
  one_argument(argument, arg);

  if (!*arg) {
    log("(GC) Shutdown by %s.", GET_NAME(ch));
    send_to_all("Shutting down.\r\n");
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "reboot")) {
    log("(GC) Reboot by %s.", GET_NAME(ch));
    send_to_all("Rebooting.. come back in a minute or two.\r\n");
    touch(FASTBOOT_FILE);
    circle_shutdown = circle_reboot = 1;
  } else if (!str_cmp(arg, "die")) {
    log("(GC) Shutdown by %s.", GET_NAME(ch));
    send_to_all("Shutting down for maintenance.\r\n");
    touch(KILLSCRIPT_FILE);
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "now")) {
    log("(GC) Shutdown NOW by %s.", GET_NAME(ch));
    send_to_all("Rebooting.. come back in a minute or two.\r\n");
    circle_shutdown = 1;
    circle_reboot = 2; /* do not autosave olc */
  } else if (!str_cmp(arg, "pause")) {
    log("(GC) Shutdown by %s.", GET_NAME(ch));
    send_to_all("Shutting down for maintenance.\r\n");
    touch(PAUSE_FILE);
    circle_shutdown = 1;
  } else
    send_to_char(ch, "Unknown shutdown option.\r\n");
}


void snoop_check(struct char_data *ch)
{
  /*  This short routine is to ensure that characters that happen
   *  to be snooping (or snooped) and get advanced/demoted will
   *  not be snooping/snooped someone of a higher/lower level (and
   *  thus, not entitled to be snooping.
   */
  if (!ch || !ch->desc)
    return;
  if (ch->desc->snooping &&
     (GET_LEVEL(ch->desc->snooping->character) >= GET_LEVEL(ch))) {
    ch->desc->snooping->snoop_by = NULL;
    ch->desc->snooping = NULL;
  }

  if (ch->desc->snoop_by &&
     (GET_LEVEL(ch) >= GET_LEVEL(ch->desc->snoop_by->character))) {
    ch->desc->snoop_by->snooping = NULL;
    ch->desc->snoop_by = NULL;
  }
}

void stop_snooping(struct char_data *ch)
{
  if (!ch->desc->snooping)
    send_to_char(ch, "You aren't snooping anyone.\r\n");
  else {
    send_to_char(ch, "You stop snooping.\r\n");
    ch->desc->snooping->snoop_by = NULL;
    ch->desc->snooping = NULL;
  }
}


ACMD(do_snoop)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *victim, *tch;

  if (!ch->desc)
    return;

  one_argument(argument, arg);

  if (!*arg)
    stop_snooping(ch);
  else if (!(victim = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD)))
    send_to_char(ch, "No such person around.\r\n");
  else if (!victim->desc)
    send_to_char(ch, "There's no link.. nothing to snoop.\r\n");
  else if (victim == ch)
    stop_snooping(ch);
  else if (victim->desc->snoop_by)
    send_to_char(ch, "Busy already. \r\n");
  else if (victim->desc->snooping == ch->desc)
    send_to_char(ch, "Don't be stupid.\r\n");
  else {
    if (victim->desc->original)
      tch = victim->desc->original;
    else
      tch = victim;

    if (GET_LEVEL(tch) >= GET_LEVEL(ch)) {
      send_to_char(ch, "You can't.\r\n");
      return;
    }
    send_to_char(ch, "%s", CONFIG_OK);

    if (ch->desc->snooping)
      ch->desc->snooping->snoop_by = NULL;

    ch->desc->snooping = victim->desc;
    victim->desc->snoop_by = ch->desc;
  }
}

ACMD(do_switch)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *victim;

  one_argument(argument, arg);

  if (ch->desc->original)
    send_to_char(ch, "You're already switched.\r\n");
  else if (!*arg)
    send_to_char(ch, "Switch with who?\r\n");
  else if (!(victim = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD)))
    send_to_char(ch, "No such character.\r\n");
  else if (ch == victim)
    send_to_char(ch, "Hee hee... we are jolly funny today, eh?\r\n");
  else if (victim->desc)
    send_to_char(ch, "You can't do that, the body is already in use!\r\n");
  else if ((GET_LEVEL(ch) < LVL_IMPL) && !IS_NPC(victim))
    send_to_char(ch, "You aren't holy enough to use a mortal's body.\r\n");
  else if (GET_LEVEL(ch) < LVL_GRGOD && ROOM_FLAGGED(IN_ROOM(victim), ROOM_GODROOM))
    send_to_char(ch, "You are not godly enough to use that room!\r\n");
  else if (GET_LEVEL(ch) < LVL_GRGOD && ROOM_FLAGGED(IN_ROOM(victim), ROOM_HOUSE)
		&& !House_can_enter(ch, GET_ROOM_VNUM(IN_ROOM(victim))))
    send_to_char(ch, "That's private property -- no trespassing!\r\n");
  else {
    send_to_char(ch, "%s", CONFIG_OK);

    ch->desc->character = victim;
    ch->desc->original = ch;

    victim->desc = ch->desc;
    ch->desc = NULL;
  }
}


ACMD(do_return)
{
  if (ch->desc && ch->desc->original) {
    send_to_char(ch, "You return to your original body.\r\n");

    /*
     * If someone switched into your original body, disconnect them.
     *   - JE 2/22/95
     *
     * Zmey: here we put someone switched in our body to disconnect state
     * but we must also NULL his pointer to our character, otherwise
     * close_socket() will damage our character's pointer to our descriptor
     * (which is assigned below in this function). 12/17/99
     */
    if (ch->desc->original->desc) {
      ch->desc->original->desc->character = NULL;
      STATE(ch->desc->original->desc) = CON_DISCONNECT;
    }

    /* Now our descriptor points to our original body. */
    ch->desc->character = ch->desc->original;
    ch->desc->original = NULL;

    /* And our body's pointer to descriptor now points to our descriptor. */
    ch->desc->character->desc = ch->desc;
    ch->desc = NULL;
  }
}



ACMD(do_load)
{
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    send_to_char(ch, "Usage: load { obj | mob } <number>\r\n");
    return;
  }
  if (!is_number(buf2)) {
    send_to_char(ch, "That is not a number.\r\n");
    return;
  }

  if (is_abbrev(buf, "mob")) {
    struct char_data *mob;
    mob_rnum r_num;

    if ((r_num = real_mobile(atoi(buf2))) == NOBODY) {
      send_to_char(ch, "There is no monster with that number.\r\n");
      return;
    }
    mob = read_mobile(r_num, REAL);
    char_to_room(mob, IN_ROOM(ch));

    act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
	0, 0, TO_ROOM);
    act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
    act("You create $N.", FALSE, ch, 0, mob, TO_CHAR);
    load_mtrigger(mob);
  } else if (is_abbrev(buf, "obj")) {
    struct obj_data *obj;
    obj_rnum r_num;

    if ((r_num = real_object(atoi(buf2))) == NOTHING) {
      send_to_char(ch, "There is no object with that number.\r\n");
      return;
    }
    obj = read_object(r_num, REAL);
    add_unique_id(obj);
    if (CONFIG_LOAD_INVENTORY)
      obj_to_char(obj, ch);
    else
      obj_to_room(obj, IN_ROOM(ch));
    act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
    act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
    act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
    load_otrigger(obj);
  } else
    send_to_char(ch, "That'll have to be either 'obj' or 'mob'.\r\n");
}

ACMD(do_vstat)
{
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    send_to_char(ch, "Usage: vstat { obj | mob } <number>\r\n");
    return;
  }
  if (!is_number(buf2)) {
    send_to_char(ch, "That's not a valid number.\r\n");
    return;
  }

  if (is_abbrev(buf, "mob")) {
    struct char_data *mob;
    mob_rnum r_num;

    if ((r_num = real_mobile(atoi(buf2))) == NOBODY) {
      send_to_char(ch, "There is no monster with that number.\r\n");
      return;
    }
    mob = read_mobile(r_num, REAL);
    char_to_room(mob, 0);
    do_stat_character(ch, mob);
    extract_char(mob);
  } else if (is_abbrev(buf, "obj")) {
    struct obj_data *obj;
    obj_rnum r_num;

    if ((r_num = real_object(atoi(buf2))) == NOTHING) {
      send_to_char(ch, "There is no object with that number.\r\n");
      return;
    }
    obj = read_object(r_num, REAL);
    do_stat_object(ch, obj);
    extract_obj(obj);
  } else
    send_to_char(ch, "That'll have to be either 'obj' or 'mob'.\r\n");
}

ACMD(do_vwear)
{
    send_to_char(ch, "Vwear called successfully!\r\n");
}

/* clean a room of all mobiles and objects */
ACMD(do_purge)
{
  char buf[MAX_INPUT_LENGTH];
  struct char_data *vict;
  struct obj_data *obj;

  one_argument(argument, buf);

  /* argument supplied. destroy single object or char */
  if (*buf) {
    if ((vict = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM)) != NULL) {
      if (!IS_NPC(vict) && (GET_LEVEL(ch) <= GET_LEVEL(vict))) {
	send_to_char(ch, "That action is ill-advised.\r\n");
	return;
      }
      act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

      if (!IS_NPC(vict)) {
	mudlog(BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s has purged %s.", GET_NAME(ch), GET_NAME(vict));
	if (vict->desc) {
	  STATE(vict->desc) = CON_CLOSE;
	  vict->desc->character = NULL;
	  vict->desc = NULL;
	}
      }
      extract_char(vict);
    } else if ((obj = get_obj_in_list_vis(ch, buf, NULL, world[IN_ROOM(ch)].contents)) != NULL) {
      act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
      extract_obj(obj);
    } else {
      send_to_char(ch, "Nothing here by that name.\r\n");
      return;
    }

    send_to_char(ch, "%s", CONFIG_OK);
  } else {			/* no argument. clean out the room */
    int i;

    act("$n makes a sweeping gesture and you are surrounded by scorching flames!",
	FALSE, ch, 0, 0, TO_ROOM);
    send_to_room(IN_ROOM(ch), "The world seems a little cleaner.\r\n");

    for (vict = world[IN_ROOM(ch)].people; vict; vict = vict->next_in_room) {
      if (!IS_NPC(vict))
        continue;

      /* Dump inventory. */
      while (vict->carrying)
        extract_obj(vict->carrying);

      /* Dump equipment. */
      for (i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(vict, i))
          extract_obj(GET_EQ(vict, i));

      /* Dump character. */
      extract_char(vict);
    }

    /* Clear the ground. */
    while (world[IN_ROOM(ch)].contents)
      extract_obj(world[IN_ROOM(ch)].contents);
  }
}



const char *logtypes[] = {
  "off", "brief", "normal", "complete", "\n"
};

ACMD(do_syslog)
{
  char arg[MAX_INPUT_LENGTH];
  int tp;

  one_argument(argument, arg);
  if (!*arg) {
    send_to_char(ch, "Your syslog is currently %s.\r\n",
	logtypes[(PRF_FLAGGED(ch, PRF_LOG1) ? 1 : 0) + (PRF_FLAGGED(ch, PRF_LOG2) ? 2 : 0)]);
    return;
  }
  if (((tp = search_block(arg, logtypes, FALSE)) == -1)) {
    send_to_char(ch, "Usage: syslog { Off | Brief | Normal | Complete }\r\n");
    return;
  }
  REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_LOG1);
  REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_LOG2);
  if (tp & 1) SET_BIT_AR(PRF_FLAGS(ch), PRF_LOG1);
  if (tp & 2) SET_BIT_AR(PRF_FLAGS(ch), PRF_LOG2);

  send_to_char(ch, "Your syslog is now %s.\r\n", logtypes[tp]);
}

#define EXE_FILE "bin/circle" /* maybe use argv[0] but it's not reliable */

/* (c) 1996-97 Erwin S. Andreasen <erwin@pip.dknet.dk> */
ACMD(do_copyover)
{
#ifdef CIRCLE_WINDOWS
  send_to_char(ch, "Copyover is not available for Windows.\r\n");
#else
  FILE *fp;
  struct descriptor_data *d, *d_next;
  char buf [100], buf2[100];
	
  fp = fopen (COPYOVER_FILE, "w");
	
  if (!fp) {
    send_to_char (ch, "Copyover file not writeable, aborted.\n\r");
    return;
  }
	
  /* Consider changing all saved areas here, if you use OLC */
  save_all();
  sprintf (buf, "\t\x1B[1;31m \007\007\007Time stops for a moment as %s folds space and time.\x1B[0;0m\r\n", GET_NAME(ch));
  /* For each playing descriptor, save its state */
  for (d = descriptor_list; d ; d = d_next) {
    struct char_data * och = d->character;
    d_next = d->next; /* We delete from the list , so need to save this */
    if (!d->character || d->connected > CON_PLAYING) {
      write_to_descriptor (d->descriptor, "\n\rSorry, we are rebooting. Come back in a few seconds.\n\r", d->comp);
      close_socket (d); /* throw'em out */
    } else {
      fprintf (fp, "%d %s %s %d\n", d->descriptor, GET_NAME(och), d->host, GET_ROOM_VNUM(IN_ROOM(och)));
      log("printing descriptor name and host of connected players");
      /* save och */
      Crash_rentsave(och, 0);
      save_char(och);
#ifdef HAVE_ZLIB_H
      if (d->comp->state == 2) {
        d->comp->state = 3; /* Code to use Z_FINISH for deflate */
      }
#endif /* HAVE_ZLIB_H */
      write_to_descriptor (d->descriptor, buf, d->comp);
      d->comp->state = 0;
#ifdef HAVE_ZLIB_H
      if (d->comp->stream) {
        deflateEnd(d->comp->stream);
        free(d->comp->stream);
        free(d->comp->buff_out);
        free(d->comp->buff_in);
      }
#endif /* HAVE_ZLIB_H */
    }
  }
	
  fprintf (fp, "-1\n");
  fclose (fp);

  /* Close reserve and other always-open files and release other resources
     since we are now using ASCII pfiles, closing the player_fl would crash
     the game, since it's no longer around, so I commented it out. I'll
     leave the code here, for historical reasons -spl
     fclose(player_fl); */

  /* exec - descriptors are inherited */
	
  sprintf (buf, "%d", port);
  sprintf (buf2, "-C%d", mother_desc);
  chdir ("..");
  execl (EXE_FILE, "circle", buf2, buf, (char *) NULL);
  /* Failed - sucessful exec will not return */
	
  perror ("do_copyover: execl");
  send_to_char (ch, "Copyover FAILED!\n\r");
	
  exit (1); /* too much trouble to try to recover! */
#endif
}

ACMD(do_advance)
{
  struct char_data *victim;
  char name[MAX_INPUT_LENGTH], level[MAX_INPUT_LENGTH];
  int newlevel, oldlevel;

  two_arguments(argument, name, level);

  if (*name) {
    if (!(victim = get_char_vis(ch, name, NULL, FIND_CHAR_WORLD))) {
      send_to_char(ch, "That player is not here.\r\n");
      return;
    }
  } else {
    send_to_char(ch, "Advance who?\r\n");
    return;
  }

  if (GET_LEVEL(ch) <= GET_LEVEL(victim)) {
    send_to_char(ch, "Maybe that's not such a great idea.\r\n");
    return;
  }
  if (IS_NPC(victim)) {
    send_to_char(ch, "NO!  Not on NPCs.\r\n");
    return;
  }
  if (!*level || (newlevel = atoi(level)) <= 0) {
    send_to_char(ch, "That's not a level!\r\n");
    return;
  }
  if (newlevel > LVL_IMPL) {
    send_to_char(ch, "%d is the highest possible level.\r\n", LVL_IMPL);
    return;
  }
  if (newlevel > GET_LEVEL(ch)) {
    send_to_char(ch, "Yeah, right.\r\n");
    return;
  }
  if (newlevel == GET_LEVEL(victim)) {
    send_to_char(ch, "They are already at that level.\r\n");
    return;
  }
  oldlevel = GET_LEVEL(victim);
  if (newlevel < GET_LEVEL(victim)) {
    do_start(victim);
    GET_LEVEL(victim) = newlevel;
    send_to_char(victim, "You are momentarily enveloped by darkness!\r\nYou feel somewhat diminished.\r\n");
  } else {
    act("$n makes some strange gestures.\r\n"
	"A strange feeling comes upon you,\r\n"
	"Like a giant hand, light comes down\r\n"
	"from above, grabbing your body, that\r\n"
	"begins to pulse with colored lights\r\n"
	"from inside.\r\n\r\n"
	"Your head seems to be filled with demons\r\n"
	"from another plane as your body dissolves\r\n"
	"to the elements of time and space itself.\r\n"
	"Suddenly a silent explosion of light\r\n"
	"snaps you back to reality.\r\n\r\n"
	"You feel slightly different.", FALSE, ch, 0, victim, TO_VICT);
  }

  send_to_char(ch, "%s", CONFIG_OK);

  if (newlevel < oldlevel)
    log("(GC) %s demoted %s from level %d to %d.",
		GET_NAME(ch), GET_NAME(victim), oldlevel, newlevel);
  else
    log("(GC) %s has advanced %s to level %d (from %d)",
		GET_NAME(ch), GET_NAME(victim), newlevel, oldlevel);

  if (oldlevel >= LVL_IMMORT && newlevel < LVL_IMMORT) {
    /* If they are no longer an immortal, let's remove some of the
     * nice immortal only flags, shall we?
     */
  REMOVE_BIT_AR(PRF_FLAGS(victim), PRF_LOG1);
  REMOVE_BIT_AR(PRF_FLAGS(victim), PRF_LOG2);
  REMOVE_BIT_AR(PRF_FLAGS(victim), PRF_NOHASSLE);
  REMOVE_BIT_AR(PRF_FLAGS(victim), PRF_HOLYLIGHT);
  REMOVE_BIT_AR(PRF_FLAGS(victim), PRF_ROOMFLAGS);
  run_autowiz();
  } else if (oldlevel < LVL_IMMORT && newlevel >= LVL_IMMORT) {
     SET_BIT_AR(PRF_FLAGS(victim), PRF_LOG2);
     SET_BIT_AR(PRF_FLAGS(victim), PRF_HOLYLIGHT);
     SET_BIT_AR(PRF_FLAGS(victim), PRF_ROOMFLAGS);
     SET_BIT_AR(PRF_FLAGS(victim), PRF_AUTOEXIT);
     SET_BIT_AR(PRF_FLAGS(victim), PRF_COLOR_1);
     SET_BIT_AR(PRF_FLAGS(victim), PRF_COLOR_2);
     run_autowiz();
  }

  gain_exp_regardless(victim, level_exp(GET_CLASS(victim), newlevel) - GET_EXP(victim));
  save_char(victim);
}

ACMD(do_restore)
{
  char buf[MAX_INPUT_LENGTH];
  struct char_data *vict;
  int i;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char(ch, "Whom do you wish to restore?\r\n");
  else if (!(vict = get_char_vis(ch, buf, NULL, FIND_CHAR_WORLD)))
    send_to_char(ch, "%s", CONFIG_NOPERSON);
  else if (!IS_NPC(vict) && ch != vict && GET_LEVEL(vict) >= GET_LEVEL(ch))
    send_to_char(ch, "They don't need your help.\r\n");
  else {
    GET_HIT(vict) = GET_MAX_HIT(vict);
    GET_MANA(vict) = GET_MAX_MANA(vict);
    GET_MOVE(vict) = GET_MAX_MOVE(vict);

    if (!IS_NPC(vict) && GET_LEVEL(ch) >= LVL_GRGOD) {
      if (GET_LEVEL(vict) >= LVL_IMMORT)
        for (i = 1; i <= MAX_SKILLS; i++)
          SET_SKILL(vict, i, 100);

      if (GET_LEVEL(vict) >= LVL_GRGOD) {
	vict->real_abils.intel = 25;
	vict->real_abils.wis = 25;
	vict->real_abils.dex = 25;
	vict->real_abils.str = 25;
	vict->real_abils.con = 25;
	vict->real_abils.cha = 25;
      }
    }
    update_pos(vict);
    affect_total(vict);
    send_to_char(ch, "%s", CONFIG_OK);
    act("You have been fully healed by $N!", FALSE, vict, 0, ch, TO_CHAR);
  }
}


void perform_immort_vis(struct char_data *ch)
{
  if (GET_INVIS_LEV(ch) == 0 && (!AFF_FLAGGED(ch, AFF_HIDE) && !AFF_FLAGGED(ch, AFF_INVISIBLE))) {
    send_to_char(ch, "You are already fully visible.\r\n");
    return;
  }
   
  GET_INVIS_LEV(ch) = 0;
  appear(ch);
  send_to_char(ch, "You are now fully visible.\r\n");
}


void perform_immort_invis(struct char_data *ch, int level)
{
  struct char_data *tch;

  for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room) {
    if (tch == ch)
      continue;
    if (GET_LEVEL(tch) >= GET_INVIS_LEV(ch) && GET_LEVEL(tch) < level)
      act("You blink and suddenly realize that $n is gone.", FALSE, ch, 0,
	  tch, TO_VICT);
    if (GET_LEVEL(tch) < GET_INVIS_LEV(ch) && GET_LEVEL(tch) >= level)
      act("You suddenly realize that $n is standing beside you.", FALSE, ch, 0,
	  tch, TO_VICT);
  }

  GET_INVIS_LEV(ch) = level;
  send_to_char(ch, "Your invisibility level is %d.\r\n", level);
}
  

ACMD(do_invis)
{
  char arg[MAX_INPUT_LENGTH];
  int level;

  if (IS_NPC(ch)) {
    send_to_char(ch, "You can't do that!\r\n");
    return;
  }

  one_argument(argument, arg);
  if (!*arg) {
    if (GET_INVIS_LEV(ch) > 0)
      perform_immort_vis(ch);
    else
      perform_immort_invis(ch, GET_LEVEL(ch));
  } else {
    level = atoi(arg);
    if (level > GET_LEVEL(ch))
      send_to_char(ch, "You can't go invisible above your own level.\r\n");
    else if (level < 1)
      perform_immort_vis(ch);
    else
      perform_immort_invis(ch, level);
  }
}


ACMD(do_gecho)
{
  struct descriptor_data *pt;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument)
    send_to_char(ch, "You must provide a message to send.\r\n");
  else {
    for (pt = descriptor_list; pt; pt = pt->next)
      if (IS_PLAYING(pt) && pt->character && pt->character != ch)
	send_to_char(pt->character, "%s\r\n", argument);

    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(ch, "%s", CONFIG_OK);
    else
      send_to_char(ch, "%s\r\n", argument);
  }
}

ACMD(do_aecho)
{
  struct descriptor_data *pt;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument)
    send_to_char(ch, "You must provide a message to send.\r\n");
  else {
    for (pt = descriptor_list; pt; pt = pt->next)
      if (IS_PLAYING(pt) && pt->character && pt->character != ch)
	send_to_char(pt->character, "%s\r\n", argument);

    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(ch, "%s", CONFIG_OK);
    else
      send_to_char(ch, "%s\r\n", argument);
  }
}

ACMD(do_poofset)
{
  char **msg;

  switch (subcmd) {
  case SCMD_POOFIN:    msg = &(POOFIN(ch));    break;
  case SCMD_POOFOUT:   msg = &(POOFOUT(ch));   break;
  default:    return;
  }

  skip_spaces(&argument);

  if (*msg)
    free(*msg);

  if (!*argument)
    *msg = NULL;
  else
    *msg = strdup(argument);

  send_to_char(ch, "%s", CONFIG_OK);
}



ACMD(do_dc)
{
  char arg[MAX_INPUT_LENGTH];
  struct descriptor_data *d;
  int num_to_dc;

  one_argument(argument, arg);
  if (!(num_to_dc = atoi(arg))) {
    send_to_char(ch, "Usage: DC <user number> (type USERS for a list)\r\n");
    return;
  }
  for (d = descriptor_list; d && d->desc_num != num_to_dc; d = d->next);

  if (!d) {
    send_to_char(ch, "No such connection.\r\n");
    return;
  }
  if (d->character && GET_LEVEL(d->character) >= GET_LEVEL(ch)) {
    if (!CAN_SEE(ch, d->character))
      send_to_char(ch, "No such connection.\r\n");
    else
      send_to_char(ch, "Umm.. maybe that's not such a good idea...\r\n");
    return;
  }

  /* We used to just close the socket here using close_socket(), but
   * various people pointed out this could cause a crash if you're
   * closing the person below you on the descriptor list.  Just setting
   * to CON_CLOSE leaves things in a massively inconsistent state so I
   * had to add this new flag to the descriptor. -je
   *
   * It is a much more logical extension for a CON_DISCONNECT to be used
   * for in-game socket closes and CON_CLOSE for out of game closings.
   * This will retain the stability of the close_me hack while being
   * neater in appearance. -gg 12/1/97
   *
   * For those unlucky souls who actually manage to get disconnected
   * by two different immortals in the same 1/10th of a second, we have
   * the below 'if' check. -gg 12/17/99
   */
  if (STATE(d) == CON_DISCONNECT || STATE(d) == CON_CLOSE)
    send_to_char(ch, "They're already being disconnected.\r\n");
  else {
    /*
     * Remember that we can disconnect people not in the game and
     * that rather confuses the code when it expected there to be
     * a character context.
     */
    if (STATE(d) == CON_PLAYING)
      STATE(d) = CON_DISCONNECT;
    else
      STATE(d) = CON_CLOSE;

    send_to_char(ch, "Connection #%d closed.\r\n", num_to_dc);
    log("(GC) Connection closed by %s.", GET_NAME(ch));
  }
}



ACMD(do_wizlock)
{
  char arg[MAX_INPUT_LENGTH];
  int value;
  const char *when;

  one_argument(argument, arg);
  if (*arg) {
    value = atoi(arg);
    if (value < 0 || value > GET_LEVEL(ch)) {
      send_to_char(ch, "Invalid wizlock value.\r\n");
      return;
    }
    circle_restrict = value;
    when = "now";
  } else
    when = "currently";

  switch (circle_restrict) {
  case 0:
    send_to_char(ch, "The game is %s completely open.\r\n", when);
    break;
  case 1:
    send_to_char(ch, "The game is %s closed to new players.\r\n", when);
    break;
  default:
    send_to_char(ch, "Only level %d and above may enter the game %s.\r\n", circle_restrict, when);
    break;
  }
}


ACMD(do_date)
{
  char *tmstr;
  time_t mytime;
  int d, h, m;

  if (subcmd == SCMD_DATE)
    mytime = time(0);
  else
    mytime = boot_time;

  tmstr = (char *) asctime(localtime(&mytime));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  if (subcmd == SCMD_DATE)
    send_to_char(ch, "Current machine time: %s\r\n", tmstr);
  else {
    mytime = time(0) - boot_time;
    d = mytime / 86400;
    h = (mytime / 3600) % 24;
    m = (mytime / 60) % 60;

    send_to_char(ch, "Up since %s: %d day%s, %d:%02d\r\n", tmstr, d, d == 1 ? "" : "s", h, m);
  }
}



ACMD(do_last)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict = NULL;

  one_argument(argument, arg);
  if (!*arg) {
    send_to_char(ch, "For whom do you wish to search?\r\n");
    return;
  }
  CREATE(vict, struct char_data, 1);
  clear_char(vict);
  CREATE(vict->player_specials, struct player_special_data, 1);
  if (load_char(arg, vict) <  0) {
    send_to_char(ch, "There is no such player.\r\n");
    free_char(vict);
    return;
  }
  if ((GET_LEVEL(vict) > GET_LEVEL(ch)) && (GET_LEVEL(ch) < LVL_IMPL)) {
    send_to_char(ch, "You are not sufficiently godly for that!\r\n");
    return;
  }

  send_to_char(ch, "[%5ld] [%2d %s %s] %-12s : %-18s : %-20s\r\n",
    GET_IDNUM(vict), (int) GET_LEVEL(vict),
    race_abbrevs[(int) GET_RACE(vict)], class_abbrevs[(int) GET_CLASS(vict)],
    GET_NAME(vict), vict->player_specials->host && *vict->player_specials->host
    ? vict->player_specials->host : "(NOHOST)",
    ctime(&vict->player.time.logon));
  free_char(vict);
}


ACMD(do_force)
{
  struct descriptor_data *i, *next_desc;
  struct char_data *vict, *next_force;
  char arg[MAX_INPUT_LENGTH], to_force[MAX_INPUT_LENGTH], buf1[MAX_INPUT_LENGTH + 32];

  half_chop(argument, arg, to_force);

  snprintf(buf1, sizeof(buf1), "$n has forced you to '%s'.", to_force);

  if (!*arg || !*to_force)
    send_to_char(ch, "Whom do you wish to force do what?\r\n");
  else if ((GET_LEVEL(ch) < LVL_GRGOD) || (str_cmp("all", arg) && str_cmp("room", arg))) {
    if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD)))
      send_to_char(ch, "%s", CONFIG_NOPERSON);
    else if (!IS_NPC(vict) && GET_LEVEL(ch) <= GET_LEVEL(vict))
      send_to_char(ch, "No, no, no!\r\n");
    else {
      send_to_char(ch, "%s", CONFIG_OK);
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      mudlog(NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s forced %s to %s", GET_NAME(ch), GET_NAME(vict), to_force);
      command_interpreter(vict, to_force);
    }
  } else if (!str_cmp("room", arg)) {
    send_to_char(ch, "%s", CONFIG_OK);
    mudlog(NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s forced room %d to %s",
		GET_NAME(ch), GET_ROOM_VNUM(IN_ROOM(ch)), to_force);

    for (vict = world[IN_ROOM(ch)].people; vict; vict = next_force) {
      next_force = vict->next_in_room;
      if (!IS_NPC(vict) && GET_LEVEL(vict) >= GET_LEVEL(ch))
	continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  } else { /* force all */
    send_to_char(ch, "%s", CONFIG_OK);
    mudlog(NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s forced all to %s", GET_NAME(ch), to_force);

    for (i = descriptor_list; i; i = next_desc) {
      next_desc = i->next;

      if (STATE(i) != CON_PLAYING || !(vict = i->character) || (!IS_NPC(vict) && GET_LEVEL(vict) >= GET_LEVEL(ch)))
	continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  }
}



ACMD(do_wiznet)
{
  char buf1[MAX_INPUT_LENGTH + MAX_NAME_LENGTH + 32],
	buf2[MAX_INPUT_LENGTH + MAX_NAME_LENGTH + 32];
  struct descriptor_data *d;
  char emote = FALSE;
  char any = FALSE;
  int level = LVL_IMMORT;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument) {
    send_to_char(ch, "Usage: wiznet <text> | #<level> <text> | *<emotetext> |\r\n        wiznet @<level> *<emotetext> | wiz @\r\n");
    return;
  }
  switch (*argument) {
  case '*':
    emote = TRUE;
  case '#':
    one_argument(argument + 1, buf1);
    if (is_number(buf1)) {
      half_chop(argument+1, buf1, argument);
      level = MAX(atoi(buf1), LVL_IMMORT);
      if (level > GET_LEVEL(ch)) {
	send_to_char(ch, "You can't wizline above your own level.\r\n");
	return;
      }
    } else if (emote)
      argument++;
    break;

  case '@':
    send_to_char(ch, "God channel status:\r\n");
    for (any = 0, d = descriptor_list; d; d = d->next) {
      if (STATE(d) != CON_PLAYING || GET_LEVEL(d->character) < LVL_IMMORT)
        continue;
      if (!CAN_SEE(ch, d->character))
        continue;

      send_to_char(ch, "  %-*s%s%s%s\r\n", MAX_NAME_LENGTH, GET_NAME(d->character),
		PLR_FLAGGED(d->character, PLR_WRITING) ? " (Writing)" : "",
		PLR_FLAGGED(d->character, PLR_MAILING) ? " (Writing mail)" : "",
		PRF_FLAGGED(d->character, PRF_NOWIZ) ? " (Offline)" : "");
    }
    return;

  case '\\':
    ++argument;
    break;
  default:
    break;
  }
  if (PRF_FLAGGED(ch, PRF_NOWIZ)) {
    send_to_char(ch, "You are offline!\r\n");
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    send_to_char(ch, "Don't bother the gods like that!\r\n");
    return;
  }
  if (level > LVL_IMMORT) {
    snprintf(buf1, sizeof(buf1), "%s: <%d> %s%s\r\n", GET_NAME(ch), level, emote ? "<--- " : "", argument);
    snprintf(buf2, sizeof(buf1), "Someone: <%d> %s%s\r\n", level, emote ? "<--- " : "", argument);
  } else {
    snprintf(buf1, sizeof(buf1), "%s: %s%s\r\n", GET_NAME(ch), emote ? "<--- " : "", argument);
    snprintf(buf2, sizeof(buf1), "Someone: %s%s\r\n", emote ? "<--- " : "", argument);
  }

  for (d = descriptor_list; d; d = d->next) {
    if ((STATE(d) == CON_PLAYING) && (GET_LEVEL(d->character) >= level) &&
	(!PRF_FLAGGED(d->character, PRF_NOWIZ)) &&
	(!PLR_FLAGGED(d->character, PLR_WRITING | PLR_MAILING))
	&& (d != ch->desc || !(PRF_FLAGGED(d->character, PRF_NOREPEAT)))) {
      send_to_char(d->character, "%s", CCCYN(d->character, C_NRM));
      if (CAN_SEE(d->character, ch))
	send_to_char(d->character, "%s", buf1);
      else
	send_to_char(d->character, "%s", buf2);
      send_to_char(d->character, "%s", CCNRM(d->character, C_NRM));
    }
  }

  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(ch, "%s", CONFIG_OK);
}



ACMD(do_zreset)
{
  char arg[MAX_INPUT_LENGTH];
  zone_rnum i;
  zone_vnum j;

  one_argument(argument, arg);

  if (*arg == '*') {
    if (GET_LEVEL(ch) < LVL_GOD){
      send_to_char(ch, "You do not have permission to reset the entire world.\r\n");
      return;
   } else {
    for (i = 0; i <= top_of_zone_table; i++)
      reset_zone(i);
    send_to_char(ch, "Reset world.\r\n");
    mudlog(NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s reset entire world.", GET_NAME(ch));
    return; }
  } else if (*arg == '.' || !*arg)
    i = world[IN_ROOM(ch)].zone;
  else {
    j = atoi(arg);
    for (i = 0; i <= top_of_zone_table; i++)
      if (zone_table[i].number == j)
	break;
  }
  if (i <= top_of_zone_table && (can_edit_zone(ch, i) || GET_LEVEL(ch) > LVL_IMMORT)) {
    reset_zone(i);
    send_to_char(ch, "Reset zone #%d: %s.\r\n", zone_table[i].number, zone_table[i].name);
    mudlog(NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s reset zone %d (%s)", GET_NAME(ch), zone_table[i].number, zone_table[i].name);
  } else
    send_to_char(ch, "You do not have permission to reset this zone. Try %d.\r\n", GET_OLC_ZONE(ch));
}

/*
 *  General fn for wizcommands of the sort: cmd <player>
 */
ACMD(do_wizutil)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;
  int taeller;
  long result;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char(ch, "Yes, but for whom?!?\r\n");
  else if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD)))
    send_to_char(ch, "There is no such player.\r\n");
  else if (IS_NPC(vict))
    send_to_char(ch, "You can't do that to a mob!\r\n");
  else if (GET_LEVEL(vict) > GET_LEVEL(ch))
    send_to_char(ch, "Hmmm...you'd better not.\r\n");
  else {
    switch (subcmd) {
    case SCMD_REROLL:
      send_to_char(ch, "Rerolled...\r\n");
      roll_real_abils(vict);
      log("(GC) %s has rerolled %s.", GET_NAME(ch), GET_NAME(vict));
      send_to_char(ch, "New stats: Str %d, Int %d, Wis %d, Dex %d, Con %d, Cha %d\r\n",
	      GET_STR(vict), GET_INT(vict), GET_WIS(vict),
	      GET_DEX(vict), GET_CON(vict), GET_CHA(vict));
      break;
    case SCMD_PARDON:
      if (!PLR_FLAGGED(vict, PLR_THIEF) &&  !PLR_FLAGGED(vict, PLR_KILLER)) {
	send_to_char(ch, "Your victim is not flagged.\r\n");
	return;
      }
      REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_THIEF);
      REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_KILLER);
      send_to_char(ch, "Pardoned.\r\n");
      send_to_char(vict, "You have been pardoned by the Gods!\r\n");
      mudlog(BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s pardoned by %s", GET_NAME(vict), GET_NAME(ch));
      break;
    case SCMD_NOTITLE:
      result = PLR_TOG_CHK(vict, PLR_NOTITLE);
      mudlog(NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) Notitle %s for %s by %s.",
		ONOFF(result), GET_NAME(vict), GET_NAME(ch));
      send_to_char(ch, "(GC) Notitle %s for %s by %s.\r\n", ONOFF(result), GET_NAME(vict), GET_NAME(ch));
      break;
    case SCMD_SQUELCH:
      result = PLR_TOG_CHK(vict, PLR_NOSHOUT);
      mudlog(BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) Squelch %s for %s by %s.",
		ONOFF(result), GET_NAME(vict), GET_NAME(ch));
      send_to_char(ch, "(GC) Squelch %s for %s by %s.\r\n", ONOFF(result), GET_NAME(vict), GET_NAME(ch));
      break;
    case SCMD_FREEZE:
      if (ch == vict) {
	send_to_char(ch, "Oh, yeah, THAT'S real smart...\r\n");
	return;
      }
      if (PLR_FLAGGED(vict, PLR_FROZEN)) {
	send_to_char(ch, "Your victim is already pretty cold.\r\n");
	return;
      }
      SET_BIT_AR(PLR_FLAGS(vict), PLR_FROZEN);
      GET_FREEZE_LEV(vict) = GET_LEVEL(ch);
      send_to_char(vict, "A bitter wind suddenly rises and drains every erg of heat from your body!\r\nYou feel frozen!\r\n");
      send_to_char(ch, "Frozen.\r\n");
      act("A sudden cold wind conjured from nowhere freezes $n!", FALSE, vict, 0, 0, TO_ROOM);
      mudlog(BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      break;
    case SCMD_THAW:
      if (!PLR_FLAGGED(vict, PLR_FROZEN)) {
	send_to_char(ch, "Sorry, your victim is not morbidly encased in ice at the moment.\r\n");
	return;
      }
      if (GET_FREEZE_LEV(vict) > GET_LEVEL(ch)) {
	send_to_char(ch, "Sorry, a level %d God froze %s... you can't unfreeze %s.\r\n",
		GET_FREEZE_LEV(vict), GET_NAME(vict), HMHR(vict));
	return;
      }
      mudlog(BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s un-frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_FROZEN);
      send_to_char(vict, "A fireball suddenly explodes in front of you, melting the ice!\r\nYou feel thawed.\r\n");
      send_to_char(ch, "Thawed.\r\n");
      act("A sudden fireball conjured from nowhere thaws $n!", FALSE, vict, 0, 0, TO_ROOM);
      break;
    case SCMD_UNAFFECT:
      if (vict->affected || AFF_FLAGS(vict) || vict->affectedv) {
	while (vict->affected)
	  affect_remove(vict, vict->affected);
    for(taeller=0; taeller < AF_ARRAY_MAX; taeller++)
      AFF_FLAGS(ch)[taeller] = 0;
        while (vict->affectedv)
          affectv_remove(vict, vict->affectedv);
          for(taeller=0; taeller < AF_ARRAY_MAX; taeller++)
          AFF_FLAGS(ch)[taeller] = 0;
	send_to_char(vict, "There is a brief flash of light!\r\nYou feel slightly different.\r\n");
	send_to_char(ch, "All spells removed.\r\n");
      } else {
	send_to_char(ch, "Your victim does not have any affections!\r\n");
	return;
      }
      break;
    default:
      log("SYSERR: Unknown subcmd %d passed to do_wizutil (%s)", subcmd, __FILE__);
      /*  SYSERR_DESC:
       *  This is the same as the unhandled case in do_gen_ps(), but this
       *  function handles 'reroll', 'pardon', 'freeze', etc.
       */
      break;
    }
    save_char(vict);
  }
}


/* single zone printing fn used by "show zone" so it's not repeated in the
   code 3 times ... -je, 4/6/93 */

/* FIXME: overflow possible */
size_t print_zone_to_buf(char *bufptr, size_t left, zone_rnum zone, int listall)
{
  size_t tmp;
  
  if (listall) {
    int i, j, k, l, m, n, o;
    extern int top_of_trigt;
    extern struct index_data **trig_index;
    int count_shops(shop_vnum low, shop_vnum high);
    int count_guilds(shop_vnum low, shop_vnum high);

    tmp = snprintf(bufptr, left,
	"%3d %-30.30s By: %-10.10s Age: %3d; Reset: %3d (%1d); Range: %5d-%5d\r\n",
	zone_table[zone].number, zone_table[zone].name, zone_table[zone].builders,
	zone_table[zone].age, zone_table[zone].lifespan,
	zone_table[zone].reset_mode,
	zone_table[zone].bot, zone_table[zone].top);
        i = j = k = l = m = n = 0;
        
        for (i = 0; i < top_of_world; i++)
          if (world[i].number >= zone_table[zone].bot && world[i].number <= zone_table[zone].top)
            j++;

        for (i = 0; i < top_of_objt; i++)
          if (obj_index[i].vnum >= zone_table[zone].bot && obj_index[i].vnum <= zone_table[zone].top)
            k++;
        
        for (i = 0; i < top_of_mobt; i++)
          if (mob_index[i].vnum >= zone_table[zone].bot && mob_index[i].vnum <= zone_table[zone].top)
            l++;

        m = count_shops(zone_table[zone].bot, zone_table[zone].top);

        for (i = 0; i < top_of_trigt; i++)
          if (trig_index[i]->vnum >= zone_table[zone].bot && trig_index[i]->vnum <= zone_table[zone].top)
            n++;

        o = count_guilds(zone_table[zone].bot, zone_table[zone].top);

	tmp += snprintf(bufptr + tmp, left - tmp,
                        "       Zone stats:\r\n"
                        "       ---------------\r\n"
                        "         Rooms:    %2d\r\n"
                        "         Objects:  %2d\r\n"
                        "         Mobiles:  %2d\r\n"
                        "         Shops:    %2d\r\n"
                        "         Triggers: %2d\r\n" 
                        "         Guilds:   %2d\r\n", 
                          j, k, l, m, n, o);
        
    return tmp;
  } 
  
  return snprintf(bufptr, left,
        "%3d %-*s%s By: %-10.10s%s Range: %5d-%5d\r\n", zone_table[zone].number,
        count_color_chars(zone_table[zone].name)+30, zone_table[zone].name, KNRM,
        zone_table[zone].builders, KNRM, zone_table[zone].bot, zone_table[zone].top);
}


ACMD(do_show)
{
  int i, j, k, l, con;                /* i, j, k to specifics? */
  size_t len, nlen;
  zone_rnum zrn;
  zone_vnum zvn;
  int low, high;
  byte self = FALSE;
  struct char_data *vict = NULL;
  struct obj_data *obj;
  struct descriptor_data *d;
  char field[MAX_INPUT_LENGTH], value[MAX_INPUT_LENGTH], *strp,
	arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  extern int top_of_trigt;

  struct show_struct {
    const char *cmd;
    const char level;
  } fields[] = {
    { "nothing",	0  },				/* 0 */
    { "zones",		LVL_IMMORT },			/* 1 */
    { "player",		LVL_GOD },
    { "rent",		LVL_GRGOD },
    { "stats",		LVL_IMMORT },
    { "errors",		LVL_IMPL },			/* 5 */
    { "death",		LVL_GOD },
    { "godrooms",	LVL_IMMORT },
    { "shops",		LVL_IMMORT },
    { "houses",		LVL_GOD },
    { "snoop",		LVL_GRGOD },			/* 10 */
    { "assemblies",     LVL_IMMORT },
    { "guilds",         LVL_GOD },
    { "uniques",        LVL_GRGOD },
    { "\n", 0 }
  };

  skip_spaces(&argument);

  if (!*argument) {
    send_to_char(ch, "Show options:\r\n");
    for (j = 0, i = 1; fields[i].level; i++)
      if (fields[i].level <= GET_LEVEL(ch))
	send_to_char(ch, "%-15s%s", fields[i].cmd, (!(++j % 5) ? "\r\n" : ""));
    send_to_char(ch, "\r\n");
    return;
  }

  strcpy(arg, two_arguments(argument, field, value));	/* strcpy: OK (argument <= MAX_INPUT_LENGTH == arg) */

  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;

  if (GET_LEVEL(ch) < fields[l].level) {
    send_to_char(ch, "You are not godly enough for that!\r\n");
    return;
  }
  if (!strcmp(value, "."))
    self = TRUE;
  buf[0] = '\0';

  switch (l) {
  /* show zone */
  case 1:
    /* tightened up by JE 4/6/93 */
    if (self)
      print_zone_to_buf(buf, sizeof(buf), world[IN_ROOM(ch)].zone, 1);
    else if (*value && is_number(value)) {
      for (zvn = atoi(value), zrn = 0; zone_table[zrn].number != zvn && zrn <= top_of_zone_table; zrn++);
      if (zrn <= top_of_zone_table)
	print_zone_to_buf(buf, sizeof(buf), zrn, 1);
      else {
	send_to_char(ch, "That is not a valid zone.\r\n");
	return;
      }
    } else
      for (len = zrn = 0; zrn <= top_of_zone_table; zrn++) {
	nlen = print_zone_to_buf(buf + len, sizeof(buf) - len, zrn, 0);
        if (len + nlen >= sizeof(buf) || nlen < 0)
          break;
        len += nlen;
      }
    page_string(ch->desc, buf, TRUE);
    break;

  /* show player */
  case 2:
    if (!*value) {
      send_to_char(ch, "A name would help.\r\n");
      return;
    }

    CREATE(vict, struct char_data, 1);
    clear_char(vict);
    CREATE(vict->player_specials, struct player_special_data, 1);
    if (load_char(value, vict) < 0) {
      send_to_char(ch, "There is no such player.\r\n");
      free_char(vict);
      return;
    }
    send_to_char(ch, "Player: %-12s (%s) [%2d %s %s]\r\n", GET_NAME(vict),
      genders[(int) GET_SEX(vict)], GET_LEVEL(vict), class_abbrevs[(int)
      GET_CLASS(vict)], race_abbrevs[(int) GET_RACE(vict)]);
    send_to_char(ch, "Au: %-8d  Bal: %-8d  Exp: %-8d  Align: %-5d  Lessons: %-3d\r\n",
      GET_GOLD(vict), GET_BANK_GOLD(vict), GET_EXP(vict),
      GET_ALIGNMENT(vict), GET_PRACTICES(vict));

    /* ctime() uses static buffer: do not combine. */
    send_to_char(ch, "Started: %-20.16s  ", ctime(&vict->player.time.birth));
    send_to_char(ch, "Last: %-20.16s  Played: %3dh %2dm\r\n",
      ctime(&vict->player.time.logon),
      (int) (vict->player.time.played / 3600),
      (int) (vict->player.time.played / 60 % 60));
    free_char(vict);
    break;

  /* show rent */
  case 3:
    if (!*value) {
      send_to_char(ch, "A name would help.\r\n");
      return;
    }
    Crash_listrent(ch, value);
    break;

  /* show stats */
  case 4:
    i = 0;
    j = 0;
    k = 0;
    con = 0;
    for (vict = character_list; vict; vict = vict->next) {
      if (IS_NPC(vict))
	j++;
      else if (CAN_SEE(ch, vict)) {
	i++;
	if (vict->desc)
	  con++;
      }
    }
    for (obj = object_list; obj; obj = obj->next)
      k++;
    send_to_char(ch,
	"Current stats:\r\n"
	"  %5d players in game  %5d connected\r\n"
	"  %5d registered\r\n"
	"  %5d mobiles          %5d prototypes\r\n"
	"  %5d objects          %5d prototypes\r\n"
	"  %5d rooms            %5d zones\r\n"
        "  %5d triggers\r\n"
	"  %5d large bufs\r\n"
	"  %5d buf switches     %5d overflows\r\n",
	i, con,
	top_of_p_table + 1,
	j, top_of_mobt + 1,
	k, top_of_objt + 1,
	top_of_world + 1, top_of_zone_table + 1,
	top_of_trigt + 1,
	buf_largecount,
	buf_switches, buf_overflows
	);
    break;

  /* show errors */
  case 5:
    len = strlcpy(buf, "Errant Rooms\r\n------------\r\n", sizeof(buf));
    for (i = 0, k = 0; i <= top_of_world; i++)
      for (j = 0; j < NUM_OF_DIRS; j++) {
      	if (!W_EXIT(i,j))
      	  continue;
        if (W_EXIT(i,j)->to_room == 0) {
            nlen = snprintf(buf + len, sizeof(buf) - len, "%2d: (void   ) [%5d] %-*s%s (%s)\r\n", ++k, GET_ROOM_VNUM(i), count_color_chars(world[i].name)+40, world[i].name, QNRM, dirs[j]);
          if (len + nlen >= sizeof(buf) || nlen < 0)
            break;
          len += nlen;
        }
        if (W_EXIT(i,j)->to_room == NOWHERE && !W_EXIT(i,j)->general_description) {
            nlen = snprintf(buf + len, sizeof(buf) - len, "%2d: (Nowhere) [%5d] %-*s%s (%s)\r\n", ++k, GET_ROOM_VNUM(i), count_color_chars(world[i].name)+ 40, world[i].name, QNRM, dirs[j]);
          if (len + nlen >= sizeof(buf) || nlen < 0)
            break;
          len += nlen;
        }
      }
    page_string(ch->desc, buf, TRUE);
    break;

  /* show death */
  case 6:
    len = strlcpy(buf, "Death Traps\r\n-----------\r\n", sizeof(buf));
    for (i = 0, j = 0; i <= top_of_world; i++)
      if (ROOM_FLAGGED(i, ROOM_DEATH)) {
        nlen = snprintf(buf + len, sizeof(buf) - len, "%2d: [%5d] %s\r\n", ++j, GET_ROOM_VNUM(i), world[i].name);
        if (len + nlen >= sizeof(buf) || nlen < 0)
          break;
        len += nlen;
      }
    page_string(ch->desc, buf, TRUE);
    break;

  /* show godrooms */
  case 7:
    len = strlcpy(buf, "Godrooms\r\n--------------------------\r\n", sizeof(buf));
    for (i = 0, j = 0; i <= top_of_world; i++)
      if (ROOM_FLAGGED(i, ROOM_GODROOM)) {
        nlen = snprintf(buf + len, sizeof(buf) - len, "%2d: [%5d] %s\r\n", ++j, GET_ROOM_VNUM(i), world[i].name);
        if (len + nlen >= sizeof(buf) || nlen < 0)
          break;
        len += nlen;
      }
    page_string(ch->desc, buf, TRUE);
    break;

  /* show shops */
  case 8:
    show_shops(ch, value);
    break;

  /* show houses */
  case 9:
    hcontrol_list_houses(ch);
    break;

  /* show snoop */
  case 10:
    i = 0;
    send_to_char(ch, "People currently snooping:\r\n--------------------------\r\n");
    for (d = descriptor_list; d; d = d->next) {
      if (d->snooping == NULL || d->character == NULL)
	continue;
      if (STATE(d) != CON_PLAYING || GET_LEVEL(ch) < GET_LEVEL(d->character))
	continue;
      if (!CAN_SEE(ch, d->character) || IN_ROOM(d->character) == NOWHERE)
	continue;
      i++;
      send_to_char(ch, "%-10s - snooped by %s.\r\n", GET_NAME(d->snooping->character), GET_NAME(d->character));
    }
    if (i == 0)
      send_to_char(ch, "No one is currently snooping.\r\n");
    break;
  /* show assembly */
  case 11:
    assemblyListToChar(ch);
    break;
  /* show guilds */
  case 12:
    show_guild(ch, value);
    break;

  case 13:
    if (value && *value) {
      if (sscanf(value, "%d-%d", &low, &high) != 2) {
        if (sscanf(value, "%d", &low) != 1) {
          send_to_char(ch, "Usage: show uniques, show uniques [vnum], or show uniques [low-high]\r\n");
          return;
        } else {
          high = low;
        }
      }
    } else {
      low = -1;
      high = 9999999;
    }
    strp = sprintuniques(low, high);
    page_string(ch->desc, strp, TRUE);
    free(strp);
    break;

  /* show what? */
  default:
    send_to_char(ch, "Sorry, I don't understand that.\r\n");
    break;
  }
}


/***************** The do_set function ***********************************/

#define PC   1
#define NPC  2
#define BOTH 3

#define MISC	0
#define BINARY	1
#define NUMBER	2

#define SET_OR_REMOVE(flagset, flags) { \
	if (on) SET_BIT_AR(flagset, flags); \
	else if (off) REMOVE_BIT_AR(flagset, flags); }

#define RANGE(low, high) (value = MAX((low), MIN((high), (value))))


/* The set options available */
  struct set_struct {
    const char *cmd;
    const char level;
    const char pcnpc;
    const char type;
  } set_fields[] = {
   { "brief",		LVL_GOD, 	PC, 	BINARY },  /* 0 */
   { "invstart", 	LVL_GOD, 	PC, 	BINARY },  /* 1 */
   { "title",		LVL_GOD, 	PC, 	MISC   },  /* 2 */
   { "nosummon", 	LVL_GRGOD, 	PC, 	BINARY },  /* 3 */
   { "maxhit",		LVL_GRGOD, 	BOTH, 	NUMBER },  /* 4 */
   { "maxmana", 	LVL_GRGOD, 	BOTH, 	NUMBER },  /* 5 */
   { "maxmove", 	LVL_GRGOD, 	BOTH, 	NUMBER },
   { "hit", 		LVL_GRGOD, 	BOTH, 	NUMBER },
   { "mana",		LVL_GRGOD, 	BOTH, 	NUMBER },
   { "move",		LVL_GRGOD, 	BOTH, 	NUMBER },
   { "align",		LVL_GOD, 	BOTH, 	NUMBER },  /* 10 */
   { "str",		    LVL_GRGOD, 	BOTH, 	NUMBER },
   { "stradd",		LVL_GRGOD, 	BOTH, 	NUMBER },
   { "int", 		LVL_GRGOD, 	BOTH, 	NUMBER },
   { "wis", 		LVL_GRGOD, 	BOTH, 	NUMBER },
   { "dex", 		LVL_GRGOD, 	BOTH, 	NUMBER },  /* 15 */
   { "con", 		LVL_GRGOD, 	BOTH, 	NUMBER },
   { "cha",		    LVL_GRGOD, 	BOTH, 	NUMBER },
   { "ac", 		    LVL_GRGOD, 	BOTH, 	NUMBER },
   { "gold",		LVL_GOD, 	BOTH, 	NUMBER },
   { "bank",		LVL_GOD, 	PC, 	NUMBER },  /* 20 */
   { "exp", 		LVL_GRGOD, 	BOTH, 	NUMBER },
   { "hitroll", 	LVL_GRGOD, 	BOTH, 	NUMBER },
   { "damroll", 	LVL_GRGOD, 	BOTH, 	NUMBER },
   { "invis",		LVL_IMPL, 	PC, 	NUMBER },
   { "nohassle", 	LVL_GRGOD, 	PC, 	BINARY },  /* 25 */
   { "frozen",		LVL_FREEZE, PC, 	BINARY },
   { "practices", 	LVL_GRGOD, 	PC, 	NUMBER },
   { "lessons", 	LVL_GRGOD, 	PC, 	NUMBER },
   { "drunk",		LVL_GRGOD, 	BOTH, 	MISC   },
   { "hunger",		LVL_GRGOD, 	BOTH, 	MISC   },  /* 30 */
   { "thirst",		LVL_GRGOD, 	BOTH, 	MISC   },
   { "killer",		LVL_GOD, 	PC, 	BINARY },
   { "thief",		LVL_GOD, 	PC, 	BINARY },
   { "level",		LVL_IMPL, 	BOTH, 	NUMBER },
   { "room",		LVL_IMPL, 	BOTH, 	NUMBER },  /* 35 */
   { "roomflag", 	LVL_GRGOD, 	PC, 	BINARY },
   { "siteok",		LVL_GRGOD, 	PC, 	BINARY },
   { "deleted", 	LVL_IMPL, 	PC, 	BINARY },
   { "class",		LVL_GRGOD, 	BOTH, 	MISC   },
   { "nowizlist", 	LVL_GOD, 	PC, 	BINARY },  /* 40 */
   { "quest",		LVL_GOD, 	PC, 	BINARY },
   { "loadroom", 	LVL_GRGOD, 	PC, 	MISC   },
   { "color",		LVL_GOD, 	PC, 	BINARY },
   { "idnum",		LVL_IMPL, 	PC, 	NUMBER },
   { "passwd",		LVL_IMPL, 	PC, 	MISC   },  /* 45 */
   { "nodelete", 	LVL_GOD, 	PC, 	BINARY },
   { "sex", 		LVL_GRGOD, 	BOTH, 	MISC   },
   { "age",		    LVL_GRGOD,	BOTH,	NUMBER },
   { "height",		LVL_GOD,	BOTH,	NUMBER },
   { "weight",		LVL_GOD,	BOTH,	NUMBER },  /* 50 */
   { "olc",		    LVL_GRGOD,	PC,	    MISC   },
   { "race",        LVL_GRGOD,  PC,     MISC   },
   { "alias",       LVL_GOD,    PC,     MISC   },  /* 53 */
   { "short",       LVL_GOD,    PC,     MISC   },  /* 54 */
   { "\n", 0,       BOTH, MISC }
  };


int perform_set(struct char_data *ch, struct char_data *vict, int mode,
		char *val_arg)
{
  int i, on = 0, off = 0, value = 0;
  room_rnum rnum;
  room_vnum rvnum;

  /* Check to make sure all the levels are correct */
  if (GET_LEVEL(ch) != LVL_IMPL) {
    if (!IS_NPC(vict) && GET_LEVEL(ch) <= GET_LEVEL(vict) && vict != ch) {
      send_to_char(ch, "Maybe that's not such a great idea...\r\n");
      return (0);
    }
  }
  if (GET_LEVEL(ch) < set_fields[mode].level) {
    send_to_char(ch, "You are not godly enough for that!\r\n");
    return (0);
  }

  /* Make sure the PC/NPC is correct */
  if (IS_NPC(vict) && !(set_fields[mode].pcnpc & NPC)) {
    send_to_char(ch, "You can't do that to a beast!\r\n");
    return (0);
  } else if (!IS_NPC(vict) && !(set_fields[mode].pcnpc & PC)) {
    send_to_char(ch, "That can only be done to a beast!\r\n");
    return (0);
  }

  /* Find the value of the argument */
  if (set_fields[mode].type == BINARY) {
    if (!strcmp(val_arg, "on") || !strcmp(val_arg, "yes"))
      on = 1;
    else if (!strcmp(val_arg, "off") || !strcmp(val_arg, "no"))
      off = 1;
    if (!(on || off)) {
      send_to_char(ch, "Value must be 'on' or 'off'.\r\n");
      return (0);
    }
    send_to_char(ch, "%s %s for %s.\r\n", set_fields[mode].cmd, ONOFF(on), GET_NAME(vict));
  } else if (set_fields[mode].type == NUMBER) {
    value = atoi(val_arg);
    send_to_char(ch, "%s's %s set to %d.\r\n", GET_NAME(vict), set_fields[mode].cmd, value);
  } else
    send_to_char(ch, "%s", CONFIG_OK);

  switch (mode) {
  case 0:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_BRIEF);
    break;
  case 1:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_INVSTART);
    break;
  case 2:
    set_title(vict, val_arg);
    send_to_char(ch, "%s's title is now: %s\r\n", GET_NAME(vict), GET_TITLE(vict));
    break;
  case 3:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_SUMMONABLE);
    send_to_char(ch, "Nosummon %s for %s.\r\n", ONOFF(!on), GET_NAME(vict));
    break;
  case 4:
    vict->points.max_hit = RANGE(1, 5000);
    affect_total(vict);
    break;
  case 5:
    vict->points.max_mana = RANGE(1, 5000);
    affect_total(vict);
    break;
  case 6:
    vict->points.max_move = RANGE(1, 5000);
    affect_total(vict);
    break;
  case 7:
    vict->points.hit = RANGE(-9, vict->points.max_hit);
    affect_total(vict);
    break;
  case 8:
    vict->points.mana = RANGE(0, vict->points.max_mana);
    affect_total(vict);
    break;
  case 9:
    vict->points.move = RANGE(0, vict->points.max_move);
    affect_total(vict);
    break;
  case 10:
    GET_ALIGNMENT(vict) = RANGE(-1000, 1000);
    affect_total(vict);
    break;
  case 11:
      RANGE(3, 25);
    vict->real_abils.str = value;
    affect_total(vict);
    break;
  case 12:
    send_to_char(ch, "This attribute has been removed. Case preserved so they don't have to all be reordered.");
    break;
  case 13:
      RANGE(3, 25);
    vict->real_abils.intel = value;
    affect_total(vict);
    break;
  case 14:
      RANGE(3, 25);
    vict->real_abils.wis = value;
    affect_total(vict);
    break;
  case 15:
      RANGE(3, 25);
    vict->real_abils.dex = value;
    affect_total(vict);
    break;
  case 16:
      RANGE(3, 25);
    vict->real_abils.con = value;
    affect_total(vict);
    break;
  case 17:
      RANGE(3, 25);
    vict->real_abils.cha = value;
    affect_total(vict);
    break;
  case 18:
    vict->points.armor = RANGE(-100, 100);
    affect_total(vict);
    break;
  case 19:
    GET_GOLD(vict) = RANGE(0, 100000000);
    break;
  case 20:
    GET_BANK_GOLD(vict) = RANGE(0, 100000000);
    break;
  case 21:
    vict->points.exp = RANGE(0, 50000000);
    break;
  case 22:
    vict->points.hitroll = RANGE(-20, 20);
    affect_total(vict);
    break;
  case 23:
    vict->points.damroll = RANGE(-20, 20);
    affect_total(vict);
    break;
  case 24:
    if (GET_LEVEL(ch) < LVL_IMPL && ch != vict) {
      send_to_char(ch, "You aren't godly enough for that!\r\n");
      return (0);
    }
    GET_INVIS_LEV(vict) = RANGE(0, GET_LEVEL(vict));
    break;
  case 25:
    if (GET_LEVEL(ch) < LVL_IMPL && ch != vict) {
      send_to_char(ch, "You aren't godly enough for that!\r\n");
      return (0);
    }
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_NOHASSLE);
    break;
  case 26:
    if (ch == vict && on) {
      send_to_char(ch, "Better not -- could be a long winter!\r\n");
      return (0);
    }
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_FROZEN);
    break;
  case 27:
  case 28:
    GET_PRACTICES(vict) = RANGE(0, 100);
    break;
  case 29:
  case 30:
  case 31:
    if (!str_cmp(val_arg, "off")) {
      GET_COND(vict, (mode - 29)) = -1; /* warning: magic number here */
      send_to_char(ch, "%s's %s now off.\r\n", GET_NAME(vict), set_fields[mode].cmd);
    } else if (is_number(val_arg)) {
      value = atoi(val_arg);
      RANGE(0, 24);
      GET_COND(vict, (mode - 29)) = value; /* and here too */
      send_to_char(ch, "%s's %s set to %d.\r\n", GET_NAME(vict), set_fields[mode].cmd, value);
    } else {
      send_to_char(ch, "Must be 'off' or a value from 0 to 24.\r\n");
      return (0);
    }
    break;
  case 32:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_KILLER);
    break;
  case 33:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_THIEF);
    break;
  case 34:
    if ((!IS_NPC(vict) && value > GET_LEVEL(ch)) || value > LVL_IMPL) {
      send_to_char(ch, "You can't do that.\r\n");
      return (0);
    }
    RANGE(0, LVL_IMPL);
    vict->player.level = value;
    break;
  case 35:
    if ((rnum = real_room(value)) == NOWHERE) {
      send_to_char(ch, "No room exists with that number.\r\n");
      return (0);
    }
    if (IN_ROOM(vict) != NOWHERE)	/* Another Eric Green special. */
      char_from_room(vict);
    char_to_room(vict, rnum);
    break;
  case 36:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_ROOMFLAGS);
    break;
  case 37:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_SITEOK);
    break;
  case 38:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_DELETED);
    break;
  case 39:
    if ((i = search_block(val_arg, class_names, FALSE)) < 0) {
      send_to_char(ch, "That is not a class.\r\n");
      return (0);
    }
    GET_CLASS(vict) = i;
    break;
  case 40:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOWIZLIST);
    break;
  case 41:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_QUEST);
    break;
  case 42:
    if (!str_cmp(val_arg, "off")) {
      REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_LOADROOM);
      GET_LOADROOM(vict) = NOWHERE;
    } else if (is_number(val_arg)) {
      rvnum = atoi(val_arg);
      if (real_room(rvnum) != NOWHERE) {
        SET_BIT_AR(PLR_FLAGS(vict), PLR_LOADROOM);
	GET_LOADROOM(vict) = rvnum;
	send_to_char(ch, "%s will enter at room #%d.\r\n", GET_NAME(vict), GET_LOADROOM(vict));
      } else {
	send_to_char(ch, "That room does not exist!\r\n");
	return (0);
      }
    } else {
      send_to_char(ch, "Must be 'off' or a room's virtual number.\r\n");
      return (0);
    }
    break;
  case 43:
    SET_OR_REMOVE(PRF_FLAGS(vict), (PRF_COLOR_1 | PRF_COLOR_2));
    break;
  case 44:
    if (GET_IDNUM(ch) != 1 || !IS_NPC(vict))
      return (0);
    GET_IDNUM(vict) = value;
    break;
  case 45:
    if (GET_IDNUM(ch) > 1) {
      send_to_char(ch, "Please don't use this command, yet.\r\n");
      return (0);
    }
    if (GET_LEVEL(vict) >= LVL_GRGOD) {
      send_to_char(ch, "You cannot change that.\r\n");
      return (0);
    }
    strncpy(GET_PASSWD(vict), CRYPT(val_arg, GET_NAME(vict)), MAX_PWD_LENGTH);	/* strncpy: OK (G_P:MAX_PWD_LENGTH) */
    *(GET_PASSWD(vict) + MAX_PWD_LENGTH) = '\0';
    send_to_char(ch, "Password changed to '%s'.\r\n", val_arg);
    break;
  case 46:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NODELETE);
    break;
  case 47:
    if ((i = search_block(val_arg, genders, FALSE)) < 0) {
      send_to_char(ch, "Must be 'male', 'female', or 'neutral'.\r\n");
      return (0);
    }
    GET_SEX(vict) = i;
    break;
  case 48:	/* set age */
    if (value < 2 || value > 200) {	/* Arbitrary limits. */
      send_to_char(ch, "Ages 2 to 200 accepted.\r\n");
      return (0);
    }
    /*
     * NOTE: May not display the exact age specified due to the integer
     * division used elsewhere in the code.  Seems to only happen for
     * some values below the starting age (17) anyway. -gg 5/27/98
     */
    vict->player.time.birth = time(0) - ((value - 17) * SECS_PER_MUD_YEAR);
    break;

  case 49:	/* Blame/Thank Rick Glover. :) */
    GET_HEIGHT(vict) = value;
    affect_total(vict);
    break;

  case 50:
    GET_WEIGHT(vict) = value;
    affect_total(vict);
    break;

  case 51:
    if (is_abbrev(val_arg, "socials") || is_abbrev(val_arg, "actions"))
      GET_OLC_ZONE(vict) = AEDIT_PERMISSION;
    else if (is_abbrev(val_arg, "hedit"))
      GET_OLC_ZONE(vict) = HEDIT_PERMISSION;
    else if (is_abbrev(val_arg, "off"))
      GET_OLC_ZONE(vict) = NOWHERE;
    else if (!is_number(val_arg)) {
      send_to_char(ch, "Value must be either 'socials', 'actions', 'hedit', 'off' or a zone number.\r\n");
      return (0);
    } else
      GET_OLC_ZONE(vict) = atoi(val_arg);
    break;

  case 52:
    if ((i = search_block(val_arg, race_names, FALSE)) < 0) {
      send_to_char(ch, "That is not a race.\r\n");
      return (0);
    }
    GET_RACE(vict) = i;
    break;

  case 53:
    send_to_char(ch, "Set alias case called.\r\n");
    break;

  case 54:
    send_to_char(ch, "Set short description case called.\r\n");
    break;

  default:
    send_to_char(ch, "Can't set that!\r\n");
    return (0);
  }

  return (1);
}


ACMD(do_set)
{
  struct char_data *vict = NULL, *cbuf = NULL;
  char field[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  int mode, len, player_i = 0, retval;
  char is_file = 0, is_player = 0;

  half_chop(argument, name, buf);

  if (!strcmp(name, "file")) {
    is_file = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "player")) {
    is_player = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "mob"))
    half_chop(buf, name, buf);

  half_chop(buf, field, buf);

  if (!*name || !*field) {
    send_to_char(ch, "Usage: set <victim> <field> <value>\r\n");
    return;
  }

  /* find the target */
  if (!is_file) {
    if (is_player) {
      if (!(vict = get_player_vis(ch, name, NULL, FIND_CHAR_WORLD))) {
	send_to_char(ch, "There is no such player.\r\n");
	return;
      }
    } else { /* is_mob */
      if (!(vict = get_char_vis(ch, name, NULL, FIND_CHAR_WORLD))) {
	send_to_char(ch, "There is no such creature.\r\n");
	return;
      }
    }
  } else if (is_file) {
    /* try to load the player off disk */
    CREATE(cbuf, struct char_data, 1);
    clear_char(cbuf);
    CREATE(cbuf->player_specials, struct player_special_data, 1);
    if ((player_i = load_char(name, cbuf)) > -1) {
      if (GET_LEVEL(cbuf) >= GET_LEVEL(ch)) {
	free_char(cbuf);
	send_to_char(ch, "Sorry, you can't do that.\r\n");
	return;
      }
      vict = cbuf;
    } else {
      free_char(cbuf);
      send_to_char(ch, "There is no such player.\r\n");
      return;
    }
  }

  /* find the command in the list */
  len = strlen(field);
  for (mode = 0; *(set_fields[mode].cmd) != '\n'; mode++)
    if (!strncmp(field, set_fields[mode].cmd, len))
      break;

  /* perform the set */
  retval = perform_set(ch, vict, mode, buf);

  /* save the character if a change was made */
  if (retval) {
    if (!is_file && !IS_NPC(vict))
      save_char(vict);
    if (is_file) {
      GET_PFILEPOS(cbuf) = player_i;
      save_char(cbuf);
      send_to_char(ch, "Saved in file.\r\n");
    }
  }

  /* free the memory if we allocated it earlier */
  if (is_file)
    free_char(cbuf);
}

ACMD(do_saveall)
{
 if (GET_LEVEL(ch) < LVL_BUILDER)
    send_to_char (ch, "You are not holy enough to use this privelege.\n\r");
 else {
    save_all();
    House_save_all();
    send_to_char(ch, "World and house files saved.\n\r");
 }
}

void out_rent(char *name) {
  FILE *fl,*fp;
  char fname[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  char fname2[MAX_INPUT_LENGTH];
  struct obj_file_elem object;
  struct obj_data *obj;
  struct rent_info rent;

  /* if (!get_filename(name, fname, CRASH_FILE)) */
  if (!get_filename(fname, sizeof(fname), CRASH_FILE, name))
    return;
  if (!(fl = fopen(fname, "rb")))
    return;

  sprintf(buf, "%s\r\n", fname);
  if (!feof(fl))
    fread(&rent, sizeof(struct rent_info), 1, fl);

  if (!get_filename(fname2, sizeof(fname2), NEW_OBJ_FILES, name)) {
    log("SYSERR: Unable to complete conversion - unable to get new object filename.\r\n");
    return;
  }
  if (!(fp = fopen(fname2, "w+"))) {
    log("SYSERR: Unable to open new object file.\r\n");
    return;
  }

  fprintf(fp,"%d %d %d %d %d %d\r\n",rent.rentcode,rent.time,
    rent.net_cost_per_diem,rent.gold,rent.account,rent.nitems);
   /* for rent code */

  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);

    if (ferror(fl) || ferror(fp)) {
      fclose(fl);
      fclose(fp);
      log("SYSERR: Error in conversion of rent files.");
      return;
    }

    if (!feof(fl)) {
      if (real_object(object.item_number) > 0) {
        /* none of these will be unique items. just can't happen */
        obj = read_object(object.item_number, VIRTUAL);
        my_obj_save_to_disk(fp, obj,0);
        extract_obj(obj);
      }
    }
  }
  fclose(fl);

  /* write final line - this is never actually read.. but hey! */
  fprintf(fp, "$~\n");
  fclose(fp);
}

#define PLIST_FORMAT \
  "plist [minlev[-maxlev]] [-n name] [-d days] [-h hours] [-i] [-m]"

ACMD(do_players)
{
  int i, len = 0, count = 0;
  char mode, buf[MAX_STRING_LENGTH], name_search[MAX_NAME_LENGTH], time_str[MAX_STRING_LENGTH];
  struct time_info_data time_away;
  int low = 0, high = LVL_IMPL, low_day = 0, high_day = 10000, low_hr = 0, high_hr = 24;

  skip_spaces(&argument);
  strcpy(buf, argument);        /* strcpy: OK (sizeof: argument == buf) */
  name_search[0] = '\0';

  while (*buf) {
    char arg[MAX_INPUT_LENGTH], buf1[MAX_INPUT_LENGTH];

    half_chop(buf, arg, buf1);
    if (isdigit(*arg)) {
      if (sscanf(arg, "%d-%d", &low, &high) == 1)
        high = low;
      strcpy(buf, buf1);        /* strcpy: OK (sizeof: buf1 == buf) */
    } else if (*arg == '-') {
      mode = *(arg + 1);        /* just in case; we destroy arg in the switch */
      switch (mode) {
      case 'l':
        half_chop(buf1, arg, buf);
        sscanf(arg, "%d-%d", &low, &high);
        break;
      case 'n':
        half_chop(buf1, name_search, buf);
        break;
      case 'i':
        strcpy(buf, buf1);
        low = LVL_IMMORT;
        break;
      case 'm':
        strcpy(buf, buf1); 
        high = LVL_IMMORT - 1;
        break;
      case 'd':
        half_chop(buf1, arg, buf);
        if (sscanf(arg, "%d-%d", &low_day, &high_day) == 1)
          high_day = low_day;
        break;
      case 'h':
        half_chop(buf1, arg, buf);
        if (sscanf(arg, "%d-%d", &low_hr, &high_hr) == 1)
          high_hr = low_hr;
        break;
      default:
        send_to_char(ch, "%s\r\n", PLIST_FORMAT);
        return;   
      }
    } else {
      send_to_char(ch, "%s\r\n", PLIST_FORMAT);
      return;
    }
  }

  len = 0;   
  len += snprintf(buf + len, sizeof(buf) - len, "@W[ Id] (Lv) Name         Last@n\r\n"
                  "%s-----------------------------------------------%s\r\n", CCCYN(ch, C_NRM),
                  CCNRM(ch, C_NRM));

  for (i = 0; i <= top_of_p_table; i++) {
    if (player_table[i].level < low || player_table[i].level > high)
      continue;

    time_away = *real_time_passed(time(0), player_table[i].last);

    if (*name_search && str_cmp(name_search, player_table[i].name))
      continue;

    if (time_away.day > high_day || time_away.day < low_day)
      continue;
    if (time_away.hours > high_hr || time_away.hours < low_hr)
      continue;

    strcpy(time_str, asctime(localtime(&player_table[i].last)));
    time_str[strlen(time_str) - 1] = '\0';

    len += snprintf(buf + len, sizeof(buf) - len, "[%3ld] (%2d) %c%-12s %s\r\n",
                    player_table[i].id, player_table[i].level,
                    UPPER(*player_table[i].name), player_table[i].name + 1, time_str);
    count++;
  }
  snprintf(buf + len, sizeof(buf) - len, "%s-----------------------------------------------%s\r\n"
           "%d players listed.\r\n", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM), count);
  page_string(ch->desc, buf, TRUE);
}

ACMD(do_peace)
{
  struct char_data *vict, *next_v;
  send_to_room(IN_ROOM(ch), "Everything is quite peaceful now.\r\n");

    for (vict = world[IN_ROOM(ch)].people; vict; vict = next_v) {
      next_v = vict->next_in_room;  
      if (GET_LEVEL(vict) >= GET_LEVEL(ch)) 
        continue;
      stop_fighting(vict);  
      GET_POS(vict) = POS_SITTING; 
    }
    stop_fighting(ch);
    GET_POS(ch) = POS_STANDING;  
}

ACMD(do_wizupdate)
{
       run_autowiz();
       send_to_char(ch, "Wizlists updated.\n\r");
}

ACMD(do_chown)
{
 struct char_data *victim;
 struct obj_data *obj;
 char buf[80];
 char buf2[80];
 char buf3[80];
 int i, k = 0;

 two_arguments(argument, buf2, buf3);

 if (!*buf2)
   send_to_char(ch, "Syntax: chown <object> <character>.\r\n");
 else if (!(victim = get_char_vis(ch, buf3, NULL, FIND_CHAR_WORLD)))
   send_to_char(ch, "No one by that name here.\r\n");
 else if (victim == ch)
   send_to_char(ch, "Are you sure you're feeling ok?\r\n");
 else if (GET_LEVEL(victim) >= GET_LEVEL(ch))
   send_to_char(ch, "That's really not such a good idea.\r\n");
 else if (!*buf3)
   send_to_char(ch, "Syntax: chown <object> <character>.\r\n");
 else {
   for (i = 0; i < NUM_WEARS; i++) {
     if (GET_EQ(victim, i) && CAN_SEE_OBJ(ch, GET_EQ(victim, i)) &&
        isname(buf2, GET_EQ(victim, i)->name)) {
       obj_to_char(unequip_char(victim, i), victim);
       k = 1;
     }
   }

 if (!(obj = get_obj_in_list_vis(victim, buf2, NULL, victim->carrying))) {
   if (!k && !(obj = get_obj_in_list_vis(victim, buf2, NULL, victim->carrying))) {
     sprintf(buf, "%s does not appear to have the %s.\r\n", GET_NAME(victim), buf2);
     send_to_char(ch, buf);
     return;
   }
 }

 act("@n$n makes a magical gesture and $p@n flies from $N to $m.", FALSE, ch, obj, victim, TO_NOTVICT);
 act("@n$n makes a magical gesture and $p@n flies away from you to $m.", FALSE, ch, obj, victim, TO_VICT);
 act("@nYou make a magical gesture and $p@n flies away from $N to you.", FALSE, ch, obj, victim, TO_CHAR);

 obj_from_char(obj);
 obj_to_char(obj, ch);
 save_char(ch);
 save_char(victim);
 }
}

ACMD(do_zpurge)
{
   struct obj_data *obj, *next_obj;
   struct char_data *mob, *next_mob;
   int i, stored = -1, zone, found = FALSE;
   int room;
   char arg[MAX_INPUT_LENGTH];

   one_argument(argument, arg);

   if (!*arg) {
     zone = zone_table[world[IN_ROOM(ch)].zone].number;
   } else {
     zone = atoi(arg);
   }

   for (i = 0; i <= top_of_zone_table && !found; i++) {
     if (zone_table[i].number == zone) {
       stored = i;
       found = TRUE;
     }
   }

   if (!found || !can_edit_zone(ch, zone)) {
     send_to_char(ch, "You can not purge that zone. Try %d.\r\n", GET_OLC_ZONE(ch));
     return;
   }

   for (room = zone * 100;room <= zone_table[stored].top;room++) {
     if ((i = real_room(room)) != NOWHERE) {
       for (mob = world[i].people; mob; mob = next_mob) {
         next_mob = mob->next_in_room;
         if (IS_NPC(mob)) {
           extract_char(mob);
         }
        }

       for (obj = world[i].contents; obj; obj = next_obj) {
         next_obj = obj->next_content;
         extract_obj(obj);
       }
     }
   }

   send_to_char(ch, "All mobiles and objects in zone %d purged.\r\n", zone);
   mudlog(NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s has purged zone %d.", GET_NAME(ch), zone);
} 

/**************************************************************************************/
/*                         Zone Checker Code below                                    */
/**************************************************************************************/

/*mob limits*/
#define MAX_DAMROLL_ALLOWED      MAX(GET_LEVEL(mob)/5, 1)
#define MAX_HITROLL_ALLOWED      MAX(GET_LEVEL(mob)/3, 1)
#define MAX_GOLD_ALLOWED         GET_LEVEL(mob)*3000
#define MAX_EXP_ALLOWED          GET_LEVEL(mob)*GET_LEVEL(mob) * 120
#define MAX_LEVEL_ALLOWED        LVL_IMPL
#define GET_OBJ_AVG_DAM(obj)     (((GET_OBJ_VAL(obj, 2) + 1) / 2.0) * GET_OBJ_VAL(obj, 1))
/* arbitrary limit for per round dam */
#define MAX_MOB_DAM_ALLOWED      500

#define ZCMD2 zone_table[zone].cmd[cmd_no]  /*fom DB.C*/

/*item limits*/
#define MAX_DAM_ALLOWED            50    /* for weapons  - avg. dam*/
#define MAX_AFFECTS_ALLOWED        3
//#define MAX_OBJ_WEIGHT           50
//#define MAX_OBJ_COST             1000

/* Armor class limits*/
#define TOTAL_WEAR_CHECKS  (NUM_ITEM_WEARS-2)  /*minus Wield and Take*/
struct zcheck_armor {
  bitvector_t bitvector;          /* from Structs.h                       */
  int ac_allowed;                 /* Max. AC allowed for this body part  */
  char *message;                  /* phrase for error message            */
} zarmor[] = {
  {ITEM_WEAR_FINGER, 10, "Ring"},
  {ITEM_WEAR_NECK,   10, "Necklace"},
  {ITEM_WEAR_BODY,   10, "Body armor"},
  {ITEM_WEAR_HEAD,   10, "Head gear"},
  {ITEM_WEAR_LEGS,   10, "Legwear"},
  {ITEM_WEAR_FEET,   10, "Footwear"},
  {ITEM_WEAR_HANDS,  10, "Glove"},
  {ITEM_WEAR_ARMS,   10, "Armwear"},
  {ITEM_WEAR_SHIELD, 10, "Shield"},
  {ITEM_WEAR_ABOUT,  10, "Cloak"},
  {ITEM_WEAR_WAIST,  10, "Belt"},
  {ITEM_WEAR_WRIST,  10, "Wristwear"},
  {ITEM_WEAR_HOLD,   10, "Held item"}
};

/*These are strictly boolean*/
#define CAN_WEAR_WEAPONS         0     /* toggle - can a weapon also be a piece of armor? */
#define MAX_APPLIES_LIMIT        1     /* toggle - is there a limit at all?               */
#define CHECK_ITEM_RENT          0     /* do we check for rent cost == 0 ?                */
#define CHECK_ITEM_COST          0     /* do we check for item cost == 0 ?                */
/*
  Applies limits
  !! Very Important:  Keep these in the same order as in Structs.h
  To ignore an apply, set max_aff to -99.
  These will be ignored if MAX_APPLIES_LIMIT = 0
*/
struct zcheck_affs {
  int aff_type;    /*from Structs.h*/
  int min_aff;     /*min. allowed value*/
  int max_aff;     /*max. allowed value*/
  char *message;   /*phrase for error message*/
} zaffs[] = {
  {APPLY_NONE,         0, -99, "unused0"},
  {APPLY_STR,         -5,   3, "strength"},
  {APPLY_DEX,         -5,   3, "dexterity"},
  {APPLY_INT,         -5,   3, "intelligence"},
  {APPLY_WIS,         -5,   3, "wisdom"},
  {APPLY_CON,         -5,   3, "constitution"},
  {APPLY_CHA,         -5,   3, "charisma"},
  {APPLY_CLASS,        0,   0, "class"},
  {APPLY_LEVEL,        0,   0, "level"},
  {APPLY_AGE,        -10,  10, "age"},
  {APPLY_CHAR_WEIGHT,-50,  50, "character weight"},
  {APPLY_CHAR_HEIGHT,-50,  50, "character height"},
  {APPLY_MANA,       -50,  50, "mana"},
  {APPLY_HIT,        -50,  50, "hit points"},
  {APPLY_MOVE,       -50,  50, "movement"},
  {APPLY_GOLD,         0,   0, "gold"},
  {APPLY_EXP,          0,   0, "experience"},
  {APPLY_AC,         -10,  10, "magical AC"},
  {APPLY_HITROLL,      0, -99, "hitroll"},       /* Handled seperately below */
  {APPLY_DAMROLL,      0, -99, "damroll"},       /* Handled seperately below */
  {APPLY_SAVING_PARA, -2,   2, "saving throw (paralysis)"},
  {APPLY_SAVING_ROD,  -2,   2, "saving throw (rod)"},
  {APPLY_SAVING_PETRI,-2,   2, "saving throw (death)"},
  {APPLY_SAVING_BREATH,-2,  2, "saving throw (breath)"},
  {APPLY_SAVING_SPELL,-2,   2, "saving throw (spell)"}
};

/* These are ABS() values. */
#define MAX_APPLY_HITROLL_TOTAL   5
#define MAX_APPLY_DAMROLL_TOTAL   5

/*room limits*/
/* Off limit zones are any zones a player should NOT be able to walk to (ex. Limbo) */
const int offlimit_zones[] = {0,12,13,14,-1};  /*what zones can no room connect to (virtual num) */
#define MIN_ROOM_DESC_LENGTH   80       /* at least one line - set to 0 to not care. */
#define MAX_COLOUMN_WIDTH      80       /* at most 80 chars per line */


ACMD (do_zcheck)
{ 
  zone_rnum zrnum;
  struct obj_data *obj;
  struct char_data *mob = NULL;
  room_vnum exroom=0;
  int ac=0;
  int affs=0, tohit, todam, value;
  int i = 0, j = 0, k = 0, l = 0, m = 0, found = 0; /* found is used as a 'send now' flag*/
  char buf[MAX_STRING_LENGTH];
  float avg_dam;
  size_t len = 0;
  struct extra_descr_data *ext, *ext2;
  one_argument(argument, buf);

  if (!buf || !*buf || !strcmp(buf, "."))
    zrnum = world[IN_ROOM(ch)].zone;
  else 
    zrnum = real_zone(atoi(buf));
 
  if (zrnum == NOWHERE) {
    send_to_char(ch, "Check what zone ?\r\n");
    return;
  } else
    send_to_char(ch, "Checking zone %d!\r\n", zone_table[zrnum].number);

 /************** Check mobs *****************/

  send_to_char(ch, "Checking Mobs for limits...\r\n");
  /*check mobs first*/
  for (i=0; i<top_of_mobt;i++) {                   
      if (real_zone_by_thing(mob_index[i].vnum) == zrnum) {  /*is mob in this zone?*/
        mob = &mob_proto[i];
        if (!strcmp(mob->player.name, "mob unfinished") && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Alias hasn't been set.\r\n");

        if (!strcmp(mob->player.short_descr, "the unfinished mob") && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Short description hasn't been set.\r\n");

        if (!strncmp(mob->player.long_descr, "An unfinished mob stands here.", 30) && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Long description hasn't been set.\r\n");

        if (mob->player.description && *mob->player.description) {
          if (!strncmp(mob->player.description, "It looks unfinished.", 20) && (found=1))
            len += snprintf(buf + len, sizeof(buf) - len,
                            "- Description hasn't been set.\r\n");
          else if (strncmp(mob->player.description, "   ", 3) && (found=1))
            len += snprintf(buf + len, sizeof(buf) - len,
                            "- Description hasn't been formatted. (/fi)\r\n");
        }
       
        if (GET_LEVEL(mob)>MAX_LEVEL_ALLOWED && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Is level %d (limit: 1-%d)\r\n",
                          GET_LEVEL(mob), MAX_LEVEL_ALLOWED);

        if (GET_DAMROLL(mob)>MAX_DAMROLL_ALLOWED && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Damroll of %d is too high (limit: %d)\r\n",
                          GET_DAMROLL(mob), MAX_DAMROLL_ALLOWED);

        if (GET_HITROLL(mob)>MAX_HITROLL_ALLOWED && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Hitroll of %d is too high (limit: %d)\r\n",
                          GET_HITROLL(mob), MAX_HITROLL_ALLOWED);

        /* avg. dam including damroll per round of combat */
        avg_dam = (((mob->mob_specials.damsizedice / 2.0) * mob->mob_specials.damnodice)+GET_DAMROLL(mob));
        if (avg_dam>MAX_MOB_DAM_ALLOWED && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- average damage of %4.1f is too high (limit: %d)\r\n",
                          avg_dam, MAX_MOB_DAM_ALLOWED);

        if (mob->mob_specials.damsizedice == 1 &&
            mob->mob_specials.damnodice == 1 &&
            GET_LEVEL(mob) == 0 &&
            (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Needs to be fixed - %sAutogenerate!%s\r\n", CCYEL(ch, C_NRM), CCNRM(ch, C_NRM));
       
        if (MOB_FLAGGED(mob, MOB_AGGRESSIVE) &&
            MOB_FLAGGED(mob, MOB_AGGR_GOOD | MOB_AGGR_EVIL | MOB_AGGR_NEUTRAL) && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
          "- Both aggresive and agressive to align.\r\n");
       
        if ((GET_GOLD(mob) > MAX_GOLD_ALLOWED) && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Set to %d Gold (limit : %d).\r\n",
                                  GET_GOLD(mob), 
                                  MAX_GOLD_ALLOWED);

        if (GET_EXP(mob)>MAX_EXP_ALLOWED && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Has %d experience (limit: %d)\r\n",
                              GET_EXP(mob), MAX_EXP_ALLOWED);
        if (AFF_FLAGGED(mob, AFF_GROUP | AFF_CHARM | AFF_POISON) && (found = 1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Has illegal affection bits set (%s %s %s)\r\n",
                              AFF_FLAGGED(mob, AFF_GROUP) ? "GROUP" : "",
                              AFF_FLAGGED(mob, AFF_CHARM) ? "CHARM" : "",
                              AFF_FLAGGED(mob, AFF_POISON) ? "POISON" : "");
         

        if (!MOB_FLAGGED(mob, MOB_SENTINEL) && !MOB_FLAGGED(mob, MOB_STAY_ZONE) && (found = 1))
          len += snprintf(buf + len, sizeof(buf) - len,
                            "- Neither SENTINEL nor STAY_ZONE bits set.\r\n");

        if (MOB_FLAGGED(mob, MOB_SPEC) && (found = 1))
          len += snprintf(buf + len, sizeof(buf) - len,
                            "- SPEC flag needs to be removed.\r\n");

          /*****ADDITIONAL MOB CHECKS HERE*****/
          if (found) {
            send_to_char(ch,
                    "%s[%5d]%s %-30s: %s\r\n",
                    CCCYN(ch, C_NRM), GET_MOB_VNUM(mob),
                    CCYEL(ch, C_NRM), GET_NAME(mob),
                    CCNRM(ch, C_NRM));
            send_to_char(ch, buf);
          }
          /* reset buffers and found flag */
          strcpy(buf, "");
          found = 0;
          len = 0;
        }   /*mob is in zone*/
    }  /*check mobs*/

 /************** Check objects *****************/
  send_to_char(ch, "\r\nChecking Objects for limits...\r\n");
  for (i=0; i<top_of_objt; i++) {
    if (real_zone_by_thing(obj_index[i].vnum) == zrnum) { /*is object in this zone?*/
      obj = &obj_proto[i];
      switch (GET_OBJ_TYPE(obj)) {       
        case ITEM_MONEY:
          if ((value = GET_OBJ_VAL(obj, 1))>MAX_GOLD_ALLOWED && (found=1))
            len += snprintf(buf + len, sizeof(buf) - len,
                            "- Is worth %d (money limit %d coins).\r\n",
                                 value, MAX_GOLD_ALLOWED);
          break;             
        case ITEM_WEAPON:
          if (GET_OBJ_VAL(obj, 3) >= NUM_ATTACK_TYPES && (found=1))
            len += snprintf(buf + len, sizeof(buf) - len,
                            "- has out of range attack type %d.\r\n",
                                 GET_OBJ_VAL(obj, 3));
         
          if (GET_OBJ_AVG_DAM(obj)>MAX_DAM_ALLOWED && (found=1))
            len += snprintf(buf + len, sizeof(buf) - len,
                            "- Damroll is %2.1f (limit %d)\r\n",
                                 GET_OBJ_AVG_DAM(obj), MAX_DAM_ALLOWED);
          break;
        case ITEM_ARMOR:
          ac=GET_OBJ_VAL(obj,0);
          for (j=0; j<TOTAL_WEAR_CHECKS;j++) {
            if (CAN_WEAR(obj,zarmor[j].bitvector) && (ac>zarmor[j].ac_allowed) && (found=1))
              len += snprintf(buf + len, sizeof(buf) - len,
                                   "- Has AC %d (%s limit is %d)\r\n",
                                   ac, zarmor[j].message, zarmor[j].ac_allowed);
          }
          break;             
     
      }  /*switch on Item_Type*/

      if (!CAN_WEAR(obj, ITEM_WEAR_TAKE)) {
        if ((GET_OBJ_COST(obj) || (GET_OBJ_WEIGHT(obj) && GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN) ||
           GET_OBJ_RENT(obj)) && (found = 1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- is NO_TAKE, but has cost (%d) weight (%d) or rent (%d) set.\r\n",
                          GET_OBJ_COST(obj), GET_OBJ_WEIGHT(obj), GET_OBJ_RENT(obj));
      } else {
        if (GET_OBJ_COST(obj) == 0 && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- has 0 cost (min. 1).\r\n");

        if (GET_OBJ_WEIGHT(obj) == 0 && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- has 0 weight (min. 1).\r\n");

        if (GET_OBJ_WEIGHT(obj) > MAX_OBJ_WEIGHT && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "  Weight is too high: %d (limit  %d).\r\n",
                          GET_OBJ_WEIGHT(obj), MAX_OBJ_WEIGHT);


        if (GET_OBJ_COST(obj) > MAX_OBJ_COST && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- has %d cost (max %d).\r\n",
                          GET_OBJ_COST(obj), MAX_OBJ_COST);
      }

      if (GET_OBJ_LEVEL(obj) > LVL_IMMORT-1 && (found=1))
        len += snprintf(buf + len, sizeof(buf) - len,
                          "- has min level set to %d (max %d).\r\n",
                          GET_OBJ_LEVEL(obj), LVL_IMMORT-1);

      if (obj->action_description && *obj->action_description &&
          GET_OBJ_TYPE(obj) != ITEM_STAFF &&
          GET_OBJ_TYPE(obj) != ITEM_WAND &&
          GET_OBJ_TYPE(obj) != ITEM_SCROLL &&
          GET_OBJ_TYPE(obj) != ITEM_NOTE && (found=1))
        len += snprintf(buf + len, sizeof(buf) - len,
                          "- has action_description set, but is inappropriate type.\r\n");
         
      /*first check for over-all affections*/
      for (affs=0, j = 0; j < MAX_OBJ_AFFECT; j++)
        if (obj->affected[j].modifier) affs++;

      if (affs>MAX_AFFECTS_ALLOWED && (found=1))
        len += snprintf(buf + len, sizeof(buf) - len,
                          "- has %d affects (limit %d).\r\n",
                             affs, MAX_AFFECTS_ALLOWED);
       
      /*check for out of range affections. */
      for (j=0;j<MAX_OBJ_AFFECT;j++)
        if (zaffs[(int)obj->affected[j].location].max_aff != -99 && /* only care if a range is set */
            (obj->affected[j].modifier > zaffs[(int)obj->affected[j].location].max_aff ||
             obj->affected[j].modifier < zaffs[(int)obj->affected[j].location].min_aff ||
             zaffs[(int)obj->affected[j].location].min_aff == zaffs[(int)obj->affected[j].location].max_aff) && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- apply to %s is %d (limit %d - %d).\r\n",
                               zaffs[(int)obj->affected[j].location].message,
                               obj->affected[j].modifier,
                               zaffs[(int)obj->affected[j].location].min_aff,
                               zaffs[(int)obj->affected[j].location].max_aff);

     /* special handling of +hit and +dam because of +hit_n_dam */
     for (todam=0, tohit=0, j=0;j<MAX_OBJ_AFFECT;j++) {
       if (obj->affected[j].location == APPLY_HITROLL)
         tohit += obj->affected[j].modifier;
       if (obj->affected[j].location == APPLY_DAMROLL)
         todam += obj->affected[j].modifier;
     }
     if (abs(todam) > MAX_APPLY_DAMROLL_TOTAL && (found=1))
       len += snprintf(buf + len, sizeof(buf) - len,
                       "- total damroll %d out of range (limit +/-%d.\r\n",
                       todam, MAX_APPLY_DAMROLL_TOTAL);
     if (abs(tohit) > MAX_APPLY_HITROLL_TOTAL && (found=1))
       len += snprintf(buf + len, sizeof(buf) - len,
                       "- total hitroll %d out of range (limit +/-%d).\r\n",
                       tohit, MAX_APPLY_HITROLL_TOTAL);
       
     
     for (ext2 = NULL, ext = obj->ex_description; ext; ext = ext->next)
       if (strncmp(ext->description, "   ", 3))
         ext2 = ext;
     
     if (ext2 && (found = 1))
       len += snprintf(buf + len, sizeof(buf) - len,
                       "- has unformatted extra description\r\n");
     /*****ADDITIONAL OBJ CHECKS HERE*****/
     if (found) {
        send_to_char(ch, "[%5d] %-30s: \r\n", GET_OBJ_VNUM(obj), obj->short_description);
        send_to_char(ch, buf);
      }
      strcpy(buf, "");
      len = 0;
      found = 0;
    }   /*object is in zone*/
  } /*check objects*/

  /************** Check rooms *****************/
  send_to_char(ch, "\r\nChecking Rooms for limits...\r\n");
  for (i=0; i<top_of_world;i++) {
    if (world[i].zone==zrnum) {
      for (j = 0; j < NUM_OF_DIRS; j++) {
        /*check for exit, but ignore off limits if you're in an offlimit zone*/
        if (!world[i].dir_option[j])
          continue;
        exroom=world[i].dir_option[j]->to_room;
        if (exroom==NOWHERE)
          continue;
        if (world[exroom].zone == zrnum)
          continue;
        if (world[exroom].zone == world[i].zone)
          continue;

        for (k=0;offlimit_zones[k] != -1;k++) {
          if (world[exroom].zone == real_zone(offlimit_zones[k]) && (found = 1))
            len += snprintf(buf + len, sizeof(buf) - len,
                            "- Exit %s cannot connect to %d (zone off limits).\r\n",
                            dirs[j], world[exroom].number);
        } /* for (k.. */
      } /* cycle directions */         

     if (ROOM_FLAGGED(i, ROOM_ATRIUM | ROOM_HOUSE | ROOM_HOUSE_CRASH | ROOM_OLC | ROOM_BFS_MARK))
         len += snprintf(buf + len, sizeof(buf) - len,
         "- Has illegal affection bits set (%s %s %s %s %s)\r\n",
                            ROOM_FLAGGED(i, ROOM_ATRIUM) ? "ATRIUM" : "",
                            ROOM_FLAGGED(i, ROOM_HOUSE) ? "HOUSE" : "",
                            ROOM_FLAGGED(i, ROOM_HOUSE_CRASH) ? "HCRSH" : "",
                            ROOM_FLAGGED(i, ROOM_OLC) ? "OLC" : "",
                            ROOM_FLAGGED(i, ROOM_BFS_MARK) ? "*" : "");

      if ((MIN_ROOM_DESC_LENGTH) && strlen(world[i].description)<MIN_ROOM_DESC_LENGTH && (found=1))
        len += snprintf(buf + len, sizeof(buf) - len,
                        "- Room description is too short. (%4.4d of min. %d characters).\r\n",
                             strlen(world[i].description), MIN_ROOM_DESC_LENGTH);

      if (strncmp(world[i].description, "   ", 3) && (found=1))
        len += snprintf(buf + len, sizeof(buf) - len,
                        "- Room description not formatted with indent (/fi in the editor).\r\n");

      /* strcspan = size of text in first arg before any character in second arg */
      if ((strcspn(world[i].description, "\r\n")>MAX_COLOUMN_WIDTH) && (found=1))
        len += snprintf(buf + len, sizeof(buf) - len,
                        "- Room description not wrapped at %d chars (/fi in the editor).\r\n",
                             MAX_COLOUMN_WIDTH);
           
     for (ext2 = NULL, ext = world[i].ex_description; ext; ext = ext->next)
       if (strncmp(ext->description, "   ", 3))
         ext2 = ext;

     if (ext2 && (found = 1))
       len += snprintf(buf + len, sizeof(buf) - len,
                       "- has unformatted extra description\r\n");

      if (found) {
        send_to_char(ch, "[%5d] %-30s: \r\n",
                       world[i].number, world[i].name ? world[i].name : "An unnamed room");
        send_to_char(ch, buf);
        strcpy(buf, "");
        len = 0;
        found = 0;
      }
    } /*is room in this zone?*/
  } /*checking rooms*/ 

  for (i=0; i<top_of_world;i++) {
    if (world[i].zone==zrnum) {
      m++;
      for (j = 0, k = 0; j < NUM_OF_DIRS; j++)
        if (!world[i].dir_option[j])
          k++;
       
      if (k == NUM_OF_DIRS)
        l++;
    }
  }
  if (l * 3 > m)
    send_to_char(ch, "More than 1/3 of the rooms are not linked.\r\n");
 
}
/**********************************************************************************/

void mob_checkload(struct char_data *ch, mob_vnum mvnum)
{
  int cmd_no;
  zone_rnum zone;
  mob_rnum mrnum = real_mobile(mvnum);
 
  if (mrnum == NOBODY) {
      send_to_char(ch, "That mob does not exist.\r\n");
      return;
  }

  send_to_char(ch, "Checking load info for the mob [%d] %s...\r\n",
                    mvnum, mob_proto[mrnum].player.short_descr);

  for (zone=0; zone <= top_of_zone_table; zone++) {   
    for (cmd_no = 0; ZCMD2.command != 'S'; cmd_no++) {
      if (ZCMD2.command != 'M')
        continue;

      /* read a mobile */
      if (ZCMD2.arg1 == mrnum) {
        send_to_char(ch, "  [%5d] %s (%d MAX)\r\n",
                         world[ZCMD2.arg3].number,
                         world[ZCMD2.arg3].name,
                         ZCMD2.arg2);
      }
    }
  }
}

void obj_checkload(struct char_data *ch, obj_vnum ovnum)
{
  int cmd_no;
  zone_rnum zone;
  obj_rnum ornum = real_object(ovnum);
  room_vnum lastroom_v = 0;
  room_rnum lastroom_r = 0;
  mob_rnum lastmob_r = 0;

  if (ornum ==NOTHING) {
    send_to_char(ch, "That object does not exist.\r\n");
    return;
  }
 
  send_to_char(ch, "Checking load info for the obj [%d] %s...\r\n",
                   ovnum, obj_proto[ornum].short_description);

  for (zone=0; zone <= top_of_zone_table; zone++) {   
    for (cmd_no = 0; ZCMD2.command != 'S'; cmd_no++) {
      switch (ZCMD2.command) {
        case 'M':
          lastroom_v = world[ZCMD2.arg3].number;
          lastroom_r = ZCMD2.arg3;
          lastmob_r = ZCMD2.arg1;
          break;
        case 'O':                   /* read an object */
          lastroom_v = world[ZCMD2.arg3].number;
          lastroom_r = ZCMD2.arg3;
          if (ZCMD2.arg1 == ornum)                       
            send_to_char(ch, "  [%5d] %s (%d Max)\r\n",
                             lastroom_v,
                             world[lastroom_r].name,
                             ZCMD2.arg2);
          break;
        case 'P':                   /* object to object */
          if (ZCMD2.arg1 == ornum)
            send_to_char(ch, "  [%5d] %s (Put in another object [%d Max])\r\n",
                             lastroom_v,
                             world[lastroom_r].name,
                             ZCMD2.arg2);
          break;
        case 'G':                   /* obj_to_char */
          if (ZCMD2.arg1 == ornum)
            send_to_char(ch, "  [%5d] %s (Given to %s [%d][%d Max])\r\n",
                             lastroom_v,
                             world[lastroom_r].name,
                             mob_proto[lastmob_r].player.short_descr,
                             mob_index[lastmob_r].vnum,
                             ZCMD2.arg2);
          break;
        case 'E':                   /* object to equipment list */
          if (ZCMD2.arg1 == ornum)
            send_to_char(ch, "  [%5d] %s (Equipped to %s [%d][%d Max])\r\n",
                             lastroom_v,
                             world[lastroom_r].name,
                             mob_proto[lastmob_r].player.short_descr,
                             mob_index[lastmob_r].vnum,
                             ZCMD2.arg2);
            break;
          case 'R': /* rem obj from room */
            lastroom_v = world[ZCMD2.arg1].number;
            lastroom_r = ZCMD2.arg1;
            if (ZCMD2.arg2 == ornum)
              send_to_char(ch, "  [%5d] %s (Removed from room)\r\n",
                               lastroom_v,
                               world[lastroom_r].name);                   
            break;
      }/* switch */
    } /*for cmd_no......*/
  }  /*for zone...*/
}

void trg_checkload(struct char_data *ch, trig_vnum tvnum)
{
  int cmd_no, found = 0;
  zone_rnum zone;
  trig_rnum trnum = real_trigger(tvnum);
  room_vnum lastroom_v = 0;
  room_rnum lastroom_r = 0, k;
  mob_rnum lastmob_r = 0, i;
  obj_rnum lastobj_r = 0, j;
  struct trig_proto_list *tpl;
 
  if (trnum == NOTHING) {
    send_to_char(ch, "That trigger does not exist.\r\n");
    return;
  }
 
  send_to_char(ch, "Checking load info for the %s trigger [%d] '%s':\r\n",
                    trig_index[trnum]->proto->attach_type == MOB_TRIGGER ? "mobile" :
                    (trig_index[trnum]->proto->attach_type == OBJ_TRIGGER ? "object" : "room"),                   
                    tvnum, trig_index[trnum]->proto->name);

  for (zone=0; zone <= top_of_zone_table; zone++) {   
    for (cmd_no = 0; ZCMD2.command != 'S'; cmd_no++) {
      switch (ZCMD2.command) {
        case 'M':
          lastroom_v = world[ZCMD2.arg3].number;
          lastroom_r = ZCMD2.arg3;
          lastmob_r = ZCMD2.arg1;
          break;
        case 'O':                   /* read an object */
          lastroom_v = world[ZCMD2.arg3].number;
          lastroom_r = ZCMD2.arg3;
          lastobj_r = ZCMD2.arg1;
          break;
        case 'P':                   /* object to object */
          lastobj_r = ZCMD2.arg1;
          break;
        case 'G':                   /* obj_to_char */
          lastobj_r = ZCMD2.arg1;
          break;
        case 'E':                   /* object to equipment list */
          lastobj_r = ZCMD2.arg1;
          break;
        case 'R':                   /* rem obj from room */
          lastroom_v = 0;
          lastroom_r = 0;
          lastobj_r = 0;
          lastmob_r = 0;
        case 'T':                   /* trigger to something */
          if (ZCMD2.arg2 != trnum)
            break;
          if (ZCMD2.arg1 == MOB_TRIGGER) {
            send_to_char(ch, "mob [%5d] %-60s (zedit room %5d)\r\n",
                               mob_index[lastmob_r].vnum,
                               mob_proto[lastmob_r].player.short_descr,
                               lastroom_v);   
            found = 1;
          } else if (ZCMD2.arg1 == OBJ_TRIGGER) {
            send_to_char(ch, "obj [%5d] %-60s  (zedit room %d)\r\n",
                               obj_index[lastobj_r].vnum,
                               obj_proto[lastobj_r].short_description,
                               lastroom_v); 
            found = 1;
          } else if (ZCMD2.arg1==WLD_TRIGGER) {
            send_to_char(ch, "room [%5d] %-60s (zedit)\r\n",
                               lastroom_v,
                               world[lastroom_r].name);                   
            found = 1;
          }
        break;
      } /* switch */
    } /*for cmd_no......*/
  }  /*for zone...*/
 
  for (i = 0; i < top_of_mobt; i++) {
    if (!mob_proto[i].proto_script)
      continue;
   
    for (tpl = mob_proto[i].proto_script;tpl;tpl = tpl->next)
      if (tpl->vnum == tvnum) {
        send_to_char(ch, "mob [%5d] %s\r\n",
                         mob_index[i].vnum,
                         mob_proto[i].player.short_descr);
        found = 1;
      }
 
  }
   
  for (j = 0; j < top_of_objt; j++) {
    if (!obj_proto[j].proto_script)
      continue;
   
    for (tpl = obj_proto[j].proto_script;tpl;tpl = tpl->next)
      if (tpl->vnum == tvnum) {
        send_to_char(ch, "obj [%5d] %s\r\n",
                         obj_index[j].vnum,
                         obj_proto[j].short_description);
        found = 1;
      }
 
  }
 
  for (k = 0;k < top_of_world; k++) {
    if (!world[k].proto_script)
      continue;

    for (tpl = world[k].proto_script;tpl;tpl = tpl->next)
      if (tpl->vnum == tvnum) {
        send_to_char(ch, "room[%5d] %s\r\n",
                         world[k].number,
                         world[k].name);
        found = 1;
      }
  }

  if (!found)
    send_to_char(ch, "This trigger is not attached to anything.\r\n");
}

ACMD(do_checkloadstatus)
{ 
  char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

  two_arguments(argument, buf1, buf2);

  if ((!*buf1) || (!*buf2) || (!isdigit(*buf2))) {
    send_to_char(ch, "Checkload <M | O | T> <vnum>\r\n");
    return;
  }   

  if (LOWER(*buf1) == 'm') {         
    mob_checkload(ch, atoi(buf2));
    return;
  }
 
  if (LOWER(*buf1) == 'o') {         
    obj_checkload(ch, atoi(buf2));
    return;
  }

  if (LOWER(*buf1) == 't') {
    trg_checkload(ch, atoi(buf2));
    return;
  }
}
/**************************************************************************************/
/*                         Zone Checker code above                                    */
/**************************************************************************************/ 

/* do_findkey, finds where the key to a door loads to using do_checkloadstatus() */
ACMD(do_findkey)
{
  int dir, key;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  any_one_arg(argument, arg); /* Because "in" is a valid direction */

  if (!*arg) {
    send_to_char(ch, "Format: findkey <dir>\r\n");
  } else if ((dir = search_block(arg, dirs, FALSE)) >= 0 ||
      (dir = search_block(arg, abbr_dirs, FALSE)) >= 0 ) {
    if (!EXIT(ch, dir)) {
      send_to_char(ch, "There's no exit in that direction!\r\n");
    } else if ((key = EXIT(ch, dir)->key) == NOTHING || key == 0) {
      send_to_char(ch, "There's no key for that exit.\r\n");
    } else {
      sprintf(buf, "obj %d", key);
      do_checkloadstatus(ch, buf, 0, 0);
    }
  } else {
    send_to_char(ch, "What direction is that?!?\r\n");
  }
} 

/* do_finddoor, finds the door(s) that a key goes to */
ACMD(do_finddoor)
{
  int d, vnum = NOTHING, num = 0;
  size_t len, nlen;
  room_rnum i;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH] = {0};
  struct char_data *tmp_char;
  struct obj_data *obj;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char(ch, "Format: finddoor <obj/vnum>\r\n");
  } else if (is_number(arg)) {
    vnum = atoi(arg);
    obj = &obj_proto[real_object(vnum)];
  } else {
    generic_find(arg,
         FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_WORLD | FIND_OBJ_EQUIP,
         ch, &tmp_char, &obj);
    if (!obj)
      send_to_char(ch, "What key do you want to find a door for?\r\n");
    else
      vnum = GET_OBJ_VNUM(obj);
  }
  if (vnum != NOTHING) {
    if (GET_OBJ_TYPE(obj) != ITEM_KEY)
      send_to_char(ch, "It seems that %s isn't a key.\r\n", GET_OBJ_SHORT(obj));
    else {
      len = snprintf(buf, sizeof(buf), "Doors unlocked by key [%d] %s are:\r\n",
                      vnum, GET_OBJ_SHORT(obj));
      for (i = 0; i <= top_of_world; i++) {
        for (d = 0; d < NUM_OF_DIRS; d++) {
          if (world[i].dir_option[d] && world[i].dir_option[d]->key &&
              world[i].dir_option[d]->key == vnum) {
            nlen = snprintf(buf + len, sizeof(buf) - len,
                            "[%3d] Room %d, %s (%s)\r\n",
                            ++num, world[i].number,
                            dirs[d], world[i].dir_option[d]->keyword);
            if (len + nlen >= sizeof(buf) || nlen < 0)
              break;
            len += nlen;
          }
        } /* for all directions */
      } /* for all rooms */
      if (num > 0)
        page_string(ch->desc, buf, 1);
      else
        send_to_char(ch, "No doors were found for key [%d] %s.\r\n",
                         vnum, GET_OBJ_SHORT(obj));
    }
  }
} 


ACMD(do_skillcheck) {

  struct char_data *vict = NULL;

  skip_spaces(&argument);

  if (!argument || !*argument) {
    send_to_char(ch, "You must specify a target player or mobile.\r\n");
    return;
  }

  if (!(vict = get_char_vis(ch, argument, NULL, FIND_CHAR_WORLD))) {
    send_to_char(ch, "There is no such person or creature.\r\n");
    return;
  }
  list_skills_perct(ch, vict);


}

