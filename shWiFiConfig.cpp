#include "shWiFiConfig.h"
#include "extras/c_page.h"
#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP32)
static WebServer *http_server = NULL;
#else
static ESP8266WebServer *http_server = NULL;
#endif

static LedState led;

static FS *file_system = NULL;

static const int confSize = 1024;

static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char TEXT_HTML[] PROGMEM = "text/html";
static const char TEXT_JSON[] PROGMEM = "text/json";

static void handleGetConfigPage();
static void handleReadSetting();
static void handleWriteSetting();
static void handleReadApList();

static bool saveConfig();
static void println(String msg);
static void print(String msg);
static void readJsonSetting(StaticJsonDocument<confSize> &doc);
static void writeSettingInJson(StaticJsonDocument<confSize> &doc);

static bool find_ap(String ssid);
static void set_cur_mode(WiFiMode_t _mode);
static void set_sta_config(IPAddress ip, IPAddress gateway, IPAddress mask);
static void set_ap_config(IPAddress ip, IPAddress gateway, IPAddress mask);
static bool start_wifi();
static void stop_wifi();
static bool start_sta(String ssid, String pass);
static bool start_ap(String ssid, String pass, bool combo_mode = false);

// ==== настройки WiFi ===============================

// ==== AP ======================================
static String apSsid = "WIFI_AP_";
static String apPass = "12345678";
static IPAddress apIP(192, 168, 4, 1);
static IPAddress apGateway(192, 168, 4, 1);
static IPAddress apMask(255, 255, 255, 0);
// ==== STA =====================================
static String staSsid = "";
static String staPass = "";
static bool staticIP = false;
static IPAddress staIP(192, 168, 0, 50);
static IPAddress staGateway(192, 168, 0, 1);
static IPAddress staMask(255, 255, 255, 0);
// ==== AP + STA ================================
static bool ap_sta_mode = false;
static bool useComboMode = false;
// ==== Other ===================================
static bool useAdmPass = false;
static String admName = "";
static String admPass = "";
static bool useLed = true;
static bool ledOn = true;
static int8_t ledPin = -1;

static bool badPassword = false;
static WiFiMode_t curMode = WIFI_OFF;
static bool logOnState = true;
static String fileName = "/wifi.json";

// ==== Имена JSON-параметров ========================
static const String ap_ssid_str = "ap_ssid";
static const String ap_pass_str = "ap_pass";
static const String ap_ip_str = "ap_ip";
static const String ap_gateway_str = "ap_gateway";
static const String ap_mask_str = "ap_mask";
static const String ssid_str = "ssid";
static const String pass_str = "pass";
static const String static_ip_str = "static_ip";
static const String ip_str = "ip";
static const String gateway_str = "gateway";
static const String mask_str = "mask";
static const String use_combo_mode_str = "use_combo";
static const String ap_sta_mode_str = "ap_sta";
static const String use_adm_pass_str = "use_adm_pass";
static const String a_name_str = "a_name";
static const String a_pass_str = "a_pass";
static const String use_led_str = "use_led";
static const String led_on_off = "led_on";

// ==== shWiFiConfig class ===========================

shWiFiConfig::shWiFiConfig()
{
  WiFi.mode(curMode);
}

shWiFiConfig::shWiFiConfig(String adm_name, String adm_pass)
{
  WiFi.mode(curMode);
  setAdminData(adm_name, adm_pass);
}

void shWiFiConfig::setCheckTimer(uint32_t _timer) { checkTimer = _timer; }

void shWiFiConfig::setLogOnState(bool log_on) { logOnState = log_on; }

void shWiFiConfig::setApSsid(String ap_ssid) { apSsid = ap_ssid; }

void shWiFiConfig::setApPass(String ap_pass) { apPass = ap_pass; }

void shWiFiConfig::setApIP(IPAddress ap_ip) { apIP = ap_ip; }

void shWiFiConfig::setApGateway(IPAddress ap_gateway) { apGateway = ap_gateway; };

void shWiFiConfig::setApMask(IPAddress ap_mask) { apMask = ap_mask; }

void shWiFiConfig::setStaSsid(String sta_ssid) { staSsid = sta_ssid; }

void shWiFiConfig::setStaPass(String sta_pass) { staPass = sta_pass; }

