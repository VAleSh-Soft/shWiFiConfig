#include "shWiFiConfig.h"
#include <Arduino.h>

static ESP8266WebServer *http_server = NULL;
static FS *file_system = NULL;

// ==== настройки WiFi ===============================
static String apSsid = AP_SSID;
static String apPass = AP_PASS;
static IPAddress apIP = AP_IP;
static String staSsid = STA_SSID;
static String staPass = STA_PASS;
static IPAddress staIP = STA_IP;
static IPAddress staGateway = STA_GATEWAY;
static IPAddress staMask = STA_MASK;
static bool staticIP = false;
static String fileName = "/wifi.json";
static bool badPassword = false;
static bool ap_sta_mode = false;

// ==== shWiFiConfig class ===========================

shWiFiConfig::shWiFiConfig()
{
  WiFi.mode(curMode);
}

void shWiFiConfig::println(String msg)
{
  if (logOnState)
  {
    Serial.println(msg);
  }
}

void shWiFiConfig::print(String msg)
{
  if (logOnState)
  {
    Serial.print(msg);
  }
}

void shWiFiConfig::setLogOnState(bool log_on) { logOnState = log_on; }

void shWiFiConfig::setCurMode(WiFiMode _mode)
{
  curMode = _mode;
  WiFi.mode(_mode);
}

void shWiFiConfig::setApSsid(String ap_ssid) { apSsid = ap_ssid; }

void shWiFiConfig::setApPass(String ap_pass) { apPass = ap_pass; }

void shWiFiConfig::setApIP(IPAddress ap_ip) { apIP = ap_ip; }

void shWiFiConfig::setStaSsid(String sta_ssid) { staSsid = sta_ssid; }

void shWiFiConfig::setStaPass(String sta_pass) { staPass = sta_pass; }

void shWiFiConfig::setStaIP(IPAddress sta_ip) { staIP = sta_ip; }

void shWiFiConfig::setStaGateway(IPAddress sta_gateway) { staGateway = sta_gateway; }

void shWiFiConfig::setStaMask(IPAddress sta_mask) { staMask = sta_mask; }

void shWiFiConfig::setStaticIpMode(bool static_ip) { staticIP = static_ip; }

void shWiFiConfig::setConfigFileName(String file_name) { fileName = file_name; }

void shWiFiConfig::setApStaMode(bool mode_on) { ap_sta_mode = mode_on; }

bool shWiFiConfig::getLogOnState() { return (logOnState); }

WiFiMode_t shWiFiConfig::getCurMode() { return (curMode); }

String shWiFiConfig::getApSsid() { return (apSsid); }

String shWiFiConfig::getApPass() { return (apPass); }

IPAddress shWiFiConfig::getApIP() { return (apIP); }

String shWiFiConfig::getStaSsid() { return (staSsid); }

String shWiFiConfig::getStaPass() { return (staPass); }

IPAddress shWiFiConfig::getStaIP() { return (staIP); }

IPAddress shWiFiConfig::getStaGateway() { return (staGateway); }

IPAddress shWiFiConfig::getStaMask() { return (staMask); }

bool shWiFiConfig::getStaticIpMode() { return (staticIP); }

String shWiFiConfig::getConfigFileName() { return (fileName); }

bool shWiFiConfig::getApStaMode() { return (ap_sta_mode); }

void shWiFiConfig::setApConfig()
{
  setApConfig(apIP, apIP, (IPAddress(255, 255, 255, 0)));
}

void shWiFiConfig::setApConfig(IPAddress ip)
{
  setApConfig(ip, ip, (IPAddress(255, 255, 255, 0)));
}

void shWiFiConfig::setApConfig(IPAddress ip, IPAddress gateway, IPAddress mask)
{
  WiFi.softAPConfig(ip, gateway, mask);
}

void shWiFiConfig::setStaConfig()
{
  setStaConfig(staIP, staGateway, staMask);
}

