/************************************************************************
 * OasisOLC - Objects / oedit.c					v2.0	*
 * Original author: Levork						*
 * Copyright 1996 by Harvey Gilpin					*
 * Copyright 1997-2001 by George Greer (greerga@circlemud.org)		*
 ************************************************************************/

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "spells.h"
#include "utils.h"
#include "db.h"
#include "handler.h"
#include "boards.h"
#include "constants.h"
#include "shop.h"
#include "genolc.h"
#include "genobj.h"
#include "genzon.h"
#include "oasis.h"
#include "improved-edit.h"
#include "dg_olc.h"

/*------------------------------------------------------------------------*/

/*
 * External variable declarations.
 */

extern struct obj_data *obj_proto;
extern struct index_data *obj_index;
extern struct obj_data *object_list;
extern obj_rnum top_of_objt;
extern struct zone_data *zone_table;
extern zone_rnum top_of_zone_table;
extern struct shop_data *shop_index;
extern struct attack_hit_type attack_hit_text[];
extern struct spell_info_type spell_info[];
extern struct board_info *bboards;
extern struct descriptor_data *descriptor_list;
extern zone_rnum real_zone_by_thing(room_vnum vznum);
extern long max_obj_id;

/*------------------------------------------------------------------------*/

/*
 * Handy macros.
 */
#define S_PRODUCT(s, i) ((s)->producing[(i)])

/*------------------------------------------------------------------------*\
  Utility and exported functions
\*------------------------------------------------------------------------*/

ACMD(do_oasis_oedit)
{
  int number = NOWHERE, save = 0, real_num;
  struct descriptor_data *d;
  char *buf3;
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  
  /****************************************************************************/
  /** Parse any arguments.                                                   **/
  /****************************************************************************/
  buf3 = two_arguments(argument, buf1, buf2);
  
  /****************************************************************************/
  /** If there aren't any arguments...well...they can't modify nothing now   **/
  /** can they?                                                              **/
  /****************************************************************************/
  if (!*buf1) {
    send_to_char(ch, "Specify an object VNUM to edit.\r\n");
    return;
  } else if (!isdigit(*buf1)) {
    if (str_cmp("save", buf1) != 0) {
      send_to_char(ch, "Yikes!  Stop that, someone will get hurt!\r\n");
      return;
    }
    
    save = TRUE;
    
    if (is_number(buf2))
      number = atoi(buf2);
    else if (GET_OLC_ZONE(ch) > 0) {
      zone_rnum zlok;
      
      if ((zlok = real_zone(GET_OLC_ZONE(ch))) == NOWHERE)
        number = NOWHERE;
      else
        number = genolc_zone_bottom(zlok);
    }
    
    if (number == NOWHERE) {
      send_to_char(ch, "Save which zone?\r\n");
      return;
    }
  }
  
  /****************************************************************************/
  /** If a numeric argument was given, get it.                               **/
  /****************************************************************************/
  if (number == NOWHERE)
    number = atoi(buf1);
  
  /****************************************************************************/
  /** Check that whatever it is isn't already being edited.                  **/
  /****************************************************************************/
  for (d = descriptor_list; d; d = d->next) {
    if (STATE(d) == CON_OEDIT) {
      if (d->olc && OLC_NUM(d) == number) {
        send_to_char(ch, "That object is currently being edited by %s.\r\n",
          PERS(d->character, ch));
        return;
      }
    }
  }
  
  /****************************************************************************/
  /** Point d to the builder's descriptor (for easier typing later).         **/
  /****************************************************************************/
  d = ch->desc;
  
  /****************************************************************************/
  /** Give the descriptor an OLC structure.                                  **/
  /****************************************************************************/
  if (d->olc) {
    mudlog(BRF, LVL_IMMORT, TRUE,
      "SYSERR: do_oasis: Player already had olc structure.");
    free(d->olc);
  }
  
  CREATE(d->olc, struct oasis_olc_data, 1);
  
  /****************************************************************************/
  /** Find the zone.                                                         **/
  /****************************************************************************/
  OLC_ZNUM(d) = save ? real_zone(number) : real_zone_by_thing(number);
  if (OLC_ZNUM(d) == NOWHERE) {
    send_to_char(ch, "Sorry, there is no zone for that number!\r\n");
    
    /**************************************************************************/
    /** Free the descriptor's OLC structure.                                 **/
    /**************************************************************************/
    free(d->olc);
    d->olc = NULL;
    return;
  }
  
  /****************************************************************************/
  /** Everyone but IMPLs can only edit zones they have been assigned.        **/
  /****************************************************************************/
  if (!can_edit_zone(ch, OLC_ZNUM(d))) {
    send_cannot_edit(ch, zone_table[OLC_ZNUM(d)].number);
    
    /**************************************************************************/
    /** Free the descriptor's OLC structure.                                 **/
    /**************************************************************************/
    free(d->olc);
    d->olc = NULL;
    return;
  }
  
  /****************************************************************************/
  /** If we need to save, save the objects.                                  **/
  /****************************************************************************/
  if (save) {
    send_to_char(ch, "Saving all objects in zone %d.\r\n",
      zone_table[OLC_ZNUM(d)].number);
    mudlog(CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE,
      "OLC: %s saves object info for zone %d.", GET_NAME(ch),
      zone_table[OLC_ZNUM(d)].number);
    
    /**************************************************************************/
    /** Save the objects in this zone.                                       **/
    /**************************************************************************/
    save_objects(OLC_ZNUM(d));
    
    /**************************************************************************/
    /** Free the descriptor's OLC structure.                                 **/
    /**************************************************************************/
    free(d->olc);
    d->olc = NULL;
    return;
  }
  
  OLC_NUM(d) = number;
  
  /****************************************************************************/
  /** If this is a new object, setup a new object, otherwise setup the       **/
  /** existing object.                                                       **/
  /****************************************************************************/
  if ((real_num = real_object(number)) != NOTHING)
    oedit_setup_existing(d, real_num);
  else
    oedit_setup_new(d);
    
  STATE(d) = CON_OEDIT;
  
  /****************************************************************************/
  /** Send the OLC message to the players in the same room as the builder.   **/
  /****************************************************************************/
  act("$n starts using OLC.", TRUE, d->character, 0, 0, TO_ROOM);
  SET_BIT_AR(PLR_FLAGS(ch), PLR_WRITING);
  
  /****************************************************************************/
  /** Log the OLC message.                                                   **/
  /****************************************************************************/
  mudlog(BRF, LVL_IMMORT, TRUE, "OLC: %s starts editing zone %d allowed zone %d",
    GET_NAME(ch), zone_table[OLC_ZNUM(d)].number, GET_OLC_ZONE(ch));
}

