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
 * \file       sx1276-Hal.c
 * \brief      SX1276 Hardware Abstraction Layer
 *
 * \version    2.0.B2 
 * \date       Nov 21 2012
 * \author     Miguel Luis
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */
#include <stdint.h>
#include <stdbool.h> 
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "loragw_spi.h"
#include "loralog.h"
#include "platform/platform.h"
#include "platform/sx1276-Hal.h"

//add 2015.08.04 unlocked_ioctl cmd
enum SX12XX_NR{
	NR_SET_DEINIT = 0xF0, 
	NR_GET_DIO = 0xF1,
	NR_SET_RXTX = 0xF2,
};

#define SX12XX_TYPE (0x31)

#define	SX12XX_IOCTL_SET_DEINIT	_IO(SX12XX_TYPE, NR_SET_DEINIT) 
#define SX12XX_IOCTL_GET_DIO	_IOR(SX12XX_TYPE, NR_GET_DIO, int)
#define SX12XX_IOCTL_SET_RXTX	_IOW(SX12XX_TYPE, NR_SET_RXTX, char)

static void *spi_target = NULL;
static const uint8_t spi_mux_mode = LGW_SPI_MUX_MODE0;

static volatile uint8_t need_tx = 0;

static int eint_poll_fd = -1;
static pthread_mutex_t dio_0_poll_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t dio_0_poll_cond = PTHREAD_COND_INITIALIZER;
//dev 0
static const char *eint_poll_dev_path = "/dev/sx12xx@0";

static inline void sx12xx_tx_rx_ctrl(uint8_t rx_tx)
{
	if(eint_poll_fd >= 0 && (rx_tx == 0 || rx_tx == 1)) {
		if(ioctl(eint_poll_fd, SX12XX_IOCTL_SET_RXTX, &rx_tx)) {
			DBGPRINT(DEBUG_ERROR, "+%s: ERROR: ioctl(SX12XX_IOCTL_SET_RXTX) => %s", __FUNCTION__,
							strerror(errno));
		}
	}
}

static void cleanup_handler(void *arg)
{
	pthread_mutex_t *p = arg;
    pthread_mutex_unlock(p);
}

void wakeup_by_xmit()
{
	DBGPRINT(DEBUG_TRACE, "wakeup_by_xmit");

	pthread_cleanup_push(cleanup_handler, &dio_0_poll_mutex);
	pthread_mutex_lock(&dio_0_poll_mutex);
	need_tx = 1;
	pthread_cond_signal(&dio_0_poll_cond);
	pthread_mutex_unlock(&dio_0_poll_mutex);
	pthread_cleanup_pop(0);
}

void dio_eint_callback(int s)
{
	switch(s) {
	case SIGIO:
		/*do the job when catch the sigwait*/
		DBGPRINT(DEBUG_TRACE, "\nsigwait, receive signal: %d", s);
		pthread_mutex_lock(&dio_0_poll_mutex);
		pthread_cond_signal(&dio_0_poll_cond);
		pthread_mutex_unlock(&dio_0_poll_mutex);
		break;
	default:
		break;
	}
}

struct sigaction action;

int linux_setup_poll(int *poll_fd)
{
	int fd = -1;

	fd = open(eint_poll_dev_path, O_RDWR);
	if(fd < 0) {
		DBGPRINT(DEBUG_ERROR, "+%s: !!open(%s) => %s", __FUNCTION__, eint_poll_dev_path,
						strerror(errno));
		return -ENODEV;
	}

	*poll_fd = fd;

	memset(&action, 0, sizeof(action));
	action.sa_handler = dio_eint_callback;
	action.sa_flags = 0;
	int err = sigaction(SIGIO, &action, NULL); //set up async handler
	if (err == 0) {
		if (fcntl(fd, F_SETOWN, gettid()) == -1) { //enable async notification
			DBGPRINT(DEBUG_ERROR, "setFsync(), F_SETOWN error! %s",(char*)strerror(errno));
		}
		if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | FASYNC | FD_CLOEXEC) == -1) {
			DBGPRINT(DEBUG_ERROR, "setFsync(), F_SETFL error! %s", (char*)strerror(errno));
		}
	}

	return err;
}

