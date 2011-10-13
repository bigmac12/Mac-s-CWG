  :::::  DISCLAIMER AND GOOD ADVICE  :::::  DISCLAIMER AND GOOD ADVICE  :::::
  :::::                                                                 :::::
  ::::: Back up your code first!                                        :::::
  ::::: Then HAND PATCH the changes in.                                 :::::
  ::::: This code may wreck yours! Remember, I'm only human as well...  :::::
  :::::                                                                 :::::
  :::::  DISCLAIMER AND GOOD ADVICE  :::::  DISCLAIMER AND GOOD ADVICE  :::::

This patch will install

 - DG scripts 1.0.13

on your stock circle 3.1 with OasisOLC 2.0.6 and dg scripts 1.0.12

  To apply this patch If you disregard handpatch advice above,
  cd to your src/ dir and type
  
  patch < name_of_patch
  
  When you have patched (to update your Makefile):
  cd ..
  ./config.status
  cd src
  rm depend
  make

  If you have any problems with this, send me a mail at welcor@builderacademy.net
  
  
Welcor  
  
********        NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE    ***************
*                                                                          *
*   This version of DG Scripts supports player variables and having them   *
*   saved to disk. You MUST create, in the lib directory, the "plrvars"    *
*   sub-directory, and the various alphabetic sub-sub-directories. See     *
*   plrobjs for a list of these alphabetics.                               *
*                                                                          *
*   If you are new to DG Scripts, you must:                                *
*   1) mkdir ../lib/world/trg                                              *
*   2) mv 0.trg ../lib/world/trg                                           *
*   3) mv index ../lib/world/trg                                           *
*                                                                          *
*   You can create all necessary directories by running 
*                                                                          *
*                       ./maketrigdir                                      *
*                                                                          *
*   from your src/ dir.                                                    *
*                                                                          *
*   *****   FAILING TO DO THIS WILL RENDER YOUR MUD INOPERABLE!   *****    *
*                                                                          *
********        NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE   ****************

-----------------------------------------------------------------------
DOCUMENTATION:
   Aside from this readme file, you will find documentation online
   at http://welcor.n3.net
   
   Mark A. Heilperns pages are still available at
   http://www.mindspring.com/~heilpern/dg_scripts/

-----------------------------------------------------------------------
ToDo for pl1.0.14: 
    Allow players to have certain script types attached.
    Find and fix the remove room bug completely.             
    Known bug: Sometimes objects are created with UID = 0.
               This is still being investigated.
    Known bug: Script formatter doesn't like multiple case statements.
    The hash lookup table needs debugging. Apparently some items are not added.
    
