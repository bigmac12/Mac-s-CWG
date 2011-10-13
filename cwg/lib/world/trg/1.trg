#100
Obj Command 100 - portal to Midgaard~
1 c 7
en~
if %cmd.mudcommand% == enter && %arg% /= portal
  %send% %actor% You enter the portal.
  %echoaround% %actor% %actor.name% bravely enters the portal.
  %teleport% %actor% 3001
  %force% %actor% look
  %echoaround% %actor% %actor.name% just stepped through a portal.
else
  %send% %actor% %cmd% what?!
end
~
#101
Room Command - portal to Midgaard~
2 c 100
en~
if %cmd.mudcommand% == enter && portal /= %arg%
  %send% %actor% You enter the portal.
  %echoaround% %actor% %actor.name% bravely enters the portal.
  %teleport% %actor% 3001
  %force% %actor% look
  %echoaround% %actor% %actor.name% just stepped through a portal.
else
  %send% %actor% enter what?!
end
~
#102
Mob Command - portal to Midgaard~
0 c 100
en~
if %cmd.mudcommand% == enter && portal /= %arg%
  %send% %actor% You enter the portal.
  %echoaround% %actor% %actor.name% bravely enters the portal.
  %teleport% %actor% 3001
  %force% %actor% look
  %echoaround% %actor% %actor.name% just stepped through a portal.
else
  %send% %actor% enter what?!
end
~
#103
Mob Greet Newbie Guide - 196~
0 g 100
~
if %actor.is_pc%
  if %actor.level% <= 3
    wait 1 sec
    bow
    wait 1 sec
    say may I suggest you visit the newbie zones under HELP AREAS.
  end
  if %actor.level% >= 30
    wait 1 sec
    bow %actor.name%
  end
end
~
#104
Mob Speech Speaker of the Land - 156~
0 d 100
entered~
wait 1 sec
gos All Welcome %actor.name% to our Realm!
~
#105
Mob Greet Hannibal - 140~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  if %actor.sex% == male
    say Good day sir, what would you like? 
  elseif %actor.sex% == female
    wait 1 sec
    say Good day maam, what can I get you?
  else
    say What do you want?
  end
end
~
#106
Mob Greet Carpenter - 197~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  if %actor.sex% == male
    say Can't you see the place is under repairs!
    wait 1 sec
    say don't worry, the inn will be open again soon.
  elseif %actor.sex% == female
    wait 1 sec
    say come to work have you?
    wait 1 sec
    wink %actor.name%
  else
    frown %actor.name%
  end
end
~
#107
Mob Greet Shiro - 103~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  say I sell the finest weapons in all the realm. See for yourself.
end
~
#108
Mob Greet Rhian - 104~
0 g 100
~
if %actor.is_pc%
  wait 3 sec
  if %actor.sex% == male
    smile %actor.name%
  elseif %actor.sex% == female
    wait 1 sec
    frown %actor.name%
  else
    say I hate your kind.
  end
end
~
#109
Mob Greet Sarge - 109~
0 g 100
~
if %actor.is_pc%
  look %actor.name%
  wait 1 sec
  if %actor.sex% == male
    say See anything you like?
  elseif %actor.sex% == female
    wait 1
    gaze %actor.name%
    wait 1
    say What can I get you pretty lady?
  else
    say What do you want?
  end
end
~
#110
Mob Greet Logan - 110~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  grin %actor.name%
  wait 1 sec
  if %actor.sex% == male
    say Look at this fine vest.
    wait 1 sec
    emote holds up a gaudy red vest.
    wait 1 sec
    say this would wear well on you.
  elseif %actor.sex% == female
    say for you my young lady, I have a fine silk shirt.
  else
    say What do you want?
  end
end
~
#111
Mob Greet Branwen - 111~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  if %actor.sex% == male
    say if it's made from leather, I have it.
  elseif %actor.sex% == female
    wait 1 sec
    say the finest leather in the realms is what I sell.
  else
    say What do you want?
  end
end
~
#112
Mob Greet Morgan - 184~
0 g 33
~
if %actor.is_pc%
  wait 1 sec
  sigh
  wait 1 sec
  if %actor.sex% == male
    say need a drink. I sure do
    wait 1 sec
    emote downs a shot of whisky.
  elseif %actor.sex% == female
    wait 1 sec
    say can I get you a drink.
    wait 1 sec
    ogle %actor.name%
  else
    say What do you want?
  end
