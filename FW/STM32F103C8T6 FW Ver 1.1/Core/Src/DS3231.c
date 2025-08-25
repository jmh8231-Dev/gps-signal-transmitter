/*
 * stm32_ds3231.c
 *
 *  Created on: 2019. 3. 17.
 *      Author: kiki
 */
#include "stm32f1xx_hal.h"
#include "DS3231.h"
#include "GPS.h"
#include "CLCD.h"

#define DS3231_ADDR  0xD0

I2C_HandleTypeDef *i2c;

void DS3231_Init(I2C_HandleTypeDef *handle)
{
  i2c = handle;
}

bool DS3231_GetTime(_RTC *rtc)
{
	uint8_t startAddr = DS3231_REG_TIME;
	uint8_t buffer[7] = {0};

	uint8_t Error;

	if(HAL_I2C_Mem_Read(i2c, DS3231_ADDR, startAddr, I2C_MEMADD_SIZE_8BIT, buffer, sizeof(buffer), 1000) != HAL_OK)
		return false;

	return true;
}

bool DS3231_SetTime(_RTC *rtc)
{
	uint8_t startAddr = DS3231_REG_TIME;
	uint8_t buffer[7] = {D2B(rtc->Sec), D2B(rtc->Min), D2B(rtc->Hour), rtc->DaysOfWeek, D2B(rtc->Date), D2B(rtc->Month), D2B(rtc->Year)};

	if(HAL_I2C_Mem_Write(i2c, DS3231_ADDR, startAddr, I2C_MEMADD_SIZE_8BIT, buffer, sizeof(buffer), HAL_MAX_DELAY) != HAL_OK)
	    return false;

	return true;
}

// 월별 일 수를 정의 (윤년 계산 포함)
const uint8_t days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

