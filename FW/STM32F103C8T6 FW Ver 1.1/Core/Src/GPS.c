#include "GPS.h"
#include "usart.h"
#include "DS3231.h"
#include "GPS_Data_Repeater.h"

uint8_t m6n_rx_buf[TIMEUTC_LEN];
uint8_t m6n_rx_cplt_flag = 0;

const unsigned char UBX_CFG_PRT[] = {
		0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00,
		0x80, 0x25, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9A, 0x79
};

const unsigned char UBX_CFG_MSG[] = {
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x21, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x32, 0x97
};

const unsigned char UBX_CFG_RATE[] = {
		0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xC8, 0x00, 0x01, 0x00, 0x01, 0x00, 0xDE, 0x6A
};

const unsigned char UBX_CFG_CFG[] = {
		0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x17, 0x31, 0xBF
};

uint8_t M6N_UBX_NAV_TIMEUTC_Parsing(uint8_t* data, M6N_UBX_NAV_TIMEUTC* timeutc)
{
	if(m6n_rx_cplt_flag == 1)
	{
	  m6n_rx_cplt_flag = 0;

		if(M6N_UBX_CHKSUM_Check(&m6n_rx_buf, sizeof(m6n_rx_buf)) == 1)
		{
			timeutc->CLASS = data[2];
			timeutc->ID = data[3];
			timeutc->LENGTH = data[4] | (data[5] << 8);

			timeutc->iTOW = data[6] | (data[7] << 8) | (data[8] << 16) | (data[9] << 24);
			timeutc->tAcc = data[10] | (data[11] << 8) | (data[12] << 16) | (data[13] << 24);
			timeutc->nano = data[14] | (data[15] << 8) | (data[16] << 16) | (data[17] << 24);
			timeutc->year = data[18] | (data[19] << 8);
			timeutc->month = data[20];
			timeutc->day = data[21];
			timeutc->hour = data[22];
			timeutc->min = data[23];
			timeutc->sec = data[24];
			timeutc->valid = data[25];

			return HAL_OK;
		}
	}

	return HAL_BUSY;
}


unsigned char M6N_UBX_CHKSUM_Check(uint8_t* data, unsigned char len)
{
	unsigned char CK_A = 0, CK_B = 0;

	for(int i = 2; i < len - 2; i++)
	{
		CK_A = CK_A + data[i];
		CK_B = CK_B + CK_A;
	}

	return ((CK_A == data[len - 2]) && (CK_B== data[len - 1]));
}

uint8_t Check_Time_Valid(M6N_UBX_NAV_TIMEUTC* timeutc)
{
	// Check if the validUTC bit (Bit 2) is set
	return (timeutc->valid & (1 << 2)) != 0;
}

void M6N_TransmitData(const unsigned char* data, uint8_t len)
{
	HAL_UART_Transmit(&GPS_USART, data, len, UART_TIME_OUT);
}

void M6N_Init()
{
	M6N_TransmitData(&UBX_CFG_PRT, sizeof(UBX_CFG_PRT));
	HAL_Delay(150);
	M6N_TransmitData(&UBX_CFG_MSG, sizeof(UBX_CFG_MSG));
	HAL_Delay(150);
	M6N_TransmitData(&UBX_CFG_RATE, sizeof(UBX_CFG_RATE));
	HAL_Delay(150);
	M6N_TransmitData(&UBX_CFG_CFG, sizeof(UBX_CFG_CFG));
}

