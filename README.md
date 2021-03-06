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
 - Implement code for DC brushed motors control using L298N H Bridge. https://www.youtube.com/watch?v=dyjo_ggEtVU
 - Implement code for more powerful DC brushed motors using Cytron 13A DC Motor Driver MD10C. https://www.youtube.com/watch?v=Rc892r--njw&t=691s
 - Implement code for MPU6050.