bool is_leap_year(uint16_t year)
{
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

uint8_t get_days_in_month(uint8_t month, uint16_t year)
{
    if (month == 2 && is_leap_year(year))
        return 29;
    return days_in_month[month - 1];
}

bool DS3231_Calibration(M6N_UBX_NAV_TIMEUTC* timeutc, _RTC *rtc)
{
    static int8_t last_calibration_hour = -1; // 마지막 보정된 시간을 저장하는 정적 변수

    // UTC 기반 시간 설정
    rtc->Year = timeutc->year - 2000;
    rtc->Month = timeutc->month;
    rtc->Date = timeutc->day;

    rtc->Hour = timeutc->hour + 9; // 시간대를 조정
    rtc->Min = timeutc->min;
    rtc->Sec = timeutc->sec;

    //조정된 시간이 24시간을 넘어가는 경우 처리
    if (rtc->Hour >= 24)
    {
        rtc->Hour -= 24;
        rtc->Date += 1;

        //만약 +1된 일이 해당하는 월에 존재하지 않는다면
        if (rtc->Date > get_days_in_month(rtc->Month, rtc->Year + 2000))
        {
            rtc->Date = 1;
            rtc->Month += 1;

            //만약 +1된 달이 해당되는 년도에 존재하지 않는다면
            if (rtc->Month > 12)
            {
                rtc->Month = 1;
                rtc->Year += 1;
            }
        }
    }

    // 만약 보정한지 시간이 지났다면
    if (rtc->Hour != last_calibration_hour && rtc->Hour > 0)
    {
        last_calibration_hour = rtc->Hour; // 마지막 보정된 시간 업데이트
        DS3231_SetTime(rtc);

        lcd_clear();
        lcd_setCurStr(0, 0, "RTC Calibration...");
        HAL_Delay(500);
        lcd_clear();
        return true; // 보정이 실행되었음을 알림
    }
    return false; // 보정이 실행되지 않았음을 알림
}

bool DS3231_ReadTemperature(float *temp)
{
  uint8_t startAddr = DS3231_REG_TEMP;
  uint8_t buffer[2] = {0,};

  if(HAL_I2C_Master_Transmit(i2c, DS3231_ADDR, &startAddr, 1, HAL_MAX_DELAY) != HAL_OK) return false;
  if(HAL_I2C_Master_Receive(i2c, DS3231_ADDR, buffer, sizeof(buffer), HAL_MAX_DELAY) != HAL_OK) return false;

  int16_t value = (buffer[0] << 8) | (buffer[1]);
  value = (value >> 6);

  *temp = value / 4.0f;
  return true;
}

bool DS3231_SetAlarm1(uint8_t mode, uint8_t date, uint8_t hour, uint8_t min, uint8_t sec)
{
  uint8_t alarmSecond = D2B(sec);
  uint8_t alarmMinute = D2B(min);
  uint8_t alarmHour = D2B(hour);
  uint8_t alarmDate = D2B(date);

  switch(mode)
  {
  case ALARM_MODE_ALL_MATCHED:
    break;
  case ALARM_MODE_HOUR_MIN_SEC_MATCHED:
    alarmDate |= 0x80;
    break;
  case ALARM_MODE_MIN_SEC_MATCHED:
    alarmDate |= 0x80;
    alarmHour |= 0x80;
    break;
  case ALARM_MODE_SEC_MATCHED:
    alarmDate |= 0x80;
    alarmHour |= 0x80;
    alarmMinute |= 0x80;
    break;
  case ALARM_MODE_ONCE_PER_SECOND:
    alarmDate |= 0x80;
    alarmHour |= 0x80;
    alarmMinute |= 0x80;
    alarmSecond |= 0x80;
    break;
  default:
    break;
  }

  /* Write Alarm Registers */
  uint8_t startAddr = DS3231_REG_ALARM1;
  uint8_t buffer[5] = {startAddr, alarmSecond, alarmMinute, alarmHour, alarmDate};
  if(HAL_I2C_Master_Transmit(i2c, DS3231_ADDR, buffer, sizeof(buffer), HAL_MAX_DELAY) != HAL_OK) return false;

  /* Enable Alarm1 at Control Register */
  uint8_t ctrlReg = 0x00;
  ReadRegister(DS3231_REG_CONTROL, &ctrlReg);
  ctrlReg |= DS3231_CON_A1IE;
  ctrlReg |= DS3231_CON_INTCN;
  WriteRegister(DS3231_REG_CONTROL, ctrlReg);

  return true;
}

bool DS3231_ClearAlarm1()
{
  uint8_t ctrlReg;
  uint8_t statusReg;

  /* Clear Control Register */
  ReadRegister(DS3231_REG_CONTROL, &ctrlReg);
  ctrlReg &= ~DS3231_CON_A1IE;
  WriteRegister(DS3231_REG_CONTROL, ctrlReg);

  /* Clear Status Register */
  ReadRegister(DS3231_REG_STATUS, &statusReg);
  statusReg &= ~DS3231_STA_A1F;
  WriteRegister(DS3231_REG_STATUS, statusReg);

  return true;
}

bool ReadRegister(uint8_t regAddr, uint8_t *value)
{
  if(HAL_I2C_Master_Transmit(i2c, DS3231_ADDR, &regAddr, 1, HAL_MAX_DELAY) != HAL_OK) return false;
  if(HAL_I2C_Master_Receive(i2c, DS3231_ADDR, value, 1, HAL_MAX_DELAY) != HAL_OK) return false;

  return true;
}

bool WriteRegister(uint8_t regAddr, uint8_t value)
{
  uint8_t buffer[2] = {regAddr, value};
  if(HAL_I2C_Master_Transmit(i2c, DS3231_ADDR, buffer, sizeof(buffer), HAL_MAX_DELAY) != HAL_OK) return false;

  return true;
}

uint8_t D2B(int val)
{
  return (uint8_t)((val / 10 * 16) + (val % 10));
}

int B2D(uint8_t val)
{
  return (int)((val / 16 * 10) + (val % 16));
}
