/* ************************************************************************
*   File: constants.h                                   Part of CircleMUD *
*  Usage: Header file for constants.                                      *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  tbaMUD is based on CircleMUD and DikuMUD, Copyright (C) 1990, 1991.    *
************************************************************************ */

extern const char *circlemud_version;
extern const char *oasisolc_version;
extern const char *ascii_pfiles_version;
extern const char *dirs[];
extern const char *abbr_dirs[];
extern const char *room_bits[];
extern const char *exit_bits[];
extern const char *sector_types[];
extern const char *genders[];
extern const char *position_types[];
extern const char *player_bits[];
extern const char *action_bits[];
extern const char *preference_bits[];
extern const char *affected_bits[];
extern const char *connected_types[];
extern const char *wear_where[];
extern const char *equipment_types[];
extern const char *item_types[];
extern const char *wear_bits[];
extern const char *extra_bits[];
extern const char *apply_types[];
extern const char *container_bits[];
extern const char *drinks[];
extern const char *drinknames[];
extern const char *color_liquid[];
extern const char *fullness[];
extern const char *npc_class_types[];
extern const char *weekdays[];
extern const char *month_name[];
extern const char *trig_types[];
extern const char *otrig_types[];
extern const char *wtrig_types[];
#if defined(CONFIG_OASIS_MPROG)
extern const char *mobprog_types[];
#endif
extern const struct str_app_type str_app[];
extern const struct dex_skill_type dex_app_skill[];
extern const struct dex_skill_type race_app_skill[NUM_RACES];
extern const struct dex_app_type dex_app[];
extern const struct con_app_type con_app[];
extern const struct int_app_type int_app[];
extern const struct wis_app_type wis_app[];
extern int rev_dir[];
extern int movement_loss[];
extern int drink_aff[][3];
extern const char *trig_types[];
extern const char *otrig_types[];
extern const char *wtrig_types[];
#if defined(CONFIG_OASIS_MPROG)
extern const char *mobprog_types[];
#endif
extern size_t room_bits_count;
extern size_t action_bits_count;
extern size_t affected_bits_count;
extern size_t extra_bits_count;
extern size_t wear_bits_count;
extern const char *patch_list[];
extern const char *AssemblyTypes[];
extern const char *creation_methods[];
extern const char *zone_bits[];
