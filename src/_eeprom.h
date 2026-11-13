#pragma once

#include <EEPROM.h>

uint16_t wfc_eeprom_size = 1026;

void start_eeprom(uint16_t size)
{
  EEPROM.end();

  wfc_eeprom_size = size + 2; // резервируем два байта для размера строки
  if (wfc_eeprom_size > 0x1000)
  {
    wfc_eeprom_size = 0x1000; // максимально допустимый размер EEPROM (4096 байт)
  }
  
  EEPROM.begin(wfc_eeprom_size);
}

uint16_t get_json_size(uint16_t address)
{
  uint16_t result;
  EEPROM.get(address, result);

  return (result);
}

void write_string_to_eeprom(char *data, uint16_t address)
{
  uint16_t len = strlen(data);
  EEPROM.put(address, len); // сохраняем размер строки - первые два байта
  address += 2;             // саму строку пишем следом за размером

  for (uint16_t i = 0; i < len && (address + i) < wfc_eeprom_size; i++) // собственно, пишем
  {
    EEPROM.put(address + i, data[i]);
  }
  EEPROM.commit();
}

char *read_string_from_eeprom(uint16_t address)
{
  uint16_t len = get_json_size(address); // считываем размер строки
  if (len >= wfc_eeprom_size - 2)
  {
    len = wfc_eeprom_size - 2;
  }

  char *data = (char *)calloc(len + 1, sizeof(char)); // выделяем память под строку - размер строки + нулевой символ
  if (data)
  {
    address += 2; // пропускаем первые два байта - там записан размер строки

    for (uint16_t i = 0; i < len && (address + i) < wfc_eeprom_size; i++) // читаем строку
    {
      data[i] = EEPROM.read(address + i);
    }
    data[len] = '\0'; // добавляем нулевой символ в конец строки
  }

  return data;
}
