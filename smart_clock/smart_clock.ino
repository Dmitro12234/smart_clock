#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <time.h>

#if defined(ESP32)
  #include <WiFi.h>
  #define SDA_PIN 21
  #define SCL_PIN 22
  #define DHTPIN  4
#endif

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
const char* NTP_SERVER = "pool.ntp.org";
// Ukraine: automatic transition to summer/winter time
const char* TZ_INFO = "EET-2EEST,M3.5.0/3,M10.5.0/4";

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define DHTTYPE DHT11

const unsigned long SCREEN_TIME = 5000;  
const int ANIM_STEPS = 16;               
const int ANIM_DELAY = 8;                

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);

bool oledOk = false;
float hum = 0, temp = 0;
int currentScreen = 0;                 
unsigned long lastSwitch = 0, lastDht = 0, lastOledRetry = 0;

const char* DAYS[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

uint8_t findOled() {
  const uint8_t addrs[] = {0x3C, 0x3D};
  for (uint8_t a : addrs) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) return a;
  }
  return 0;
}

bool initOled() {
  uint8_t addr = findOled();
  if (addr == 0) {
    Serial.println(F("OLED not found: Check SDA/SCL, Power, Soldering"));
    return false;
  }
  Serial.print(F("OLED found, address 0x"));
  Serial.println(addr, HEX);
  if (!display.begin(SSD1306_SWITCHCAPVCC, addr)) {
    Serial.println(F("SSD1306 begin() failed"));
    return false;
  }
  display.clearDisplay();
  display.display();
  return true;
}

void drawTimeScreen(int dx) {
  display.setTextColor(SSD1306_WHITE);
  time_t now = time(nullptr);

  if (now < 1700000000) {           
    display.setTextSize(1);
    display.setCursor(dx + 14, 28);
    display.print(F("Syncing time..."));
    return;
  }

  struct tm t;
  localtime_r(&now, &t);
  char buf[16];

  display.setTextSize(1);
  display.setCursor(dx + 40, 0);
  display.print(DAYS[t.tm_wday]);
  display.drawFastHLine(dx, 10, 128, SSD1306_WHITE);

  display.setTextSize(2);
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
  display.setCursor(dx + 16, 22);
  display.print(buf);

  display.setTextSize(1);
  snprintf(buf, sizeof(buf), "%02d.%02d.%04d", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);
  display.setCursor(dx + 34, 52);
  display.print(buf);
}

void drawHumidityScreen(int dx) {
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(dx + 34, 0);
  display.print(F("HUMIDITY"));
  display.drawFastHLine(dx, 10, 128, SSD1306_WHITE);

  display.setTextSize(3);
  display.setCursor(dx + 30, 17);
  display.print((int)hum);
  display.print(F("%"));

  display.drawRect(dx + 4, 46, 120, 7, SSD1306_WHITE);
  display.fillRect(dx + 4, 46, (int)(120 * hum / 100.0), 7, SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(dx + 36, 56);
  display.print(F("Temp: "));
  display.print(temp, 1);
  display.print(F(" C"));
}

void drawScreen(int screen, int dx) {
  if (screen == 0) drawTimeScreen(dx);
  else drawHumidityScreen(dx);
}


void slideTo(int nextScreen) {
  for (int i = 0; i <= ANIM_STEPS; i++) {
    float k = (float)i / ANIM_STEPS;
    k = k * k * (3 - 2 * k);                       // smooth start and stop
    int offset = (int)(SCREEN_WIDTH * k);

    display.clearDisplay();
    drawScreen(currentScreen, -offset);
    drawScreen(nextScreen, SCREEN_WIDTH - offset);
    display.display();
    delay(ANIM_DELAY);
    yield();
  }
  currentScreen = nextScreen;
}

void readDht() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h)) hum = h;
  if (!isnan(t)) temp = t;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  dht.begin();
  oledOk = initOled();                 // if it doesn't work, the program doesn't freeze

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

#if defined(ESP32)
  configTzTime(TZ_INFO, NTP_SERVER);
#else
  configTime(TZ_INFO, NTP_SERVER);
#endif

  readDht();
  lastSwitch = millis();
}

void loop() {
  unsigned long ms = millis();

  // If the screen is not found at the start, try again every 3 seconds
  if (!oledOk) {
    if (ms - lastOledRetry >= 3000) {
      lastOledRetry = ms;
      oledOk = initOled();
    }
    delay(10);
    return;
  }

  // DHT11 we poll once every 2 seconds
  if (ms - lastDht >= 2000) {
    lastDht = ms;
    readDht();
  }

  // Switching the screen
  if (ms - lastSwitch >= SCREEN_TIME) {
    slideTo(1 - currentScreen);
    lastSwitch = millis();
  }

  // Normal update of the current screen
  display.clearDisplay();
  drawScreen(currentScreen, 0);
  display.display();
  delay(100);
}