void shWiFiConfig::setStaIP(IPAddress sta_ip) { staIP = sta_ip; }

void shWiFiConfig::setStaGateway(IPAddress sta_gateway) { staGateway = sta_gateway; }

void shWiFiConfig::setStaMask(IPAddress sta_mask) { staMask = sta_mask; }

void shWiFiConfig::setStaticIpMode(bool static_ip) { staticIP = static_ip; }

void shWiFiConfig::setConfigFileName(String file_name) { fileName = file_name; }

void shWiFiConfig::setApStaMode(bool mode_on) { ap_sta_mode = mode_on; }

void shWiFiConfig::setUseComboMode(bool mode_on) { useComboMode = mode_on; }

void shWiFiConfig::setUseAdminPass(bool pass_on) { useAdmPass = pass_on; }

void shWiFiConfig::setUseLed(bool _use, int8_t _pin)
{
  ledPin = _pin;
  useLed = (ledPin >= 0) ? _use : false;
  led.setPin(_pin);
  if (!useLed)
  {
    led.setUseLed(useLed);
    setLedOnMode(false);
  }
}

void shWiFiConfig::setLedOnMode(bool mode_on)
{
  ledOn = mode_on;
  led.setUseLed(mode_on);
}

void shWiFiConfig::setStaSsidData(String ssid, String pass)
{
  staSsid = ssid;
  staPass = pass;
}

void shWiFiConfig::setApSsidData(String ssid, String pass)
{
  apSsid = ssid;
  apPass = pass;
}

void shWiFiConfig::setAdminData(String name, String pass)
{
  if (name != emptyString && pass != emptyString)
  {
    useAdmPass = true;
    admName = name;
    admPass = pass;
  }
}

uint32_t shWiFiConfig::getCheckTimer() { return (checkTimer); }

bool shWiFiConfig::getLogOnState() { return (logOnState); }

WiFiMode_t shWiFiConfig::getCurMode() { return (curMode); }

String shWiFiConfig::getApSsid() { return (apSsid); }

String shWiFiConfig::getApPass() { return (apPass); }

IPAddress shWiFiConfig::getApIP() { return (apIP); }

IPAddress shWiFiConfig::getApGateway() { return (apGateway); };

IPAddress shWiFiConfig::getApMask() { return (apMask); }

String shWiFiConfig::getStaSsid() { return (staSsid); }

String shWiFiConfig::getStaPass() { return (staPass); }

IPAddress shWiFiConfig::getStaIP() { return (staIP); }

IPAddress shWiFiConfig::getStaGateway() { return (staGateway); }

IPAddress shWiFiConfig::getStaMask() { return (staMask); }

bool shWiFiConfig::getStaticIpMode() { return (staticIP); }

String shWiFiConfig::getConfigFileName() { return (fileName); }

bool shWiFiConfig::getApStaMode() { return (ap_sta_mode); }

bool shWiFiConfig::getUseComboMode() { return (useComboMode); }

bool shWiFiConfig::getUseAdminPass() { return (useAdmPass); }

String shWiFiConfig::getAdminPass() { return (admPass); }

String shWiFiConfig::getAdminName() { return (admName); }

bool shWiFiConfig::getUseLed() { return (useLed); }

bool shWiFiConfig::getLedOnMode() { return (ledOn); }

int8_t shWiFiConfig::getLedPin() { return (ledPin); }

void shWiFiConfig::setApConfig()
{
  set_ap_config(apIP, apGateway, apMask);
}

void shWiFiConfig::setApConfig(IPAddress ip)
{
  set_ap_config(ip, ip, (IPAddress(255, 255, 255, 0)));
}

void shWiFiConfig::setApConfig(IPAddress ip, IPAddress gateway, IPAddress mask)
{
  set_ap_config(ip, gateway, mask);
}

void shWiFiConfig::setStaConfig()
{
  set_sta_config(staIP, staGateway, staMask);
}

void shWiFiConfig::setStaConfig(IPAddress ip, IPAddress gateway, IPAddress mask)
{
  set_sta_config(ip, gateway, mask);
}

