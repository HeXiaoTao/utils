/*
 * MAC functions.
 *
 */

#ifndef _LORA_MAC_H
#define _LORA_MAC_H

#ifdef __cplusplus
extern "C" {
#endif

struct lora_addr {
	uint8_t seq;
	uint16_t host_addr;
	uint32_t node_addr;
};

struct lora_msg {
	uint16_t len;
	uint8_t *data;
};

int lora_sendmsg(struct lora_addr *addr, const struct lora_msg *msg);
int lora_recvmsg(struct lora_addr *addr, struct lora_msg *msg);

int lora_mac_init();

#ifdef __cplusplus
}
#endif
#endif
