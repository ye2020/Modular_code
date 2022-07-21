#ifndef __USART2_H__
#define __USART2_H__
#include "struct_typedef.h"


void usart2_init(uint8_t usart_NVIC);
extern void (*usart2_callback)(uint8_t *, uint16_t);


#endif


