#pragma once

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
// #include <user_interface.h>
#else
#error "The library is designed to be used in an ESP8266 or ESP32 environment"
#endif
#include <FS.h>
#include <ArduinoJson.h>
#include <Ticker.h>

// ==== shWiFiConfig class ===========================
class shWiFiConfig
{
private:
  uint32_t checkTimer = 20000;

public:
  shWiFiConfig();
  shWiFiConfig(String adm_name, String adm_pass);

  void setCheckTimer(uint32_t _timer);
  void setLogOnState(bool log_on);
  void setApSsid(String ap_ssid);
  void setApPass(String ap_pass);
  void setApIP(IPAddress ap_ip);
  void setApGateway(IPAddress ap_gateway);
  void setApMask(IPAddress ap_mask);
  void setStaSsid(String sta_ssid);
  void setStaPass(String sta_pass);
  void setStaIP(IPAddress sta_ip);
  void setStaGateway(IPAddress sta_gateway);
  void setStaMask(IPAddress sta_mask);
  void setStaticIpMode(bool static_ip);
  void setConfigFileName(String file_name);
  void setApStaMode(bool mode_on);
  void setUseComboMode(bool mode_on);
  void setUseAdminPass(bool pass_on);
  void setUseLed(bool _use, int8_t _pin = -1);
  void setLedOnMode(bool mode_on);

  void setAdminNameEndPass(String a_name, String a_pass);

  uint32_t getCheckTimer();
  bool getLogOnState();
  WiFiMode_t getCurMode();
  String getApSsid();
  String getApPass();
  IPAddress getApIP();
  IPAddress getApGateway();
  IPAddress getApMask();
  String getStaSsid();
  String getStaPass();
  IPAddress getStaIP();
  IPAddress getStaGateway();
  IPAddress getStaMask();
  bool getStaticIpMode();
  String getConfigFileName();
  bool getApStaMode();
  bool getUseComboMode();
  bool getUseAdminPass();
  String getAdminPass();
  String getAdminName();
  bool getUseLed();
  bool getLedOnMode();
  int8_t getLedPin();

  void setApConfig();
  void setApConfig(IPAddress ip);
  void setApConfig(IPAddress ip, IPAddress gateway, IPAddress mask);
  void setStaConfig();
  void setStaConfig(IPAddress ip, IPAddress gateway, IPAddress mask);

#if defined(ARDUINO_ARCH_ESP32)
  bool begin(WebServer *_server, FS *_file_system, String _config_page = "/wifi_config");
#else
  bool begin(ESP8266WebServer *_server, FS *_file_system, String _config_page = "/wifi_config");
#endif
  void tick();
  bool loadConfig();
  bool startWiFi();
  void stopWiFi();

  bool startSoftAP();
  bool startSoftAP(String ssid, String pass);
  bool startSTA();
  bool startSTA(String ssid, String pass);
  bool findSavedAp();

  void checkStaConnection();
};

// ==== LedState class ===============================

class LedState
{
private:
  Ticker blink;
  int16_t pwr_value = 280;
  bool toUp = false;
  int8_t pin;
  bool use_led = true;
#if defined(ARDUINO_ARCH_ESP32)
  int16_t max_pwm = 1023;
  int16_t min_pwm = 0;
#else
  int16_t max_pwm = 280;
  int16_t min_pwm = 0;
#endif
public:
  LedState();
  void setPin(int8_t _pin);
  void init(uint32_t interval, bool force = false);
  void init();
  void stopLed();
  void digitalCheck();
  void analogCheck();
  void setUseLed(bool _use);
};
