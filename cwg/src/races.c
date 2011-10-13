#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"


const char *race_abbrevs[] = {
        "Hum",
        "Elf",
        "Gno",
        "Dwa",
        "\n"
};

const char *pc_race_types[] = {
        "Human",
        "Elf",
        "Gnome",
        "Dwarf",
        "\n"
};

const char *race_names[] = {
  "human",
  "elf",
  "gnome",
  "dwarf",
  "\n"
};

#define Y   TRUE
#define N   FALSE

/* Original Race/Gender Breakout */
int race_ok_gender[NUM_SEX][NUM_RACES] = {
/*        H, E, G, D */
/* N */ { N, N, N, N },
/* M */ { Y, Y, Y, Y }, 
/* F */ { Y, Y, Y, Y } 
};

const char *race_display[NUM_RACES] = {
  "@B1@W) Human\r\n",
  "@B2@W) @GElf\r\n",
  "@B3@W) @MGnome\r\n",
  "@B4@W) @BDwarf\r\n",
};

/*
 * The code to interpret a race letter (used in interpreter.c when a
 * new character is selecting a race).
 */
int parse_race(struct char_data *ch, int arg)
{
  int race = RACE_UNDEFINED;

  switch (arg) {
  case 1:  race = RACE_HUMAN      ; break;
  case 2:  race = RACE_ELF        ; break;
  case 3:  race = RACE_GNOME      ; break;
  case 4:  race = RACE_DWARF      ; break;
  default: race = RACE_UNDEFINED  ; break;
  }
  if (race >= 0 && race < NUM_RACES)
    if (!race_ok_gender[(int)GET_SEX(ch)][race])
      race = RACE_UNDEFINED;

  return (race);
}

void racial_ability_modifiers(struct char_data *ch)
{
  switch (GET_RACE(ch)) {
    case RACE_HUMAN:
      break;
    case RACE_ELF:
      ch->real_abils.dex += 1;
      ch->real_abils.con -= 1;
      break;
    case RACE_GNOME:
      ch->real_abils.intel += 1;
      ch->real_abils.wis -= 1;
      break;
    case RACE_DWARF:
      ch->real_abils.con += 1;
      ch->real_abils.cha -= 1;
      break;
    default:
      break;
  }
}

/* Converted into metric units: cm and kg; SRD has english units. */
struct {
  int height[NUM_SEX];  /* cm */
  int heightdie;        /* 2d(heightdie) added to height */
  int weight[NUM_SEX];  /* kg */
  int weightfac;        /* added height * weightfac/100 added to weight */
} hw_info[NUM_RACES] = {
/* RACE_HUMAN      */ { {141, 147, 135}, 26, {46, 54, 39}, 89},
/* RACE_ELF        */ { {135, 135, 135}, 15, {37, 39, 36}, 63},
/* RACE_GNOME      */ { {93, 91, 96}, 10, {17, 18, 16}, 18},
/* RACE_DWARF      */ { {111, 114, 109}, 10, {52, 59, 45}, 125}
};

void set_height_and_weight_by_race(struct char_data *ch)
{
  int race, sex, mod;

  race = GET_RACE(ch);
  sex = GET_SEX(ch);
  if (sex < SEX_NEUTRAL || sex >= NUM_SEX) {
    log("Invalid gender in set_height_and_weight_by_race: %d", sex);
    sex = SEX_NEUTRAL;
  }
  if (race <= RACE_UNDEFINED || race >= NUM_RACES) {
    log("Invalid gender in set_height_and_weight_by_race: %d", GET_SEX(ch));
    race = RACE_UNDEFINED + 1; /* first defined race */
  }

  mod = dice(2, hw_info[race].heightdie);
  GET_HEIGHT(ch) = hw_info[race].height[sex] + mod;
  mod *= hw_info[race].weightfac;
  mod /= 100;
  GET_WEIGHT(ch) = hw_info[race].weight[sex] + mod;
}

int invalid_race(struct char_data *ch, struct obj_data *obj)
{
  if (GET_LEVEL(ch) >= LVL_IMMORT)
    return FALSE;

  if (OBJ_FLAGGED(obj, ITEM_ANTI_HUMAN) && IS_HUMAN(ch))
    return (TRUE);

  if (OBJ_FLAGGED(obj, ITEM_ANTI_ELF) && IS_ELF(ch))
    return (TRUE);

  if (OBJ_FLAGGED(obj, ITEM_ANTI_DWARF) && IS_DWARF(ch))
    return (TRUE);

  if (OBJ_FLAGGED(obj, ITEM_ANTI_GNOME) && IS_GNOME(ch))
    return (TRUE);

  return (FALSE);
}