void oedit_setup_new(struct descriptor_data *d)
{
  CREATE(OLC_OBJ(d), struct obj_data, 1);

  clear_object(OLC_OBJ(d));
  OLC_OBJ(d)->name = strdup("unfinished object");
  OLC_OBJ(d)->description = strdup("An unfinished object is lying here.");
  OLC_OBJ(d)->short_description = strdup("an unfinished object");
  SET_BIT_AR(GET_OBJ_WEAR(OLC_OBJ(d)), ITEM_WEAR_TAKE);
  OLC_VAL(d) = 0;
  OLC_ITEM_TYPE(d) = OBJ_TRIGGER;

  SCRIPT(OLC_OBJ(d)) = NULL;
  OLC_OBJ(d)->proto_script = OLC_SCRIPT(d) = NULL;

  oedit_disp_menu(d);
}

/*------------------------------------------------------------------------*/

void oedit_setup_existing(struct descriptor_data *d, int real_num)
{
  struct obj_data *obj;

  /*
   * Allocate object in memory.
   */
  CREATE(obj, struct obj_data, 1);
  copy_object(obj, &obj_proto[real_num]);

  /*
   * Attach new object to player's descriptor.
   */
  OLC_OBJ(d) = obj;
  OLC_VAL(d) = 0;
  OLC_ITEM_TYPE(d) = OBJ_TRIGGER;
  dg_olc_script_copy(d);
  /*
   * The edited obj must not have a script.
   * It will be assigned to the updated obj later, after editing.
   */
  SCRIPT(obj) = NULL;
  OLC_OBJ(d)->proto_script = NULL;

  oedit_disp_menu(d);
}

/*------------------------------------------------------------------------*/

void oedit_save_internally(struct descriptor_data *d)
{
  int i;
  obj_rnum robj_num;
  struct descriptor_data *dsc;
  struct obj_data *obj;
  
  i = (real_object(OLC_NUM(d)) == NOTHING);

  if ((robj_num = add_object(OLC_OBJ(d), OLC_NUM(d))) == NOTHING) {
    log("oedit_save_internally: add_object failed.");
    return;
  }

  /* Update triggers : */
  /* Free old proto list  */
  if (obj_proto[robj_num].proto_script &&
      obj_proto[robj_num].proto_script != OLC_SCRIPT(d)) 
    free_proto_script(&obj_proto[robj_num], OBJ_TRIGGER);   
  /* this will handle new instances of the object: */
  obj_proto[robj_num].proto_script = OLC_SCRIPT(d);

  /* this takes care of the objects currently in-game */
  for (obj = object_list; obj; obj = obj->next) {
    if (obj->item_number != robj_num)
      continue;
    /* remove any old scripts */
    if (SCRIPT(obj)) 
      extract_script(obj, OBJ_TRIGGER);

    free_proto_script(obj, OBJ_TRIGGER);
    copy_proto_script(&obj_proto[robj_num], obj, OBJ_TRIGGER);
    assign_triggers(obj, OBJ_TRIGGER);
  }
  /* end trigger update */

  if (!i)	/* If it's not a new object, don't renumber. */
    return;

  /*
   * Renumber produce in shops being edited.
   */
  for (dsc = descriptor_list; dsc; dsc = dsc->next)
    if (STATE(dsc) == CON_SEDIT)
      for (i = 0; S_PRODUCT(OLC_SHOP(dsc), i) != NOTHING; i++)
	if (S_PRODUCT(OLC_SHOP(dsc), i) >= robj_num)
	  S_PRODUCT(OLC_SHOP(dsc), i)++;


  /* Update other people in zedit too. From: C.Raehl 4/27/99 */
  for (dsc = descriptor_list; dsc; dsc = dsc->next)
    if (STATE(dsc) == CON_ZEDIT)
      for (i = 0; OLC_ZONE(dsc)->cmd[i].command != 'S'; i++)
        switch (OLC_ZONE(dsc)->cmd[i].command) {
          case 'P':
            OLC_ZONE(dsc)->cmd[i].arg3 += (OLC_ZONE(dsc)->cmd[i].arg3 >= robj_num);
            /* Fall through. */
          case 'E':
          case 'G':
          case 'O':
            OLC_ZONE(dsc)->cmd[i].arg1 += (OLC_ZONE(dsc)->cmd[i].arg1 >= robj_num);
            break;
          case 'R':
            OLC_ZONE(dsc)->cmd[i].arg2 += (OLC_ZONE(dsc)->cmd[i].arg2 >= robj_num);
            break;
          default:
          break;
        }
}

/*------------------------------------------------------------------------*/

