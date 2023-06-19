#pragma once

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ArduinoJson.h>

// ==== Настройки точки доступа ======================
static const String AP_SSID = "WIFI_AP";
static const String AP_PASS = "12345678";
static const IPAddress AP_IP(192, 168, 4, 1);

// ===== Настройки внешней WiFi-сети =================
static const String STA_SSID = "WIFI_SSID";
static const String STA_PASS = "password";
static const IPAddress STA_IP(192, 168, 0, 50);
static const IPAddress STA_GATEWAY(192, 168, 0, 1);
static const IPAddress STA_MASK(255, 255, 255, 0);

// ==== Имена JSON-параметров ========================
static const String ssid_ap_str = "ssid_ap";
static const String ap_pass_str = "ap_pass";
static const String ssid_str = "ssid";
static const String pass_str = "pass";
static const String static_ip_str = "static_ip";
static const String ip_str = "ip";
static const String gateway_str = "gateway";
static const String mask_str = "mask";

// ===================================================

class shWiFiConfig
{
private:
  bool logOnState = true;
  WiFiMode_t curMode = WIFI_OFF;

  void println(String msg);
  void print(String msg);

public:
  shWiFiConfig();

  void setLogOnState(bool log_on);
  void setCurMode(WiFiMode _mode);
  void setApSsid(String ap_ssid);
  void setApPass(String ap_pass);
  void setApIP(IPAddress ap_ip);
  void setStaSsid(String sta_ssid);
  void setStaPass(String sta_pass);
  void setStaIP(IPAddress sta_ip);
  void setStaGateway(IPAddress sta_gateway);
  void setStaMask(IPAddress sta_mask);
  void setStaticIpMode(bool static_ip);
  void setConfigFileName(String file_name);
  void setApStaMode(bool mode_on);

  bool getLogOnState();
  WiFiMode_t getCurMode();
  String getApSsid();
  String getApPass();
  IPAddress getApIP();
  String getStaSsid();
  String getStaPass();
  IPAddress getStaIP();
  IPAddress getStaGateway();
  IPAddress getStaMask();
  bool getStaticIpMode();
  String getConfigFileName();
  bool getApStaMode();

  void setApConfig();
  void setApConfig(IPAddress ip);
  void setApConfig(IPAddress ip, IPAddress gateway, IPAddress mask);
  void setStaConfig();
  void setStaConfig(IPAddress ip, IPAddress gateway, IPAddress mask);

  void begin(ESP8266WebServer *_server, FS *_file_system, String _config_page = "/wifi_config");
  bool loadConfig();
  bool saveConfig();

  bool startSoftAP();
  bool startSTA();
  bool startSTA(String ssid, String pass);
  bool findSavedAp();
  bool findAp(String ssid);

  void checkStaConnection();
};

void handleGetConfigPage();