-----------------------------------------------------------------------
New changes to 1.0.13
    Added findmob and findobj special variables:
      %findobj.<room vnum X>(<object vnum/id/name>)% 
        - count number of objects in room X with this name/id/vnum
      %findmob.<room vnum X>(<mob vnum Y>)% 
        - count number of mobs in room X with vnum Y
    Added subfield handling to vnum field. %some_var.vnum(XXX)% now 
      gives same result as (%some_var.vnum% == XXX) with less computing power.
      Thanks to Jamie Nelson (Mordecai of 4d) for the contributions above .
    Partial fix to script formatter regarding multiple case statements with no breaks.
    Got rid many of the 'Uid 0 not found in lookup table' logs. More to come on this one.
    Added subfields to int, str, con, cha, dex and wis fields. %actor.wis(2)% adds 2 to actors wisdom.
    Added new character field skillset. Syntax : %actor.skillset("skillname" new_value)% (ie. %self.skillset("magic missile" 95)%
    Added new(?) trigger type TIME. Thanks to Mark Garringer for reminding me of this one:
      Narg sets hour to trigger off from 0-23, 
      GLOBAL sets if it will work without players in the zone. 
      Variables: %time% equals the hour of running.
    Removed clumsy handling of gcc strict-aliasing. Thanks to Chris Gilbert for opening my eyes.
    Added ID field to 'stat' output.
    Removed redundant old var_subst code.
    Fixed small overflow bug in dg_read_trigger().
    
    
    General OLC related: buildwalk has been fixed. Apparently the former implementation
    of buildwalk messed with the pointers in memory, especially when dealing with triggers.
    This caused a lot of triggers with wait statements to crash the game when the room they
    were in were moved in memory. 
    
-----------------------------------------------------------------------
New changes to 1.0.12:
    Removed a lot (all?) of the more or less confusing '#if STOCK_CIRCLE' tags.
    Changed find_char() and find_obj() to use a hash table. Result is 250 times faster.
    Added union in dg_triggers.c to avoid typepunning warnings in gcc 3.3.0
    Added '[TRIG]' tag to all listings for people with roomflags on.
    Removed old erroneous checks for (!obj) after trigger calls.
    Added leave object trigger to go with th mob and room ones.
    Fixed crash bug related to removing rooms with wait commands.
    Fixed crash bug related to empty scripts in the editor.
    Added script formatting with /f when editing scripts (still incomplete).
    "find_end()-bug" seems to finally have found an end :P
    Scripts can now call detach with 'all' as argument.
    Larger example file 0.trg includes examples of a lot of the new features. 
      -- Thanks to Rumble of TBA for supplying it.
    Thanks to Jamie Nelson for these two additions:
      %variable.field.field_of_field% work (%actor.room.vnum%) now works
      %variable.field(subfield)% work with variables : %var.fi(%sub%)% now works
    Version numbering changed. Before: Version 0.99 pl 12 - Now: Version 1.0.12
    give/receive triggers are now checked when stealing as well.
    find_replacement() cleanup has been completed. Due to the nature of switch() it should be faster.
    Added several character fields:
      eq(*) subfield to characters to check for any eq.
      saving_spell, saving_para etc. fields for characters - with subfields for altering.
      prac for getting practices - subfield alters the count.
      has_item(name/vnum/var) - thanks to Jamie Nelson of 4 dimensions for this.
    Add %room.direction(room)% variable to get a room variable from an exit.
    Added %load% obj vnum [container | character [ wear_pos ] ] functionality
    Added Consume trigger type for objects. Eating and drinking can be blocked by returning 0.
    Plugged all memory leaks in the command list - Thanks to James Long for the assistance.
    
-----------------------------------------------------------------------
New changes to patch level 11:

    %purge% is safer - Now passes the calling thing by reference to script_driver().
    Fixed bug where %teleport%'ing in LEAVE triggers would cause a crash.
    Made 'set olc' more intuitive (set someone olc off)
    Context help now looks for a help file, if the entry in context_help starts with
         a * (ie. *"oedit main menu" calls 'help "oedit main menu"')
    Fixed some serious issues with freeing scripts, then dereferencing them.
         Now each entity has its own proto_script (no sharing).
    Added support for having tildes in texts - they are removed from the end of the
         line, though.
    Room ENTER scripts are now called which ever way you get to the room.
    Tlist now looks like the other oasis lists (and I just HAD to fix that mess ;[
    Fixed some issues with NULL room descs etc. causing crashes in strdup()
    
-----------------------------------------------------------------------
New changes in patch level 10:

    Focused on developing scripts, and removed a lot of the 'extra' olc stuff, 
      now that oasis is being developed again.
    Fixed the find_end() bug.
    Removed free_script() function.
    Fixed memory leak in command list.
    Fixed memory leak(s) when freeing scripts.
    Added more example triggers - courtesy of The Builder Academy
    Fixed some of the bugs related to %purge% in GIVE, WEAR, REMOVE, RECEIVE and DROP triggers.
    Added trigger info to 'show stat'
    - triggers are now stored by rnum in the internal tables - more consistent with the rest.
    Made variable assignments through zedit accept values of more than one word.
    Fixed crash bug in trig_data_init().
    Made %damage% accept negative amounts to heal instead of harm.
    Added new mfollow command to allow mobs to follow unnoticed.
    Added overflow protection throughout the dg files (sprintf/strcpy -> snprintf)
    Added support for %transform% as an alias for mtransform and otransform.
    Added support for subfields on room exit variables:
        %room.north%         - Exit bits          - eg. DOOR CLOSED
        %room.north(bits)%   - as above
        %room.north(vnum)%   - vnum of room north - eg. 1021 or ""
        %room.north(key)%    - vnum of key        - eg. -1 or 1203
    Added %room.weather% field: returns sunny, cloudy, rainy, lightning or "" if indoors.
    Fixed crash bugs related to excess 'done' statements.
    Made remote vars on players work again (added a script struct to all players in read_saved_vars)
    Made summon spells call the correct trigger hooks.
    
-----------------------------------------------------------------------
New changes in patch level 9: 

    Added 'file' command to show in-game the contents of some of the log files.
    Added context sensitive help system for all OLC menus.
      This is currently not finished as most of the help files need to be written.
      Contributions are accepted :)
      To make this help work, copy the file named 'contexthelp' to lib/text/
    Fixed (some) typos in demo triggers.
    Fixed bug causing "can't carry the weight" message to override triggers.
    Fixed bug causing ACT triggers not to work.
    Fixed bug causing logs like 'mob #-1 foo has unknown bit bar set', 
      by moving the iteration near the call to check_object in db.c
    Made script_log, wld_log, mob_log and obj_log use variadic args.
    Removed all global buffers from script files.
    Fixed bug in mpurge. Object vars can now be used as argument: %purge% %object%
    Made valid_dg_target usage more obvious - ALLOW_GODS macro instead of 'TRUE'
    Added oat command, thanks to PurpleOnyx.
    Made oat, wat and mat available as %at%.
    Fixed bug in trigedit causing crashes when given an empty script.
    Made the wizard 'attach' command accept 'object', 'mobile' or 'room' 
      for those who can't remember 'otr', 'mtr' and 'wtr'. Both work now.
    Fixed bug causing game to crash if set to long waitstates, and detached.
    Added %obj.room% field - gives roomvar of object.
    Added check to prevent PCs from having triggers attached through scripts.
    Fixed the notoriously faulty real_trigger() from pl8.
    Added file validity check in read_saved_vars to prevent crashes if no file.
    Fixed crash bug in script freeing in free_mobile()
    Made triggers set via zone file save correctly.
    
    
  General OLC enhancements:
  Note: The additions below are 'general' OLC enhancements. 
  I added them in my running code, for it to work as a fullscale builder port.
  When these items are available in an OasisOLC patch of their own, they will
  be removed from my patches. Until then, I guess this is OasisOLC 2.0.2, sortof.
    Added list commands for rooms/objs/mobs/shops/triggers
    Added dig/rclone/buildwalk commands
    Added more info (vnums) when ROOMFLAGS toggle is on.
    Fixed copy_ex_descriptions() and copy_room_strings() crash bugs when desc==NULL.
    Added can_edit_zone() function.
    Made create_new_zone save to disk right away.
    Fixed zedit !=NOWHERE -> ==NOWHERE bug 
    Added oedit 'abort quit' option.
    Fixed small OasisOLC bug - unaffect now removes all affects, including 
      flags from items, at least until next save.

-----------------------------------------------------------------------
New in patch level 8:
   
  Important Notice:
    I changed the return value of %actor.room% to be a UID value, returning the id
    of the room. More often than not, one might wish to attach a script, or simply
    need a UID var pointing to a specific room. The way this was set up, returning
    a room number, was unwieldy in these cases.

    This change means your scripts will need a little change, namely from using
    
    >     if %actor.room%==3001
    to
    >     eval roomvar %actor.room%
    >     if %roomvar.vnum%==3001
 
    And so on.   
    If you do not wish to use this, remove the definition of 
    ACTOR_ROOM_IS_UID in dg_scripts.h .
    
  The full DG_Events package V1.1 has been added. This shouldn't affect the feel
  or workings of the game, but will allow for easier expansion.
  Check out Ryan Biggs' pages at http://cmrsrc.8m.com/dgevent/ for more info.

  Made 'dg_affect' work as intended. Syntax is:
    dg_affect <target> <property> <value> <duration>   
    where
      target    is a UID var - like %actor%, %self%, or a name.
      property  is either an apply or an affection, by named bitvector.
                Applies are checked before affections.(STR before POISON)
      value     is the change. It can be both positive and negative.
      duration  is the number of ticks the affect will be working. Must be > 0.

  Added more variable fields:
    pc/npc:
      eq()  - expanded functionality to accept 'hold', 'wield' and 'light' as subfields. 
      affect() - to query for spell affect - usage: if %actor.affect(spellname)%
      hisher, himher, heshe - returns appropriate adjective/pronoun
      master - returns UID of master, blank if none.
      inventory() - with no subfield, returns UID of first item in inventory,
                    with subfield, returns UID of first item in inventory with that name,
                    blank if no inventory/no item with that name.
      
    object:
      cost, cost_per_day, weight - returns the appropriate value.
      next_in_list - for iterating through inventories and rooms.
        
    room:
      id - contains the UID of the room.
      
  Added %damage% command for all entities. Usage: %damage% <target> <amount> (negative to heal)
  Added %zoneecho% - usage: %zoneecho% <room vnum> <message>
  Added %asound% - usage: %asound% <message>
  
  Expanded makeuid functionality: 
    Usage: makeuid <varname> <idnum | < 'mob' | 'obj' | 'room' > < name > >
    where
      varname  is any name you wish to use to associate with the target
      idnum    is a ID num - %actor.id%, %room.id%, %self.id%
      name     is a name relating to the 'obj' or 'mob' - ignored if 'room'
      
  Made it easier to control who's targetted by scripts, by moving the controls to
  a seperate function, dg_valid_target, which then decides if a trigger may run,
  and if certain commands have any effect (%teleport%, %force%).
  
  Removed deprecated mgold and mexp commands. Usage is now - as of pl7 - 
  nop %actor.gold(change)%
  nop %actor.exp(change)%
  
-----------------------------------------------------------------------
Death's Gate Scripts:
   This patch is for the scripting system first found in the
   Death's Gate MUD, which is a distant derivative of the MobProg
   patch. The patch is for CircleMUD, currently for version 3.0bpl21.
   This is NOT a faithful reproduction of the original DG script
   system, however it will retain the name unless the original DG
   script designer requests otherwise. Read the code history, below,
   to see what has been added beyond the original implementation.
   Lines with new features are denoted with an asterix (*).

-----------------------------------------------------------------------

Changes that are not tagged with another author were most likely
implemented by Mark A. Heilpern of OmegaMUD. In all known situations,
the author or suggestor of a change is mentioned. If you suggested
something to me which I added but did not give you credit for it in here,
please contact me.

As of 2001 (pl 8), maintenance and development has been taken over by
Thomas Arp, Welcor of Cruelworld MUD. The rest still holds true.

Changes tagged with [RB] are to a great deal inspired by Ryan Biggs,
as can be seen on his pages at http://cmrsrc.8m.com/dgscript/
-----------------------------------------------------------------------

Implementors: A note on mtransform and otransform:
  Beginning with 0.99 pl5, mobiles and objects may change who they
  are via the aforementioned commands. The implementation of these
  loads the new mob/obj, copies it to a temporary buffer, copies
  necessary fields from the mob/obj doing the transformation into
  the temporary buffer, then copies the whole temporary buffer over the
  original space. This allows all stock pointers in the mud to still be
  valid. HOWEVER; if you have added fields to your structures, such
  as mounts, object damage, for example, and you want that sort of info
  to stay with the mob/obj you must copy the information over!

-----------------------------------------------------------------------

DG Code History:
Release     Date      What Happened
--------   --------   -------------

0.99 pl9    10/02   * Added 'file' command to show in-game the contents of some of the log files.
                    * Added context sensitive help system for all OLC menus.
                        This is currently not finished as most of the help files need to be written.
                        Contributions are accepted :)
                        To make this help work, copy the file named 'contexthelp' to lib/text/
                      Fixed (some) typos in demo triggers.
                      Fixed bug causing "can't carry the weight" message to override triggers.
                      Fixed bug causing ACT triggers not to work sometimes.
                      Fixed bug causing logs like 'mob #-1 foo has unknown bit bar set', 
                        by moving the iteration near the call to check_object in db.c
                      Made script_log, wld_log, mob_log and obj_log use variadic args.
                      Removed all global buffers from script files.
                    * Fixed bug in mpurge. Object vars can now be used as argument: %purge% %object%
                      Made valid_dg_target usage more obvious - ALLOW_GODS macro instead of 'TRUE'
                    * Added oat command, thanks to PurpleOnyx.
                    * Made oat, wat and mat available as %at%.
                      Fixed bug in trigedit causing crashes when given an empty script.
                      Made the wizard 'attach' command accept 'object', 'mobile' or 'room' 
                        for those who can't remember 'otr', 'mtr' and 'wtr'. Both work now.
                      Fixed bug causing game to crash if set to long waitstates, and detached.
                    * Added %obj.room% field - gives roomvar of object.
                      Added check to prevent PCs from having triggers attached through scripts.
                      Fixed the notoriously faulty real_trigger() from pl8.
                      Added file validity check in read_saved_vars to prevent crashes if no file.
                      Fixed crash bug in script freeing in free_mobile()
                      Made triggers set via zone file save correctly.
                  
0.99 pl8    7/02      Made dg_affect set the correct affections+show it in 'stat'.
                      Allowed negative damage to heal.
                      Fixed bug causing 'stat' on mobs with wait commands to crash.
                      Fixed triggers saving badly if they had no trigger types set.
                      Made '%purge% self' more stable. 
                      Added more examples.
                      Fixed Disconnect bug in olc.
                      Also tightened Valid_Name.
                      Added support for full dg events package
                      Moved string definitions (Xtrig_type[] arrays) to constants.c
                      Moved global definitions to db.c. 
                      Changed real_zone to use zone_[rv]num instead of room_[rv]num.
                      Cleaned up sub_write() and made it readable (?)
                      Made better error message when requesting nonexistant trigger.
                      Made errors show to online players by changing calls to log() to mudlog().
                    * Added full dg_events package. See events_readme for expansion possibilities.
                        You might wish to look at http://cmrsrc.8m.com/dgevent/ for more info.
                      Added send_pos() function to be used in do_Xdamage.
                      Added valid_dg_target() function.
                    * Added do_mdamage. 
                      Cleaned up mkill().
                    * Added do_mzoneecho.
                    * Added do_ozoneecho.
                      Fixed bugs related to NOWHERE, and changed some ->in_room to IN_ROOM().
                    * Fixed bug related to mistyped commands like oforceanyone.
                      Adjusted trigedit_save to work with zones at odd numbers.
                      Fixed some NOTHING issues
                      Moved DG_NO_TRIG to comm.h where it resides with the other act() options.
                      Added find_eq_pos_script() for %actor.eq()% subfield
                      Added get_char_in_room(), get_obj_near_obj(), get_char_near_obj(), get_obj_in_room() 
                    * Added %damage%, %zoneecho% and %asound% support
                      Fixed bug in "self.foo" causing self to point to the room forever.
                    * Added char field affect() - call method : if %actor.affect(armor)%
                    * Changed the output of %actor.room% to a room variable instead of a vnum.
               [RB] * Added char fields hisher, himher, heshe and master.
                    * Added char field inventory.
               [RB] * Added item fields cost, cost_per_day and weight
               [RB] * Added item field next_in_list.
                    * Added room field id
                    * Expanded makeuid functionality.
                      Made real_trigger() call faster
                      Cleaned up read_saved_vars().
                      Added MOB_ID_BASE and OBJ_ID_BASE to differentiate.
                      Removed the old MOBOBJ_ID_BASE.
                      Removed function delete_mob_memory() - incoorporated into greet_memory_mtrigger()
                      Added safety in COMMAND triggers.
               [RB] * Added mob/room triggers DOOR, CAST and LEAVE
               [RB] * Added obj trigger CAST
                    * Made room SPEECH triggers accept * as argument (wildcard for all)
                      Cleaned up do_wdamage
                      Fixed NOHASSLE issues with wforce+wteleport              
                      Made mobs stand up when calling death_mtrigger raw_kill() - all commands will then work.
                      Stabilized add_room by making sure people are in the right room afterwards.
                      Made add_room adjust scripts as well.
                      Removed old real_zone that was commented out already.
                      Removed level check on command triggers, since dg_valid_target checks for level, nohassle, etc.
                      Fixed bugs dealing with setting triggers and vars in the zone file. Thanks to Artovil (Torgny Bjers)
                      Fix loading of variables to correct context.
                    * Fixed crashbug in [r|o|m]edit_setup_existing() - it happened when choosing not to 
                        save after altering the items' script, and then reentering the editor.
                        Thanks to Jeremy Stanley (fungi@yuggoth.org) for the bug report. 
                        The bug resided in dg_script_edit_parse() and was freeing pointers to the prototype.

                      Reformatted the files to get the same 'feel' as the stock code.
                      Removed deprecated mgold+mexp commands.

0.99 pl7    5/99
                    * added "dg_affect" command
                      made "mjunk all" work better
                    * added %text.mudcommand(cmd)% boolean test
                    * extended actor.gold to allow a subfield that
                      has a value to add to the actor's gold
                    * added actor.exp to get (or set, ala gold) the
                      actor's experience points. MAX addition is 1000, as
                      hardcoded in dg_scripts.c
                    * added actor.is_thief and .is_killer
                    * added object carried_by and worn_by fields.
                    * gave players the ability to have variables that
                         scripts can set/read, and that save to a file
                         (if they don't start with '-')
                    * added rdelete script command to delete a variable
                         from a remote script. also added vdelete as a
                         command for imps.
                    * added %actor.varexists(varname)% boolean
                    * added flexibility for more generic commands
                      replaced dummy_mob checks with IS_NPC() (Andrey Fidrya)
                      fixed a bug in dg_olc.c (Angus Mezick)
                      fixed mob action triggers yet again
                         (the bug that wouldn't die!)
                      fixed a zedit bug where the switch case fell through
                         to global assignment
                      fixed trg world file save routine
                      fixed minor zedit-obj-load/otrigger error (tdubroff)
                      fixed log bugs in process_remote()
                      fixed the "no whitespace before while" bug
0.99 pl6   10/98
                    * added %actor.% hitp, maxhitp, move, maxmove, mana,
                        maxmana -- self-explanatory (Angus Mezick)
                      added more complete trigger checks (Doppleganger)
                    * added %actor.next_in_room%, %room.people%, and
                        %text.contains()%
                    * added dg_cast command
                    * added mdoor and odoor commands
                    * added %send%, %echo% and %echoaround%
                    * added trigger zone commands (MAH/Doppleganger)
                    * added script attach/detach commands
                      added context handling to add_var()
                      set OLC to properly create the trg index (Del Minturn)
                      removed unneeded free(cmd)
                      changed 'case' implementation to fix bug (Chris Jacobson)
                      fixed bug in %actor.eq()%
0.99 pl5a   8/10/98   fixed extract command
                      corrected mecho/mechoaround order in interpreter.c
                      fixed CHECK_PLAYER_SPECIALS cheesiness
                      added DG version info to 'version' mud command
                      removed gcc-dependent NULL printf functionality
                      finished context handling code :)
                      fixed a but in find_eq_pos() {stock bpl14} which
                          made actor.eq(pos) break
                      made mobs not able to trigger their own command or
                          action triggers
                    * added %actor.eq(position)%
0.99 pl5    8/5/98  * added 'otransform <vnum>' command
                    * added 'mtransform <vnum>' command
                    * added "actor global referencing" (Doppleganger)
                    * added .id field for chars and objects
                    * added 'makeuid <var> <id>' command
                    * gave objects a Remove trigger
                    * allowed '*' as 1st char in string arguments to
                          cause everything to match. (this is NOT a
                          general purpose wildcard)
                    * added mob Memory trigger
                    * added mremember and mforget commands
                    * added mhunt command
                    * add %actor.skill(skill name)% to return %
                          learnedness of the skill
                    * added global context
                    * added 'remote <varname> <id>' to let locally owned
                          variables be copied to the global list of other
                          script owners
                    * added %text.car% and %text.cdr%
                    * added %text.strlen%, %text.trim%
                    * (Doppleganger): added script 'extract' command
                          to pull text out of a larger field. usage:
                          extract <to-variable> <word-num> <from-variable>
                    * added 'version' command
                      extended script status info to include wait info
                      fixed mgold (missing from interpreter.c)
                      cleaned up some compiler warnings
                      made greet triggers parse all mobs in the room,
                          instead of stopping with the first hit.
                          (thanks, Doppleganger)
                      fixed bug with mob/obj purging itself and the
                          script able to continue execution
                      fixed the .weight field bug (reported by Angus Mezick)
                      fixed reset_wtrigger() bug (Belgarath)
                      made CHECK_PLAYER_SPECIALS not log in dg source files
0.99 pl4    7/10/98   ported to patch against circle bpl14
                    * added actor "fighting", "riding", and "ridden_by"
                        fields (riding/ridden_by are conditional)
                      cured olc bug creating scripts on new obj/rooms
                    * added otimer cmd, %object.timer%, and timer trigger
                      added %actor.weight%
                    * added mob and object "load" triggers
                      gave %cmd% to command triggers
                      changed "this" to "this_data" to be C++ friendly
                      fixed rnum/vnum issue on attach (Belgarath)
                    * added %self.vnum% for rooms
                    * added the zone-reset room trigger
                      made command-triggers require a full match on the
                          command, making "d" allow "down" instead of
                          triggering a "drink" command, e.g.
                    * added str/stradd/int/wis/dex/con/cha to %actor%
                      oforce made to accept multiple arguments
0.99 pl3    2/15/98   added olc-cleanup to trigedit lost links
                      reversed the ordering in this file :P
                      made 'force mob say text' not trigger its own
                          speech trigger
                      same for action triggers
                      fixed #define for trigedit's SCMD, thanks to
                          James Hadden
                    * added switch and while to triggers, thanks to
                          Chris Jacobson
                      fixed room.fields bug found by Andy Hubbard
                      fixed is_num() to recognize neg nums (thanks George)
                      fixed the perform_act bug again :(
                      put in arg pointer validation/error logging
                      prototyped parse_trigger() in db.c
                      made more friendly to MSVC (thanks to
                          Andrey Fidrya and Francis Hotchkiss)
                      fixed an olc bug with lost script assignments
                      fixed a process_event-remove_event bug
0.99 pl2     1/15/98  Event endless loop repaired
                      act triggers made to work
                      speech triggers made to work
                    * speech phrases implemented
                    * %speech% var given to speech triggers
                      log message fix in do_msend()
                      fixed mechoaround to not send to target
                    * added %actor.align% for alignment
                      changed %actor.room% to a room vnum, not rnum
                      changed %actor.vnum% to a vnum, not rnum
                      changed %obj.vnum% to a vnum, not rnum
                      made msend/mechoaround/mat/mteleport work well
                          with %actor%
                    * gave %direction% to room entry triggers
                      made mteleport work as documented
                      made command triggers work on existing commands
0.99 pl1      1/6/98  Oasis save routines repaired
0.99          1/5/98  Initial C patch released
