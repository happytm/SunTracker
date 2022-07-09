#include <sys/time.h>

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int sleepMinutes = 1;      // Adjust sleep time in minutes if needed.
RTC_DATA_ATTR int sun_azimuth, sun_elevation, panPosition, tiltPosition, sunriseAzimuth, sunriseSaved;

int timezone = 4;                 // Use 4 or 5 for eastern USA.
float Lon = -72.71 * DEG_TO_RAD,  // Enter your Longtitude. Use www.suncalc.org.
      Lat = 43.40 * DEG_TO_RAD;   // Enter your Longtitude. Use www.suncalc.org.
     
struct tm getTimeStruct()
{
  struct tm timeinfo;
  getLocalTime(&timeinfo, 0);
  return timeinfo;
}

// --------- End of configuration section ---------------

void setup() {

  Serial.begin(115200); delay(500);
  
  Serial.printf("Bootcount = %d\n", bootCount);
  if (!bootCount)
  {
    struct tm timeinfo = getTimeStruct();
    timeinfo.tm_hour = 5;  // Enter hour in 24 hour format.
    timeinfo.tm_min = 31;
    timeinfo.tm_sec = 00;
    timeinfo.tm_mon = 6;    // Enter Month - 1. January is 0 & December is 11.
    timeinfo.tm_mday = 9; 
    timeinfo.tm_year = 2022 - 1900;
    struct timeval tv;
    tv.tv_sec = mktime(&timeinfo);
    settimeofday(&tv, NULL);
  }
  ++bootCount;
  
  Serial.print("Last pan position was:  ");   Serial.println(panPosition);
  Serial.print("Last tilt position was: ");  Serial.println(tiltPosition);
  
} // End of setup.

void loop() {
  int lastPanPosition = panPosition; int lastTiltPosition = tiltPosition;
  struct tm timeinfo = getTimeStruct();
  Calculate_Sun_Position(timeinfo.tm_hour + timezone, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);    // parameters are HH:MM:SS DD:MM:YYYY start from midnight and work out all 24 hour positions.
  if (sunriseSaved == 0 && sun_elevation > 0) {sunriseAzimuth = sun_azimuth; sunriseSaved = 1;}     // Save azimuth at sunrise in RTC memory once a day after sunrise to bring tracker back at this pan position for the next day's start point.
  if (sunriseSaved == 1 && sun_elevation < 0) {sunriseSaved = 0; /*Bring tracker back to sunrise azimuth position in the evening to lock it up during the night to save it from heavy winds*/} 
  if (sun_azimuth > sunriseAzimuth)  {panPosition =  sun_azimuth - sunriseAzimuth;}                // Set pan position in RTC memory.
  if (sun_elevation > 0) {tiltPosition = sun_elevation;}                                           // Set tilt position in RTC memory.
  int panMove = panPosition - lastPanPosition; int tiltMove = tiltPosition - lastTiltPosition;     // Find out the movement of both motors in degrees.
  
  Serial.print("Current Date is: "); Serial.printf("%d/%02d/%02d \n", timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_year + 1900);
  Serial.print("Current time is: "); Serial.printf("%d:%02d:%02d \n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  Serial.print("Longitude: "); Serial.println(String(Lon / DEG_TO_RAD, 3)); Serial.print("Latitude:   ");Serial.println(String(Lat / DEG_TO_RAD, 3));
  Serial.print("Sun Azimuth:   "); Serial.println(sun_azimuth); Serial.print("Sun Elevation: "); Serial.println(sun_elevation); 
  Serial.print("Sunrise Azimuth: "); Serial.println(sunriseAzimuth);        // Set sunrise as 0 (start position).
  Serial.print("Pan position:  "); Serial.println(panPosition);                     // Value ranges between (0 - 300).
  Serial.print("Tilt position: "); Serial.println(tiltPosition); Serial.println();  // When panel is vertical this value is 0.value ranges between (0 - 90).
  Serial.print("Moved pan position by degrees:   "); Serial.println(panMove);       // Value ranges between (0 - 300).
  Serial.print("Moved tilt position by degrees:  "); Serial.println(tiltMove);      // When panel is vertical this value is 0.value ranges between (0 - 90).
 
  esp_sleep_enable_timer_wakeup(sleepMinutes * 60000000); // 60000000 for 1 minute.
  esp_deep_sleep_start();  
} // End of loop.


void Calculate_Sun_Position(int hour, int minute, int second, int day, int month, int year) {
  float T, JD_frac, L0, M, e, C, L_true, f, R, GrHrAngle, Obl, RA, Decl, HrAngle;
  long JDate, JDx;
  
  JDate      = JulianDate(year, month, day);
  JD_frac    = (hour + minute / 60.0 + second / 3600.0) / 24.0 - 0.5;
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
  sun_azimuth   = azimuth   / DEG_TO_RAD;
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
