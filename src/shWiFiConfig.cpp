#include <Arduino.h>
#include "shWiFiConfig.h"
#include "extras/c_page.h"
#include "functions.h"

// ==== shWiFiConfig class ===========================

shWiFiConfig::shWiFiConfig()
{
  set_config();
}

shWiFiConfig::shWiFiConfig(String &adm_name, String &adm_pass)
{
  set_config();
  setAdminData(adm_name, adm_pass);
}

void shWiFiConfig::setCheckTimer(uint32_t _timer) { checkTimer = _timer; }

void shWiFiConfig::setLogOnState(bool log_on, Print *_serial)
{
  logOnState = log_on;
  serial = (logOnState) ? _serial : NULL;
}

void shWiFiConfig::setApSsid(String &ap_ssid) { apSsid = ap_ssid; }

void shWiFiConfig::setApPass(String &ap_pass) { apPass = ap_pass; }

void shWiFiConfig::setApIP(IPAddress &ap_ip) { apIP = ap_ip; }

void shWiFiConfig::setApGateway(IPAddress &ap_gateway) { apGateway = ap_gateway; };

void shWiFiConfig::setApMask(IPAddress &ap_mask) { apMask = ap_mask; }

void shWiFiConfig::setStaSsid(String &sta_ssid) { staSsid = sta_ssid; }

void shWiFiConfig::setStaPass(String &sta_pass) { staPass = sta_pass; }

void shWiFiConfig::setStaIP(IPAddress &sta_ip) { staIP = sta_ip; }

void shWiFiConfig::setStaGateway(IPAddress &sta_gateway) { staGateway = sta_gateway; }

void shWiFiConfig::setStaMask(IPAddress &sta_mask) { staMask = sta_mask; }

void shWiFiConfig::setStaticIpMode(bool static_ip) { staticIP = static_ip; }

void shWiFiConfig::setConfigFileName(String &file_name) { fileName = file_name; }

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

void shWiFiConfig::setLedPwmLevels(int16_t _max, int16_t _min)
{
  led.setLevelsForPWM(_max, _min);
}

void shWiFiConfig::setStaSsidData(String &ssid, String &pass)
{
  staSsid = ssid;
  staPass = pass;
}

void shWiFiConfig::setApSsidData(String &ssid, String &pass)
{
  apSsid = ssid;
  apPass = pass;
}

void shWiFiConfig::setAdminData(String &name, String &pass)
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

void shWiFiConfig::setApConfig(IPAddress &ip)
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

void shWiFiConfig::setStaConfig(IPAddress &ip, IPAddress &gateway, IPAddress &mask)
{
  set_sta_config(ip, gateway, mask);
}

void shWiFiConfig::begin(shWebServer *_server, FS *_file_system, const String &_config_page)
{
  file_system = _file_system;
  use_eeprom = false;

  _begin(_server, _config_page);
}

void shWiFiConfig::begin(shWebServer *_server, const String &_config_page)
{
  file_system = NULL;
  use_eeprom = true;
  start_eeprom(CONFIG_SIZE);

  _begin(_server, _config_page);
}

void shWiFiConfig::tick()
{
  static uint32_t timer = millis();
  if (millis() - timer > checkTimer)
  {
    timer = millis();
    checkStaConnection();
  }

  http_server->handleClient();
  delay(1);
}

bool shWiFiConfig::loadConfig()
{
  return (load_config());
}

bool shWiFiConfig::saveConfig()
{
  return (save_config());
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

bool shWiFiConfig::startSoftAP(String &ssid, String &pass)
{
  return (start_ap(ssid, pass));
}

bool shWiFiConfig::startSTA()
{
  return (start_sta(staSsid, staPass));
}

bool shWiFiConfig::startSTA(String &ssid, String &pass)
{
  return (start_sta(ssid, pass));
}

bool shWiFiConfig::findSavedAp()
{
  return (find_ap(staSsid));
}

bool shWiFiConfig::findAp(String &ssid)
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
      WFC_PRINTLN(F("WiFi connection lost"));
      // если пропала связь с роутером, перейти в режим точки доступа
      WiFi.disconnect();
      if (curMode == WIFI_AP_STA)
      {
        WiFi.softAPdisconnect();
      }
      if (!start_ap(apSsid, apPass))
      {
        ESP.restart();
      }
    }
  }
  else
  {
    // пытаться искать и подключаться к сети только если пароль не помечен неверным
    if (!badPassword && find_ap(staSsid))
    {
      // если наша сеть найдена, подключиться к ней
      if (curMode == WIFI_AP)
      {
        WiFi.softAPdisconnect();
      }
      // если не удалось, перейти в режим точки доступа
      if (!start_sta(staSsid, staPass, false))
      {
        start_ap(apSsid, apPass);
      }
    }
  }
}

// ==== LedState class ===============================

void changeLedState()
{
  led.analogCheck();
}

void changeLedState1()
{
  led.digitalCheck();
}

void LedState::writeLed(int16_t _value)
{
#if defined(ARDUINO_ARCH_ESP32) && ESP_ARDUINO_VERSION_MAJOR < 3
  ledcWrite(0, _value);
#else
  analogWrite(pin, _value);
#endif
}

#if defined(ARDUINO_ARCH_ESP32)
void LedState::setPwmData()
{
#if ESP_ARDUINO_VERSION_MAJOR < 3
  ledcSetup(0, 1000, 10);
  ledcAttachPin(pin, 0);
#else
  ledcAttach(pin, 1000, 10);
#endif
}
#endif

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
#if defined(ARDUINO_ARCH_ESP32)
    setPwmData();
#endif
    toUp = false;
    writeLed(max_pwm);
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
    if (use_led)
    {
      pwr_value = max_pwm;
      toUp = false;
#if defined(ARDUINO_ARCH_ESP32)
      setPwmData();
      blink.attach_ms(3, changeLedState);
#else
      blink.attach_ms(10, changeLedState);
#endif
      writeLed(max_pwm);
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
    pwr_value = (toUp) ? min_pwm : max_pwm;
    toUp = !toUp;
    writeLed(pwr_value);
  }
}

void LedState::analogCheck()
{
  if (pin >= 0)
  {
    writeLed(pwr_value);

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

void LedState::setLevelsForPWM(int16_t _max, int16_t _min)
{

  max_pwm = (_max < 1024) ? ((_max >= 0) ? _max : 0) : 1023;
  min_pwm = (_min < 1024) ? ((_min >= 0) ? _min : 0) : 1023;
}
