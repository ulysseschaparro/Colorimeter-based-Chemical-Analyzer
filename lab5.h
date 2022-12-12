//Ulysses Chaparro 1001718774

#ifndef LAB5_H_
#define LAB5_H_

#include "lab4.h"
#define DEBUG 0

// Bitband aliases
#define RED_LED      (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))
#define GREEN_LED    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))

// PortF masks
#define GREEN_LED_MASK 8
#define RED_LED_MASK 2

void parseFields(USER_DATA *data);
char* getFieldString(USER_DATA *data, uint8_t fieldNumber);
int32_t getFieldInteger(USER_DATA *data, uint8_t fieldNumber);
bool str_cmp(char *string1, char *string2);
bool isCommand(USER_DATA *data, const char strCommand[], uint8_t minArguments);

#endif
