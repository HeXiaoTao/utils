/*
 * MAC functions.
 *
 */

#ifndef _LORA_MAC_H
#define _LORA_MAC_H

#ifdef __cplusplus
extern "C" {
#endif

#define HOST_NVRAM_ADDR (0x8001)

#define HOST_ADDR_SIZE (2)
#define NODE_ADDR_SIZE (4)

#define MAC_HEAD_SIZE (HOST_ADDR_SIZE + NODE_ADDR_SIZE + 1)
#define MAC_CRC_SIZE (2)

#define MAC_SEQ_MASK (0x1F)

#define MAC_TYPE_AUTH_REQ (1)
#define MAC_TYPE_AUTH_RSP (2)
#define MAC_TYPE_DATA (3)
#define MAC_TYPE_ACK (4)

#define IS_TOME(x) \
	({	ntohs(x) == HOST_NVRAM_ADDR; \
	})

struct lora_mac_head {
        union {
                struct {
                        uint16_t dst;
                        uint32_t src;
                } __attribute__((aligned(1),packed)) u;
                struct {
                        uint32_t dst;
                        uint16_t src;
                } __attribute__((aligned(1),packed)) d;
        } addr;

        uint8_t seq:5;
        uint8_t type:3;
} __attribute__((aligned(1),packed));

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