void oedit_save_to_disk(int zone_num)
{
  save_objects(zone_num);
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

/*
 * For container flags.
 */
void oedit_disp_container_flags_menu(struct descriptor_data *d)
{
  char bits[MAX_STRING_LENGTH];
  get_char_colors(d->character);
  clear_screen(d);

  sprintbit(GET_OBJ_VAL(OLC_OBJ(d), 1), container_bits, bits, sizeof(bits));
  write_to_output(d, 
	  "%s1%s) CLOSEABLE\r\n"
	  "%s2%s) PICKPROOF\r\n"
	  "%s3%s) CLOSED\r\n"
	  "%s4%s) LOCKED\r\n"
	  "Container flags: %s%s%s\r\n"
	  "Enter flag, 0 to quit : ",
	  grn, nrm, grn, nrm, grn, nrm, grn, nrm, cyn, bits, nrm);
}

/*
 * For extra descriptions.
 */
void oedit_disp_extradesc_menu(struct descriptor_data *d)
{
  struct extra_descr_data *extra_desc = OLC_DESC(d);

  get_char_colors(d->character);
  clear_screen(d);
  write_to_output(d,
	  "Extra desc menu\r\n"
	  "%s1%s) Keyword: %s%s\r\n"
	  "%s2%s) Description:\r\n%s%s\r\n"
	  "%s3%s) Goto next description: %s\r\n"
	  "%s0%s) Quit\r\n"
	  "Enter choice : ",

     	  grn, nrm, yel, (extra_desc->keyword && *extra_desc->keyword) ? extra_desc->keyword : "<NONE>",
	  grn, nrm, yel, (extra_desc->description && *extra_desc->description) ? extra_desc->description : "<NONE>",
	  grn, nrm, !extra_desc->next ? "<Not set>\r\n" : "Set.", grn, nrm);
  OLC_MODE(d) = OEDIT_EXTRADESC_MENU;
}

/*
 * Ask for *which* apply to edit.
 */
void oedit_disp_prompt_apply_menu(struct descriptor_data *d)
{
  char apply_buf[MAX_STRING_LENGTH];
  int counter;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 0; counter < MAX_OBJ_AFFECT; counter++) {
    if (OLC_OBJ(d)->affected[counter].modifier) {
      sprinttype(OLC_OBJ(d)->affected[counter].location, apply_types, apply_buf, sizeof(apply_buf));
      write_to_output(d, " %s%d%s) %+d to %s\r\n", grn, counter + 1, nrm,
	      OLC_OBJ(d)->affected[counter].modifier, apply_buf);
    } else {
      write_to_output(d, " %s%d%s) None.\r\n", grn, counter + 1, nrm);
    }
  }
  write_to_output(d, "\r\nEnter affection to modify (0 to quit) : ");
  OLC_MODE(d) = OEDIT_PROMPT_APPLY;
}

/*
 * Ask for liquid type.
 */
void oedit_liquid_type(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 0; counter < NUM_LIQ_TYPES; counter++) {
    write_to_output(d, " %s%2d%s) %s%-20.20s %s", grn, counter, nrm, yel,
	    drinks[counter], !(++columns % 2) ? "\r\n" : "");
  }
  write_to_output(d, "\r\n%sEnter drink type : ", nrm);
  OLC_MODE(d) = OEDIT_VALUE_3;
}

/*
 * The actual apply to set.
 */
void oedit_disp_apply_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 0; counter < NUM_APPLIES; counter++) {
    write_to_output(d, "%s%2d%s) %-20.20s %s", grn, counter, nrm,
		apply_types[counter], !(++columns % 3) ? "\r\n" : "");
  }
  write_to_output(d, "\r\nEnter apply type (0 is no apply) : ");
  OLC_MODE(d) = OEDIT_APPLY;
}

/*
 * Weapon type.
 */
void oedit_disp_weapon_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 0; counter < NUM_ATTACK_TYPES; counter++) {
    write_to_output(d, "%s%2d%s) %-20.20s %s", grn, counter, nrm,
		attack_hit_text[counter].singular,
		!(++columns % 2) ? "\r\n" : "");
  }
  write_to_output(d, "\r\nEnter weapon type : ");
}

/*
 * Spell type.
 */
void oedit_disp_spells_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 1; counter < NUM_SPELLS; counter++) {
    write_to_output(d, "%s%2d%s) %s%-20.20s %s", grn, counter, nrm, yel,
		spell_info[counter].name, !(++columns % 3) ? "\r\n" : "");
  }
  write_to_output(d, "\r\n%sEnter spell choice (-1 for none) : ", nrm);
}

/*
 * Object value #1
 */
void oedit_disp_val1_menu(struct descriptor_data *d)
{
  OLC_MODE(d) = OEDIT_VALUE_1;
  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
  case ITEM_LIGHT:
    /*
     * values 0 and 1 are unused.. jump to 2 
     */
    oedit_disp_val3_menu(d);
    break;
  case ITEM_SCROLL:
  case ITEM_WAND:
  case ITEM_STAFF:
  case ITEM_POTION:
    write_to_output(d, "Spell level : ");
    break;
  case ITEM_WEAPON:
    /*
     * This doesn't seem to be used if I remembe right.
     */
    write_to_output(d, "Modifier to Hitroll : ");
    break;
  case ITEM_ARMOR:
    write_to_output(d, "Apply to AC : ");
    break;
  case ITEM_CONTAINER:
    write_to_output(d, "Max weight to contain (-1 for unlimited) : ");
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    write_to_output(d, "Max drink units (-1 for unlimited) : ");
    break;
  case ITEM_FOOD:
    write_to_output(d, "Hours to fill stomach : ");
    break;
  case ITEM_MONEY:
    write_to_output(d, "Number of gold coins : ");
    break;
  case ITEM_NOTE:
    /*
     * This is supposed to be language, but it's unused.
     */
    break;
  case ITEM_VEHICLE:
    write_to_output(d, "Enter room vnum of vehicle interior : ");
    break;
  case ITEM_HATCH:
    write_to_output(d, "Enter vnum of the vehicle this hatch belongs to : ");
    break;
  case ITEM_WINDOW:
    write_to_output(d, "Enter vnum of the vehicle this window belongs to, or -1 to specify the viewport room : ");
    break;
  case ITEM_CONTROL:
    write_to_output(d, "Enter vnum of the vehicle these controls belong to : ");
    break;
  case ITEM_PORTAL:
    write_to_output(d, "Which room number is the destination? : ");
    break;
  default:
    oedit_disp_menu(d);
  }
}

/*
 * Object value #2
 */
void oedit_disp_val2_menu(struct descriptor_data *d)
{
  OLC_MODE(d) = OEDIT_VALUE_2;
  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
  case ITEM_SCROLL:
  case ITEM_POTION:
    oedit_disp_spells_menu(d);
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    write_to_output(d, "Max number of charges : ");
    break;
  case ITEM_WEAPON:
    write_to_output(d, "Number of damage dice : ");
    break;
  case ITEM_FOOD:
    /*
     * Values 2 and 3 are unused, jump to 4...Odd.
     */
    oedit_disp_val4_menu(d);
    break;
  case ITEM_CONTAINER:
  case ITEM_VEHICLE:
  case ITEM_HATCH:
  case ITEM_WINDOW:
  case ITEM_PORTAL:
    /*
     * These are flags, needs a bit of special handling.
     */
    oedit_disp_container_flags_menu(d);
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    write_to_output(d, "Initial drink units : ");
    break;
  case ITEM_BOARD:
    write_to_output(d, "Enter the minimum level to read this board: ");
    break;
  default:
    oedit_disp_menu(d);
  }
}

