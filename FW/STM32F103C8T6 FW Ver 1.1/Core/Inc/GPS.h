#ifndef _GPS_H_
#define _GPS_H_

#include "main.h"

#define TIMEUTC_LEN		28

typedef struct M6N_UBX_NAV_TIMEUTC
{
    uint32_t iTOW;  // GPS Time of Week (ms)
    uint32_t tAcc;  // Time Accuracy Estimate (ns)
    int32_t nano;   // Fraction of a second (ns)
    uint16_t year;  // Year (UTC)
    uint8_t month;  // Month (UTC)
    uint8_t day;    // Day (UTC)
    uint8_t hour;   // Hour (UTC)
    uint8_t min;    // Minute (UTC)
    uint8_t sec;    // Second (UTC)
    uint8_t valid;  // Validity flags

    unsigned char CLASS;  // Class ID
    unsigned char ID;     // Message ID
    uint16_t LENGTH;      // Payload Length
} M6N_UBX_NAV_TIMEUTC;



void M6N_Init();
uint8_t M6N_UBX_NAV_TIMEUTC_Parsing(uint8_t* data, M6N_UBX_NAV_TIMEUTC* timeutc);
unsigned char M6N_UBX_CHKSUM_Check(uint8_t* data, unsigned char len);
uint8_t Check_Time_Valid(M6N_UBX_NAV_TIMEUTC* timeutc);
void M6N_TransmitData(const unsigned char* data, uint8_t len);



#endif
