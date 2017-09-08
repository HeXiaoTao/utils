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
#include <string.h>
#include <errno.h>

#include "loralog.h"
#include "radio/radio.h"
#include "platform/platform.h"
#include "platform/sx1276-Hal.h"

#include "queue/fixed_queue.h"

uint8_t DebugLevel = DEBUG_TRACE;
/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */
struct {
	fixed_queue_t *rx;
	fixed_queue_t *tx;
} g_queue;

#define QUEUE_DATA_HEAD_SIZE (sizeof(uint32_t))
typedef struct {
	uint32_t size;
	double rssi;
	uint8_t next;
} __attribute__((aligned(1),packed)) queue_data;

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

static int native_queue_init()
{
	g_queue.tx = fixed_queue_new(10);
	if(g_queue.tx == NULL) {
		fixed_queue_free(g_queue.tx, NULL);
		return -1;
	}

	g_queue.rx = fixed_queue_new(10);
	if(g_queue.rx == NULL) {
		fixed_queue_free(g_queue.tx, NULL);
		fixed_queue_free(g_queue.rx, NULL);
		return -1;
	}

	DBGPRINT(DEBUG_INFO, "+%s : ~@@~", __FUNCTION__);
	return 0;
}

static void* rx_queue_resolver(void *arg)
{
	queue_data *pdata = arg;
	queue_data *pdata_ack = arg;

	while(1) {
		pdata = fixed_queue_dequeue(g_queue.rx);
		if(pdata != NULL) {
			DBGPRINT(DEBUG_INFO, "+%s : queue_data: size=%d rssi=%f data=%s", __FUNCTION__,
							pdata->size, pdata->rssi, &pdata->next);
			//free(pdata);
			pdata_ack = pdata;
			(&pdata_ack->next)[0] = 'A';
			(&pdata_ack->next)[1] = 'C';
			(&pdata_ack->next)[2] = 'K';
			strcpy((char *)&pdata_ack->next + 3, (char *)&pdata_ack->next + 4);
			pdata_ack->size = pdata_ack->size - 1;
			fixed_queue_enqueue(g_queue.tx, pdata_ack);
			wakeup_by_xmit();
			pdata = NULL;
			pdata_ack = NULL;
		}
	}
}

static int rx_queue_resolver_init()
{
	pthread_t temp;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	errno = pthread_create(&temp, &attr, rx_queue_resolver, NULL);

	return errno;
}

/* -------------------------------------------------------------------------- */
/*
 * Manages the master operation
 */
static uint32_t run_state_machine_once(tRadioDriver *Radio)
{
	static void *_raw = NULL;
	static uint16_t _size = 0;
	static queue_data *_data = NULL;

	uint32_t result = RF_IDLE;

process_again:
	result = Radio->Process();

	switch(result) {
	case RF_BUSY:
		goto process_again;
	case RF_RX_TIMEOUT:
		DBGPRINT(DEBUG_INFO, "+%s : RF_RX_TIMEOUT", __FUNCTION__);
		break;
	case RF_RX_DONE:
		Radio->GetRxPacket(&_raw, &_size);
		DBGPRINT(DEBUG_INFO, "+%s : RF_RX_DONE: size=%d", __FUNCTION__, _size);
		if(_size > 0) {
			_data = malloc(QUEUE_DATA_HEAD_SIZE + _size + 1);
			if(!_data) {
				DBGPRINT(DEBUG_ERROR, "+%s : malloc ????????????", __FUNCTION__);
				break;	
			}
			memset(_data, 0, QUEUE_DATA_HEAD_SIZE + _size + 1);
			_data->size = _size;
			_data->rssi = Radio->GetPacketRssi();
			memcpy(&_data->next, _raw, _size);
			if(!fixed_queue_try_enqueue(g_queue.rx, _data)) {
				DBGPRINT(DEBUG_ERROR, "+%s : fixed_queue_try_enqueue rx: ???????????", __FUNCTION__);
				free(_data);
			}
		}
		break;
	case RF_TX_DONE:
		DBGPRINT(DEBUG_INFO, "+%s : RF_TX_DONE", __FUNCTION__);
		break;
	case RF_TX_TIMEOUT:
		break;
	case RF_LEN_ERROR:
		break;
	case RF_CHANNEL_EMPTY:
		DBGPRINT(DEBUG_INFO, "+%s : RF_CHANNEL_EMPTY", __FUNCTION__);
		_data = fixed_queue_try_dequeue(g_queue.tx);
		if(_data != NULL) {
			Radio->SetTxPacket(&_data->next, _data->size);
			free(_data);
		} else {
			Radio->StartRx();
		}
		goto process_again;
	case RF_CHANNEL_ACTIVITY_DETECTED:
		DBGPRINT(DEBUG_INFO, "+%s : RF_CHANNEL_ACTIVITY_DETECTED", __FUNCTION__);
		Radio->StartRx();
		goto process_again;
	default:
		break;
	}

	return result;
}

static void* run_state_machine(void *arg)
{
	uint32_t result = RF_IDLE;

	tRadioDriver *Radio = arg;

	DBGPRINT(DEBUG_INFO, "+%s : id(%d:%d)", __FUNCTION__, getpid(), gettid());

	while(1) {
		result = run_state_machine_once(Radio);
		switch(result) {
		case RF_IDLE:
		case RF_RX_DONE:
		case RF_RX_TIMEOUT:
		case RF_TX_DONE:
		case RF_TX_TIMEOUT:
			if(fixed_queue_is_empty(g_queue.tx)) {
				DBGPRINT(DEBUG_INFO, "+%s : fixed_queue_is_empty", __FUNCTION__);
				Radio->StartRx();
			} else {
				DBGPRINT(DEBUG_INFO, "+%s : fixed_queue_isn't_empty StartCad", __FUNCTION__);
				Radio->StartCad();
			}
			break;
		default:
			Radio->StartRx();
			break;
		}
	}
}

static int stata_machine_init(tRadioDriver *Radio)
{
	pthread_t temp;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	errno = pthread_create(&temp, &attr, run_state_machine, Radio);

	return errno;
}

/* -------------------------------------------------------------------------- */
/* --- MAIN FUNCTION -------------------------------------------------------- */

int main()
{
	int err = 0;

	tRadioDriver *Radio = NULL;

	DBGPRINT(DEBUG_INFO, "+%s : RadioDriverInit()", __FUNCTION__);
	Radio = RadioDriverInit();

	DBGPRINT(DEBUG_INFO, "+%s : Radio->Init()", __FUNCTION__);
	Radio->Init();
	Radio->StartRx();

	err = native_queue_init();
	if(err) {
		DBGPRINT(DEBUG_ERROR, "+%s : native_queue_init error", __FUNCTION__);
		return err;
	}

	if(rx_queue_resolver_init()) {
		DBGPRINT(DEBUG_ERROR, "+%s : rx_queue_resolver_init error", __FUNCTION__);
		return errno;
	}

	if(stata_machine_init(Radio)) {
		DBGPRINT(DEBUG_ERROR, "+%s : stata_machine_init error", __FUNCTION__);
		return errno;
	}

	do {
		DBGPRINT(DEBUG_ERROR, "+%s : main ??????", __FUNCTION__);
	} while(pause());

	return 0;
}
/* --- EOF ------------------------------------------------------------------ */