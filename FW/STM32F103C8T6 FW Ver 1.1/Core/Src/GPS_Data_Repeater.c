/*
 * GPS_Data_Repeater.c
 *
 *  Created on: Jun 7, 2024
 *      Author: binbe
 */

#include "GPS_Data_Repeater.h"
#include "DS3231.h"
#include "usart.h"
#include "GPS.h"

bool ATmega328p_flag = 0;

void Time_CAL_GPS(M6N_UBX_NAV_TIMEUTC *timeutc, TIME *time)
{
	time->year = timeutc->year - 2000;
	time->month = timeutc->month;
	time->day = timeutc->day;

	time->hour = timeutc->hour + 9;
	time->min = timeutc->min;
	time->sec = timeutc->sec;

	if(time->hour > 23)
	{
		time->hour = time->hour - 24;
		time->day = time->day + 1;
	}

	time->UTCvalid = 1;
}

void Time_CAL_RTC(_RTC *rtc, TIME *time)
{
	time->year = rtc->Year;
	time->month = rtc->Month;
	time->day = rtc->Date;

	time->hour = rtc->Hour;
	time->min = rtc->Min;
	time->sec = rtc->Sec;


	time->UTCvalid = 0;
}

void Time_Calibration(TIME *time, uint8_t *rx_buf, _RTC *rtc)
{
    M6N_UBX_NAV_TIMEUTC timeutc;

    // GPS 데이터를 파싱하여 UTC 시간을 가져옴
    if (M6N_UBX_NAV_TIMEUTC_Parsing(rx_buf, &timeutc) == HAL_OK)
    {
        // GPS 데이터가 유효한지 확인
        if (Check_Time_Valid(&timeutc))
        {
            // 유효한 경우 GPS 시간으로 RTC를 보정하고 RTC 시간을 반환
            DS3231_Calibration(&timeutc, rtc);
            Time_CAL_GPS(&timeutc, time);
        }
        else
        {
            // 유효하지 않은 경우 RTC 시간을 가져옴
            DS3231_GetTime(rtc);
            Time_CAL_RTC(rtc, time);
        }
    }
}

void lcd_TimeDP(TIME *time)
{
	uint8_t strTime_Data[20];

	sprintf(strTime_Data, "20%d:%02d:%02d  %02d:%02d:%02d",
	       time->year, time->month, time->day, time->hour, time->min, time->sec);
	lcd_setCurStr(0, 0, strTime_Data);
}

void lcd_Time_ValidDP(TIME *time)
{
	if(time->UTCvalid)
		lcd_setCurStr(0, 1, "(GPS)");
	else
		lcd_setCurStr(0, 1, "(RTC)");
}

//연결된 Node 수를 보여줌
void lcd_Node_CountDP()
{
	lcd_string("    RF Nodes: ");
}

void lcd_Temp_HumDP()
{
	extern uint8_t rx_uart3_Data[20];
	uint8_t strTempHum_Data[20];

	if(ATmega328p_flag)
	{
		lcd_setCurStr(0, 2, "Sub MCU No Response ");

		return;
	}

	sprintf(strTempHum_Data, "Temp:%c%c.%c%c Hum:%c%c.%c%c",
	rx_uart3_Data[5], rx_uart3_Data[6], rx_uart3_Data[8], rx_uart3_Data[9], rx_uart3_Data[0], rx_uart3_Data[1], rx_uart3_Data[3], rx_uart3_Data[4]);
	lcd_setCurStr(0, 2, strTempHum_Data);
}

void lcd_VinDP(uint16_t* data)
{
	uint8_t strADC_Data[20];

	double ADCV = 0;
	double VIN = 0;

	ADCV = data[0] * 0.00806 / 10;
	VIN = ADCV * 2.52631 + 0.199;

	double vsense = (float)data[1] * 3.3 / 4095;
	double MCUTemp = (1.38 - vsense) / 0.0043 + 25;

	sprintf(strADC_Data, "VIN:%.2fV CPU:%.2fC", VIN, MCUTemp);
	lcd_setCurStr(0, 3, strADC_Data);
}

void Display(TIME *time, uint16_t *val)
{
	lcd_TimeDP(time);
	lcd_Time_ValidDP(time);
	lcd_Node_CountDP();
	lcd_Temp_HumDP();
	lcd_VinDP(val);
}

void send_Data(TIME *time)
{
	uint8_t sendData[6];

	sendData[0] = time->year;
	sendData[1] = time->month;
	sendData[2] = time->day;
	sendData[3] = time->hour;
	sendData[4] = time->min;
	sendData[5] = time->sec;

	HAL_UART_Transmit(&Arduino_USART, sendData, 6, UART_TIME_OUT);
}

void ATmega328p_Reset()
{
	static int ATmega328p_ResetCount = -1;

	if(ATmega328p_ResetCount < MAX_RESET_COUNT)
	{
		HAL_GPIO_WritePin(ATmega328p_Reset_GPIO_Port, ATmega328p_Reset_Pin, GPIO_PIN_RESET);
		HAL_Delay(10);
		HAL_GPIO_WritePin(ATmega328p_Reset_GPIO_Port, ATmega328p_Reset_Pin, GPIO_PIN_SET);
		ATmega328p_ResetCount++;

		return;
	}

	ATmega328p_flag = 1;
}

bool SubMCU_Check(bool *led_flag)
{
	extern uint8_t rx_uart3_Data[20];

	if(rx_uart3_Data[10] != 'A' && HAL_GPIO_ReadPin(ISP_GPIO_Port, ISP_Pin) == GPIO_PIN_RESET)
		ATmega328p_Reset();

	rx_uart3_Data[10] = 0;

	if(rx_uart3_Data[11] == 'B')
		return true;

	return false;
}
