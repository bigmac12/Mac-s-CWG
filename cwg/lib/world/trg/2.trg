#200
Chunky Philosopher - 206~
0 b 5
~
eval max %random.9% -1
set  txt[0] I signed up for an exercise class and was told to wear loose-fitting clothing. Hell, if I HAD any loose-fitting clothing, I wouldn't  have signed up for the damn class in the first place!
set  txt[1] When I was young and trim we used to go "skinny dipping,"  now... I just "chunky dunkin."
set  txt[2] Remember... the early bird still has to eat worms.
set  txt[3] The worst thing about accidents in the kitchen is having to eat them anyway.
set  txt[4] Never argue with an idiot; people watching the two of you squabbling may not be able to tell who's who.
set  txt[5] Wouldn't it be nice if whenever we messed up our life we could simply press 'Delete' and then 'copy/paste' to do the really great parts again?
set  txt[6] Real stress is when you wake up screaming and then you realize  you haven't fallen asleep yet.
set  txt[7] my wife says I never listen to her. At least I think that's what she said.
set  speech %%txt[%max%]%%
set  txt[8]  Just remember...if the world didn't suck, we'd all fall off.
set  speech %%txt[%max%]%%
eval speech %speech%
say %speech%
~
#201
Demo trigger: heal/harm/cast spell when eating.~
1 c 2
eat~
return 0
wait 2
if %self.carried_by% == 0
  switch %self.vnum%
    case 201
      %damage% %actor% -10
      break 
    case 202
      %damage% %actor% 10
      %send% %actor% Ouch, that hurt a little!
      break
  done
end
~
#202
Free Trig~
0 b 100
~
if %mhunt_target.room% == %self.room%
  mkill %mhunt_target%
  %echo% killing %mhunt_target.name%.
else
  %echo% hunting %mhunt_target.name%
  mhunt %mhunt_target%
end
~
$~