/*
 * Object value #3
 */
void oedit_disp_val3_menu(struct descriptor_data *d)
{
  OLC_MODE(d) = OEDIT_VALUE_3;
  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
  case ITEM_LIGHT:
    write_to_output(d, "Number of hours (0 = burnt, -1 is infinite) : ");
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    oedit_disp_spells_menu(d);
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    write_to_output(d, "Number of charges remaining : ");
    break;
  case ITEM_WEAPON:
    write_to_output(d, "Size of damage dice : ");
    break;
  case ITEM_CONTAINER:
    write_to_output(d, "Vnum of key to open container (-1 for no key) : ");
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    oedit_liquid_type(d);
    break;
  case ITEM_VEHICLE:
    write_to_output(d, "Vnum of key to unlock vehicle (-1 for no key) : ");
    break;
  case ITEM_HATCH:
    write_to_output(d, "Vnum of key to unlock hatch (-1 for no key) : ");
    break;
  case ITEM_WINDOW:
    write_to_output(d, "Vnum of key to unlock window (-1 for no key) : ");
    break;
  case ITEM_PORTAL:
    write_to_output(d, "Vnum of the key to unlock portal (-1 for no key) : ");
    break;
  case ITEM_BOARD:
    write_to_output(d, "Minimum level to write: ");
    break;
  default:
    oedit_disp_menu(d);
  }
}

/*
 * Object value #4
 */
void oedit_disp_val4_menu(struct descriptor_data *d)
{
  OLC_MODE(d) = OEDIT_VALUE_4;
  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
  case ITEM_SCROLL:
  case ITEM_POTION:
  case ITEM_WAND:
  case ITEM_STAFF:
    oedit_disp_spells_menu(d);
    break;
  case ITEM_WEAPON:
    oedit_disp_weapon_menu(d);
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
  case ITEM_FOOD:
    write_to_output(d, "Poisoned (0 = not poison) : ");
    break;
  case ITEM_VEHICLE:
    write_to_output(d, "What is the vehicle's appearance? (-1 for transparent) : ");
    break;
  case ITEM_PORTAL:
    write_to_output(d, "What is the portal's appearance? (-1 for transparent) : ");
    break;
  case ITEM_WINDOW:
    if (GET_OBJ_VAL(OLC_OBJ(d), 0) < 0)
      write_to_output(d, "What is the viewport room vnum (-1 for default location) : ");
    else
      oedit_disp_menu(d);
    break;
  case ITEM_BOARD:
    write_to_output(d, "Minimum level to remove messages: ");
    break;
  default:
    oedit_disp_menu(d);
  }
}

/*
 * Object type.
 */
void oedit_disp_type_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 0; counter < NUM_ITEM_TYPES; counter++) {
    write_to_output(d, "%s%2d%s) %-20.20s %s", grn, counter, nrm,
		item_types[counter], !(++columns % 2) ? "\r\n" : "");
  }
  write_to_output(d, "\r\nEnter object type : ");
}

/*
 * Object extra flags.
 */
void oedit_disp_extra_menu(struct descriptor_data *d)
{
  char bits[MAX_STRING_LENGTH];
  int counter, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 0; counter < NUM_ITEM_FLAGS; counter++) {
    write_to_output(d, "%s%2d%s) %-20.20s %s", grn, counter + 1, nrm,
		extra_bits[counter], !(++columns % 2) ? "\r\n" : "");
  }
  sprintbitarray(GET_OBJ_EXTRA(OLC_OBJ(d)), extra_bits, EF_ARRAY_MAX, bits);
  write_to_output(d, "\r\nObject flags: %s%s%s\r\n"
	  "Enter object extra flag (0 to quit) : ",
	  cyn, bits, nrm);
}

/*
 * Object perm flags.
 */
void oedit_disp_perm_menu(struct descriptor_data *d)
{
  char bitbuf[MAX_INPUT_LENGTH];
  int counter, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 1; counter <= NUM_AFF_FLAGS; counter++) {
    /* Setting AFF_CHARM on objects like this is dangerous. */
    if (counter == AFF_CHARM)
      continue; 
    write_to_output(d, "%s%2d%s) %-20.20s %s", grn, counter, 
                       nrm, affected_bits[counter], !(++columns % 2) ? "\r\n" : "");
  }
  sprintbitarray(GET_OBJ_PERM(OLC_OBJ(d)), affected_bits, EF_ARRAY_MAX, bitbuf);
  write_to_output(d, "\r\nObject permanent flags: %s%s%s\r\n"
          "Enter object perm flag (0 to quit) : ", cyn, bitbuf, nrm);
}

/*
 * Object wear flags.
 */
void oedit_disp_wear_menu(struct descriptor_data *d)
{
  char bits[MAX_STRING_LENGTH];
  int counter, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 0; counter < NUM_ITEM_WEARS; counter++) {
    write_to_output(d, "%s%2d%s) %-20.20s %s", grn, counter + 1, nrm,
		wear_bits[counter], !(++columns % 2) ? "\r\n" : "");
  }
  sprintbitarray(GET_OBJ_WEAR(OLC_OBJ(d)), wear_bits, TW_ARRAY_MAX, bits);
  write_to_output(d, "\r\nWear flags: %s%s%s\r\n"
	  "Enter wear flag, 0 to quit : ", cyn, bits, nrm);
}

/*
 * Display main menu.
 */
