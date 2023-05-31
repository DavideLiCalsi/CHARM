#ifndef UTILITIES_HEADER
#define UTILITIES_HEADER

/*
##########################################
            UTILITY FUNCTIONS
##########################################
*/

/*Convert a uint32_t data to a string of 4 chars*/
void uint32_to_char_array(uint32_t source, char* dest){

  char b0 = source & 0x000000ff;
  char b1 = (source & 0x0000ff00) >> 8;
  char b2 = (source & 0x00ff0000) >> 16;
  char b3 = (source & 0xff000000) >> 24;

  dest[0]=b3;
  dest[1]=b2;
  dest[2]=b1;
  dest[3]=b0;
}
#endif