#if defined(ARDUINO_ARCH_ESP32)
void shWiFiConfig::begin(WebServer *_server, FS *_file_system, String _config_page)
#else
void shWiFiConfig::begin(ESP8266WebServer *_server, FS *_file_system, String _config_page)
#endif
{
  http_server = _server;
  file_system = _file_system;

  // вызов страницы настройки WiFi
  http_server->on(_config_page, HTTP_GET, &handleGetConfigPage);
  // заполнение полей страницы настройки WiFi
  http_server->on("/wifi_getconfig", HTTP_GET, handleReadSetting);
  // сохранение настроек
  http_server->on("/wifi_setconfig", HTTP_POST, handleWriteSetting);
  // получение списка доступных точек доступа
  http_server->on("/wifi_getaplist", HTTP_GET, handleReadApList);
}

void shWiFiConfig::tick()
{
  static uint32_t timer = millis();
  if (millis() - timer > checkTimer)
  {
    timer = millis();
    checkStaConnection();
  }
}

bool shWiFiConfig::loadConfig()
{
  File configFile;
  // находим и открываем для чтения файл конфигурации
  bool result = file_system->exists(fileName) &&
                (configFile = file_system->open(fileName, "r"));

  // если файл конфигурации не найден, сохранить настройки по умолчанию
  if (!result)
  {
    println(F("WiFi config file not found, default config used."));
    saveConfig();
    return (result);
  }

  // Проверяем размер файла, будем использовать файл размером меньше 1024 байта
  size_t size = configFile.size();
  if (size > 1024)
  {
    println(F("WiFi configuration file size is too large."));
    configFile.close();
    return (false);
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<confSize> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    print("Data serialization error: ");
    println(error.f_str());
    println(F("Failed to read WiFi config file, default config is used"));
    result = false;
  }
  else
  // Теперь можно получить значения из doc
  {
    readJsonSetting(doc);
  }

  return (result);
}

bool shWiFiConfig::startWiFi()
{
  return (start_wifi());
}

void shWiFiConfig::stopWiFi()
{
  stop_wifi();
}

bool shWiFiConfig::startSoftAP()
{
  return (start_ap(apSsid, apPass));
}

bool shWiFiConfig::startSoftAP(String ssid, String pass)
{
  return (start_ap(ssid, pass));
}

bool shWiFiConfig::startSTA()
{
  return (start_sta(staSsid, staPass));
}

bool shWiFiConfig::startSTA(String ssid, String pass)
{
  return (start_sta(ssid, pass));
}

bool shWiFiConfig::findSavedAp()
{
  return (find_ap(staSsid));
}

bool shWiFiConfig::findAp(String ssid)
{
  return (find_ap(ssid));
}

void shWiFiConfig::checkStaConnection()
{
  if (curMode == WIFI_STA || curMode == WIFI_AP_STA)
  {
    // если мы в режиме точки доступа
    if (!WiFi.isConnected())
    {
      println(F("WiFi connection lost"));
      // если пропала связь с роутером, перейти в режим точки доступа
      WiFi.disconnect();
      if (curMode == WIFI_AP_STA)
      {
        WiFi.softAPdisconnect();
      }
      if (!startSoftAP())
      {
        ESP.restart();
      }
    }
  }
  else
  {
    // пытаться искать и подключаться к сети только если пароль не помечен неверным
    if (!badPassword && findSavedAp())
    {
      // если наша сеть найдена, подключиться к ней
      if (curMode == WIFI_AP)
      {
        WiFi.softAPdisconnect();
      }
      // если не удалось, перейти в режим точки доступа
      if (!startSTA())
      {
        startSoftAP();
      }
    }
  }
}

// ==== реакции сервера ==============================

static void handleGetConfigPage()
{
  if (useAdmPass &&
      admName != emptyString &&
      admPass != emptyString &&
      !http_server->authenticate(admName.c_str(), admPass.c_str()))
  {
    return http_server->requestAuthentication();
  }
  http_server->send(200, FPSTR(TEXT_HTML), FPSTR(config_page));
}

static void handleReadSetting()
{
  StaticJsonDocument<confSize> doc;

  writeSettingInJson(doc);
  doc[use_combo_mode_str] = (byte)useComboMode;
  doc[use_led_str] = (byte)useLed;

  String json = "";
  serializeJson(doc, json);

  http_server->send(200, FPSTR(TEXT_JSON), json);
}