void oedit_disp_menu(struct descriptor_data *d)
{
  char tbitbuf[MAX_INPUT_LENGTH], ebitbuf[MAX_INPUT_LENGTH];
  struct obj_data *obj;

  obj = OLC_OBJ(d);
  get_char_colors(d->character);
  clear_screen(d);

  /*
   * Build buffers for first part of menu.
   */
  sprinttype(GET_OBJ_TYPE(obj), item_types, tbitbuf, sizeof(tbitbuf));
  sprintbitarray(GET_OBJ_EXTRA(obj), extra_bits, EF_ARRAY_MAX, ebitbuf);

  /*
   * Build first half of menu.
   */
  write_to_output(d,
	  "-- Item number : [%s%d%s]\r\n"
	  "%s1%s) Namelist : %s%s\r\n"
	  "%s2%s) S-Desc   : %s%s\r\n"
	  "%s3%s) L-Desc   :-\r\n%s%s\r\n"
	  "%s4%s) A-Desc   :-\r\n%s%s"
	  "%s5%s) Type        : %s%s\r\n"
	  "%s6%s) Extra flags : %s%s\r\n",

	  cyn, OLC_NUM(d), nrm,
	  grn, nrm, yel, (obj->name && *obj->name) ? obj->name : "undefined",
	  grn, nrm, yel, (obj->short_description && *obj->short_description) ? obj->short_description : "undefined",
	  grn, nrm, yel, (obj->description && *obj->description) ? obj->description : "undefined",
	  grn, nrm, yel, (obj->action_description && *obj->action_description) ? obj->action_description : "<not set>\r\n",
	  grn, nrm, cyn, tbitbuf,
	  grn, nrm, cyn, ebitbuf
	  );
  /*
   * Send first half.
   */

  /*
   * Build second half of menu.
   */
  sprintbitarray(GET_OBJ_WEAR(OLC_OBJ(d)), wear_bits, EF_ARRAY_MAX, tbitbuf);
  sprintbitarray(GET_OBJ_PERM(OLC_OBJ(d)), affected_bits, EF_ARRAY_MAX, ebitbuf);

  write_to_output(d,
	  "%s7%s) Wear flags  : %s%s\r\n"
	  "%s8%s) Weight      : %s%d\r\n"
	  "%s9%s) Cost        : %s%d\r\n"
	  "%sA%s) Cost/Day    : %s%d\r\n"
	  "%sB%s) Timer       : %s%d\r\n"
	  "%sC%s) Values      : %s%d %d %d %d\r\n"
	  "%sD%s) Applies menu\r\n"
	  "%sE%s) Extra descriptions menu\r\n"
          "%sM%s) Min Level   : %s%d\r\n"
          "%sP%s) Perm Affects: %s%s\r\n"
          "%sS%s) Script      : %s%s\r\n"
          "%sQ%s) Quit\r\n"
	  "Enter choice : ",

	  grn, nrm, cyn, tbitbuf,
	  grn, nrm, cyn, GET_OBJ_WEIGHT(obj),
	  grn, nrm, cyn, GET_OBJ_COST(obj),
	  grn, nrm, cyn, GET_OBJ_RENT(obj),
	  grn, nrm, cyn, GET_OBJ_TIMER(obj),
	  grn, nrm, cyn, GET_OBJ_VAL(obj, 0),
	  GET_OBJ_VAL(obj, 1),
	  GET_OBJ_VAL(obj, 2),
	  GET_OBJ_VAL(obj, 3),
	  grn, nrm, grn, nrm,
          grn, nrm, cyn, GET_OBJ_LEVEL(obj),
          grn, nrm, cyn, ebitbuf,
          grn, nrm, cyn, OLC_SCRIPT(d) ? "Set." : "Not Set.",
          grn, nrm
  );
  OLC_MODE(d) = OEDIT_MAIN_MENU;
}

/***************************************************************************
 main loop (of sorts).. basically interpreter throws all input to here
 ***************************************************************************/

