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
 - Implement code for DC brushed motors control using TA6586 or BTS7960. https://alexgyver.ru/lessons/dc-motors/  && https://github.com/GyverLibs/GyverMotor && https://www.youtube.com/watch?v=tJYDV3FFCWQ&t=8s
 - implement VNH5019A-E driver. https://www.hobbytronics.co.uk/datasheets/pololu/vnh5019.pdf && https://de.aliexpress.com/item/1005003223186789.html?src=google&src=google&albch=shopping&acnt=708-803-3821&slnk=&plac=&mtctp=&albbt=Google_7_shopping&albagn=888888&isSmbAutoCall=false&needSmbHouyi=false&albcp=9594035441&albag=102695258807&trgt=313429380675&crea=en1005003223186789&netw=u&device=c&albpg=313429380675&albpd=en1005003223186789&gclid=Cj0KCQiAys2MBhDOARIsAFf1D1e_SyeNa_EwzV6kDSRn9fB1b3Mw9UzwBvi3GRLe-xzTz5kQPqZ_XJAaAg8IEALw_wcB&gclsrc=aw.ds&aff_fcid=d13ac66e049a4b32a52f7891f6fb39c3-1637103819300-00133-UneMJZVf&aff_fsk=UneMJZVf&aff_platform=aaf&sk=UneMJZVf&aff_trace_key=d13ac66e049a4b32a52f7891f6fb39c3-1637103819300-00133-UneMJZVf&terminal_id=41d5c1ecad064e6384007703d0fb05d9 using https://github.com/bmellink/VNH3SP30.
 - Implement code for MPU6050.
 - Implement  Builtin RTC time using https://github.com/fbiego/ESP32Time
