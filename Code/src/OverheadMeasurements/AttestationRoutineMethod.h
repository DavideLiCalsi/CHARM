/**
 * @file AttestationRoutine.h
 * @author Davide Li Calsi
 * @brief Implements the Attestation Routine. The routine is a classical
 * static RA routine, i.e. it computes a HMAC over the entire Flash Memory
 * and a nonce. We use a 16 bit key and the SHA256 function.
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

#include <STM32FreeRTOS.h>
#include <task.h>

#define KEY_LENGTH 16

extern bool attesting, malicious_flag;
extern PerfCountersMonitor counters;
extern TaskHandle_t htask2, htask4, htask6,htask7;

// The shared key to compute the HMAC
const byte key_method[KEY_LENGTH] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

/*
##########################################
          ATTESTATION ROUTINE
##########################################
*/

/**
 * @brief Attestation routine. At the moment,
 * it simply computes a hash over the flash memory
 * and the nonce.
 * 
 * @param nonce The random nonce used to ensure freshness of the response
 * @return byte* The hash
 */
uint32_t attestation_routine_method(char* nonce, uint32_t size ){

  SHA256HMAC hasher(key,KEY_LENGTH);
  
  byte hash[32];
  char word_as_string[4];
  uint32_t word;
  uint32_t address;

  uint32_t init = DWT->CYCCNT;

  counters.resetTotalCounters();

  #if configUSECASE == USECASE_GENERAL
  vTaskResume(htask2);
  vTaskResume(htask4);
  vTaskResume(htask6);
  vTaskResume(htask7);
  #endif

  PRINTF("BEGIN ATTESTTION\n");

  // Read word by word the flash memory,and update the hasher's internal state
  for (address=FLASH_BEGIN; address<size+FLASH_BEGIN; address+=4){
    
    counters.BeginMonitored();
    word = pgm_read_dword_far(address);
    
    uint32_to_char_array(word, word_as_string);

    hasher.doUpdate(word_as_string,4);
    counters.EndMonitored();
    
  }

  if (!DATA_ONLY)
    Serial.printf("Attested until address: %x\n", address);

  // Concatenate the nonce for freshness
  counters.BeginMonitored();
  hasher.doUpdate(nonce);
  counters.EndMonitored();

  // Finalize the HMAC computation
  counters.BeginMonitored();
  hasher.doFinal(hash);
  counters.EndMonitored();

  uint32_t end = DWT->CYCCNT;

  if (!DATA_ONLY){
    Serial.println("Attestation complete! Obtained HMAC:");
  
    for (int j=0; j<32; ++j){
      Serial.print(hash[j], HEX);
    }

    Serial.printf("\n\nTime taken in ms: %d\n", end-init);
    Serial.println("\n###############################\n");
  }

  PRINTF("%d\n", end-init);

  // Print the counters collected so far
  //counters.printTotalCounters(true, malicious_flag);
  PRINTF("%d\n", end-init);

  // Reset the malicious flag
  malicious_flag = false;

  // The attestation is over, reset all counters
  counters.resetTotalCounters();

  return end-init;
}

static void MethodAttest(void* arg){

  char nonce[20];
  int repetitions=20;
  uint32_t interval= 32* (1<<10); // Attest sizes at intervals of 32 KB

  uint32_t times[16];
  int j,i;

  for ( j=0; j<16;++j)
    times[j]=0;

  for (i=0; i<(FLASH_SIZE_BYTES/interval); ++i){
    
    for (j=0;j<repetitions;++j){
        strcpy(nonce,SEED);
        attesting = !attesting;
        uint32_t time=attestation_routine_method(nonce,interval*(i+1));
        times[i]+=time;
        attesting = !attesting;
    }
  }

  for ( j=0; j<16;++j){
      times[j]/=repetitions;
      Serial.printf("%d - %u clock cycles\n",j,times[j]);
  }

  vTaskDelete(NULL);
}