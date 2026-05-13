#include "el_udp.h"

extern struct mem_mng g_mem_mng;
#define SERV_PORT 9999
void udp_echo_server_demo(void * args)
{
	uint16_t msg_len, port;
	ip4addr_t remote_ip;
	
	udp_t * server = udp_create();
	if (!server)
		while(1);
	
	void * msg_buf = mem_alloc(&g_mem_mng, ETHER_MTU);
	if (!msg_buf) {
		udp_destroy(server);
		while(1);
	}

	endpoint_t server_endp = {
		.ip4addr = {.ipa[0] = 0,.ipa[1] = 0,
					.ipa[2] = 0,.ipa[3] = 0,
		}, .port = SERV_PORT,
	};
	udp_bind(server, &server_endp);  /* bind local endpoint */
	while(1) {
        /* server remote host */
        udp_recvfrom(server, msg_buf, ETHER_MTU, &remote_ip, &port, &msg_len);
		udp_sendto(server, msg_buf, msg_len, &remote_ip, port);
	}
}
