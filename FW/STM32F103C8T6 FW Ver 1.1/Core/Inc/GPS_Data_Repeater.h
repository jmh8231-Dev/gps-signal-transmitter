/*
 * GPS_Data_Repeater.h
 *
 *  Created on: Jun 7, 2024
 *      Author: binbe
 */

#include "main.h"
#include "stdbool.h"

#ifndef INC_GPS_DATA_REPEATER_H_
#define INC_GPS_DATA_REPEATER_H_

#define	GPS_USART					huart1
#define	Arduino_USART				huart3

#define UART_TIME_OUT				100

#define MAX_RESET_COUNT				25

typedef struct TIME
{
	uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t UTCvalid;
} TIME;

#endif /* INC_GPS_DATA_REPEATER_H_ */
