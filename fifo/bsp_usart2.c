#include "bsp_usart2.h"
#include "usart.h"
#include "fifo_buff.h"

extern UART_HandleTypeDef huart2;
//extern DMA_HandleTypeDef hdma_usart2_rx;
//extern DMA_HandleTypeDef hdma_usart2_tx;


#if 1
void usart_send_string(uint8_t *str)
{
    unsigned int k = 0;
    do
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)(str + k), 1, 1000);
        k++;
    }
    while (*(str + k) != '\0');

}
///重定向c库函数printf到串口DEBUG_USART，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
    /* 发送一个字节数据到串口DEBUG_USART */
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 1000);

    return (ch);
}

///重定向c库函数scanf到串口DEBUG_USART，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
    int ch;
    HAL_UART_Receive(&huart2, (uint8_t *)&ch, 1, 1000);
    return (ch);
}

#endif





#define USART2_RX_LEN   256
#define USART2_TX_LEN   512
static uint8_t Usart2_Rx[USART2_RX_LEN] = {0};
static uint8_t Usart2_Tx[USART2_TX_LEN] = {0};
static uint8_t Usart2_Rx_Buffer[USART2_RX_LEN] = {0};
static uint8_t Usart2_Tx_Buffer[USART2_TX_LEN] = {0};

fifo_rx_def fifo_usart2_rx;
fifo_rx_def fifo_usart2_tx;



void bsp_usart2_init(void)
{
	/* 使能串口中断 */
//	__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);  // 接收数据寄存器不为空中断
//	__HAL_UART_DISABLE_IT(&huart2, UART_IT_TXE);  // 传输数据寄存器空中断
//	__HAL_UART_ENABLE_IT(&huart2, UART_IT_TC);    // 开启传输完成中断
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);  // 使能空闲中断
	
    /* 使能DMA中断 */
//    __HAL_DMA_ENABLE_IT(huart2.hdmarx, DMA_IT_TC);
//    __HAL_DMA_ENABLE_IT(huart2.hdmatx, DMA_IT_TC);

    /* 使能串口DMA接收 */
	UART_Start_Receive_DMA(&huart2, Usart2_Rx, USART2_RX_LEN); // 开启串口DMA接收（记得开启循环模式）
    if (fifo_init(&fifo_usart2_rx, Usart2_Rx_Buffer, USART2_RX_LEN) == -1)
    {
        Error_Handler(); // 必须 2 的幂次方
    }

    /* 使能串口DMA发送 */
    __HAL_DMA_DISABLE(huart2.hdmatx);  // 关闭串口DMA发送通道 （不用开启循环模式）
    if (fifo_init(&fifo_usart2_tx, Usart2_Tx_Buffer, USART2_TX_LEN) == -1)
    {
        Error_Handler(); // 必须 2 的幂次方
    }
}


void USART2_IRQHandler_callback(void)
{
	if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE) == SET) //空闲中断
	{
		volatile uint32_t num = 0;
		
		num = huart2.Instance->SR; //清除RXNE标志
		num = huart2.Instance->DR; //清USART_IT_IDLE标志
        __HAL_DMA_CLEAR_FLAG(huart2.hdmarx, DMA_FLAG_GL6); 

		__HAL_DMA_DISABLE(huart2.hdmarx);  // 关闭串口DMA发送通道
		
		num = USART2_RX_LEN - __HAL_DMA_GET_COUNTER(huart2.hdmarx);  //! 获取DMA中未传输的数据个数，NDTR寄存器分析参考中文参考手册 （DMA_Channel_TypeDef）  这个不同的芯片HAL库里面定义的命名有点不同
		fifo_write_buff(&fifo_usart2_rx, Usart2_Rx, num);
		
		HAL_UART_Receive_DMA(&huart2, Usart2_Rx, USART2_RX_LEN); // 启动DMA接收
	}
}



/**
  * @brief          串口二 + DMA 发送
  * @param[in]      *data
  * @param[in]      len
  * @retval         void
  */
uint32_t usart2_dma_send(uint8_t *data, uint16_t len)
{	
	uint32_t result = fifo_write_buff(&fifo_usart2_tx, data, len); //将数据放入循环缓冲区
	
    if (result != 0 && huart2.gState == HAL_UART_STATE_READY)
    {
        len = fifo_read_buff(&fifo_usart2_tx, Usart2_Tx, USART2_TX_LEN); //从循环缓冲区获取数据
		
		if (HAL_UART_Transmit_DMA(&huart2, Usart2_Tx, len) != HAL_OK)
		{
			//Error_Handler();
		}
    }
	
	if (result == len)
	{
		return len;
	}
	else
	{
		return result;
	}
}


/**
  * @brief          读取串口接收的数据
  * @param[in]      *data: 数据指针
  * @param[in]      len: 数据长度
  * @retval         成功读取的数据长度
  */
uint32_t usart2_read(uint8_t *data, uint16_t len)
{
    uint32_t result = 0;

    if (data != NULL)
        result = fifo_read_buff(&fifo_usart2_rx, data, len);

    return result;
}

uint32_t usart2_fifo_get_full(void)
{
    return fifo_get_full(&fifo_usart2_rx);
}

