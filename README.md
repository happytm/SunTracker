# SunTracker
## Sun Tracker using ESP32 for automatic dual axis solar tracker or heliostat.
 
 - Requires only geo location data (Latitude and Longitude coordinates) and Daylight Saving Time (DST) offset number for the location.
 
 - Based on:
   - https://github.com/G6EJD/ESP32_2D_Sun_Tracker (for sun position) https://www.youtube.com/watch?v=R1tFOd0s6uk&t=2s
   - https://github.com/ruenahcmohr/EGMK (for motor movement)
 
 - Uses built in arduino libraries.
 - No photoresistors or accelerometer needed.
 - Tracker results verified at https://www.suncalc.org/
 - The National Renewable Energy Laboratory link to calculation details - https://www.nrel.gov/docs/fy08osti/34302.pdf

### To Do:

 - Implement code for inexpensive modified 28BYJ-48 12V Stepper Motor for prototype.
 
 - Implement code for DC brushed motors control using RZ7899, BDR6133, TA6586 or BTS7960.
   - https://alexgyver.ru/lessons/dc-motors/
   - https://github.com/GyverLibs/GyverMotor
   - https://www.youtube.com/watch?v=tJYDV3FFCWQ&t=8s
 
 - https://www.youtube.com/watch?v=yt_4_t4DEqo
 - https://www.youtube.com/watch?v=uVJXskpkuEE 
 - http://faq.solarbotics.net/suneater/suneater.html
