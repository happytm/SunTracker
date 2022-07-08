// Minimal NTP Time Demo with DST correction
//
// Uses built-in ESP8266 LWIP sntp library to get time
// Based on https://github.com/G6EJD/ESP32_2D_Sun_Tracker

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <simpleDSTadjust.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif
#include <time.h>    
#include <Servo.h>   // https://github.com/jkb-git/ESP32Servo


// -------------- Configuration options -----------------

const char* ssid = "ssid";          // your ssid
const char* password = "password";  // your password

float Lon = -75.67 * DEG_TO_RAD,    // your lon data - get it from https://www.suncalc.org/
      Lat = 43.39 * DEG_TO_RAD,     // your lat data - get it from https://www.suncalc.org/
      elevation,
      azimuth;

#define elevation_servo 12
#define azimuth_servo   13

int sun_azimuth;
int sun_elevation;


// Update time from NTP server every 5 hours
//#define NTP_UPDATE_INTERVAL_SEC 5*3600
#define NTP_UPDATE_INTERVAL_SEC 5*1

// Maximum of 3 servers
#define NTP_SERVERS "us.pool.ntp.org", "pool.ntp.org", "time.nist.gov"

// Daylight Saving Time (DST) rule configuration
// Rules work for most contries that observe DST - see https://en.wikipedia.org/wiki/Daylight_saving_time_by_country for details and exceptions
// See http://www.timeanddate.com/time/zones/ for standard abbreviations and additional information
// Caution: DST rules may change in the future
#if 1
//US Eastern Time Zone (New York, Boston)
#define Timezone -5 // US Eastern Time Zone
struct dstRule StartRule = {"EDT", Second, Sun, Mar, 2, 3600};    // Daylight time = UTC/GMT -4 hours
struct dstRule EndRule = {"EST", First, Sun, Nov, 2, 0};       // Standard time = UTC/GMT -5 hour
#else
//Australia Eastern Time Zone (Sydney)
#define Timezone +10 // Australian Eastern Time Zone
struct dstRule dstStartRule = {"AEDT", First, Sun, Oct, 2, 3600};    // Daylight time = UTC/GMT +11 hours
struct dstRule dstEndRule = {"AEST", First, Sun, Apr, 2, 0};      // Standard time = UTC/GMT +10 hour
#endif



// --------- End of configuration section ---------------

Ticker ticker1;
int32_t tick;

String Hour, Minute, Day, Month, Year;
int Hour24;
Servo Azi_servo;
Servo Ele_servo;

// flag changed in the ticker function to start NTP Update
bool readyForNtpUpdate = false;

// Setup simpleDSTadjust Library rules
simpleDSTadjust dstAdjusted(StartRule, EndRule);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nConnected to network");
  updateNTP(); // Init the NTP time
  printTime(0); // print initial time time now.

  tick = NTP_UPDATE_INTERVAL_SEC; // Init the NTP update countdown ticker
  ticker1.attach(1, secTicker); // Run a 1 second interval Ticker
  Serial.print("Next NTP Update: ");
  printTime(tick);
  Azi_servo.attach(azimuth_servo);
  Ele_servo.attach(elevation_servo);
  test_azimuth();    // Bring panel to center
  delay(1000);
  test_elevation(); // Stop at 145 which is vertical or 0 elevation
  delay(1000);
}
void loop()
{
  if(readyForNtpUpdate)
   {
    readyForNtpUpdate = false;
    printTime(0);
    updateNTP();
    Serial.print("\nUpdated time from NTP Server: ");
    printTime(0);
    Serial.print("Next NTP Update: ");
    printTime(tick);

  }

  Calculate_Sun_Position(Hour.toInt(), Minute.toInt(), 0, Day.toInt(), Month.toInt(), Year.toInt());    // parameters are HH:MM:SS DD:MM:YY start from midnight and work out all 24 hour positions.
  
  Azi_servo.write(map(sun_azimuth, 90, 270, 180, 0));        // Align to azimuth
  if (sun_elevation < 0) sun_elevation = 0; // Point at horizon if less than horizon
  Ele_servo.write(145 - sun_elevation);  // map(value, fromLow, fromHigh, toLow, toHigh)
  delay(5000);  // to reduce upload failures
}

void Calculate_Sun_Position(int hour, int minute, int second, int day, int month, int year) {
  float T, JD_frac, L0, M, e, C, L_true, f, R, GrHrAngle, Obl, RA, Decl, HrAngle;
  long JDate, JDx;
  int zone = 0;
  JDate      = JulianDate(year, month, day);
  JD_frac = (Hour24 - (24+Timezone) + minute / 60.0 + second / 3600.0) / 24.0 - 0.5;
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
  sun_azimuth   = azimuth   / DEG_TO_RAD;
  sun_elevation = elevation / DEG_TO_RAD;
  Serial.println(); Serial.print("Elevation: "); Serial.print(sun_elevation); Serial.print("  Azimuth: "); Serial.println(sun_azimuth); Serial.println();
  Serial.println("Longitude and latitude " + String(Lon / DEG_TO_RAD, 3) + " " + String(Lat / DEG_TO_RAD, 3));
  Serial.println("Year\tMonth\tDay\tHour\tMinute\tElevation\tELServo\tAzimuth\tAZServo");
  Serial.print(String(year) + "\t" + String(month) + "\t" + String(day) + "\t" + String(hour - zone) + "\t" + String(minute) + "\t");
  Serial.println(String(elevation / DEG_TO_RAD, 0) + "\t\t" + (145 - sun_elevation) + "\t" + String(azimuth / DEG_TO_RAD, 0) + "\t" + (map(sun_azimuth, 90, 270, 180, 0)));

}

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

//----------------------- Time Functions -------------------------------


// NTP timer update ticker
void secTicker()
{
  tick--;
  if(tick<=0)
   {
    readyForNtpUpdate = true;
    tick= NTP_UPDATE_INTERVAL_SEC; // Re-arm
   }

  // printTime(0);  // Uncomment if you want to see time printed every second
}


void updateNTP() {
  
  configTime(Timezone * 3600, 0, NTP_SERVERS);

  delay(500);
  while (!time(nullptr)) {
    Serial.print("#");
    delay(1000);
  }
}


void printTime(time_t offset)
{
  char buf[30];
  char *dstAbbrev;
  time_t t = dstAdjusted.time(&dstAbbrev)+offset;
  struct tm *timeinfo = localtime (&t);
  
  int hour = (timeinfo->tm_hour+11)%12+1;  // take care of noon and midnight
  sprintf(buf, "%02d/%02d/%04d %02d:%02d:%02d%s %s\n",timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_year+1900, hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_hour>=12?"pm":"am", dstAbbrev);
  Hour = hour;
  Hour24 = timeinfo->tm_hour;
  Minute = timeinfo->tm_min;
  Day = (timeinfo->tm_mday);
  Month = (timeinfo->tm_mon+1);
  Year = (timeinfo->tm_year+1900);
  Serial.print(buf);
  Serial.println();
  Serial.print("Current Hour: "); 
  Serial.println(timeinfo->tm_hour);
}
