/*
 * MAC functions.
 *
 */

#ifndef _LORA_MAC_H
#define _LORA_MAC_H

#define HOST_NVRAM_ADDR (0x8001)

#define HOST_ADDR_SIZE (2)
#define NODE_ADDR_SIZE (4)

#define MAC_HEAD_SIZE (HOST_ADDR_SIZE + NODE_ADDR_SIZE + 1)
#define MAC_CRC_SIZE (2)

#define MAC_SEQ_MASK (0x1F)

#define MAC_TYPE_MASK (0xE0)
#define MAC_TYPE_AUTH_REQ (0x001 << 5)
#define MAC_TYPE_AUTH_RSP (0x010 << 5)
#define MAC_TYPE_DATA (0x011 << 5)
#define MAC_TYPE_ACK (0x100 << 5)

#define IS_TOME(x) \
	({	uint16_t addr = *(uint16_t *)x; \
		ntohs(addr) == HOST_NVRAM_ADDR; \
	})

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

#endif