#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "ESPDateTime.h"    // Thanks to : https://github.com/mcxiaoke/ESPDateTime

const char* ssid =     "";
const char* password = "";

int Timezone = -4;      // Adjust according to your time zone.

float Lon = -75.67 * DEG_TO_RAD,
      Lat = 43.39 * DEG_TO_RAD;
     

int sun_azimuth, sun_elevation, sunriseAzimuth;
bool sunriseSaved;

// --------- End of configuration section ---------------

void setup() {
  Serial.begin(115200); delay(500);
  WiFi.mode(WIFI_STA);WiFi.begin(ssid, password);Serial.println("\nConnecting to WiFi"); while (WiFi.status() != WL_CONNECTED) {delay(500);Serial.print(".");} Serial.println("\nConnected to network");
}

void loop() {

  DateTime.setTimeZone(Timezone);   // Eastern USA Time Zone (-5).
  DateTime.setServer("us.pool.ntp.org");
  DateTime.begin();  
  DateTimeParts p = DateTime.getParts();
  
  Calculate_Sun_Position(p.getHours(), p.getMinutes(), 0, p.getMonthDay(), (p.getMonth() + 1), p.getYear());    // parameters are HH:MM:SS DD:MM:YY start from midnight and work out all 24 hour positions.
  
  if (sunriseSaved = false && sun_elevation > 0) {sunriseAzimuth = sun_azimuth; sunriseSaved = true;} // Record azimuth once a day after sunrise to bring tracker back at this point for the next day's start point.
  if (sunriseSaved = true && sun_elevation < 0) {sunriseSaved = false; /*Bring tracker back to sunrise azimuth position in the evening to lock it up during the night to save it from heavy winds*/} 
  
  Serial.print("Current time: "); Serial.println(DateTime.toString());
  Serial.print("Longitude: "); Serial.println(String(Lon / DEG_TO_RAD, 3)); Serial.print("Latitude: ");Serial.println(String(Lat / DEG_TO_RAD, 3));
  Serial.print("Sun Azimuth: "); Serial.println(sun_azimuth); Serial.print("Sun Elevation: "); Serial.println(sun_elevation); 
  Serial.print("Start Pan position:  "); Serial.println(sunriseAzimuth);                             // Set sunrise as 0 (start position).
  Serial.print("Pan position:  "); Serial.println(map(sun_azimuth, sunriseAzimuth, 300, 0, 300));    // Align to azimuth. map(value, fromLow, fromHigh, toLow, toHigh).value ranges between (0 - 300).
  Serial.print("Tilt position: "); Serial.println(sun_elevation); Serial.println();                  // When panel is vertical this value is 0.value ranges between (0 - 90).
  
  delay(5000);  
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
