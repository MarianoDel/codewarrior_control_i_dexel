//#include "pub_def.h"
#include "utils.h"
//#include "application.h"


void hex_to_char (char *c, unsigned short value) {
  
  unsigned char v,i;
  c += 4;
  *c = 0;
  c--;
  
  for (i=0; i < 4;i++) {
    
    v =(value & 0x0f);
    if (v > 9)
      *c = v + 55;
    else
      *c = v + '0';
    
    c--;
    value >>= 4;
  }
}
  
unsigned short char_to_hex (char *c){
	unsigned char i;
  unsigned long value;
  
  for (i=0; i < 4; i++){
    
    value <<= 4;
    if (*c < 'A')
      value |= *c - '0';
    else
      value |= *c - 55;
    
    c++;
  }
  return (unsigned short) value;
}