end
~
#113
Mob Greet Ingrid - 182~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  cackle %actor.name%
  wait 1 sec
  if %actor.sex% == male
    say ahhh, doesn't that smell good.
  elseif %actor.sex% == female
    wait 1 sec
    say what can I get you, my pretty.
  else
    say what would you like?
  end
end
~
#114
Mob Greet Corwin - 110~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  emote frowns at a large stack of mail.
end
~
#115
Mob Greet Banker - 119~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  emote puts all his money in the safe when he notices you eyeing it.
end
~
#116
Mob Greet Hazel - 109~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  chuckle
  wait 1 sec
  if %actor.sex% == male
    say thirsty?
  elseif %actor.sex% == female
    wait 1 sec
    say I sell pure water, no worry about contaminants from me.
  else
    say need some water?
  end
end
~
#117
Mob Greet Carla - 158~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  emote steps away from her sewing machine.
  wait 1 sec
  if %actor.sex% == male
    say anything I can help you with sir?
  elseif %actor.sex% == female
    wait 1 sec
    say I could make something nice for a woman like you.
  else
    say need some clothes?
  end
end
~
#118
Mob Greet Ian - 101~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  if %actor.sex% == male
    say I make furs for the common man, because that is what I am.
  elseif %actor.sex% == female
    wait 1 sec
    say a fine fur coat would suit you well.
  else
    say need some fur?
  end
end
~
#119
Mob Greet Liam - 119~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  if %actor.sex% == male
    say Take your time, i've got all day.
  elseif %actor.sex% == female
    wait 1 sec
    say you aren't really an adventurer are you?
    wait 1 sec
    say who woulda thought a woman adventuring.
  else
    say need some supplies?
  end
end
~
#120
Mob Greet Baker - 187~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  say best bread in town.
  wait 1 sec      
  if %actor.sex% == male
    emote slams some dough down onto the counter.
  elseif %actor.sex% == female
    wait 1 sec
    say I'm hiring if you can cook.
  else
    say need some food?
  end
end
~
#121
Mob Greet Butcher - 199~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  grin
  wait 1 sec
  say I can butcher anything.
  wait 1 sec
  if %actor.sex% == male
    emote splatters some blood on you as he hacks at some meat.
  elseif %actor.sex% == female
    wait 1 sec
    say sorry about the mess mam.
  else
    emote rubs his bloody hands on his apron.
  end
end
~
#122
Mob Greet Rowan - 111~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  smile
  wait 1 sec
  say see anything you would like.
  wait 1 sec
  if %actor.sex% == male
    emote watches you carefully.
  elseif %actor.sex% == female
    wait 1 sec
    say isn't this diamond beautiful.
  else
    emote points you to the display cases.
  end
end
~
#123
Mob Greet Fiona - 124~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  glare
  wait 1 sec
  if %actor.sex% == male
    say much better food here than the baker or butcher sells.
    wait 1 sec
    whisper %actor.name% I hear the butcher's meat is tainted.
  elseif %actor.sex% == female
    wait 1 sec
    say hungry?
  else
    say hungry?
  end
end
~
#124
Mob Greet Lugdach - 106~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  if %actor.sex% == male
    say Aye! What can I getcha!
  elseif %actor.sex% == female
    wait 1 sec
    say What's a fine lass like you doing here?
  else
    emote need a boat?
  end
end
~
#125
Mob Greet Healer - 186~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  smile %actor.name%
  wait 1 sec
  if %actor.sex% == male
    say What aid do you need?
  elseif %actor.sex% == female
    wait 1 sec
    say Can I help you miss?
  else
    say What do you want?
  end
end
~
#126
Mob Greet Sareth - 185~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  bow
  wait 1 sec
  if %actor.sex% == male
    say Are you in need of a scroll?
  elseif %actor.sex% == female
    wait 1 sec
    say I have many types of scrolls
  else
    say What do you want?
  end
end
~
#127
Mob Act - 156 speaker greet~
0 e 0
has entered the game.~
eval inroom %self.room%
%zoneecho% %inroom.vnum% %self.name% shouts, 'Welcome, %actor.name%!'
~
#128
Mob Act - 156 speaker goodbye~
0 e 0
has left the game.~
eval inroom %self.room%
%zoneecho% %inroom.vnum% %self.name% shouts, 'Farewell, %actor.name%!'
~
#129
Mob Greet Beggar - 165~
0 g 100
~
if %actor.is_pc%
  wait 1 sec
  say Money for the poor?
