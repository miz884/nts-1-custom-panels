#include "flash.h"
#include "sequencer.h"
#include "stm32f0xx_hal.h"

// cf. https://www.stmcu.jp/design/document/reference_manual/51370/
#define PAGE60_START_ADDR 0x0800F000

void save_config() {
  // Unlock flash.
  HAL_FLASH_Unlock();
  // Erase the page 60.
  FLASH_EraseInitTypeDef erase;
  erase.TypeErase = FLASH_TYPEERASE_PAGES;
  erase.PageAddress = PAGE60_START_ADDR;
  erase.NbPages = 1;
  uint32_t pageError = 0;
  if (HAL_FLASHEx_Erase(&erase, &pageError) != HAL_OK) {
  }
  // Write to the page.
  uint32_t *data = (uint32_t*) &seq_config;
  for (uint32_t a = PAGE60_START_ADDR; a < PAGE60_START_ADDR + sizeof(seq_config); a += 4) {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, a, *data) != HAL_OK) {
    }
    data++;
  }
  // Lock flash.
  HAL_FLASH_Lock();
}

void load_config() {
  // Load from flash.
  memcpy(&seq_config, (uint8_t*) PAGE60_START_ADDR, sizeof(seq_config));
}