void SX1276InitIo(void)
{
	DBGPRINT(DEBUG_TRACE, "+%s", __FUNCTION__);

	lgw_spi_open(&spi_target);
	if(spi_target == NULL)
		goto spi_open_error;

	if(linux_setup_poll(&eint_poll_fd)) {
		DBGPRINT(DEBUG_ERROR, "%s: linux_setup_poll ERROR!!!!", __FUNCTION__);
	}
	return;

spi_open_error:
	DBGPRINT(DEBUG_ERROR, "+%s: spi_open_error", __FUNCTION__);
	lgw_spi_close(spi_target);
}

void SX1276SetReset(uint8_t state)
{
	DBGPRINT(DEBUG_TRACE, "+%s state=%d", __FUNCTION__, state);
}

void SX1276Write(uint8_t addr, uint8_t data)
{
	int x = 0;

	//DBGPRINT(DEBUG_TRACE, "+%s addr=0x%02X data=0x%02X", __FUNCTION__, addr, data);
	if(spi_target != NULL) {
		x = lgw_spi_w(spi_target, spi_mux_mode, LGW_SPI_MUX_TARGET_SX127X, addr, data);
		if (x != LGW_SPI_SUCCESS) {
			DBGPRINT(DEBUG_ERROR, "+%s ERROR: Failed to write sx1276 register: 0x%02X", __FUNCTION__, addr);
		}
	} else {
		DBGPRINT(DEBUG_ERROR, "+%s ERROR: spi_target is NULL", __FUNCTION__);
	}
	//DBGPRINT(DEBUG_TRACE, "-%s", __FUNCTION__);
}

void SX1276Read(uint8_t addr, uint8_t *data)
{
	int x = 0;

	//DBGPRINT(DEBUG_TRACE, "+%s addr=0x%02X", __FUNCTION__, addr);
	if(spi_target != NULL) {
		x = lgw_spi_r(spi_target, spi_mux_mode, LGW_SPI_MUX_TARGET_SX127X, addr, data);
		if (x != LGW_SPI_SUCCESS) {
			DBGPRINT(DEBUG_ERROR, "+%s ERROR: Failed to read sx1276 register: 0x%02X", __FUNCTION__, addr);
		} else {
			//DBGPRINT(DEBUG_TRACE, "+%s register 0x%02X=>0x%02X", __FUNCTION__, addr, *data);
		}
	} else {
		DBGPRINT(DEBUG_ERROR, "+%s ERROR: spi_target is NULL", __FUNCTION__);
	}
	//DBGPRINT(DEBUG_TRACE, "-%s", __FUNCTION__);
}

void SX1276WriteBuffer(uint8_t addr, uint8_t *buffer, uint8_t size)
{
	int x = 0;

	//DBGPRINT(DEBUG_TRACE, "+%s addr=0x%02X buffer=%p size=%d", __FUNCTION__, addr, buffer, size);

	if(spi_target != NULL) {
		x = lgw_spi_wb(spi_target, spi_mux_mode, LGW_SPI_MUX_TARGET_SX127X, addr, buffer, size);
		if (x != LGW_SPI_SUCCESS) {
			DBGPRINT(DEBUG_ERROR, "+%s ERROR: Failed to write sx1276 buffer: 0x%02X", __FUNCTION__, addr);
		}
	} else {
		DBGPRINT(DEBUG_ERROR, "+%s ERROR: spi_target is NULL", __FUNCTION__);
	}
	//DBGPRINT(DEBUG_TRACE, "-%s", __FUNCTION__);
}