end
~
#130
Mob Bribe Beggar - 165~
0 m 1
~
wait 1 sec
if %actor.sex% == MALE
  say Thank you, kind sir.
elseif %actor.sex% == FEMALE
  say Thank you, ma'am.
else
  emote looks you over trying to determine your sex.
  say Thank you.....
end
~
#131
Room Command 365 - Jump~
2 c 100
jump~
wait 1 sec
%send% %actor% You jump from the window ledge to certain death.
%echoaround% %actor% %actor.name% decides to test fate and takes a dive out the window.
%teleport% %actor% 292
wait 1 sec
%echoaround% %actor% %actor.name% falls from above screaming %actor.hisher% lungs out. %actor.heshe% hits the ground with a loud thump.
%force% %actor% look
%send% %actor% You strike the ground hard but somehow manage to survive the impact.
%damage% %actor% 1
~
#132
dg_cast by level~
0 k 100
~
switch %actor.level%
  case 1
  case 2
  case 3
    dg_cast 'magic missile' %actor%
  break
  case 5
    dg_cast 'chill touch' %actor%
  break
  case 6
    dg_cast 'burning hands' %actor%
  break
  case 7
    dg_cast 'shocking grasp' %actor%
  break
  case 8
    dg_cast 'sleep' %actor%
  break
  case 9
    dg_cast 'lightning bolt' %actor%
  break
  case 10
    dg_cast 'blindness' %actor%
  break
  case 11
    dg_cast 'color spray' %actor%
  break
  case 12
    dg_cast 'lightning bolt' %actor%
  break
  case 13
    dg_cast 'energy drain' %actor%
  break
  case 14
    dg_cast 'curse' %actor%
  break
  case 15
    dg_cast 'poison' %actor%
  break
  case 16
    if %actor.align% > 0
      dg_cast 'dispel good' %actor%
    else
      dg_cast 'dispel evil' %actor%
    end
 break
  case 17
    dg_cast 'call lightning' %actor%
  break
  case 18
  case 19
    dg_cast 'harm' %actor%
  break
  default
    dg_cast 'fireball' %actor%
  break
done
~
#133
Warrior Guildguard - 127~
0 q 100
~
* Check the direction the player must go to enter the guild.
if %direction% == up
  * Stop them if they are not the appropriate class.
  if %actor.class% != warrior
    return 0
    %send% %actor% The guard humiliates you, and blocks your way.
    %echoaround% %actor% The guard humiliates %actor.name%, and blocks %actor.hisher% way.
  end
end
~
#134
Mage Guildguard - 173~
0 q 100
~
* Check the direction the player must go to enter the guild.
if %direction% == up
  * Stop them if they are not the appropriate class.
  if %actor.class% != magic user
    return 0
    %send% %actor% The guard humiliates you, and blocks your way.
    %echoaround% %actor% The guard humiliates %actor.name%, and blocks %actor.hisher% way.
  end
end
~
#135
Cleric Guildguard - 174~
0 q 100
~
* Check the direction the player must go to enter the guild.
if %direction% == up
  * Stop them if they are not the appropriate class.
  if %actor.class% != cleric
    return 0
    %send% %actor% The guard humiliates you, and blocks your way.
    %echoaround% %actor% The guard humiliates %actor.name%, and blocks %actor.hisher% way.
  end
end
~
#136
Thief Guildguard - 177~
0 q 100
~
* Check the direction the player must go to enter the guild.
if %direction% == up
  if %actor.vnum% == 122
    halt
  end
  * Stop them if they are not the appropriate class.
  if %actor.class% != thief
    return 0
    %send% %actor% The guard humiliates you, and blocks your way.
    %echoaround% %actor% The guard humiliates %actor.name%, and blocks %actor.hisher% way.
  end
end
~
#137
Thief Guildmaster Steal - 122~
0 b 5
~
* Idea taken from cheesymud.com
* Thief guildmaster steals from actor inventory and pawns it in the shop
* downstairs, player then has to buy their equipment back.
set actor %random.char%
if %actor%
  eval item %actor.inventory%
  eval item_to_steal %%actor.inventory(%item.vnum%)%%
  if %item_to_steal%
    %echo% %self.name% examines %item.shortdesc%.
    wait 2 sec
    eval stolen %item_to_steal.vnum%
    eval name %item_to_steal.name%
    %load% obj %stolen%
    %purge% %item_to_steal% 
    wait 2 sec
    down
    sell %name.car%
    wait 2 sec
    whisper morgan hehe, silly mortals.
    wait 2 sec
    up
  else
    emote grumbles unhappily.
  end
