#ifndef __BSP_USART2_H__
#define __BSP_USART2_H__
#include "struct_typedef.h"
#include "main.h"
#include "stdio.h"
#include <string.h>


void usart_send_string(uint8_t *str);
int fputc(int ch, FILE *f);
int fgetc(FILE *f);


void bsp_usart2_init(void);
uint32_t usart2_dma_send(uint8_t *data, uint16_t len);
uint32_t usart2_read(uint8_t *data, uint16_t len);
uint32_t usart2_fifo_get_full(void);
	
#endif /* __BSP_USART2_H__ */
