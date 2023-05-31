/**
 * @file CryptoLock.h
 * @author Davide Li Calsi
 * @brief Contains utilities for the Locker's cryptographic operations
 * @version 0.1
 * @date 2022-04-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <Crypto.h>
#include <Arduino.h>
#include <string.h>

#include "constants.h"


#define USER_NAME_SIZE 20
#define USERS_IN_DB 5

/**
 * @brief Error codes to be returned
 * 
 */
#define LOCK_SUCCESS_CLOSE -0x1
#define LOCK_SUCCESS_OPEN 0x0
#define LOCK_BAD_FORMATTED_REQUEST 0x1
#define LOCK_UNAUTHORIZED 0x2
#define LOCK_IMPOSSIBLE_REQUEST 0x3

#if DATA_ONLY == 0
#define PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

const uint8_t aes_key[32] = {0x78, 0x2F, 0x41, 0x3F, 0x44, 0x28, 0x47, 0x2B, 0x4B, 0x62, 0x50, 0x65, 0x53, 0x68, 0x56, 0x6D, 0x59, 0x71, 0x33, 0x74, 0x36, 0x77, 0x39, 0x7A, 0x24, 0x42, 0x26, 0x45, 0x29, 0x48, 0x40, 0x4D};
const uint8_t IV[16] = {0x51,0x66,0x54,0x6A,0x57,0x6E,0x5A,0x72,0x34,0x75,0x37,0x78,0x21,0x41,0x25,0x44};
uint8_t target_hmac[32] = {0x57,0x03,0x34,0x68,0xd6,0x18,0xfb,0x6c,0xde,0x71,0xa1,0xa5,0x1a,0x9e,0xcb,0x14,0xca,0x8e,0xd8,0x81,0x85,0x38,0xa2,0x2d,0x4b,0xb8,0xdd,0x7d,0xde,0xba,0x39,0xf7};
static const char* users[USERS_IN_DB] = {"Vittorio","Davide", "Goku", "Pippo", "Samurai"};

static uint8_t buffer[64];

static bool open = false;

struct request{
    bool open;
    bool close;
    char user[USER_NAME_SIZE];
    byte hmac[32];
};

typedef struct request Request;

/**
 * @brief Copy dest to src
 * 
 * @param src 
 * @param dst 
 * @param size 
 */
void hashcpy(uint8_t* src, uint8_t* dst, int size){

    for ( int i=0; i<size;++i){

        dst[i]=src[i];;
    }
}

/**
 * @brief Print and hex buffer's content
 * 
 * @param buffer Buffer to print
 * @param size its size
 */
void PrintHex(uint8_t* buffer, int size){
    Serial.printf("Hex request");
    for(int i=0; i<size; ++i){
        Serial.printf("%02x", buffer[i]);
    }

    Serial.printf("\n");
}

/**
 * @brief Compare binary strings b1 and b2
 * 
 * @param b1 
 * @param b2 
 * @param size Size of strings to compare
 * @return true If two strings are equal
 * @return false Otherwise
 */
bool hashcmp(uint8_t* b1, uint8_t* b2, int size){

    for ( int i=0; i<size;++i){

        if(b1[i]!=b2[i])
            return false;
    }

    return true;
}


/**
 * @brief Serializes the smart lock request req
 * 
 * @param req The request to serialize
 * @param s_req Buffer containing the serialized request
 */
void SerializeRequest(Request req, byte* s_req){

    bool end_reached;

    s_req[0] = req.open ? 1:0;
    s_req[1] = req.close ? 1:0;

    for (int i =2, end_reached = false; i<2 + USER_NAME_SIZE; ++i){

        if ( !end_reached )
            s_req[i] = (byte) req.user[i-2];
        else
            s_req[i] = 0;

        if ( s_req[i] == 0 )
            end_reached = true;
    }

    for (int j = 2 + USER_NAME_SIZE; j < 2+ USER_NAME_SIZE + 32; ++j){
        s_req[j] = req.hmac[j-2-USER_NAME_SIZE];
    }
}

/**
 * @brief Check if the user is valid
 * 
 * @param user The user issuing the request
 * @return true If user is allowed to act on the door
 * @return false Otherwise
 */
bool ValidUser(char* user){

    for(int i=0; i<USERS_IN_DB; ++i){

        if ( !strcmp(users[i], user) )
            return true;
    }

    return false;
}

/**
 * @brief Deserializes a serialized request
 * 
 * @param s_req The serialized request
 * @return Request the deserialized request
 */
Request DeserializeRequest(byte* s_req){
     
     Request ret;

     ret.open = s_req[0] == 1 ? 1:0; 
     ret.close = s_req[1] == 1 ? 1:0; 
     
     for (int i =2; i<2 + USER_NAME_SIZE; ++i){

        ret.user[i-2] = (char) s_req[i];
    }

    for (int j = 2 + USER_NAME_SIZE; j < 2+ USER_NAME_SIZE + 32; ++j){
        ret.hmac[j-2-USER_NAME_SIZE] = s_req[j];
    }
    
    return ret;
}

/**
 * @brief Verify the input request
 * 
 * @param s_req The input request
 * @return int 0 for success, and error code otherwise
 */
int VerifyRequest(byte* s_req){
    
    AES encrypter(aes_key,IV, AES::AES_MODE_256, AES::CIPHER_DECRYPT);
    uint8_t d_req[2+USER_NAME_SIZE+32];
    encrypter.process((uint8_t*) s_req, d_req,64);

    Request req = DeserializeRequest((byte*) d_req);

    PRINTF("Verifying request\n");
    PRINTF("Open: %d\n", req.open==1);
    PRINTF("Close: %d\n", req.close==1);
    PRINTF("Name: %s\n", req.user);
    PRINTF("HMAC: %s\n\n", req.hmac);

    if ( req.open && req.close)
        return LOCK_BAD_FORMATTED_REQUEST;

    if ( req.open && open || req.close && !open )
        return LOCK_IMPOSSIBLE_REQUEST;

    if ( !hashcmp(req.hmac, target_hmac,32) || !ValidUser(req.user))
        return LOCK_UNAUTHORIZED;


    PRINTF("Request ok\n");
    return req.open ? LOCK_SUCCESS_OPEN: LOCK_SUCCESS_CLOSE;
}

/**
 * @brief Simulate a request issued to the lock
 * 
 * @param user The user trying to issue the request
 * @param open_close True if you open the lock, false otherwise
 */
void IssueRequest(char* user, bool open_close){

    Request req;

    req.open = open_close;
    req.close = !open_close;
    strcpy(req.user, user);
    hashcpy(target_hmac, (uint8_t*) req.hmac, 32);

    PRINTF("Issuing %s request from %s\n", open_close ? "Open":"Close", user);

    byte s_req[2+USER_NAME_SIZE+32];
    SerializeRequest(req,s_req);

    AES encrypter(aes_key, IV,AES::AES_MODE_256, AES::CIPHER_ENCRYPT);
    int msg_size = encrypter.calcSizeAndPad(2+USER_NAME_SIZE+32);
    uint8_t e_req[512];

    encrypter.process((uint8_t*) s_req, e_req,msg_size);
    
    hashcpy( e_req,buffer,64);
}