void shWiFiConfig::setStaConfig(IPAddress ip, IPAddress gateway, IPAddress mask)
{
  WiFi.config(ip, gateway, mask);
}
void shWiFiConfig::begin(ESP8266WebServer *_server, FS *_file_system, String _config_page)
{
  http_server = _server;
  file_system = _file_system;

  // вызов страницы настройки WiFi
  http_server->on(_config_page, HTTP_GET, &handleGetConfigPage);
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
    println(F("WiFi config file not found, default settings used."));
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
  StaticJsonDocument<1024> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    println(F("Failed to read WiFi config file, default config is used"));
    result = false;
  }
  else
  // Теперь можно получить значения из doc
  {
    String _str[] = {ssid_ap_str, ap_pass_str, ssid_str, pass_str,
                     ip_str, gateway_str, mask_str};
    String *str_val[] = {&apSsid, &apPass, &staSsid, &staPass};

    IPAddress *ip_val[] = {&staIP, &staGateway, &staMask};

    staticIP = doc[static_ip_str].as<bool>();

    for (byte i = 0; i < 4; i++)
    {
      String s = doc[_str[i]].as<String>();
      // если такой параметр нашелся, загружаем его, иначе у переменной остается значение по умолчанию
      if (s != "null")
      {
        *str_val[i] = s;
      }
    }

    for (byte i = 4; i < 7; i++)
    {
      IPAddress ip;
      // если такой параметр нашелся, загружаем его, иначе у переменной остается значение по умолчанию
      if (ip.fromString(doc[_str[i]].as<String>()))
      {
        *ip_val[i - 4] = ip;
      }
    }
  }

  return (result);
}

bool shWiFiConfig::saveConfig()
{
  bool result = false;

  // удалить существующий файл, иначе конфигурация будет добавлена ​​к файлу
  file_system->remove(fileName);

  // Открыть файл для записи
  File file = file_system->open(fileName, "w");
  if (!file)
  {
    println(F("Failed to create WiFi configuration file"));
    return (result);
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<1024> doc;

  // задать данные для сохранения
  doc[ssid_ap_str] = apSsid;
  doc[ap_pass_str] = apPass;
  doc[ssid_str] = staSsid;
  doc[pass_str] = staPass;
  doc[static_ip_str] = (byte)staticIP;
  doc[ip_str] = staIP.toString();
  doc[gateway_str] = staGateway.toString();
  doc[mask_str] = staMask.toString();

  // сериализовать JSON-файл
  result = !serializeJson(doc, file);

  if (!result)
  {
    println(F("Failed to write WiFi configuration file"));
  }

  file.close();
  return (result);
}

bool shWiFiConfig::startSoftAP()
{
  bool result = false;
  println(F("Create WiFi access point ") + apSsid);

  setCurMode(WIFI_AP);
  setApConfig();
  result = WiFi.softAP(apSsid, apPass);
  if (result)
  {
    println(F("Access point SSID: ") + WiFi.softAPSSID());
    println(F("Access point IP: ") + WiFi.softAPIP().toString() + "\n");
  }
  else
  {
    println(F("Access point failed"));
  }

  return (result);
}

bool shWiFiConfig::startSTA()
{
  return (startSTA(staSsid, staPass));
}

bool shWiFiConfig::startSTA(String ssid, String pass)
{
  bool result = false;
  println(F("Connecting to WiFi network ") + ssid);
  (ap_sta_mode) ? setCurMode(WIFI_AP_STA) : setCurMode(WIFI_STA);
  WiFi.hostname(apSsid);
  if (staticIP)
  {
    setStaConfig();
  }
  WiFi.begin(ssid, pass);
  result = WiFi.waitForConnectResult() == WL_CONNECTED;
  if (result)
  {
    println(F("Connection: ") + WiFi.SSID());
    println(F("IP: ") + WiFi.localIP().toString());
    println("HostName: " + WiFi.hostname() + "\n");

    badPassword = false;
  }
  else
  {
    println(F("Failed to connect to  ") + ssid);
    if (wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
    {
      println(F("Incorrect password!"));
      badPassword = true;
    }
    println("");
  }

  return (result);
}

bool shWiFiConfig::findSavedAp()
{
  return (findAp(staSsid));
}

bool shWiFiConfig::findAp(String ssid)
{
  bool result = false;

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
  return (result);
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

void handleGetConfigPage()
{
  const String successResponse =
      F("Save settings...");
  http_server->send(200, "text/html", successResponse);
}