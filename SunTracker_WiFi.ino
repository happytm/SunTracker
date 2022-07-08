#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "ESPDateTime.h"  

unsigned long lastMs = 0;
unsigned long ms = millis();

const char* ssid = "HAPPYHOME";
const char* password = "kb1henna";

int Timezone = -4;      // Adjust according to your time zone.

float Lon = -75.67 * DEG_TO_RAD,
      Lat = 43.39 * DEG_TO_RAD,
      elevation,
      azimuth;

int sun_azimuth;
int sun_elevation;

// --------- End of configuration section ---------------

void setup() {
  Serial.begin(115200);
  delay(500);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }

  Serial.println("\nConnected to network");
  setupDateTime();
}

void loop()
{
  if (millis() - ms > 5000) {
    ms = millis();
  
   setupDateTime();
  }

  DateTimeParts p = DateTime.getParts();
  
  Calculate_Sun_Position(p.getHours(), p.getMinutes(), 0, p.getMonthDay(), (p.getMonth() + 1), p.getYear());    // parameters are HH:MM:SS DD:MM:YY start from midnight and work out all 24 hour positions.
  Serial.println("Longitude and latitude " + String(Lon / DEG_TO_RAD, 3) + " & " + String(Lat / DEG_TO_RAD, 3));
  Serial.print("Sun Azimuth: "); Serial.println(sun_azimuth); Serial.print("Sun Elevation: "); Serial.println(sun_elevation); 
  
  Serial.print("Pan servo position:  "); Serial.println(map(sun_azimuth, 90, 270, 180, 0));                    // Align to azimuth
  if (sun_elevation < 0) sun_elevation = 0;                                                                   // Point at horizon if less than horizon
  Serial.print("Tilt servo position: "); Serial.println(145 - sun_elevation); Serial.println();                               // map(value, fromLow, fromHigh, toLow, toHigh)
  
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
  elevation  = asin(sin(Lat) * sin(Decl) + cos(Lat) * (cos(Decl) * cos(HrAngle)));
  azimuth    = PI + atan2(sin(HrAngle), cos(HrAngle) * sin(Lat) - tan(Decl) * cos(Lat)); // Azimuth measured east from north, so 0 degrees is North
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

void setupDateTime() {
  
  DateTime.setTimeZone(Timezone);   // Eastern USA Time Zone (-5).
  DateTime.setServer("us.pool.ntp.org");
  DateTime.begin();  
   
  if (!DateTime.isTimeValid()) {Serial.println("Failed to get time from server.");}
  delay(1000);
  Serial.print("Current time: ");Serial.println(DateTime.toString());
}