void SX1276ReadBuffer(uint8_t addr, uint8_t *buffer, uint8_t size)
{
	int x = 0;

	//DBGPRINT(DEBUG_TRACE, "+%s addr=0x%02X buffer=%p size=%d", __FUNCTION__, addr, buffer, size);
	if(spi_target != NULL) {
		x = lgw_spi_rb(spi_target, spi_mux_mode, LGW_SPI_MUX_TARGET_SX127X, addr, buffer, size);
		if (x != LGW_SPI_SUCCESS) {
			DBGPRINT(DEBUG_ERROR, "+%s ERROR: Failed to read sx1276 buffer: 0x%02X", __FUNCTION__, addr);
		}
	} else {
		DBGPRINT(DEBUG_ERROR, "+%s ERROR: spi_target is NULL", __FUNCTION__);
	}
	//DBGPRINT(DEBUG_TRACE, "-%s", __FUNCTION__);
}

void SX1276WriteFifo( uint8_t *buffer, uint8_t size )
{
	SX1276WriteBuffer(0, buffer, size);
}

void SX1276ReadFifo(uint8_t *buffer, uint8_t size)
{
	SX1276ReadBuffer(0, buffer, size);
}

static inline uint8_t get_dio_x_value(uint8_t x)
{
	uint32_t gpio_map = 0xFFFFFFFF;

	if(eint_poll_fd >= 0 && x <= 5) {
		if(ioctl(eint_poll_fd, SX12XX_IOCTL_GET_DIO, &gpio_map)) {
			DBGPRINT(DEBUG_ERROR, "+%s: ERROR: ioctl(SX12XX_IOCTL_SET_RXTX) => %s", __FUNCTION__,
							strerror(errno));
		} else {
			return (gpio_map >> x) & 0x01;
		}
	}

	return 1;
}

uint8_t SX1276ReadDio0(int timeout)
{
	int ret = 0;
	int dio_0 = 0;
	long begain_time = 0;
	struct timespec ts;
	struct timeval t = {0, 0};

	pthread_cleanup_push(cleanup_handler, &dio_0_poll_mutex);
	pthread_mutex_lock(&dio_0_poll_mutex);
	while(!(dio_0 = get_dio_x_value(0)) && !need_tx) {
		if(timeout > 0) {
			gettimeofday(&t, NULL);
			ts.tv_sec = t.tv_sec;
			ts.tv_nsec = t.tv_usec*1000;
			ts.tv_sec += timeout/1000;
			ts.tv_nsec+= (timeout%1000) * 1000000;
			begain_time = t.tv_sec*1000 + t.tv_usec/1000;
			ret = pthread_cond_timedwait(&dio_0_poll_cond, &dio_0_poll_mutex, &ts);
			dio_0 = get_dio_x_value(0);
			gettimeofday(&t, NULL);
			begain_time = t.tv_sec*1000 + t.tv_usec/1000 - begain_time;
			DBGPRINT(DEBUG_TRACE, "+%s cond_timedwait: ret=%d dio_0=%d need_tx=%d (%ld/%d)ms", __FUNCTION__,
							ret, dio_0, need_tx, begain_time, timeout);
			timeout = timeout - begain_time;
			if(need_tx || timeout <= 0)
				break;
		} else if(timeout == 0) {
			break;
		} else if(timeout < 0) {
			ret = pthread_cond_wait(&dio_0_poll_cond, &dio_0_poll_mutex);
			DBGPRINT(DEBUG_TRACE, "+%s cond_wait: ret=%d", __FUNCTION__, ret);
			dio_0 = get_dio_x_value(0);
			if((timeout == -1 && need_tx) || dio_0)
				break;
		}
	}

	need_tx = 0;

	pthread_mutex_unlock(&dio_0_poll_mutex);
	pthread_cleanup_pop(0);

    return dio_0;
}

uint8_t SX1276ReadDio1(void)
{
	return 0;
}

uint8_t SX1276ReadDio2(void)
{
	return 0;
}

uint8_t SX1276ReadDio3(void)
{
	return 0;
}

uint8_t SX1276ReadDio4(void)
{
	return 0;
}

uint8_t SX1276ReadDio5(void)
{
	return 0;
}

void SX1276WriteRxTx(uint8_t txEnable)
{
	DBGPRINT(DEBUG_TRACE, "+%s txEnable=%d", __FUNCTION__, txEnable);
	if(txEnable != 0) {
		sx12xx_tx_rx_ctrl(1);
	} else {
		sx12xx_tx_rx_ctrl(0);
	}
}
