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
#include <arpa/inet.h>

#include "loralog.h"
#include "lora_mac.h"
#include "crc/crc16.h"
#include "radio/radio.h"
#include "platform/platform.h"
#include "platform/sx1276-Hal.h"

#include "queue/fixed_queue.h"

uint8_t DebugLevel = DEBUG_TRACE;
/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */
struct {
	fixed_queue_t *rx;
	fixed_queue_t *rx_config;
	fixed_queue_t *tx;
} g_queue;

typedef struct {
	uint32_t size;
	double rssi;
	uint8_t next;
} __attribute__((aligned(1),packed)) queue_data;

#define QUEUE_DATA_HEAD_SIZE (sizeof(queue_data))
/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

static int native_queue_init()
{
	g_queue.tx = fixed_queue_new(10);
	if(g_queue.tx == NULL) {
		return -1;
	}

	g_queue.rx = fixed_queue_new(10);
	if(g_queue.rx == NULL) {
		fixed_queue_free(g_queue.tx, NULL);
		return -1;
	}

	g_queue.rx_config = fixed_queue_new(5);
	if(g_queue.rx_config == NULL) {
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
	//queue_data *pdata_ack = arg;

	while(1) {
		pdata = fixed_queue_dequeue(g_queue.rx_config);
		if(pdata != NULL) {
			DBGPRINT(DEBUG_INFO, "+%s : queue_config_data: size=%d rssi=%f", __FUNCTION__,
							pdata->size, pdata->rssi);
			free(pdata);
		}
	}

	return NULL;
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
	static struct lora_mac_head *phead;
	static void *_raw = NULL;
	static uint16_t _size = 0;
	static queue_data *_data = NULL;

	fixed_queue_t *temp_queue = g_queue.rx;
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
			phead = _raw;
			DBGPRINT(DEBUG_INFO, "+%s : RF_RX_DONE: ? ADDR=%02X FROM=%02X TYPE=%d SEQ=%d", __FUNCTION__,
					phead->addr.u.dst, phead->addr.u.src, phead->type, phead->seq);
			if(!IS_TOME(phead->addr.u.dst)) {
				DBGPRINT(DEBUG_ERROR, "+%s : NOT_TOME: ? ADDR=%02X", __FUNCTION__, phead->addr.u.dst);
				break;
			}
			if(phead->type != MAC_TYPE_AUTH_REQ && phead->type != MAC_TYPE_DATA) {
				DBGPRINT(DEBUG_ERROR, "+%s : NOT_ATYPE: ? TYPE=%d", __FUNCTION__, phead->type);
				break;
			}

			_data = malloc(QUEUE_DATA_HEAD_SIZE + _size - 1);
			if(!_data) {
				DBGPRINT(DEBUG_ERROR, "+%s : malloc ????????????", __FUNCTION__);
				break;
			}
			memset(_data, 0, QUEUE_DATA_HEAD_SIZE + _size - 1);
			_data->size = _size;
			_data->rssi = Radio->GetPacketRssi();
			memcpy(&_data->next, _raw, _size);
			if(phead->type == MAC_TYPE_AUTH_REQ) {
				temp_queue = g_queue.rx_config;
			}
			if(!fixed_queue_try_enqueue(temp_queue, _data)) {
				DBGPRINT(DEBUG_ERROR, "+%s : fixed_queue_try_enqueue: ????????", __FUNCTION__);
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
			Radio->StartRx(-1);
		}
		goto process_again;
	case RF_CHANNEL_ACTIVITY_DETECTED:
		DBGPRINT(DEBUG_INFO, "+%s : RF_CHANNEL_ACTIVITY_DETECTED", __FUNCTION__);
		Radio->StartRx(50);
		goto process_again;
	default:
		break;
	}

	return result;
}