end
~
#138
Questmaster Greet - 3~
0 g 100
~
if %actor.is_pc% 
  if !%actor.varexists(questpoints)%
    set questpoints 0
    remote questpoints %actor.id%
    elsif %actor.questpoints% < 50
    wait 2 sec
    if !%actor.varexists(on_quest_zone_1)%
      say Welcome %actor.name%, are you interested in a simple quest?
    else
      say How is your quest going %actor.name%, do you have a quest token or the quest mobs head for me?
    end
  end
end
~
#139
Questmaster Quest Assignment - 3~
0 d 1
*~
* Don't let them do more than one quest at a time or complete more than 50.
if %actor.varexists(on_quest_zone_1)% || %actor.questpoints% > 50
  halt
end
eval word %speech.car%
eval rest %speech.cdr%
while %word%
  if yes /= %word%
    say Very well %actor.name%. Would you like to find an object or hunt a mobile?
    halt
  end
  eval loadroom 100 + %random.265%  
  if mobile /= %word% || hunt /= %word%
    %at% %loadroom% %load% m 15
    say Go kill the quest mob and bring me its head %actor.name%. You only have 10 minutes!
    %load% obj 16 %actor% inv
    %send% %actor% Gives you the quest timer.
    %echoaround% %actor% %self.name% gives %actor.name% the quest timer.
    set on_quest_zone_1 1
    remote on_quest_zone_1 %actor.id%
    halt
  elseif object /= %word% || find /= %word%
    say Go find the quest token and return it to me. You only have 10 minutes %actor.name%!
    %load% o 15
    %at% %loadroom% drop quest
    %load% obj 16 %actor% inv
    %send% %actor% Gives you the quest timer.
    %echoaround% %actor% %self.name% gives %actor.name% the quest timer.
    set on_quest_zone_1 1
    remote on_quest_zone_1 %actor.id%
    halt
  end
  eval word %rest.car%
  eval rest %rest.cdr%
done
~
#140
Quest Timer - 16~
1 c 7
l~
if %cmd.mudcommand% == look && timer /= %arg%
  return 0
  %send% %actor% You have %self.timer% minutes remaining.
else
  return 0
end
~
#141
Quest 10 min Purge - 15, 16, 17~
1 f 100
~
* Attached to quest objects 15-17. Purges itself 10 minutes after loading if player does not
* finish the quest. Part of a quest from room 113.
if %self.carried_by%
  set actor %self.carried_by%
  if %actor.is_pc%
    %send% %actor% Your quest time has run out. Try again.
    rdelete on_quest_zone_1 %actor.id%   
  end
end
%purge% %self%
~
#142
Quest Timer Random - 16~
1 b 10
~
if %self.carried_by%
  set actor %self.carried_by%
  %send% %actor% You have %self.timer% minutes remaining.
end
~
#143
Questmaster Receive - 3~
0 j 100
~
if !%actor.varexists(on_quest_zone_1)%
  say You are not even on a quest %actor.name%.
  halt
end
*
wait 1 sec
if %object.vnum% == 15 || %object.vnum% == 17
  say Well done, %actor.name%.
  nop %actor.exp(50)%
  nop %actor.gold(50)%
  rdelete on_quest_zone_1 %actor.id%   
  %purge% %object%
  if %actor.varexists(questpoints)%
    eval questpoints %actor.questpoints% + 1
    remote questpoints %actor.id%
  else
    set questpoints 1
    remote questpoints %actor.id%
  end
  say you have earned 50 gold, 50, experience points and 1 questpoint.
else
  say I don't want that!
  %purge% %object%
