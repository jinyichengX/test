#include "elnet.h"
#include "stm32f4xx_eth.h"
#include "el_netif.h"
#include "el_udp.h"
#include "el_arp.h"
#include "nmem.h"

netif_t * netif1_eth;

extern mem_mng_t g_mem_mng;
extern struct stm32_eth * g_eth;

net_err_t netif1_ll_init(netif_t * netif, void * args)
{
	struct stm32_eth * temp;
	temp = mem_alloc(&g_mem_mng, sizeof(struct stm32_eth));
	
	if (!temp) return NET_ERR_NOK;
	
	temp->sem = sys_sem_create(0);
	if (!temp->sem) {
		mem_free(&g_mem_mng, temp);
		return NET_ERR_NOK;
	}
	
	temp->lock = sys_mutex_create();
    if( temp->lock == NULL ){
		sys_sem_destroy(temp->sem);
        free(temp);
        return NET_ERR_NOK;
    }
	
	if(0 != stm32_eth_ll_init_rmii()){
		sys_sem_destroy(temp->sem);
        sys_mutex_destroy(temp->lock);
		free(temp);
		return NET_ERR_NOK;
	}
	
	list_head_init(&temp->nbuf_head);
	netif->priv_args = (void *)temp;
	g_eth = temp;
	
	return NET_ERR_OK;
}

net_err_t netif1_ll_send(netif_t * netif, void * data, uint16_t size)
{
	eth_ll_frame_transmit( (char *)data, size);
}

netif_ll_ops_t netif1_ether_ll_ops = {
    .open  = netif1_ll_init,
	.close = NULL,
    .send  = netif1_ll_send,
};

void netif1_packet_handler(void * args)
{
    netif_t * netif = (netif_t *)args;
    struct stm32_eth * eth_netif = (struct stm32_eth *)(netif->priv_args);

    nbuf_t * nbuf;
	struct eth_buffer * eth_buf;
	struct list_head * iter;
	for(;;) {
		if(!list_empty_careful(&eth_netif->nbuf_head)) {
			iter = (eth_netif->nbuf_head).next;
			eth_buf = list_entry(iter, struct eth_buffer, link);
			if(nbuf_alloc(&nbuf, eth_buf->len) == NET_ERR_OK) {
				if(nbuf_write(nbuf, (void *)eth_buf->payload, eth_buf->len) == NET_ERR_NOK) {
					plat_printf("what the fuck?!\r\n");
				}
				nbuf_acc_reset(nbuf);
			}
			else {
				plat_printf("recv and nbuf alloc err!\r\n");
				continue;
			}

			/* del from list */
			asm ("cpsid i");
			list_del(&eth_buf->link);
			mem_free(&g_mem_mng, (void *)eth_buf);
			asm ("cpsie i");

			/* instead of API netif_in_nbuf */
			netif_recv_queue_post(netif, nbuf, 0xffffffff);
			netif_in(netif);
		}
	}
}

typedef struct user_endpoint {
	struct list_head node;
	ip4addr_t remote;
	uint16_t port; }uep_t;

void chatroom_server(void * args)
{
	struct list_head user_list;
	struct list_head * pos, * tmp;
	uep_t * iter;
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
	list_head_init(&user_list);
	endpoint_t server_endp = {
		.ip4addr = {.ipa[0] = 0,.ipa[1] = 0,
					.ipa[2] = 0,.ipa[3] = 0,
		}, .port = 9999,
	};
	udp_bind(server, &server_endp);  /* bind local endpoint */
	while(1) {
        if(NET_ERR_OK != udp_recvfrom(server, msg_buf, ETHER_MTU, &remote_ip, &port, &msg_len))
			continue;
		
		/* insert new user
		 * if user ip is repeted, drop it
		 */
		int found = 0;
		list_for_each_safe(pos, tmp, &user_list) {
			iter = list_entry(pos, uep_t, node);
			if ((IPV4_ADDR_IS_EQUAL(&remote_ip, &iter->remote)) 
				&& (port == iter->port)) found = 1;
		}
		if (!found) {
			uep_t * new_user = mem_alloc(&g_mem_mng, sizeof(uep_t));
			if (new_user) {
				new_user->port = port;
				new_user->remote.ipv = IPV4_ADDR_VAL_GET(&remote_ip);
				list_head_init(&new_user->node);
				list_add_tail (&new_user->node, &user_list);
			}
		}
		
		/* broadcast the message */
		list_for_each_safe(pos, tmp, &user_list) {
			iter = list_entry(pos, uep_t, node);
			udp_sendto(server, msg_buf, msg_len, &iter->remote, iter->port);
		}
		
	}
}

void net_init(void * args) 
{
    ip4addr_t dest1;
    ipv4_str2ipaddr("192.168.1.5", &dest1);
	ip4addr_t gateway;
    ipv4_str2ipaddr("192.168.1.1", &gateway);
	netif1_eth = netif_add("if0", 
							"192.168.1.254",
							"255.255.255.0",
							&gateway, 
							LINKER_TYPE_ETHER, 
							&netif1_ether_ll_ops);
	sys_thread_create(chatroom_server, (void *)netif1_eth); /* init ok, create chatroom server thread */
	sys_thread_create(netif1_packet_handler, (void *)netif1_eth); /* init ok, create recv thread */
}

int main(void)
{ 
	Stm32_Clock_Init(360,25,2,8);	//设置时钟,180Mhz
	// delay_init(180);				//初始化延时函数
	// uart_init(90,115200);			//初始化串口
	// LED_Init();						//初始化LED时钟  
	PCF8574_Init();
	
	OSInit();

	net_start();
	sys_thread_create(net_init, NULL);
	OSStart();
}
