/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2013 Semtech-Cycleo

Description:
    Minimum test program for the loragw_spi 'library'
    Use logic analyser to check the results.

License: Revised BSD License, see LICENSE.TXT file include in the project
Maintainer: Sylvain Miermont
*/


/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "loragw_spi.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

#define BURST_TEST_SIZE 2500 /* >> LGW_BURST_CHUNK */
#define TIMING_REPEAT   1    /* repeat transactions multiple times for timing characterisation */

/* -------------------------------------------------------------------------- */
/* --- MAIN FUNCTION -------------------------------------------------------- */

int main()
{
	int x, i;
	void *spi_target = NULL;
	uint8_t dataout[BURST_TEST_SIZE];
	uint8_t datain[BURST_TEST_SIZE];
	uint8_t spi_mux_mode = LGW_SPI_MUX_MODE0;

	uint8_t reg_addr = 0x00;
	uint8_t reg_value = 0x00;

	for (i = 0; i < BURST_TEST_SIZE; ++i) {
		dataout[i] = 0x30 + (i % 10); /* ASCCI code for 0 -> 9 */
		datain[i] = 0x23; /* garbage data, to be overwritten by received data */
	}

	printf("Beginning of test for loragw_spi.c\n");
	lgw_spi_open(&spi_target);

	//lgw_spi_w(spi_target, spi_mux_mode, LGW_SPI_MUX_TARGET_SX1301, 0xAA, 0x96);
	for(reg_addr = 0x00; reg_addr < 0x44; reg_addr++) {
		x = lgw_spi_r(spi_target, spi_mux_mode, LGW_SPI_MUX_TARGET_SX127X, reg_addr, &reg_value);
		if (x != LGW_SPI_SUCCESS) {
			printf("ERROR: Failed to read sx127x register: %2X\n", reg_addr);
		}
		printf("REG: 0x%02X = 0x%02X\n", reg_addr, reg_value);
		usleep(1000 * 50);
	}

	lgw_spi_close(spi_target);
	printf("End of test for loragw_spi.c\n");

	return 0;
}

/* --- EOF ------------------------------------------------------------------ */
