/**
  *****************************东莞理工学院ACE实验室 *****************************
  * @file       upper_machine.c/h
  * @brief      上位机通讯
  * @note       
  * @history    2021.07.08
  *
  @verbatim   
  ==============================================================================
  利用 VOFA+ 上位机进行调试和pid参数调节
    - 字节接收区请勾选十六进制，以十六进制方式打印字符，否则只能打印乱码。
  使用 VOFA+ 中的 JustFloat 协议：
    - 协议特点
  本协议是小端浮点数组形式的字节流协议，纯十六进制浮点传输，节省带宽。
  此协议非常适合用在通道数量多、发送频率高的时候。  

  ==============================================================================
  @endverbatim
  *****************************东莞理工学院ACE实验室 *****************************
  */
#include "upper_machine.h"
#include "usart2.h"
#include "fifo_buff.h"
#include <string.h>
#include <stdlib.h>
#include "maths.h"

extern fifo_rx_def fifo_usart_rx_2;
static fifo_rx_def *pfifo_2 = &fifo_usart_rx_2;

void upper_machine_communication(void)
{
    usart2_init(4);
}

void upper_machine_usart2_callback(uint8_t *usart2_data, uint16_t Len)
{
    if (pfifo_2 != 0)
    {
        // Len为当前接收索引
        pfifo_2->in += ((pfifo_2->last_cnt - Len) & (pfifo_2->size - 1)); //更新in
        pfifo_2->last_cnt = Len;

        if ((pfifo_2->in - pfifo_2->out) > pfifo_2->size)
        {
            pfifo_2->out = pfifo_2->in; // 清空缓存，注意赋值顺序，pfifo->in = pfifo->out 是错误的
            pfifo_2->error |= FIFO_DMA_ERROR_RX_FULL;
        }
    }
    else
    {
        pfifo_2->error |= FIFO_DMA_ERROR_RX_POINT_NULL;
    }
}

/* pid参数接收 */
void pid_parameter_receive(pid_parameter_t *pid_speed, pid_parameter_t *pid_position)
{
    uint16_t i = 0;
    uint8_t buff_read[32];
    char buff[16];
    char *pid_p;

    uint32_t length = fifo_read_buff(pfifo_2, buff_read, sizeof(buff_read));
    if (length)
    {
        if ((uint8_t *)strstr((char *)buff_read, (char *)"nep p_Kp:"))
        {
            pid_p = strstr((char *)buff_read, (char *)"nep p_Kp:");
            for (i = 0; i < (length - ((int)buff_read - (int)pid_p)); i++)
            {
                if (*(pid_p + i) == 0x0A)
                {
                    strncpy(buff, pid_p, i);
                    pid_position->Kp = atof(buff);
                }
            }
        }
        else if ((uint8_t *)strstr((char *)buff_read, (char *)"nep p_Ki:"))
        {
            pid_p = strstr((char *)buff_read, (char *)"nep p_Ki:");
            for (i = 0; i < (length - ((int)buff_read - (int)pid_p)); i++)
            {
                if (*(pid_p + i) == 0x0A)
                {
                    strncpy(buff, pid_p, i);
                    pid_position->Ki = atof(buff);
                }
            }
        }
        else if ((uint8_t *)strstr((char *)buff_read, (char *)"nep p_Kd:"))
        {
            pid_p = strstr((char *)buff_read, (char *)"nep p_Kd:");
            for (i = 0; i < (length - ((int)buff_read - (int)pid_p)); i++)
            {
                if (*(pid_p + i) == 0x0A)
                {
                    strncpy(buff, pid_p, i);
                    pid_position->Kd = atof(buff);
                }
            }
        }
        else if ((uint8_t *)strstr((char *)buff_read, (char *)"nep s_Kp:"))
        {
            pid_p = strstr((char *)buff_read, (char *)"nep s_Kp:");
            for (i = 0; i < (length - ((int)buff_read - (int)pid_p)); i++)
            {
                if (*(pid_p + i) == 0x0A)
                {
                    strncpy(buff, pid_p, i);
                    pid_speed->Kp = atof(buff);
                }
            }
        }
        else if ((uint8_t *)strstr((char *)buff_read, (char *)"nep s_Ki:"))
        {
            pid_p = strstr((char *)buff_read, (char *)"nep s_Kp:");
            for (i = 0; i < (length - ((int)buff_read - (int)pid_p)); i++)
            {
                if (*(pid_p + i) == 0x0A)
                {
                    strncpy(buff, pid_p, i);
                    pid_speed->Ki = atof(buff);
                }
            }
        }
        else if ((uint8_t *)strstr((char *)buff_read, (char *)"nep s_Kd:"))
        {
            pid_p = strstr((char *)buff_read, (char *)"nep s_Kp:");
            for (i = 0; i < (length - ((int)buff_read - (int)pid_p)); i++)
            {
                if (*(pid_p + i) == 0x0A)
                {
                    strncpy(buff, pid_p, i);
                    pid_speed->Kd = atof(buff);
                }
            }
        }
    }
    else
    {
        // printf("no data rx");// 没有数据
    }

    if (pfifo_2->error)
    {
        pfifo_2->error = 0; // 接收错误
    }
}

uint8_t send_frame[16];
void package_frame(const void *data, int x)
{
    int i;
    for (i = 0; i < (4 * x); i++)
    {
        send_frame[i] = ((uint8_t *)data)[i];
    }
}
float vofa_t = 0.0f;
float data[4];
void vofa_test(void)
{

    int i;

    vofa_t += 0.1f;
    data[0] = sin_calculate(vofa_t);
    data[1] = sin_calculate(2.0f * vofa_t);
    data[2] = sin_calculate(3.0f * vofa_t);
    data[3] = sin_calculate(4.0f * vofa_t);

    package_frame(data, 4);

    //	for (i = 0; i < 1; i++)
    //	{
    //		send_frame[i] = (uint8_t)(data[i] >> 24);
    //		send_frame[i+1] = (uint8_t)(data[i] >> 16);
    //		send_frame[i+2] = (uint8_t)(data[i] >> 8);
    //		send_frame[i+3] = (uint8_t)(data[i]);
    //	}

    for (i = 0; i < 16; i++)
    {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
            ;                                           //发送数据到串口2
        USART_SendData(USART2, (uint8_t)send_frame[i]); //等待上次传输完成
    }

    // 发送帧尾
    uint8_t tail[4] = {0x00, 0x00, 0x80, 0x7f};
    for (int i = 0; i < 4; i++)
    {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
            ;                                     //发送数据到串口2
        USART_SendData(USART2, (uint8_t)tail[i]); //等待上次传输完成
    }
}