void oedit_parse(struct descriptor_data *d, char *arg)
{
  int number, max_val, min_val;
  char *oldtext = NULL;
  struct board_info *tmp;
  struct obj_data *obj;
  obj_rnum robj;

  switch (OLC_MODE(d)) {

  case OEDIT_CONFIRM_SAVESTRING:
    switch (*arg) {
    case 'y':
    case 'Y':
      oedit_save_internally(d);
      mudlog(CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(d->character)), TRUE, 
              "OLC: %s edits obj %d", GET_NAME(d->character), OLC_NUM(d));
      if (CONFIG_OLC_SAVE) {
      oedit_save_to_disk(real_zone_by_thing(OLC_NUM(d)));
      write_to_output(d, "Object saved to disk.\r\n");
      } else
      write_to_output(d, "Object saved to memory.\r\n");
      if (GET_OBJ_TYPE(OLC_OBJ(d)) == ITEM_BOARD) {
        if ((tmp=locate_board(GET_OBJ_VNUM(OLC_OBJ(d)))) != NULL) {
          save_board(tmp);
        } else {
          tmp = create_new_board(GET_OBJ_VNUM(OLC_OBJ(d)));
          BOARD_NEXT(tmp) = bboards;
          bboards = tmp;
        }
      }
      /* Fall through. */
    case 'n':
    case 'N':
      cleanup_olc(d, CLEANUP_ALL);
      return;
    case 'a': /* abort quit */
    case 'A': 
      oedit_disp_menu(d);
      return;
    default:
      write_to_output(d, "Invalid choice!\r\n");
      write_to_output(d, "Do you wish to save your changes? : \r\n");
      return;
    }

  case OEDIT_MAIN_MENU:
    /*
     * Throw us out to whichever edit mode based on user input.
     */
    switch (*arg) {
    case 'q':
    case 'Q':
      if (STATE(d) != CON_IEDIT) {
      if (OLC_VAL(d)) {	/* Something has been modified. */
        write_to_output(d, "Do you wish to save your changes? : ");
	OLC_MODE(d) = OEDIT_CONFIRM_SAVESTRING;
      } else
	cleanup_olc(d, CLEANUP_ALL);
      } else {
        send_to_char(d->character, "\r\nCommitting iedit changes.\r\n");
        obj = OLC_IOBJ(d);
        *obj = *(OLC_OBJ(d));
        GET_ID(obj) = max_obj_id++;
        /* find_obj helper */
        add_to_lookup_table(GET_ID(obj), (void *)obj);
        if (GET_OBJ_VNUM(obj) != NOTHING) {
          /* remove any old scripts */
          if (SCRIPT(obj)) {
            extract_script(obj, OBJ_TRIGGER);
            SCRIPT(obj) = NULL;
          }

          free_proto_script(obj, OBJ_TRIGGER);
          robj = real_object(GET_OBJ_VNUM(obj));
          copy_proto_script(&obj_proto[robj], obj, OBJ_TRIGGER);
          assign_triggers(obj, OBJ_TRIGGER);
        }
        SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_UNIQUE_SAVE);
  /* Xap - ought to save the old pointer, free after assignment I suppose */
        mudlog(CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(d->character)), TRUE,
               "OLC: %s iedit a unique #%d", GET_NAME(d->character), GET_OBJ_VNUM(obj));
        if (d->character) {
          REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);
          STATE(d) = CON_PLAYING;
          act("$n stops using OLC.", TRUE, d->character, 0, 0, TO_ROOM);
        }
        free(d->olc);
        d->olc = NULL;
      }
      return;
    case '1':
      write_to_output(d, "Enter namelist : ");
      OLC_MODE(d) = OEDIT_EDIT_NAMELIST;
      break;
    case '2':
      write_to_output(d, "Enter short desc : ");
      OLC_MODE(d) = OEDIT_SHORTDESC;
      break;
    case '3':
      write_to_output(d, "Enter long desc :-\r\n| ");
      OLC_MODE(d) = OEDIT_LONGDESC;
      break;
    case '4':
      OLC_MODE(d) = OEDIT_ACTDESC;
      send_editor_help(d);
      write_to_output(d, "Enter action description:\r\n\r\n");
      if (OLC_OBJ(d)->action_description) {
	write_to_output(d, "%s", OLC_OBJ(d)->action_description);
	oldtext = strdup(OLC_OBJ(d)->action_description);
      }
      string_write(d, &OLC_OBJ(d)->action_description, MAX_MESSAGE_LENGTH, 0, oldtext);
      OLC_VAL(d) = 1;
      break;
    case '5':
      oedit_disp_type_menu(d);
      OLC_MODE(d) = OEDIT_TYPE;
      break;
    case '6':
      oedit_disp_extra_menu(d);
      OLC_MODE(d) = OEDIT_EXTRAS;
      break;
    case '7':
      oedit_disp_wear_menu(d);
      OLC_MODE(d) = OEDIT_WEAR;
      break;
    case '8':
      write_to_output(d, "Enter weight : ");
      OLC_MODE(d) = OEDIT_WEIGHT;
      break;
    case '9':
      write_to_output(d, "Enter cost : ");
      OLC_MODE(d) = OEDIT_COST;
      break;
    case 'a':
    case 'A':
      write_to_output(d, "Enter cost per day : ");
      OLC_MODE(d) = OEDIT_COSTPERDAY;
      break;
    case 'b':
    case 'B':
      write_to_output(d, "Enter timer : ");
      OLC_MODE(d) = OEDIT_TIMER;
      break;
    case 'c':
    case 'C':
      /*
       * Clear any old values  
       */
      GET_OBJ_VAL(OLC_OBJ(d), 0) = 0;
      GET_OBJ_VAL(OLC_OBJ(d), 1) = 0;
      GET_OBJ_VAL(OLC_OBJ(d), 2) = 0;
      GET_OBJ_VAL(OLC_OBJ(d), 3) = 0;
      OLC_VAL(d) = 1;
      oedit_disp_val1_menu(d);
      break;
    case 'd':
    case 'D':
      oedit_disp_prompt_apply_menu(d);
      break;
    case 'e':
    case 'E':
      /*
       * If extra descriptions don't exist.
       */
      if (OLC_OBJ(d)->ex_description == NULL) {
	CREATE(OLC_OBJ(d)->ex_description, struct extra_descr_data, 1);
	OLC_OBJ(d)->ex_description->next = NULL;
      }
      OLC_DESC(d) = OLC_OBJ(d)->ex_description;
      oedit_disp_extradesc_menu(d);
      break;
    case 'm':
    case 'M':
      write_to_output(d, "Enter new minimum level: ");
      OLC_MODE(d) = OEDIT_LEVEL;
      break;
    case 'p':
    case 'P':
      oedit_disp_perm_menu(d);
      OLC_MODE(d) = OEDIT_PERM;
      break;
    case 's':
    case 'S':
      if (STATE(d) != CON_IEDIT) {
      OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_MAIN_MENU;
      dg_script_menu(d);
      } else {
        write_to_output(d, "\r\nScripts cannot be modified on individual objects.\r\nEnter choice : ");
      }
      return;
    default:
      oedit_disp_menu(d);
      break;
    }
    return;			/*
				 * end of OEDIT_MAIN_MENU 
				 */

  case OLC_SCRIPT_EDIT:
    if (dg_script_edit_parse(d, arg)) return;
    break;


  case OEDIT_EDIT_NAMELIST:
    if (!genolc_checkstring(d, arg))
      break;
    if (OLC_OBJ(d)->name)
      free(OLC_OBJ(d)->name);
    OLC_OBJ(d)->name = str_udup(arg);
    break;

  case OEDIT_SHORTDESC:
    if (!genolc_checkstring(d, arg))
      break;
    if (OLC_OBJ(d)->short_description)
      free(OLC_OBJ(d)->short_description);
    OLC_OBJ(d)->short_description = str_udup(arg);
    break;

  case OEDIT_LONGDESC:
    if (!genolc_checkstring(d, arg))
      break;
    if (OLC_OBJ(d)->description)
      free(OLC_OBJ(d)->description);
    OLC_OBJ(d)->description = str_udup(arg);
    break;

  case OEDIT_TYPE:
    number = atoi(arg);
    if ((number < 1) || (number >= NUM_ITEM_TYPES)) {
      write_to_output(d, "Invalid choice, try again : ");
      return;
    } else
      GET_OBJ_TYPE(OLC_OBJ(d)) = number;
    /* what's the boundschecking worth if we don't do this ? -- Welcor */
    GET_OBJ_VAL(OLC_OBJ(d), 0) = GET_OBJ_VAL(OLC_OBJ(d), 1) =
      GET_OBJ_VAL(OLC_OBJ(d), 2) = GET_OBJ_VAL(OLC_OBJ(d), 3) = 0;
    break;

  case OEDIT_EXTRAS:
    number = atoi(arg);
    if ((number < 0) || (number > NUM_ITEM_FLAGS)) {
      oedit_disp_extra_menu(d);
      return;
    } else if (number == 0)
      break;
    else {
      TOGGLE_BIT_AR(GET_OBJ_EXTRA(OLC_OBJ(d)), number - 1);
      oedit_disp_extra_menu(d);
      return;
    }

  case OEDIT_WEAR:
    number = atoi(arg);
    if ((number < 0) || (number > NUM_ITEM_WEARS)) {
      write_to_output(d, "That's not a valid choice!\r\n");
      oedit_disp_wear_menu(d);
      return;
    } else if (number == 0)	/* Quit. */
      break;
    else {
      TOGGLE_BIT_AR(GET_OBJ_WEAR(OLC_OBJ(d)), (number - 1));
      oedit_disp_wear_menu(d);
      return;
    }

  case OEDIT_WEIGHT:
    GET_OBJ_WEIGHT(OLC_OBJ(d)) = LIMIT(atoi(arg), 0, MAX_OBJ_WEIGHT);
    break;

  case OEDIT_COST:
    GET_OBJ_COST(OLC_OBJ(d)) = LIMIT(atoi(arg), 0, MAX_OBJ_COST);
    break;

  case OEDIT_COSTPERDAY:
    GET_OBJ_RENT(OLC_OBJ(d)) = LIMIT(atoi(arg), 0, MAX_OBJ_RENT);
    break;

  case OEDIT_TIMER:
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) { 
      case ITEM_PORTAL: 
        GET_OBJ_TIMER(OLC_OBJ(d)) = LIMIT(atoi(arg), -1, MAX_OBJ_TIMER); 
        break; 
      default: 
        GET_OBJ_TIMER(OLC_OBJ(d)) = LIMIT(atoi(arg), 0, MAX_OBJ_TIMER); 
        break; 
    } 
    break; 

  case OEDIT_LEVEL:
    GET_OBJ_LEVEL(OLC_OBJ(d)) = LIMIT(atoi(arg), 0, LVL_IMPL);
    break;

  case OEDIT_PERM:
    if ((number = atoi(arg)) == 0)
      break;
    if (number > 0 && number <= NUM_AFF_FLAGS) {
      /* Setting AFF_CHARM on objects like this is dangerous. */
      if (number != AFF_CHARM) {
        TOGGLE_BIT_AR(GET_OBJ_PERM(OLC_OBJ(d)), number);
      }
    } 
    oedit_disp_perm_menu(d);
    return;

  case OEDIT_VALUE_1:
    /*
     * Lucky, I don't need to check any of these for out of range values.
     * Hmm, I'm not so sure - Rv  
     */
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
      case ITEM_WEAPON:
	GET_OBJ_VAL(OLC_OBJ(d), 0) = MIN(MAX(atoi(arg), -50), 50);
	break;
      case ITEM_CONTAINER:
        GET_OBJ_VAL(OLC_OBJ(d), 0) = LIMIT(atoi(arg), -1, MAX_CONTAINER_SIZE); 
        break;
      default:
    GET_OBJ_VAL(OLC_OBJ(d), 0) = atoi(arg);
    }
    /*
     * proceed to menu 2 
     */
    oedit_disp_val2_menu(d);
    return;
  case OEDIT_VALUE_2:
    /*
     * Here, I do need to check for out of range values.
     */
    number = atoi(arg);
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      if (number == 0 || number == -1)
	GET_OBJ_VAL(OLC_OBJ(d), 1) = -1;
      else
        GET_OBJ_VAL(OLC_OBJ(d), 1) = LIMIT(number, 1, NUM_SPELLS-1);
	oedit_disp_val3_menu(d);
      break;
    case ITEM_CONTAINER:
    case ITEM_VEHICLE:
    case ITEM_HATCH:
    case ITEM_WINDOW:
    case ITEM_PORTAL:
      /*
       * Needs some special handling since we are dealing with flag values
       * here.
       */
      if (number < 0 || number > 4)
	oedit_disp_container_flags_menu(d);
      else if (number != 0) {
        TOGGLE_BIT(GET_OBJ_VAL(OLC_OBJ(d), 1), 1 << (number - 1));
        OLC_VAL(d) = 1;
	oedit_disp_val2_menu(d);
      } else
	oedit_disp_val3_menu(d);
      break;
    case ITEM_WEAPON:
      GET_OBJ_VAL(OLC_OBJ(d), 1) = LIMIT(number, 1, MAX_WEAPON_NDICE); 
      oedit_disp_val3_menu(d);
      break;

    default:
      GET_OBJ_VAL(OLC_OBJ(d), 1) = number;
      oedit_disp_val3_menu(d);
    }
    return;

  case OEDIT_VALUE_3:
    number = atoi(arg);
    /*
     * Quick'n'easy error checking.
     */
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      if (number == 0 || number == -1) {
	GET_OBJ_VAL(OLC_OBJ(d), 2) = -1;
        oedit_disp_val4_menu(d);
	return;
      }
      min_val = 1;
      max_val = NUM_SPELLS - 1;
      break;
    case ITEM_WEAPON:
      min_val = 1;
      max_val = MAX_WEAPON_SDICE;
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      min_val = 0;
      max_val = 20;
      break;
    case ITEM_DRINKCON:
    case ITEM_FOUNTAIN:
      min_val = 0;
      max_val = NUM_LIQ_TYPES - 1;
      break;
    case ITEM_KEY:
      min_val = 0;
      max_val = 32099;
      break;
    default:
      min_val = -32000;
      max_val = 32000;
    }
    GET_OBJ_VAL(OLC_OBJ(d), 2) = LIMIT(number, min_val, max_val);
    oedit_disp_val4_menu(d);
    return;

  case OEDIT_VALUE_4:
    number = atoi(arg);
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      if (number == 0 || number == -1) {
        GET_OBJ_VAL(OLC_OBJ(d), 3) = -1;
        oedit_disp_menu(d);
        return;
      }
      min_val = 1;
      max_val = NUM_SPELLS - 1;
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      min_val = 1;
      max_val = NUM_SPELLS - 1;
      break;
    case ITEM_WEAPON:
      min_val = 0;
      max_val = NUM_ATTACK_TYPES - 1;
      break;
    default:
      min_val = -32000;
      max_val = 32000;
      break;
    }
    GET_OBJ_VAL(OLC_OBJ(d), 3) = LIMIT(number, min_val, max_val);
    break;

  case OEDIT_PROMPT_APPLY:
    if ((number = atoi(arg)) == 0)
      break;
    else if (number < 0 || number > MAX_OBJ_AFFECT) {
      oedit_disp_prompt_apply_menu(d);
      return;
    }
    OLC_VAL(d) = number - 1;
    OLC_MODE(d) = OEDIT_APPLY;
    oedit_disp_apply_menu(d);
    return;

  case OEDIT_APPLY:
    if ((number = atoi(arg)) == 0) {
      OLC_OBJ(d)->affected[OLC_VAL(d)].location = 0;
      OLC_OBJ(d)->affected[OLC_VAL(d)].modifier = 0;
      oedit_disp_prompt_apply_menu(d);
    } else if (number < 0 || number >= NUM_APPLIES)
      oedit_disp_apply_menu(d);
    else {
      int counter;

      /* add in check here if already applied.. deny builders another */
      if (GET_LEVEL(d->character) < LVL_IMPL) {
        for (counter = 0; counter < MAX_OBJ_AFFECT; counter++) {
          if (OLC_OBJ(d)->affected[counter].location == number) {
            write_to_output(d, "Object already has that apply.");
            return;
          }
        }
      }

      OLC_OBJ(d)->affected[OLC_VAL(d)].location = number;
      write_to_output(d, "Modifier : ");
      OLC_MODE(d) = OEDIT_APPLYMOD;
    }
    return;

  case OEDIT_APPLYMOD:
    OLC_OBJ(d)->affected[OLC_VAL(d)].modifier = atoi(arg);
    oedit_disp_prompt_apply_menu(d);
    return;

  case OEDIT_EXTRADESC_KEY:
    if (genolc_checkstring(d, arg)) {
      if (OLC_DESC(d)->keyword)
        free(OLC_DESC(d)->keyword);
      OLC_DESC(d)->keyword = str_udup(arg);
    }
    oedit_disp_extradesc_menu(d);
    return;

  case OEDIT_EXTRADESC_MENU:
    switch ((number = atoi(arg))) {
    case 0:
      if (!OLC_DESC(d)->keyword || !OLC_DESC(d)->description) {
        struct extra_descr_data *temp;

	if (OLC_DESC(d)->keyword)
	  free(OLC_DESC(d)->keyword);
	if (OLC_DESC(d)->description)
	  free(OLC_DESC(d)->description);

	/*
	 * Clean up pointers  
	 */
	REMOVE_FROM_LIST(OLC_DESC(d), OLC_OBJ(d)->ex_description, next);
	free(OLC_DESC(d));
	OLC_DESC(d) = NULL;
      }
    break;

    case 1:
      OLC_MODE(d) = OEDIT_EXTRADESC_KEY;
      write_to_output(d, "Enter keywords, separated by spaces :-\r\n| ");
      return;

    case 2:
      OLC_MODE(d) = OEDIT_EXTRADESC_DESCRIPTION;
      send_editor_help(d);
      write_to_output(d, "Enter the extra description:\r\n\r\n");
      if (OLC_DESC(d)->description) {
	write_to_output(d, "%s", OLC_DESC(d)->description);
	oldtext = strdup(OLC_DESC(d)->description);
      }
      string_write(d, &OLC_DESC(d)->description, MAX_MESSAGE_LENGTH, 0, oldtext);
      OLC_VAL(d) = 1;
      return;

    case 3:
      /*
       * Only go to the next description if this one is finished.
       */
      if (OLC_DESC(d)->keyword && OLC_DESC(d)->description) {
	struct extra_descr_data *new_extra;

	if (OLC_DESC(d)->next)
	  OLC_DESC(d) = OLC_DESC(d)->next;
	else {	/* Make new extra description and attach at end. */
	  CREATE(new_extra, struct extra_descr_data, 1);
	  OLC_DESC(d)->next = new_extra;
	  OLC_DESC(d) = OLC_DESC(d)->next;
	}
      }
      /*
       * No break - drop into default case.
       */
    default:
      oedit_disp_extradesc_menu(d);
      return;
    }
    break;
  default:
    mudlog(BRF, LVL_BUILDER, TRUE, "SYSERR: OLC: Reached default case in oedit_parse()!");
    write_to_output(d, "Oops...\r\n");
    break;
  }

  /*
   * If we get here, we have changed something.  
   */
  OLC_VAL(d) = 1;
  oedit_disp_menu(d);
}

