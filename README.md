# SunTracker
## Sun Tracker for ESP8266 or ESP32 devices for automatic solar panel movement adjustment.
 - Requires geo location data (Latitude and Longitude coordinates) and Daylight Saving Time (DST) offset number for the location.
 - Based on https://github.com/G6EJD/ESP32_2D_Sun_Tracker 
 - Video link - https://www.youtube.com/watch?v=R1tFOd0s6uk&t=2s
 - Uses built in arduino libraries.
 - No photoresistors needed.
 - Tracker results verified at https://www.suncalc.org/
 - The National Renewable Energy Laboratory link to calculation details - https://www.nrel.gov/docs/fy08osti/34302.pdf

### To Do:

 - Implement code for inexpensive 28BYJ-48 ULN2003 5V Stepper Motor. https://www.youtube.com/watch?v=0qwrnUeSpYQ
 - Implement code for DC brushed motors control using RZ7886, BDR6133, TA6586 or BTS7960. https://alexgyver.ru/lessons/dc-motors/  && https://github.com/GyverLibs/GyverMotor && https://www.youtube.com/watch?v=tJYDV3FFCWQ&t=8s
 - implement VNH5019A-E driver. https://www.hobbytronics.co.uk/datasheets/pololu/vnh5019.pdf 
 - Implement code for MPU6050.
 - Implement  Builtin RTC time using https://github.com/fbiego/ESP32Time
