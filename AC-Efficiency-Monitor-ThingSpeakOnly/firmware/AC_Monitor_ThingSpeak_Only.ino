/***** NODEMCU D-PIN DEFINITIONS (SAFETY) *****/
#ifndef D0
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#endif
/***********************************************/

#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== WiFi Credentials =====
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// ===== ThingSpeak =====
unsigned long THINGSPEAK_CHANNEL_ID = 123456;  
const char* THINGSPEAK_WRITE_KEY = "YOUR_WRITE_API_KEY";

WiFiClient client;

// ===== Sensor Type =====
#define DHTTYPE DHT22

// ===== NodeMCU Pin Configuration =====
#define DHTPIN_IN   D7   // Intake DHT22
#define DHTPIN_OUT  D4   // Outlet DHT22
#define BUZZER_PIN  D6   // Buzzer
#define RED_LED     D5   // LED
#define OLED_SDA    D2   // OLED SDA
#define OLED_SCL    D1   // OLED SCL

// ===== OLED Setup =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Sensors
DHT dht_in(DHTPIN_IN, DHTTYPE);
DHT dht_out(DHTPIN_OUT, DHTTYPE);

// ===== Thresholds =====
float thr_a = 10.0, thr_b = 7.0, thr_c = 4.0, thr_d = 1.0;
float activationTemp = 28.0;

// Outlet temperature absolute levels
float out_warn1 = 28, out_warn2 = 34, out_warn3 = 38, out_warn4 = 40, out_warn5 = 42;

// ===== Helper Functions =====
void oled(String a, String b, String c, String d){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(a); display.println(b);
  display.println(c); display.println(d);
  display.display();
}

void buzzerBeep(int n,int on=150,int off=150){
  for(int i=0;i<n;i++){
    digitalWrite(BUZZER_PIN, HIGH);
    delay(on);
    digitalWrite(BUZZER_PIN, LOW);
    delay(off);
  }
}

void buzzerOff(){ digitalWrite(BUZZER_PIN, LOW); }

void blinkLED(int times, int d=300){
  for(int i=0;i<times;i++){
    digitalWrite(RED_LED,HIGH); delay(d);
    digitalWrite(RED_LED,LOW); delay(d);
  }
}

// ===== WiFi Connect =====
void connectWiFi(){
  Serial.printf("Connecting to %s...\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 30){
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED){
    Serial.println("\\nWiFi Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\\nWiFi Failed!");
  }
}

// ===== Setup =====
void setup(){
  Serial.begin(115200);

  Wire.begin(OLED_SDA, OLED_SCL);

  dht_in.begin();
  dht_out.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled("AC Monitor", "ThingSpeak Version", "", "Booting...");
  delay(1500);

  connectWiFi();
  ThingSpeak.begin(client);

  oled("AC Monitor Online", "WiFi OK", WiFi.localIP().toString(), "");
  delay(1500);
}

// ===== LOOP =====
void loop(){

  if (WiFi.status() != WL_CONNECTED){
    connectWiFi();
  }

  float Tin = dht_in.readTemperature();
  float Hin = dht_in.readHumidity();
  float Tout = dht_out.readTemperature();
  float Hout = dht_out.readHumidity();

  if (isnan(Tin)||isnan(Tout)||isnan(Hin)||isnan(Hout)){
    oled("Sensor Error", "Check DHT22", "", "");
    buzzerBeep(2);
    blinkLED(3,200);
    delay(3000);
    return;
  }

  if (Tin < activationTemp){
    oled("AC Idle", "Intake < 28C", "Monitoring Paused", "");
    buzzerOff();
    digitalWrite(RED_LED, LOW);
    delay(5000);
    return;
  }

  float deltaT = Tin - Tout;
  float deltaH = Hin - Hout;

  String diag = "Normal";
  String note = "";
  bool alert = false;

  // ===== ΔT Based Logic =====
  if      (deltaT >= thr_a) diag="Efficient", note="System OK";
  else if (deltaT >= thr_b) diag="Slight Drop", note="Filter Dusty";
  else if (deltaT >= thr_c) diag="Reduced Cooling", note="Filter Clogged", alert=true;
  else if (deltaT >= thr_d) diag="Cooling Poor", note="Condenser Issue", alert=true;
  else if (deltaT > 0)      diag="Critical Drop", note="Compressor/Gas", alert=true;
  else                      diag="System Error", note="No Cooling", alert=true;

  // ===== Outlet Temperature Overrides =====
  if      (Tout > out_warn5) diag="System Error", note="Outlet >42C", alert=true;
  else if (Tout > out_warn4) diag="Critical Drop", note="Outlet >40C", alert=true;
  else if (Tout > out_warn3) diag="Cooling Poor", note="Outlet >38C", alert=true;
  else if (Tout > out_warn2) diag="Reduced Cooling", note="Outlet >34C", alert=true;
  else if (Tout > out_warn1) diag="Slight Drop", note="Outlet >28C";

  // ===== Humidity Logic =====
  if      (Hout > 90) note += " | Coil Freeze", alert=true;
  else if (Hout < 40 && Tout > 34) note += " | Dry Hot Air", alert=true;
  else if (deltaH < 5) note += " | Weak Dehumid", alert=true;
  else if (deltaH > 20) note += " | Excess Dehumid";

  // ===== OLED Output =====
  oled(
    "In:"+String(Tin,1)+"C "+String(Hin,0)+"%",
    "Out:"+String(Tout,1)+"C "+String(Hout,0)+"%",
    "ΔT:"+String(deltaT,1)+" ΔH:"+String(deltaH,0),
    diag + " | " + note
  );

  // ===== Local Alerts =====
  if (alert){
    buzzerBeep(3);
    blinkLED(3,300);
  }
  else {
    buzzerOff();
    digitalWrite(RED_LED, LOW);
  }

  // ===== ThingSpeak Upload =====
  ThingSpeak.setField(1, Tin);
  ThingSpeak.setField(2, Tout);
  ThingSpeak.setField(3, Hin);
  ThingSpeak.setField(4, Hout);

  ThingSpeak.setStatus((diag + " | " + note).c_str());

  int tsCode = ThingSpeak.writeFields(THINGSPEAK_CHANNEL_ID, THINGSPEAK_WRITE_KEY);

  if (tsCode == 200){
    Serial.println("ThingSpeak Update OK");
  } else {
    Serial.print("ThingSpeak Error: ");
    Serial.println(tsCode);
  }

  delay(15000);  // 15 seconds
}
