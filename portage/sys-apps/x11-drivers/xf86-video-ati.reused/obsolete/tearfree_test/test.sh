sleep 1
xset dpms force suspend #can replace "suspend" with "off" or "standby", it should have the same effect!
read -p "press Enter to continue"
sleep 5 #few seconds delay is needed before the log messages appear:
grep -i 'flip.*busy' -- "${HOME}/.local/share/xorg/Xorg.0.log"
#if you see "present flip failed" and "flip queue failed in radeon_scanout_flip: Device or resource busy, TearFree inactive until next modeset" then tearfree is disabled; apply patch 4reversed_05390ae36a0abb19f1d533ff95a3fceaafdf79c8.patch to "fix" and restart X (eg. logout then startx again)
#Sometimes it takes two(or more?) dpms suspends to see the above TearFree message! but you can always still see the "present flip failed" one. With the patch you should see none of the above two, but you'll still see the other two which are: "flip queue failed: Device or resource busy" and "Page flip failed: Device or resource busy", sometimes repeated in 3 groups.


