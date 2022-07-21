#ifndef __USERREFEREE_H_
#define __USERREFEREE_H_
#include "main.h"
typedef __packed struct
{
    uint8_t operate_tpye;
    uint8_t graphic_tpye;
    uint8_t graphic_name[5];
    uint8_t layer;
    uint8_t color;
    uint8_t width;
    uint16_t start_x;
    uint16_t start_y;
    uint16_t radius;
    uint16_t end_x;
    uint16_t end_y;
    int16_t start_angle;
    int16_t end_angle;
    uint8_t text_lenght;
    uint8_t text[30];
} ClientMap_t;

uint8_t SendUserData(uint16_t DataLen, uint8_t seq, uint16_t ContentID, uint16_t SendID, uint16_t ReceiveID, uint8_t *data);
uint8_t SendPowerHeat(uint16_t SendID, uint16_t ReceiveID);
uint8_t RobotCommuni(uint16_t SendID, uint16_t ReceiveID, uint8_t Len, uint16_t ContentID, uint8_t*data);
uint8_t DeleteAllMap(uint16_t SendID, uint16_t ReceiveID);
uint8_t ClientMap(uint16_t SendID, uint16_t ReceiveID, ClientMap_t*user);
#endif
