#include <WiFi.h>
#include "ESPDateTime.h"    // Thanks to : https://github.com/mcxiaoke/ESPDateTime

RTC_DATA_ATTR int bootCount; 
RTC_DATA_ATTR int sun_azimuth, sun_elevation, panPosition, tiltPosition, sunriseAzimuth, sunriseSaved;
RTC_DATA_ATTR int sleepMinutes = 1;      // Adjust sleep time in minutes if needed. 15-30 minutes are ideal sleep time to conserve battery and at the same time relatively frequent movement of tracker.

const char* ssid =     "YourSSID";
const char* password = "YourPassword";

int Timezone = -4;      // Adjust according to your time zone.

float Lon = -72.71 * DEG_TO_RAD,
      Lat = 43.40 * DEG_TO_RAD;
     
// --------- End of configuration section ---------------

void setup() {
  Serial.begin(115200); delay(500);
  WiFi.mode(WIFI_STA);WiFi.begin(ssid, password);Serial.println("\nConnecting to WiFi"); while (WiFi.status() != WL_CONNECTED) {delay(500);Serial.print(".");} Serial.println("\nConnected to network");
  long upTime = bootCount * sleepMinutes;
  Serial.print("Up time "); Serial.print(upTime); Serial.println(" minutes.");
  ++bootCount;
  
  Serial.print("Last pan position was:   "); Serial.println(panPosition);
  Serial.print("Last tilt position was: ");  Serial.println(tiltPosition);
  //ToDo using https://github.com/ruenahcmohr/EGMK :
  //Serial.print("Now we are going to move to new pan postion target using : ");  Serial.print(pan_μs_CW); Serial.print("pan micro seconds forward and "); Serial.print(pan__μs_CW); Serial.println("pan micro seconds reverse."); 
  //Serial.print("Now we are going to move to new tilt postion target using : ");  Serial.print(tilt_μs_CW); Serial.print("tilt micro seconds forward and "); Serial.print(tilt_μs_CCW); Serial.println("tilt micro seconds reverse."); 

      
}

void loop() {

  DateTime.setTimeZone(Timezone);   // Eastern USA Time Zone (-5).
  DateTime.setServer("us.pool.ntp.org");
  DateTime.begin();  
  DateTimeParts p = DateTime.getParts();
  
  int lastPanPosition = panPosition; int lastTiltPosition = tiltPosition;
  Calculate_Sun_Position(p.getHours(), p.getMinutes(), 0, p.getMonthDay(), (p.getMonth() + 1), p.getYear());  // parameters are HH:MM:SS DD:MM:YYYY start from midnight and work out all 24 hour positions.
  
  if (sunriseSaved == 0 && sun_elevation > 0) {sunriseAzimuth = sun_azimuth; sunriseSaved = 1;}               // Save azimuth at sunrise in RTC memory once a day after sunrise to bring tracker back at this pan position for the next day's start point.
  if (sunriseSaved == 1 && sun_elevation < 0) {sunriseSaved = 0; /*Bring tracker back to sunrise azimuth position in the evening to lock it up during the night to save it from heavy winds*/} 
  if (sun_azimuth > sunriseAzimuth)  {panPosition =  sun_azimuth - sunriseAzimuth;}                           // Set pan position in RTC memory.
  if (sun_elevation > 0) {tiltPosition = sun_elevation;}                                                      // Set tilt position in RTC memory.
  int panMove = panPosition - lastPanPosition; int tiltMove = tiltPosition - lastTiltPosition;                // Find out the movement of both motors in degrees.
   
  Serial.print("Current Date is: "); Serial.printf("%d/%02d/%02d \n", (p.getMonth() + 1), p.getMonthDay(), p.getYear());
  Serial.print("Current time is: "); Serial.printf("%d:%02d:%02d \n", p.getHours(), p.getMinutes(), p.getSeconds());
  Serial.print("Longitude: "); Serial.println(String(Lon / DEG_TO_RAD, 3)); Serial.print("Latitude:   ");Serial.println(String(Lat / DEG_TO_RAD, 3));
  Serial.print("Sun Azimuth:   "); Serial.println(sun_azimuth); Serial.print("Sun Elevation: "); Serial.println(sun_elevation); 
  Serial.print("Sunrise Azimuth: "); Serial.println(sunriseAzimuth);        // Set sunrise as 0 (start position).
  Serial.print("Pan position:  "); Serial.println(panPosition);                     // Value ranges between (0 - 300).
  Serial.print("Tilt position: "); Serial.println(tiltPosition); Serial.println();  // When panel is vertical this value is 0.value ranges between (0 - 90).
  Serial.print("Moved pan position by degrees:   "); Serial.println(panMove);       // Value ranges between (0 - 300).
  Serial.print("Moved tilt position by degrees:  "); Serial.println(tiltMove);      // When panel is vertical this value is 0.value ranges between (0 - 90).
  
  esp_sleep_enable_timer_wakeup(sleepMinutes * 60000000); // 60000000 for 1 minute.
  esp_deep_sleep_start();  
}


void Calculate_Sun_Position(int hour, int minute, int second, int day, int month, int year) {
  float T, JD_frac, L0, M, e, C, L_true, f, R, GrHrAngle, Obl, RA, Decl, HrAngle;
  long JDate, JDx;
  int zone = 0;
  JDate      = JulianDate(year, month, day);
  JD_frac = (hour - (24+Timezone) + minute / 60.0 + second / 3600.0) / 24.0 - 0.5;
  T          = JDate - 2451545; T = (T + JD_frac) / 36525.0;
  L0         = DEG_TO_RAD * fmod(280.46645 + 36000.76983 * T, 360);
  M          = DEG_TO_RAD * fmod(357.5291 + 35999.0503 * T, 360);
  e          = 0.016708617 - 0.000042037 * T;
  C          = DEG_TO_RAD * ((1.9146 - 0.004847 * T) * sin(M) + (0.019993 - 0.000101 * T) * sin(2 * M) + 0.00029 * sin(3 * M));
  f          = M + C;
  Obl        = DEG_TO_RAD * (23 + 26 / 60.0 + 21.448 / 3600. - 46.815 / 3600 * T);
  JDx        = JDate - 2451545;
  GrHrAngle  = 280.46061837 + (360 * JDx) % 360 + 0.98564736629 * JDx + 360.98564736629 * JD_frac;
  GrHrAngle  = fmod(GrHrAngle, 360.0);
  L_true     = fmod(C + L0, 2 * PI);
  R          = 1.000001018 * (1 - e * e) / (1 + e * cos(f));
  RA         = atan2(sin(L_true) * cos(Obl), cos(L_true));
  Decl       = asin(sin(Obl) * sin(L_true));
  HrAngle    = DEG_TO_RAD * GrHrAngle + Lon - RA;
  float elevation  = asin(sin(Lat) * sin(Decl) + cos(Lat) * (cos(Decl) * cos(HrAngle)));
  float azimuth    = PI + atan2(sin(HrAngle), cos(HrAngle) * sin(Lat) - tan(Decl) * cos(Lat)); // Azimuth measured east from north, so 0 degrees is North
  sun_azimuth   = (azimuth   / DEG_TO_RAD);
  sun_elevation = elevation / DEG_TO_RAD;
  
}

long JulianDate(int year, int month, int day) {
  long JDate;
  int A, B;
  if (month <= 2) {year--; month += 12;}
  A = year / 100; B = 2 - A + A / 4;
  JDate = (long)(365.25 * (year + 4716)) + (int)(30.6001 * (month + 1)) + day + B - 1524;
  return JDate;
}
