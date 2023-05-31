#include "constants.h"

#include <string.h>
#include <math.h>

// Pointer to the malware in FLASH
static void* malware_ptr;

// Pointer to the benign code
void* benign_ptr;

/*
##########################################
            MALICIOUS INT.
##########################################
*/

/**
 * @brief Get the size of the malware you
 * that should relocate
 * 
 * @return int The malware size
 */
int get_malware_size(){

  int index = random(0,3);
  int size=0;

  while (size==0){

      switch (index) {
        case 0:
        size=configMALWARE_SIZE0;
        break;
  
        case 1:
        size=configMALWARE_SIZE1;
        break;

        case 2:
        size=configMALWARE_SIZE2;
        break;

        case 3:
        size=configMALWARE_SIZE3;
        break;
  
        default:
        size=0;
        break;
    }

  }
  
  return size;

}

/* 
Function that gets a random offset to choose where to
store malware in flash memory. I choose a random location to 
avoid messing with the flash memory. For easier memory management,
the offset is a multiple of a flash page.
*/
uint32_t get_flash_address(){
  int bank = FLASH_BANK_2;
  int page_number = random(40,100);
  uint32_t address = FLASH_BEGIN + (bank-1)*FLASH_BANK_SIZE +page_number*FLASH_PAGE_SIZE;
  PRINTF("Random address is %x, i.e. page %d\n",address,page_number);
  return address;

}

/**
 * @brief Erase some pages from the flash memory. The first page will be the one that contains
 * the address specified as a parameter
 * 
 * @param address The address that we want to erase
 * @param num_of_pages The number of pages erased
 */
void EraseFlash(uint32_t address, uint32_t num_of_pages){

  uint32_t page_num, bank;

  counters.BeginMonitored();
  bank = (address < (FLASH_BANK_SIZE + FLASH_BASE) ) ? FLASH_BANK_1 : FLASH_BANK_2;
  page_num = ( (address - FLASH_BASE) / FLASH_PAGE_SIZE ) - (bank == FLASH_BANK_1 ? 0:127);
  page_num-=1;
  counters.EndMonitored();
  uint32_t error = 0;

  FLASH_EraseInitTypeDef FLASH_EraseInitStruct = {
    .TypeErase = FLASH_TYPEERASE_PAGES,
    .Banks = bank,
    .Page = page_num,
    .NbPages = num_of_pages, 
  };
  
  PRINTF("Erasing %d pages starting from page %d of bank %d\n",num_of_pages,page_num,bank);

  // clear all flags before you write it to flash
  counters.BeginMonitored();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS );

  HAL_FLASHEx_Erase(&FLASH_EraseInitStruct, &error);
  counters.EndMonitored();

  if ( error != 0xffffffff){
    PRINTF("Error %x while erasing\n", error);
  }
  else{
    PRINTF("All pages correctly erased\n");
  }

}

/**
 * @brief Copy size bytes from src to dst, in flash memory
 * 
 * @param dst Destination address in flash memory
 * @param src Source address in flash memory
 * @param size Number of bytes to copy
 */
void ProgramFlash(uint32_t dst, uint32_t src, int size){

  for (int i=0; i<size; i+=8){

    uint64_t data;
    uint64_t* data_ptr = (uint64_t*) (src + i);

    counters.BeginMonitored();

    data = *data_ptr;
    // clear all flags before you write it to flash
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS );

    int status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,dst + i,data);

    counters.EndMonitored();

    int c=0;
    if ( status != HAL_OK && c==0){
      PRINTF("Error during programming dst address 0x%x from src address %x\n", dst+i, src+i);
      c++;
    }
      

    int error = HAL_FLASH_GetError();

    if ( error != 0)
      PRINTF("Error code: %x\n", error);
  }

}

void malicious_behaviour_relocation(int size){

  counters.BeginMonitored();
  if ( HAL_FLASH_Unlock() != HAL_OK )
    Serial.println("Error while unlocking flash");
  counters.EndMonitored();

  counters.BeginMonitored();
  uint32_t new_address = get_flash_address();
  int num_pages = ( size + ( FLASH_PAGE_SIZE - (size % FLASH_PAGE_SIZE) ) ) / FLASH_PAGE_SIZE;
  counters.EndMonitored();

  // Erase the flash memory to host the new malware
  EraseFlash(new_address,num_pages);

  // Copy the malware to its new address in flash
  ProgramFlash(new_address, (uint32_t) malware_ptr,size);
  uint32_t old_location = (uint32_t) malware_ptr;
  malware_ptr = (void*) new_address;

  // Erase the flash memory to delete the previous malware
  EraseFlash(old_location,num_pages);

}

/*
Function that simulates a transient malware. This function
erases the malware and copies benign code to its previous location
*/
void malicious_behaviour_transient(int size){

  int num_pages = ceil(1.0 *size / FLASH_PAGE_SIZE);

  counters.BeginMonitored();
  if ( HAL_FLASH_Unlock() != HAL_OK )
    Serial.println("Error while unlocking flash");
  counters.EndMonitored();
  

  counters.BeginMonitored();
  // Erase the new address in flash to allow relocation
  uint32_t buf_ptr =  (uint32_t) malware_ptr;
  EraseFlash((uint32_t) buf_ptr,num_pages);
  counters.EndMonitored();

  // Copy benign code from the flash beginning to the 
  // previous malware location
  uint32_t BenignAddress = FLASH_BEGIN;
  ProgramFlash(buf_ptr,BenignAddress,size);

  counters.BeginMonitored();
  HAL_FLASH_Lock();
  counters.EndMonitored();
  
}

/**
 * @brief Set the up transient malware.
 * Practically, it writes some bytes in flash
 * (copies them from a ram buffer).
 * That represents the transient malware.
 * 
 * 
 */
void setup_malware(int size){

  // Select a pointer in flash to host the transient malware
  malware_ptr =  (void*) get_flash_address();

}

/*Callback for the malicious interrupt*/
static void malicious_interrupt(void* args){

  int size;

  if (configUSECASE == USECASE_OVH){
    Serial.printf("====");
    vTaskSuspend(NULL);
  }
    
  
  while(1){
    
    PRINTF("RELOCATING\n");
    size = get_malware_size();

    if ( MAL_INT_EN && attesting){
      malicious_flag = true;

      setup_malware(size);

      if ( strcmp(configRELOCATION_TYPE,"TRANSIENT") == 0)
        malicious_behaviour_transient(size);
      else
        malicious_behaviour_relocation(size);
    }

    PRINTF("DONE RELOCATING\n");
    vTaskDelay(20000);
  }
  
  vTaskDelete(NULL);
}