void oedit_string_cleanup(struct descriptor_data *d, int terminator)
{
  switch (OLC_MODE(d)) {
  case OEDIT_ACTDESC:
    oedit_disp_menu(d);
    break;
  case OEDIT_EXTRADESC_DESCRIPTION:
    oedit_disp_extradesc_menu(d);
    break;
  }
}

/* this is all iedit stuff */
void iedit_setup_existing(struct descriptor_data *d, struct obj_data *real_num)
{
  struct obj_data *obj;

  OLC_IOBJ(d) = real_num;

  obj = create_obj();
  copy_object(obj,real_num);

  /* free any assigned scripts */
  if (SCRIPT(obj))
    extract_script(obj, OBJ_TRIGGER);
  SCRIPT(obj) = NULL;
  /* find_obj helper */
  remove_from_lookup_table(GET_ID(obj));

  OLC_OBJ(d) = obj;
  OLC_IOBJ(d) = real_num;
  OLC_VAL(d) = 0;
  oedit_disp_menu(d);
}

ACMD(do_iedit) {
  struct obj_data *k;
  int found=0;
  extern struct room_data *world;
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);

  if(!*arg || !*argument) {
    send_to_char(ch, "You must supply an object name.\r\n");
  }

  if ((k = get_obj_in_equip_vis(ch, arg, NULL, ch->equipment))) {
    found=1;
  } else if ((k = get_obj_in_list_vis(ch, arg, NULL, ch->carrying))) {
    found=1;
  } else if ((k = get_obj_in_list_vis(ch, arg, NULL, world[IN_ROOM(ch)].contents))) {
    found =1;
  } else if ((k = get_obj_vis(ch, arg, NULL))) {
    found=1;
  }

  if (!found) {
    send_to_char(ch, "Couldn't find that object. Sorry.\r\n");
    return;
  }

                /* set up here */
  CREATE(OLC(ch->desc), struct oasis_olc_data, 1);
  SET_BIT_AR(GET_OBJ_EXTRA(k), ITEM_UNIQUE_SAVE);

  SET_BIT_AR(PLR_FLAGS(ch), PLR_WRITING);
  iedit_setup_existing(ch->desc,k);
  OLC_VAL(ch->desc) = 0;

  act("$n starts using OLC.", TRUE, ch, 0, 0, TO_ROOM);

  STATE(ch->desc) = CON_IEDIT;

  return;
}


