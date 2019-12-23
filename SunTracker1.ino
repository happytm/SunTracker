
 // More examples and docs see :
 // https://github.com/mcxiaoke/ESPDateTime
 // Based on https://github.com/G6EJD/ESP32_2D_Sun_Tracker
 

#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "ESPDateTime.h"
#include <Servo.h>   // https://github.com/jkb-git/ESP32Servo

unsigned long lastMs = 0;
unsigned long ms = millis();


// -------------- Configuration options -----------------


const char* ssid = "ssid";
const char* password = "password";
int Timezone = -5;
float Lon = -75.67 * DEG_TO_RAD,
      Lat = 43.39 * DEG_TO_RAD,
      elevation,
      azimuth;

#define elevation_servo 12
#define azimuth_servo   13

int sun_azimuth;
int sun_elevation;

String Hour, Minute, Day, Month, Year;


Servo Azi_servo;
Servo Ele_servo;


void setupDateTime() {
  
 // setup this after wifi connected
  // you can use custom timeZone,server and timeout
   
   DateTime.setTimeZone(Timezone);   // Eastern USA Time Zone (-5).
   DateTime.setServer("us.pool.ntp.org");
   DateTime.begin();  
  // DateTime.begin(15 * 1000);  // NTP time update frequency.
   
  
  if (!DateTime.isTimeValid()) {
   Serial.println("Failed to get time from server.");
  }

  delay(1000);
  
  Serial.println(DateTime.toString());
  Serial.println(DateTime.toISOString());
  Serial.println(DateTime.toUTCString());
  Serial.println("--------------------");
  Serial.println(DateTime.format(DateFormatter::COMPAT));
  Serial.println(DateTime.format(DateFormatter::DATE_ONLY));
  Serial.println(DateTime.format(DateFormatter::TIME_ONLY));
  Serial.println("--------------------");
  DateTimeParts p = DateTime.getParts();
  Serial.printf("%04d/%02d/%02d %02d:%02d:%02d %ld %+05d\n", p.getYear(),
                p.getMonth(), p.getMonthDay(), p.getHours(), p.getMinutes(),
                p.getSeconds(), p.getTime(), p.getTimeZone());

  Hour = (p.getHours());
  Minute = p.getMinutes();
  Day = p.getMonthDay();
  Month = p.getMonth();
  Year = p.getYear();
  Timezone = p.getTimeZone();
  
  Serial.println("--------------------");
  time_t t = DateTime.now();
  Serial.println(DateFormatter::format("%Y/%m/%d %H:%M:%S", t));
  Serial.println(DateFormatter::format("%x - %I:%M %p", t));
  Serial.println(DateFormatter::format("Now it's %F %I:%M%p.", t));
}


// --------- End of configuration section ---------------


void setup() {
  delay(1000);
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }

  Serial.println("\nConnected to network");
  setupDateTime();
 
  Azi_servo.attach(azimuth_servo);
  Ele_servo.attach(elevation_servo);
  test_azimuth();    // Bring panel to center
  delay(1000);
  test_elevation(); // Stop at 145 which is vertical or 0 elevation
  delay(1000);
}

void loop()
{
  if (millis() - ms > 5000) {
    ms = millis();
    Serial.println("--------------------");
   setupDateTime();
  }

  
  Calculate_Sun_Position(Hour.toInt(), Minute.toInt(), 0, Day.toInt(), Month.toInt(), Year.toInt());    // parameters are HH:MM:SS DD:MM:YY start from midnight and work out all 24 hour positions.
  
  Azi_servo.write(map(sun_azimuth, 90, 270, 180, 0));        // Align to azimuth
  if (sun_elevation < 0) sun_elevation = 0; // Point at horizon if less than horizon
  Ele_servo.write(145 - sun_elevation);  // map(value, fromLow, fromHigh, toLow, toHigh)
  
  delay(5000);  // to reduce upload failures
}


void Calculate_Sun_Position(int hour, int minute, int second, int day, int month, int year) {
  float T, JD_frac, JDate_frac, L0, M, e, C, L_true, f, R, GrHrAngle, Obl, RA, Decl, HrAngle;
  long JDate, JDx;
  int zone = 0;
  JDate      = JulianDate(year, month, day);
  JD_frac = (hour - (24+Timezone) + minute / 60.0 + second / 3600.0) / 24.0 - 0.5;
  T          = JDate - 2451545; T = (T + JDate_frac) / 36525.0;
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
  sun_azimuth   = azimuth   / DEG_TO_RAD;
  sun_elevation = elevation / DEG_TO_RAD;
  Serial.println();
  Serial.print("Azimuth: "); Serial.print(sun_azimuth); Serial.print("  Elevation: "); Serial.println(sun_elevation);
  Serial.println("Longitude and latitude " + String(Lon / DEG_TO_RAD, 3) + " " + String(Lat / DEG_TO_RAD, 3));
  Serial.println("Year\tMonth\tDay\tHour\tMinute\tElevation\tELServo\tAzimuth\tAZServo");
  Serial.print(String(year) + "\t" + String(month) + "\t" + String(day) + "\t" + String(hour - zone) + "\t" + String(minute) + "\t");
  Serial.println(String(elevation / DEG_TO_RAD, 0) + "\t\t" + (145 - sun_elevation) + "\t" + String(azimuth / DEG_TO_RAD, 0) + "\t" + (map(sun_azimuth, 90, 270, 180, 0)));
  Serial.println();}

long JulianDate(int year, int month, int day) {
  long JDate;
  int A, B;
  if (month <= 2) {year--; month += 12;}
  A = year / 100; B = 2 - A + A / 4;
  JDate = (long)(365.25 * (year + 4716)) + (int)(30.6001 * (month + 1)) + day + B - 1524;
  return JDate;
}

  void test_azimuth() {
  Azi_servo.write(90);  // Centre position
  delay(500);
  Azi_servo.write(60);  // Centre position
  delay(500);
  Azi_servo.write(120); // Centre position
  delay(500);
  Azi_servo.write(90);  // Centre position
  delay(500);
}

  void test_elevation() {
  for (int a = 5; a < 145; a = a + 2) {
    Ele_servo.write(a);  // Centre position
    delay(30);
  }
  Ele_servo.write(145);  // Centre position
  delay(1000);
}