end
~
#144
Quest Mob Loads Head - 15~
0 n 100
~
*This is a load instead of a death trig because I want the head to purge 10 minutes after loading.
%load% obj 17
~
#145
Dove - 193~
0 b 5
~
eval max %random.2%
set  txt[1] pecks at your feet.
set  txt[2] coo's lightly.
set  speech %%txt[%max%]%%
eval speech %speech%
emote %speech%
~
#146
Apprentice healer - 201~
0 b 20
~
* This is required because a random trig does not have an actor.
set actor %random.char%
* only continue if an actor is defined.
if %actor%
  * if they have lost more than half their hitpoints heal em
  if %actor.hitp% < %actor.maxhitp% / 2 
    wait 1 sec
    say You are injured, let me help.
    wait 2 sec
    %echoaround% %actor% %self.name% lays %self.hisher% hands on %actor.name%'s wounds and bows %actor.hisher% head in concentration.
    %send% %actor% %self.name% lays %self.hisher% hands on your wounds and bows %actor.hisher% head in concentration.
    dg_cast 'heal' %actor%
  end
end
~
#147
Black Magi Spell - 144~
0 k 10
~
if %actor.level% > 10
  say you are a fool %actor.name%.
  dg_cast 'color spray' %actor%
end
~
#148
Mouse Emote - 194~
0 b 5
~
eval max %random.4%
set  txt[1] scurries away quickly.
set  txt[2] stands up on its hind legs and sniffs the air.
set  txt[3] chews on some trash.
set  txt[4] squeaks and shakes some water and rain out of its fur.
set  speech %%txt[%max%]%%
eval speech %speech%
emote %speech%
~
#149
Cat Emote - 139~
0 b 10
~
eval max %random.4%
set  txt[1] hisses at you.
set  txt[2] purrs happily as it brushes up against your leg.
set  txt[3] plays with something it has already killed.
set  txt[4] swishes its tail back and forth as it eyes some prey.
set  speech %%txt[%max%]%%
eval speech %speech%
emote %speech%
~
#150
Dog Emote - 192~
0 b 10
~
eval max %random.4%
set  txt[1] sniffs at you friendlily.
set  txt[2] whimpers for some attention.
set  txt[3] growls menacingly at your feet.
set  txt[4] watches your every movement suspiciously.
set  speech %%txt[%max%]%%
eval speech %speech%
emote %speech%
~
#151
Townsman Emote - 170~
0 b 10
~
eval max %random.4%
set  txt[1] mumbles something about the weather.
set  txt[2] looks up at the sky warily.
set  txt[3] seems to have forgotten where he was headed.
set  txt[4] acknowledges you with a nod as he passes.
set  speech %%txt[%max%]%%
eval speech %speech%
emote %speech%
~
#152
Angel Receives Treats - 207~
0 j 100
~
if %object.vnum% == 164
  wait 1 sec
  emote swallows %object.shortdesc% without even chewing.
  %purge% %object%
  wait 1 sec
  emote looks up at %actor.name%, hoping for some more.
  wait 1 sec
  mfollow %actor%  
else
  if %object.type% == FOOD
    emote swallows %object.shortdesc% without even chewing.
    %purge% %object%
  else
    drop %object
  end
end
~
#153
Angel Follows Masters Commands - 207~
0 d 1
*~
if %self.master% == %actor%
  wait 1 sec
  switch %speech.car%
    case sit
      sit
      wait 3 sec
      stand
    break
    case speak
      emote barks sharply.
    break
    case down
      sit
      emote lays down.  
      wait 3 sec
      stand
    break
    case shake
      emote puts a paw up to be shook.
    break
    case kill
      if %speech.cdr.id% && %self.room% == %speech.cdr.room%
        emote growls at %speech.cdr.name% menacingly.
        mkill %speech.cdr%
      else
        emote looks around for someone to attack.  
      end
    break
    case rollover
      emote drops to the ground and rolls over a few times.
    break
    case walk
      emote stands up on her hind legs and staggers around in circles
    break
    case crawl
      emote drops down to the ground and crawls towards %actor.name%.
    break
    case jump
      emote jumps up into the air.
    break
    case chase
      if %speech.cdr% == your tail
        emote looks back at %self.hisher% tail angrily and attacks it, running in tight little circles.
      end
    break
    case highfive
      emote jumps up and gives %actor.name% a highfive.
    break
    default
      * nothing is going to happen
    break
  done
end
~
#154
rubber chicken squeeze - 172~
1 c 2
sq~
if squeeze /= %cmd% && chicken /= %arg%
  %send% %actor% You squeeze on the chicken and it squeaks annoying, how fun!
  %echoaround% %actor% %actor.name% squeezes the life out of %actor.hisher% rubber chicken making a racket with its squeaking.
  %asound% A loud and annoying squeaking sound can be heard close by.
else
  %send %actor% What would you like to squeeze?
  return 0