static void handleWriteSetting()
{
  if (http_server->hasArg("plain") == false)
  {
    http_server->send(200, FPSTR(TEXT_PLAIN), F("Body not received"));
    println(F("Failed to save configuration data, no data"));
    return;
  }

  String json = http_server->arg("plain");

  StaticJsonDocument<confSize> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    println(F("Failed to save configuration data, invalid json data"));
    println(error.f_str());
  }
  else
  {
    badPassword = false;

    // определить необходимость перезагрузки модуля
    bool reconnect = false;

    if (curMode == WIFI_STA || curMode == WIFI_AP_STA)
    {
      reconnect = (staSsid != doc[ssid_str].as<String>()) ||
                  (staPass != doc[pass_str].as<String>());
      if (!reconnect)
      {
        reconnect = staticIP != (bool)doc[static_ip_str].as<byte>();
        if (!reconnect && staticIP)
        {
          IPAddress ip;
          reconnect = (ip.fromString(doc[ip_str].as<String>()) &&
                       ((uint32_t)ip != (uint32_t)staIP)) ||
                      (ip.fromString(doc[gateway_str].as<String>()) &&
                       ((uint32_t)ip != (uint32_t)staGateway)) ||
                      (ip.fromString(doc[mask_str].as<String>()) &&
                       ((uint32_t)ip != (uint32_t)staMask));
        }
      }
    }
    if (!reconnect && (curMode == WIFI_AP || curMode == WIFI_AP_STA))
    {
      reconnect = (apSsid != doc[ap_ssid_str].as<String>()) ||
                  (apPass != doc[ap_pass_str].as<String>());
      if (!reconnect)
      {
        IPAddress ip;
        reconnect = (ip.fromString(doc[ap_ip_str].as<String>()) &&
                     ((uint32_t)ip != (uint32_t)apIP)) ||
                    (ip.fromString(doc[ap_mask_str].as<String>()) &&
                     ((uint32_t)ip != (uint32_t)apMask));
      }
    }
    if (!reconnect)
    {
      reconnect = ap_sta_mode != (bool)doc[ap_sta_mode_str].as<byte>();
    }

    readJsonSetting(doc);
    saveConfig();
    const String successResponse0 =
        F("<META http-equiv=\"refresh\" content=\"5;URL=/\">The module will be reconnected, wait...");
    const String successResponse1 =
        F("<META http-equiv=\"refresh\" content=\"1;URL=/\">Save settings...");
    http_server->client().setNoDelay(true);
    // Если изменили опции, требующие переподключения, переподключить модуль
    if (reconnect)
    {
      println("");
      println(F("The module will be reconnected, wait..."));
      println("");
      http_server->send(200, FPSTR(TEXT_HTML), successResponse0);
      delay(100);
      stop_wifi();
      start_wifi();
    }
    else
    {
      http_server->send(200, FPSTR(TEXT_HTML), successResponse1);
    }
    led.startLed();
  }
}

static void handleReadApList()
{
  int n = WiFi.scanNetworks();

  StaticJsonDocument<2048> doc;
  if (n > 0)
  {
    for (byte i = 0; i < n; ++i)
    {
      doc["aps"][i] = WiFi.SSID(i);
    }
  }
  else
    doc["list"] = "empty";

  String json = "";
  serializeJson(doc, json);

  http_server->send(200, FPSTR(TEXT_JSON), json);
}

static void handleShowSavePage()
{
  const String successResponse =
      F("<META http-equiv=\"refresh\" content=\"1;URL=/\">Save settings...");
  http_server->client().setNoDelay(true);
  http_server->send(200, FPSTR(TEXT_HTML), successResponse);
}

// ===================================================

