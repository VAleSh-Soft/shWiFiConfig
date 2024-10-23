#pragma once

static const char CRYPT_KEY_DEFAULT[] PROGMEM =
    "ZnPjDMpsdM!PQn9,AqQP2CiHxFPmj*P2;oYECpb.;q|>jR:bKpc-d+,_HSoz|-<q";

// ===== shCrypt class ===============================

class shCrypt
{
private:
  String crypt_key;
  bool use_cript = false;
  uint8_t cur_char_of_pass = 0;

  void getNextCharOfKey();                   // Получить следующий символ пароля
  void getStartCharOfKey(uint8_t key_index); // получить стартовый символ пароля
  void _encode(String &data);

public:
  shCrypt();
  void setCryptState(bool state);
  bool getCryptState();
  void setCryptKey(String key);

  // кодирование/декодирование строки data
  // key_index - индекс (0..3), означает, с какой части будет использоваться ключ шифрования, с нулевого символа или с последующих, (для 64-битного ключа соответственно с 0, 16, 32 или 48) 
  // возвращает обработанную строку
  String encode(String data, uint8_t key_index = 0);
};

// ===================================================

void shCrypt::getNextCharOfKey()
{
  if (++cur_char_of_pass >= crypt_key.length())
  {
    cur_char_of_pass = 0;
  }
}

void shCrypt::getStartCharOfKey(uint8_t key_index)
{
  if (key_index >= 4)
  {
    key_index %= 4;
  }

  cur_char_of_pass = crypt_key.length() / 4 * key_index;
}

void shCrypt::_encode(String &data)
{
  uint8_t len = data.length();
  for (uint8_t i = 0; i < len; i++)
  {
    uint8_t _char = (uint8_t)data[i];

    _char ^= (uint8_t)crypt_key[cur_char_of_pass];
    getNextCharOfKey();
    _char ^= (uint8_t)crypt_key[cur_char_of_pass];
    _char = ~_char;
    getNextCharOfKey();
    _char ^= (uint8_t)crypt_key[cur_char_of_pass];

    data.setCharAt(i, (char)_char);
  }
}

// ==============================================

shCrypt::shCrypt() { crypt_key = FPSTR(CRYPT_KEY_DEFAULT); }

void shCrypt::setCryptState(bool state)
{
  use_cript = state;
}

bool shCrypt::getCryptState()
{
  return (use_cript);
}

void shCrypt::setCryptKey(String key)
{
  crypt_key = key;
}

String shCrypt::encode(String data, uint8_t key_index)
{
  if (use_cript && crypt_key != "")
  {
    getStartCharOfKey(key_index);
    _encode(data);
  }

  return (data);
}

// ===================================================

shCrypt crypt_data;