end
~
#155
Check for treats - 207~
0 g 100
~
if %actor.has_item(164)%
  wait 1 sec
  emote sits down and stands up on %self.hisher% hind legs, then starts whining pitifully staring at %actor.name%.
elseif %actor.has_item(172)%
  emote sniff %actor.name%
  wait 1 sec
  growl %actor.name%
  emote tries to get at something you are carrying.
end
~
#156
Angel plays with chicken - 207~
0 b 100
~
if %self.has_item(172)%
  growl chicken
  squeeze chicken
end
~
#157
10 sided die roll - 173~
1 c 7
roll~
if dice /= %arg% || die /= %arg%
  %send% %actor% You throw the ten sided die on the ground.
  %echoaround% %actor% %actor.name% throws %actor.hisher% ten sided die on the ground.
  set total %random.10%
  %echo% The die came up as [%total%].
else
  return 0
end
~
#158
Bell Toll - 101~
2 c 100
pull~
if rope /= %arg%
  %echoaround% %actor% %actor.name% struggles as %actor.heshe% pulls on the rope.
  %send% %actor% You pull the rope putting all your weight into it. It slowly gives.
  %zoneecho% %self.vnum% The bell tolls.
else
  %echo% I don't see what you want to "pull". The rope perhaps?
end
~
#159
Cancer Stick Smoking - 176~
1 c 2
light~
* put your objects alias here, /= will match abbreviations of it.
if cigarette /= %arg% || cancer /= %arg% || stick /= %arg%
  %send% %actor% You light up a %self.name%.
  %echoaround% %actor% %actor.name% lights up a %self.name%.
  * use as many puffs and as much time between puffs as you want.
  while %puffs% < 4
    wait 10 sec
    %send% %actor% You take a puff off of your %self.name%.
    %echoaround% %actor% %actor.name% takes a puff of smoke off of %actor.hisher% %self.name%.
    eval puffs %puffs% + 1
  done
  %send% %actor% You take a final puff and put the %self.name% out.
  %echoaround% %actor% %actor.name% takes a final puff and puts %actor.hisher% %self.name% out.
  %purge% %self%
else
  %send% %actor% What would you like to %cmd%?
end
~
#160
Puppy plays - 191~
0 g 100
~
if %actor.vnum% == 207
  wait 1 sec 
  emote growls playfully at %actor.name%, crouching down into a mock attack position.
elseif %actor.is_pc%
  wait 1 sec
  if %actor.dex% < 8
    %send% %actor% %self.name% runs under your feet causing you to fall flat on your face.
    %echoaround% %actor% %self.name% runs under %actor.name%'s feet and trips %actor.himher%.
  elseif %actor.cha% < 8
    %send% %actor% %self.name% sniffs your leg and begins to urinate on you.
    %echoaround% %actor% %self.name% sniffs %actor.name%'s leg and proceeds to urinate on %actor.himher%.
  end
end
~
#161
Annoying Kid - 117~
0 b 5
~
eval max %random.4%
set  txt[1] I know you are, but what am I?
set  txt[2] Does your parents know you are out in public?
set  txt[3] And I thought I knew ugly.
set  txt[4] I'm going to tell my father on you.
set  speech %%txt[%max%]%%
eval speech %speech%
say %speech%
~
#162
Picking Mushrooms~
2 c 100
pi~
* does not work for level 32 and above.
* Make sure the command matches MUD command pick, check for any abbrev of mushrooms.
if %cmd.mudcommand% == pick && mushrooms /= %arg%
  %load% obj 1300 %actor% inv
  %send% %actor% You pick a mushroom off the floor.
  %echoaround% %actor% %actor.name% Picks a mushroom off the floor.
else
  * return 0, otherwise players would not be able to pick locks in the same room.
  return 0
  %send% %actor% Pick What?
end
~
#163
Room Heals - 101~
2 b 100
~
* This is required because a random trig does not have an actor.
set actor %random.char%
* only continue if an actor is defined.
if %actor.is_pc%
  * check if they are hurt.
  if %actor.hitp% < %actor.maxhitp% 
    * heal them their level in hitpoints.
    %damage% %actor% -%actor.level%
  end
