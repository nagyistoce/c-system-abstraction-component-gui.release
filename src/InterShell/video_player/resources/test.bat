@clrscr
@echo .
@echo Upload will happen on schedule....
:@"c:\program files\Synametrics Technologies\DeltaCopy\rsync.exe" -z -v -r /cygdrive/m/videos rsync://root@172.20.2.86/videos --partial --progress --inplace -a --delete