static bool saveConfig()
{
  File configFile;

  bool result = false;

  // удалить существующий файл, иначе конфигурация будет добавлена ​​к файлу
  file_system->remove(fileName);

  // Открыть файл для записи
  configFile = file_system->open(fileName, "w");
  if (!configFile)
  {
    println(F("Failed to create WiFi configuration file"));
    return (result);
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<confSize> doc;

  // задать данные для сохранения
  writeSettingInJson(doc);

  // сериализовать JSON-файл
  result = serializeJson(doc, configFile);

  if (!result)
  {
    println(F("Failed to write WiFi configuration file"));
  }

  configFile.close();
  return (result);
}

static void readJsonSetting(StaticJsonDocument<confSize> &doc)
{
  String _str[] = {ap_ssid_str, ap_pass_str, ssid_str, pass_str, a_name_str, a_pass_str,
                   ap_ip_str, ap_gateway_str, ap_mask_str, ip_str, gateway_str, mask_str};
  String *str_val[] = {&apSsid, &apPass, &staSsid, &staPass, &admName, &admPass};

  IPAddress *ip_val[] = {&apIP, &apGateway, &apMask, &staIP, &staGateway, &staMask};

  staticIP = doc[static_ip_str].as<bool>();
  ap_sta_mode = doc[ap_sta_mode_str].as<bool>();
  useAdmPass = doc[use_adm_pass_str].as<bool>();
  ledOn = doc[led_on_off].as<bool>();
  led.setUseLed(ledOn);

  for (byte i = 0; i < 6; i++)
  {
    String s = doc[_str[i]].as<String>();
    // если такой параметр нашелся, загружаем его, иначе у переменной остается значение по умолчанию
    if (s != "null")
    {
      *str_val[i] = s;
    }
  }

  for (byte i = 6; i < 12; i++)
  {
    IPAddress ip;
    // если такой параметр нашелся, загружаем его, иначе у переменной остается значение по умолчанию
    if (ip.fromString(doc[_str[i]].as<String>()))
    {
      *ip_val[i - 6] = ip;
    }
  }
}

static void writeSettingInJson(StaticJsonDocument<confSize> &doc)
{
  doc[ap_ssid_str] = apSsid;
  doc[ap_pass_str] = apPass;
  doc[ap_ip_str] = apIP.toString();
  doc[ap_gateway_str] = apGateway.toString();
  doc[ap_mask_str] = apMask.toString();
  doc[ssid_str] = staSsid;
  doc[pass_str] = staPass;
  doc[static_ip_str] = (byte)staticIP;
  doc[ip_str] = staIP.toString();
  doc[gateway_str] = staGateway.toString();
  doc[mask_str] = staMask.toString();
  doc[ap_sta_mode_str] = (byte)ap_sta_mode;
  doc[use_adm_pass_str] = (byte)useAdmPass;
  doc[a_name_str] = admName;
  doc[a_pass_str] = admPass;
  doc[led_on_off] = (byte)ledOn;
}

static bool find_ap(String ssid)
{
  bool result = false;

  led.init(100, true);
  print(F("Searche for access point "));
  println(ssid);
  int8_t n = WiFi.scanNetworks();
  if (n > 0)
  {
    for (byte i = 0; i < n; ++i)
    {
      if (WiFi.SSID(i) == ssid)
      {
        result = true;
        break;
      }
    }
  }
  (result) ? println(F("OK")) : println(F("not found"));
  return (result);
}

static void set_cur_mode(WiFiMode_t _mode)
{
  curMode = _mode;
  WiFi.mode(_mode);
}

static void set_sta_config(IPAddress ip, IPAddress gateway, IPAddress mask)
{
  WiFi.config(ip, gateway, mask);
}

static void set_ap_config(IPAddress ip, IPAddress gateway, IPAddress mask)
{
  WiFi.softAPConfig(ip, gateway, mask);
}

static bool start_wifi()
{
  bool result = start_sta(staSsid, staPass);
  if (!result)
  {
    stop_wifi();
    result = start_ap(apSsid, apPass);
  }

  return (result);
}

static void stop_wifi()
{
  if (curMode == WIFI_STA || curMode == WIFI_AP_STA)
  {
    WiFi.disconnect();
  }
  if (curMode == WIFI_AP || curMode == WIFI_AP_STA)
  {
    WiFi.softAPdisconnect();
  }
  set_cur_mode(WIFI_OFF);
}

static bool start_sta(String ssid, String pass)
{
  bool result = false;

  (useComboMode && ap_sta_mode) ? set_cur_mode(WIFI_AP_STA)
                                : set_cur_mode(WIFI_STA);
  if (curMode > WIFI_STA)
  {
    start_ap(apSsid, apPass, true);
  }

  if (find_ap(ssid))
  {
    WiFi.hostname(apSsid);
    if (staticIP)
    {
      set_sta_config(staIP, staGateway, staMask);
    }
    else
    {
      WiFi.config((uint32_t)0, (uint32_t)0, (uint32_t)0);
    }
    led.init(100, true);
    print(F("Connecting to WiFi network "));
    println(ssid);
    WiFi.begin(ssid.c_str(), pass.c_str());
    result = WiFi.waitForConnectResult() == WL_CONNECTED;
    if (result)
    {
      print(F("Connected: "));
      println(WiFi.SSID());
      print(F("IP: "));
      println(WiFi.localIP().toString());
#if defined(ARDUINO_ARCH_ESP8266)
      print(F("HostName: "));
      println(WiFi.hostname());
#endif

      badPassword = false;
      led.init();
    }
    else
    {
      print(F("Failed to connect to "));
      println(ssid);
#if defined(ARDUINO_ARCH_ESP8266)
      if (wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
      {
        println(F("Incorrect password!"));
        badPassword = true;
      }
#else
      badPassword = true;
#endif
    }
  }
  println("");

  return (result);
}

static bool start_ap(String ssid, String pass, bool combo_mode)
{
  bool result = false;
  print(F("Create WiFi access point "));
  println(ssid);

  led.init(100, true);
  (combo_mode) ? set_cur_mode(WIFI_AP_STA) : set_cur_mode(WIFI_AP);
  set_ap_config(apIP, apGateway, apMask);
  result = WiFi.softAP(ssid.c_str(), pass.c_str());
  if (result)
  {
    print(F("Access point SSID: "));
    println(WiFi.softAPSSID());
    print(F("Access point IP: "));
    println(WiFi.softAPIP().toString());
    led.init(500);
  }
  else
  {
    println(F("Access point failed"));
  }
  println("");

  return (result);
}

static void println(String msg)
{
  if (logOnState)
  {
    Serial.println(msg);
  }
}

static void print(String msg)
{
  if (logOnState)
  {
    Serial.print(msg);
  }
}

// ===================================================

void changeLedState()
{
  led.analogCheck();
}

void changeLedState1()
{
  led.digitalCheck();
}

LedState::LedState() {}

void LedState::setPin(int8_t _pin)
{
  pin = _pin;
  if (pin >= 0)
  {
    pinMode(pin, OUTPUT);
  }
}

void LedState::init(uint32_t interval, bool force)
{
  blink.detach();
  if (pin >= 0)
  {
    digitalWrite(pin, HIGH);
    if (use_led || force)
    {
      blink.attach_ms(interval, changeLedState1);
    }
  }
}

void LedState::init()
{
  blink.detach();
  if (pin >= 0)
  {
    digitalWrite(pin, HIGH);
    if (use_led)
    {
      pwr_value = max_pwm;
      toUp = false;
#if defined(ARDUINO_ARCH_ESP32)
      ledcSetup(0, 1000, 10);
      ledcAttachPin(pin, 0);
      blink.attach_ms(3, changeLedState);
#else
      blink.attach_ms(10, changeLedState);
#endif
    }
  }
}

void LedState::stopLed()
{
  blink.detach();
  if (pin >= 0)
  {
    digitalWrite(pin, HIGH);
  }
}

void LedState::startLed()
{
  switch (curMode)
  {
  case WIFI_AP:
    init(500);
    break;
  case WIFI_STA:
  case WIFI_AP_STA:
    init();
    break;
  default:
    stopLed();
    break;
  }
}

void LedState::digitalCheck()
{
  if (pin >= 0)
  {
    digitalWrite(pin, !digitalRead(pin));
  }
}

void LedState::analogCheck()
{
  if (pin >= 0)
  {
#if defined(ARDUINO_ARCH_ESP8266)
    analogWrite(pin, pwr_value);
#else
    ledcWrite(0, pwr_value);
#endif

    (toUp) ? pwr_value++ : pwr_value--;
    if (pwr_value >= max_pwm || pwr_value <= min_pwm)
    {
      toUp = !toUp;
    }
  }
}

void LedState::setUseLed(bool _use)
{
  use_led = _use;
  if (!use_led)
  {
    stopLed();
  }
}