end
~
#164
Beggin Strips - 164~
1 s 100
~
%send% %actor% You gag in disgust at the foul taste of the dog treats.
%echoaround% %actor% %actor.name% gags in disgust at the foul taste of the dog treats.
return 0
%purge% %self%
~
#165
Thief - 129, 183~
0 b 10
~
set actor %random.char%
if %actor%
  if %actor.is_pc% && %actor.gold%
    %send% %actor% You discover that %self.name% has %self.hisher% hands in your wallet.
    %echoaround% %actor% %self.name% tries to steal gold from %actor.name%.
    eval coins %actor.gold% * %random.10% / 100
    nop %actor.gold(-%coins%)
    nop %self.gold(%coins%)
  end
end
~
#166
Mob Affect - 135~
0 g 10
~
switch %random.12%
  case 1
    if %actor.affect(blind)%
      dg_cast 'cure blind' %actor%
    end
  break
  case 2
    if !%actor.affect(invis)%
      dg_cast 'invisibility' %actor%
    end
  break
  case 3
    if !%actor.affect(det-align)%
      dg_cast 'detect align' %actor%
    end
  break
  case 4
    if !%actor.affect(det-magic)%
      dg_cast 'detect magic' %actor%
    end
  break
  case 5
    if !%actor.affect(sense-life)%
      dg_cast 'sense life' %actor%
    end
  break
  case 6
    if !%actor.affect(watwalk)%
      dg_cast 'waterwalk' %actor%
    end
  break
  case 7
    if !%actor.affect(sanct)%
      dg_cast 'sanctuary' %actor%
    end
  break
  case 8
    if %actor.affect(curse)%
      dg_cast 'remove curse' %actor%
    end
  break
  case 9
    if !%actor.affect(infra)%
      dg_cast 'infravision' %actor%
    end
  break
  case 10
    if %actor.affect(poison)%
      dg_cast 'cure poison' %actor%
    end
  break
  case 11
    if !%actor.affect(bless)%
      dg_cast 'bless' %actor%
    end
  break
  case 12
    if !%actor.affect(prot-evil)%
      dg_cast 'protection from evil' %actor%
    end
  break
  default
    say whoops!
  break
done
~
#167
Mob Questshop Example~
0 c 100
*~
if %cmd.mudcommand% == list
  *
  %send% %actor%  ##   Available   Item                                Cost in Questpoints
  %send% %actor% -------------------------------------------------------------------------
  %send% %actor%   1)  Unlimited   War's Blood                                         100
  %send% %actor%   2)  Unlimited   shadow stealer                                      100
  %send% %actor%   3)  Unlimited   the staff of spellfire                              100
  *
elseif %cmd.mudcommand% == buy
  if !%actor.varexists(questpoints)% 
    tell %actor.name% You have no questpoints!
    halt
  end
  if War /= %arg% || Blood /= %arg% || %arg% == 1
    set quest_item 21
    set quest_item_cost 100
  elseif shadow /= %arg% || stealer /= %arg% || %arg% == 2
    set quest_item 22
    set quest_item_cost 100
  elseif staff /= %arg% || spellfire /= %arg% || %arg% == 3
    set quest_item 23
    set quest_item_cost 100
  else
    tell %actor.name% What would you like to buy?
    halt
  end
  *
  if %actor.questpoints% < %quest_item_cost% 
    tell %actor.name% You don't have enough questpoints for that.
  else
    %load% obj %quest_item% %actor% inv
    tell %actor.name% here you go.
    eval questpoints %actor.questpoints% - %quest_item_cost%
    remote questpoints %actor.id%
  end
elseif %cmd.mudcommand% == sell
  tell %actor.name% I don't want anything you have.
else
  return 0
end
~
#168
Questpoint Setter - 44~
1 c 1
questpoints~
* Questpoint setter. For STAFF only! Make sure player has nohassle off.
* Make sure name matches a player, purge mobs or use 0.name if you have 
* troubles. 
* Usage: questpoints <player> <#>
if !%actor.is_pc% || %actor.level% < 32
  %send% %actor% Only human staff can use this.
else  
  set victim %arg.car%
  if %victim.is_pc%
    set questpoints %arg.cdr%
    remote questpoints %victim.id%
    %send% %actor% %arg.car%'s questpoints set to %arg.cdr%.
  else
    %send% %actor% Don't use it on mobs. Use 0.<name>!
    return 0
  end
end
~
#169
Quest Token check if player is on quest - 15~
1 g 100
~
if !%actor.varexists(on_quest_zone_1)%
  %send% %actor% You are not on a quest, don't steal other peoples quest items!
end
~
$~
