#pragma once

#include <EEPROM.h>

uint16_t EEPROM_SIZE = 1026;

void start_eeprom(uint16_t size)
{
  EEPROM_SIZE = size + 2; // резервируем два байта для размера строки

  EEPROM.begin(EEPROM_SIZE);
}

void write_string_to_eeprom(char *data, uint16_t address)
{
  uint16_t len = strlen(data);
  EEPROM.put(address, len); // сохраняем размер строки - первые два байта
  address += 2;             // строку пишем следом за размером

  for (uint16_t i = 0; i < len && i < EEPROM_SIZE; i++) // собственно, пишем
  {
    EEPROM.put(address + i, data[i]);
  }

  EEPROM.commit();
}

char *read_string_from_eeprom(uint16_t address)
{
  uint16_t len;
  EEPROM.get(address, len);             // считываем размер строки
  char *data = (char *)malloc(len + 1); // выделяем память под строку - размер строки + нулевой символ
  address += 2;                         // пропускаем первые два байта - там записан размер строки

  for (uint16_t i = 0; i < len && i < EEPROM_SIZE; i++) // читаем строку
  {
    data[i] = EEPROM.read(address + i);
  }
  data[len] = '\0'; // добавляем нулевой символ в конец строки

  return data;
}
