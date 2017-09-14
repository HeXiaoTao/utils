/*
* THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND 
* (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
* CONSEQUENTLY, SEMTECH SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
* CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
* OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
* CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
* 
* Copyright (C) SEMTECH S.A.
*/
/*! 
* \file       sx1276.c
* \brief      SX1276 RF chip driver
*
* \version    2.0.0 
* \date       May 6 2013
* \author     Gregory Cristian
*
* Last modified by Miguel Luis on Jun 19 2013
*/
#include <unistd.h>

#include "platform/platform.h"
#include "platform/sx1276-Hal.h"
#include "radio/radio.h"

#include "sx1276-LoRa.h"

/*!
* SX1276 registers variable
*/
uint8_t SX1276Regs[0x70];

void SX1276Reset(void)
{
	SX1276SetReset(RADIO_RESET_ON);
	//Wait 1ms
	usleep(1 * 1000);
	SX1276SetReset(RADIO_RESET_OFF);

	// Wait 6ms   
	usleep(6 * 1000);
}

void SX1276SetLoRaOn()
{
	SX1276LoRaSetOpMode(RFLR_OPMODE_SLEEP);

	SX1276LR->RegOpMode = (SX1276LR->RegOpMode & RFLR_OPMODE_LONGRANGEMODE_MASK) | RFLR_OPMODE_LONGRANGEMODE_ON;
	SX1276Write(REG_LR_OPMODE, SX1276LR->RegOpMode);

	SX1276LoRaSetOpMode(RFLR_OPMODE_STANDBY);
								// RxDone               RxTimeout                   FhssChangeChannel           CadDone
	SX1276LR->RegDioMapping1 = RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO1_00 | RFLR_DIOMAPPING1_DIO2_00 | RFLR_DIOMAPPING1_DIO3_00;
								// CadDetected          ModeReady
	SX1276LR->RegDioMapping2 = RFLR_DIOMAPPING2_DIO4_00 | RFLR_DIOMAPPING2_DIO5_00;
	SX1276WriteBuffer(REG_LR_DIOMAPPING1, &SX1276LR->RegDioMapping1, 2);

	SX1276ReadBuffer(REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1);
}

void SX1276Init(void)
{
	SX1276LR = (tSX1276LR *) SX1276Regs;

	SX1276InitIo();

	SX1276Reset();

	// REMARK: After radio reset the default modem is FSK
	SX1276SetLoRaOn();
	// Initialize LoRa modem
	SX1276LoRaInit();
}

uint8_t SX1276GetOpMode(void)
{
	return SX1276LoRaGetOpMode();
}

uint8_t SX1276ReadRxGain(void)
{
	return SX1276LoRaReadRxGain();
}

double SX1276ReadRssi(void)
{
	return SX1276LoRaReadRssi();
}

uint8_t SX1276GetPacketRxGain(void)
{
	return SX1276LoRaGetPacketRxGain();
}

int8_t SX1276GetPacketSnr(void)
{
	return SX1276LoRaGetPacketSnr();
}

double SX1276GetPacketRssi(void)
{
	return SX1276LoRaGetPacketRssi();
}

void SX1276StartRx(long timeout)
{
	SX1276LoRaStartRx(timeout);
}

void SX1276StartCad(void)
{
	SX1276LoRaSetRFState(RFLR_STATE_CAD_INIT);
}

void SX1276GetRxPacket(void **buffer, uint16_t *size)
{
	SX1276LoRaGetRxPacket(buffer, size);
}

void SX1276SetTxPacket(const void *buffer, uint16_t size)
{
	SX1276LoRaSetTxPacket(buffer, size);
}

uint8_t SX1276GetRFState(void)
{
	return SX1276LoRaGetRFState();
}

void SX1276SetRFState(uint8_t state)
{
	SX1276LoRaSetRFState(state);
}

uint32_t SX1276Process(void)
{
	return SX1276LoRaProcess();
}