if %1==display goto wait

echo !update link_hall_state join location on hall_id=location.ID set task_launched=0 where packed_name='fortunet-pc' | sqlcmd vsrvr
goto done
:wait
echo !update link_hall_state join location on hall_id=location.ID set task_launched=0 where packed_name='fortunet-pc' | sqlcmd vsrvr
echo !update link_hall_state join location on hall_id=location.ID set participating=1 where packed_name='fortunet-pc' | sqlcmd vsrvr
pause
:done
