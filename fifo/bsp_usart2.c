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
///�ض���c�⺯��printf������DEBUG_USART���ض�����ʹ��printf����
int fputc(int ch, FILE *f)
{
    /* ����һ���ֽ����ݵ�����DEBUG_USART */
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 1000);

    return (ch);
}

///�ض���c�⺯��scanf������DEBUG_USART����д����ʹ��scanf��getchar�Ⱥ���
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
	/* ʹ�ܴ����ж� */
//	__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);  // �������ݼĴ�����Ϊ���ж�
//	__HAL_UART_DISABLE_IT(&huart2, UART_IT_TXE);  // �������ݼĴ������ж�
//	__HAL_UART_ENABLE_IT(&huart2, UART_IT_TC);    // ������������ж�
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);  // ʹ�ܿ����ж�
	
    /* ʹ��DMA�ж� */
//    __HAL_DMA_ENABLE_IT(huart2.hdmarx, DMA_IT_TC);
//    __HAL_DMA_ENABLE_IT(huart2.hdmatx, DMA_IT_TC);

    /* ʹ�ܴ���DMA���� */
	UART_Start_Receive_DMA(&huart2, Usart2_Rx, USART2_RX_LEN); // ��������DMA���գ��ǵÿ���ѭ��ģʽ��
    if (fifo_init(&fifo_usart2_rx, Usart2_Rx_Buffer, USART2_RX_LEN) == -1)
    {
        Error_Handler(); // ���� 2 ���ݴη�
    }

    /* ʹ�ܴ���DMA���� */
    __HAL_DMA_DISABLE(huart2.hdmatx);  // �رմ���DMA����ͨ�� �����ÿ���ѭ��ģʽ��
    if (fifo_init(&fifo_usart2_tx, Usart2_Tx_Buffer, USART2_TX_LEN) == -1)
    {
        Error_Handler(); // ���� 2 ���ݴη�
    }
}


void USART2_IRQHandler_callback(void)
{
	if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE) == SET) //�����ж�
	{
		volatile uint32_t num = 0;
		
		num = huart2.Instance->SR; //���RXNE��־
		num = huart2.Instance->DR; //��USART_IT_IDLE��־
        __HAL_DMA_CLEAR_FLAG(huart2.hdmarx, DMA_FLAG_GL6); 

		__HAL_DMA_DISABLE(huart2.hdmarx);  // �رմ���DMA����ͨ��
		
		num = USART2_RX_LEN - __HAL_DMA_GET_COUNTER(huart2.hdmarx);  //! ��ȡDMA��δ��������ݸ�����NDTR�Ĵ��������ο����Ĳο��ֲ� ��DMA_Channel_TypeDef��  �����ͬ��оƬHAL�����涨��������е㲻ͬ
		fifo_write_buff(&fifo_usart2_rx, Usart2_Rx, num);
		
		HAL_UART_Receive_DMA(&huart2, Usart2_Rx, USART2_RX_LEN); // ����DMA����
	}
}



/**
  * @brief          ���ڶ� + DMA ����
  * @param[in]      *data
  * @param[in]      len
  * @retval         void
  */
uint32_t usart2_dma_send(uint8_t *data, uint16_t len)
{	
	uint32_t result = fifo_write_buff(&fifo_usart2_tx, data, len); //�����ݷ���ѭ��������
	
    if (result != 0 && huart2.gState == HAL_UART_STATE_READY)
    {
        len = fifo_read_buff(&fifo_usart2_tx, Usart2_Tx, USART2_TX_LEN); //��ѭ����������ȡ����
		
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
  * @brief          ��ȡ���ڽ��յ�����
  * @param[in]      *data: ����ָ��
  * @param[in]      len: ���ݳ���
  * @retval         �ɹ���ȡ�����ݳ���
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

