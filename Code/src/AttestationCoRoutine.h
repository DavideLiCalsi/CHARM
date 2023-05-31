/**
 * @file AttestationRoutine.h
 * @author Davide Li Calsi
 * @brief Implements the attestation routine
 * @version 0.1
 * @date 2022-04-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <Crypto.h>
#include <pgmspace.h>

#include "constants.h"
#include "Utilities.h"
#include "constants.h"

#if USE_FREERTOS == 1
#include <STM32FreeRTOS.h>
#include "croutine.h"
#endif

extern bool attesting, malicious_flag;
extern PerfCountersMonitor counters;

/*
##########################################
          ATTESTATION ROUTINE
##########################################
*/

/*Attestation routine. At the moment, it is nothing too fancy:
it simply scans the memory linearly and hashes it using sha256.*/
byte* attestation_routine(char* nonce){

  SHA256 hasher;
  
  byte hash[32];
  char word_as_string[4];
  uint32_t word;
  uint32_t address;

  uint32_t init = millis();
  counters.resetTotalCounters();

  //Read word by word the flash memory,and update the hasher's internal state
  for (address=FLASH_BEGIN; address<FLASH_SIZE_BYTES+FLASH_BEGIN; address+=4){
    
    word = pgm_read_dword_far(address);
    
    uint32_to_char_array(word, word_as_string);

    hasher.doUpdate(word_as_string,4);
  }

  if (!DATA_ONLY)
    Serial.printf("Attested until address: %x\n", address);

  //Concatenate the nonce for freshness
  hasher.doUpdate(nonce);

  //Finalize the digest computation
  hasher.doFinal(hash);

  uint32_t end = millis();

  if (!DATA_ONLY){
    Serial.println("Attestation complete! Obtained hash:");
  
    for (int j=0; j<32; ++j){
      Serial.print(hash[j], HEX);
    }

    Serial.printf("\n\nTime taken in ms: %d\n", end-init);
    Serial.println("\n###############################\n");
  }

  counters.printTotalCounters(true, malicious_flag);
  Serial.printf("%d\n", end-init);

  //Reset the malicious flag
  malicious_flag = false;

  counters.resetTotalCounters();

  return hash;
}

#if USE_FREERTOS == 1
void AttestCoRoutine(CoRoutineHandle_t xHandle, UBaseType_t uxIndex){

    crSTART(xHandle);

    while(true){
    
        attesting = !attesting;
        attestation_routine(SEED);
        attesting = !attesting;
        crDELAY(xHandle,10000);
    }

    crEND();
}
#endif