static void* run_state_machine(void *arg)
{
	sigset_t set;
	tRadioDriver *Radio = arg;
	uint32_t result = RF_IDLE;

	DBGPRINT(DEBUG_INFO, "+%s : id(%d:%d)", __FUNCTION__, getpid(), gettid());
	sigemptyset(&set);
	sigaddset(&set, SIGIO);
	if(pthread_sigmask(SIG_BLOCK, &set, NULL)) {
		DBGPRINT(DEBUG_ERROR, "+%s : SIGIO mask ??????", __FUNCTION__);
	}

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
				Radio->StartRx(-1);
			} else {
				DBGPRINT(DEBUG_INFO, "+%s : fixed_queue_isn't_empty StartCad", __FUNCTION__);
				Radio->StartCad();
			}
			break;
		default:
			Radio->StartRx(-1);
			break;
		}
	}

	return NULL;
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
/* --- EXTERN FUNCTION -------------------------------------------------------- */
int lora_sendmsg(struct lora_addr *addr, const struct lora_msg *msg)
{
	struct lora_mac_head *phead = NULL;
	uint8_t *payload = NULL;
	uint16_t crc = 0;
	uint16_t mac_len = MAC_HEAD_SIZE + msg->len + MAC_CRC_SIZE;
	queue_data *_data = NULL;
	_data = malloc(QUEUE_DATA_HEAD_SIZE - 1 + mac_len);
	if(_data == NULL) {
		return -ENOMEM;
	}

	_data->size = mac_len;
	//head payload
	phead = (struct lora_mac_head *)(&_data->next);
	phead->addr.d.dst = htonl(addr->node_addr);
	phead->addr.d.src = htons(HOST_NVRAM_ADDR);
	phead->type = MAC_TYPE_DATA;
	phead->seq = addr->seq & MAC_SEQ_MASK;

	//buf payload
	payload = &_data->next + MAC_HEAD_SIZE;
	memcpy(payload, msg->data, msg->len);
	payload += msg->len;
	//CRC value
	crc = crc16(&_data->next, MAC_HEAD_SIZE + msg->len);
	memcpy(payload, &crc, MAC_CRC_SIZE);

	fixed_queue_enqueue(g_queue.tx, _data);
	wakeup_by_xmit();

	return 0;
}

int lora_recvmsg(struct lora_addr *addr, struct lora_msg *msg)
{
	uint16_t crc = 0;
	struct lora_mac_head *phead = NULL;
	uint8_t *payload = NULL;
	queue_data *_data = fixed_queue_dequeue(g_queue.rx);
	if(_data != NULL) {
		DBGPRINT(DEBUG_INFO, "+%s: size=%d rssi=%f", __FUNCTION__,
				_data->size, _data->rssi);
		phead = (struct lora_mac_head *)(&_data->next);
		addr->host_addr = ntohs(phead->addr.u.dst);
		addr->node_addr = ntohl(phead->addr.u.src);
		addr->seq = phead->seq;
		//CRC value
		payload = &_data->next + MAC_HEAD_SIZE + msg->len;
		_data->size -= MAC_CRC_SIZE;
		crc = crc16(&_data->next, _data->size);
		DBGPRINT(DEBUG_INFO, "+%s: crc=%04X crc_recv=%04X", __FUNCTION__,
				crc, *(uint16_t *)payload);
		//msg data
		payload = &_data->next + MAC_HEAD_SIZE;
		_data->size -= MAC_HEAD_SIZE;
		msg->len = (msg->len > _data->size) ? _data->size : msg->len;
		memcpy(msg->data, payload, msg->len);
		free(_data);

		return 0;
	}

	return -ENODATA;
}

int lora_mac_init()
{
	int err = 0;

	tRadioDriver *Radio = NULL;

	DBGPRINT(DEBUG_INFO, "+%s : RadioDriverInit()", __FUNCTION__);
	Radio = RadioDriverInit();

	DBGPRINT(DEBUG_INFO, "+%s : Radio->Init()", __FUNCTION__);
	Radio->Init();
	Radio->StartRx(-1);

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

	return 0;
}
/* --- EOF ------------------------------------------------------------------ */
