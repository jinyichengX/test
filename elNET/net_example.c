// #include "plat.h"
// #include "elnet.h"
// #include "el_netif.h"
// #include "el_ipv4.h"
// #include "el_nbuf.h"
// #include "timer.h"
// #include "el_udp.h"
// #include "el_tcp.h"
// #include "bitmap.h"
// #include "el_ether.h"
// #include "el_icmpv4.h"
// #include "hash.h"
// #include "socket.h"
// #include "global.h"
// #include "sock.h"
// #include "tftp.h"
// #include "nmem.h"
// #include "dbg.h"
// #include <time.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include "el_ipaddr.h"
// pcap_t * netif;
// netif_t * netif_eth0;
// uint8_t fake_data[21] = {1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1};

// struct pcap_netif_t{
//     pcap_t * netif;
//     struct pcap_pkthdr * header;    /* 数据包头 */
//     const u_char * packet_data;     /* 数据载荷 */
//     sys_mutex_t lock;
//     sys_sem_t sem;//同步接收
//     sys_sem_t ready_recv;//只有一个同步接收可能会出问题，再加一个同步信号，用于准备接收，先不用，因为没环境测试//20250115
// };

// extern mem_mng_t g_mem_mng;

// typedef struct user_endpoint {
// 	struct list_head node;
// 	ip4addr_t remote;
// 	uint16_t port; }uep_t;

// void thread1(void * args)
// {
// 	struct list_head user_list;
// 	struct list_head * pos, * tmp;
// 	uep_t * iter;
// 	uint16_t msg_len, port;
// 	ip4addr_t remote_ip;
	
// 	udp_t * server = udp_create();
// 	if (!server)
// 		while(1);
	
// 	void * msg_buf = mem_alloc(&g_mem_mng, ETHER_MTU);
// 	if (!msg_buf) {
// 		udp_destroy(server);
// 		while(1);
// 	}
// 	INIT_LIST_HEAD(&user_list);
// 	endpoint_t server_endp = {
// 		.ip4addr = {.ipa[0] = 0,.ipa[1] = 0,
// 					.ipa[2] = 0,.ipa[3] = 0,
// 		}, .port = 9999,
// 	};
// 	udp_bind(server, &server_endp);  /* bind local endpoint */
// 	while (1) {
//         udp_recvfrom(server, msg_buf, ETHER_MTU, &remote_ip, &port, &msg_len);
		
// 		/* insert new user
// 		 * if user ip is repeted, drop it
// 		 */
// 		int found = 0;
// 		list_for_each_safe(pos, tmp, &user_list) {
// 			iter = list_entry(pos, uep_t, node);
// 			if ((IPV4_ADDR_IS_EQUAL(&remote_ip, &iter->remote)) 
// 				&& (port == iter->port)) found = 1;
// 		}
// 		if (!found) {
// 			uep_t * new_user = mem_alloc(&g_mem_mng, sizeof(uep_t));
// 			if (new_user) {
// 				new_user->port = port;
// 				new_user->remote.ipv = IPV4_ADDR_VAL_GET(&remote_ip);
// 				INIT_LIST_HEAD(&new_user->node);
// 				list_add_tail (&new_user->node, &user_list);
// 			}
// 		}
		
// 		/* broadcast the message */
// 		list_for_each_safe(pos, tmp, &user_list) {
// 			iter = list_entry(pos, uep_t, node);
// 			udp_sendto(server, msg_buf, msg_len, &iter->remote, iter->port);
// 		}
		
// 	}
// }

// DWORD thread2(LPVOID args) {
//     nbuf_t * nbuf_udp1 = NULL;
// 	plat_printf("thread2 is running\n");
//     ip4addr_t dest1;
//     ipv4_str2ipaddr("192.168.1.3", &dest1);
// 	while (1) {
//         // if(NET_ERR_OK == generate_base_packet(&nbuf_udp1, (void *)fake_data, sizeof(fake_data))){
//         //     udp_out_raw(&dest1, &netif_eth0->ipaddr, 40010, 55554, nbuf_udp1);
//         //     //plat_printf("send to 192.168.1.201\r\n");
//         // }
//         // udp_out(&dest1, &netif_eth0->ipaddr, 8888, 8888,  (void *)fake_data, sizeof(fake_data));
//         // Sleep(1000);
// 	}
// 	return 1;
// }

// DWORD netif0_packet_recv(LPVOID args)
// {
//     int res;
//     int count;
//     netif_t * netif = (netif_t *)args;
//     struct pcap_netif_t * pcap_netif = (struct pcap_netif_t *)(netif->priv_args);
//     do{
//         // sys_sem_take(pcap_netif->ready_recv, 0xffffffff);
//         sys_mutex_lock(pcap_netif->lock);
//         while((res = pcap_next_ex(pcap_netif->netif, &pcap_netif->header, &pcap_netif->packet_data)) > 0)
//         {
//             if (res == 0)
//             {
//                 //sys_sem_release(pcap_netif->ready_recv);//pcap_next_ex返回值改为">=0"时取消注释
//                 break;
//             }
//             if(pcap_netif->header->len > 1514)
//             {
//                 plat_printf("recv len > 1514!\r\n");
//                 break;
//             }
//             //plat_printf("recv %d\r\n",count++);
//             sys_sem_release(pcap_netif->sem);
//             break;
//         }
//         sys_mutex_unlock(pcap_netif->lock);
//     }while(1);

// 	return 1;
// }

// DWORD netif0_packet_handler(LPVOID args)
// {
//     netif_t * netif = (netif_t *)args;
//     struct pcap_netif_t * pcap_netif = (struct pcap_netif_t *)(netif->priv_args);

//     nbuf_t * nbuf;
//     while(1){
//         sys_sem_take(pcap_netif->sem, 0xffffffff);
//         sys_mutex_lock(pcap_netif->lock);
//         if(nbuf_alloc(&nbuf, pcap_netif->header->len) == NET_ERR_OK){
//             if(nbuf_write(nbuf, (void *)pcap_netif->packet_data, pcap_netif->header->len) == NET_ERR_NOK){
//                 plat_printf("why pcap_netif->header->len changed?!\r\n");
//             }
//             sys_mutex_unlock(pcap_netif->lock);
//             nbuf_acc_reset(nbuf);
//         }
//         else{
//             sys_mutex_unlock(pcap_netif->lock);
//             plat_printf("recv and nbuf alloc err!\r\n");
//             continue;
//         }
//         // sys_sem_release(pcap_netif->ready_recv);

//         /* 下面两行可以用netif_in_nbuf代替 */
//         netif_recv_queue_post(netif, nbuf, 0xffffffff);
//         netif_in(netif);
//     }
//     return 1;
// }

// DWORD netif0_packet_handler2(LPVOID args)
// {
//     netif_t * netif = (netif_t *)args;
//     struct pcap_netif_t * pcap_netif = (struct pcap_netif_t *)(netif->priv_args);

//     nbuf_t * nbuf;
//     while(1){
//         sys_sem_take(pcap_netif->sem, 0xffffffff);
//         sys_mutex_lock(pcap_netif->lock);
//         if(nbuf_alloc(&nbuf, pcap_netif->header->len) == NET_ERR_OK){
//             if(nbuf_write(nbuf, (void *)pcap_netif->packet_data, pcap_netif->header->len) == NET_ERR_NOK){
//                 plat_printf("why pcap_netif->header->len changed?!\r\n");
//             }
//             sys_mutex_unlock(pcap_netif->lock);
//             nbuf_acc_reset(nbuf);
//         }
//         else{
//             sys_mutex_unlock(pcap_netif->lock);
//             plat_printf("recv and nbuf alloc err!\r\n");
//             continue;
//         }
//         // sys_sem_release(pcap_netif->ready_recv);

//         /* 下面两行可以用netif_in_nbuf代替 */
//         netif_recv_queue_post(netif, nbuf, 0xffffffff);
//         netif_in(netif);
//     }
//     return 1;
// }

// DWORD netif0_packet_handler3(LPVOID args)
// {
//     netif_t * netif = (netif_t *)args;
//     struct pcap_netif_t * pcap_netif = (struct pcap_netif_t *)(netif->priv_args);

//     nbuf_t * nbuf;
//     while(1){
//         sys_sem_take(pcap_netif->sem, 0xffffffff);
//         sys_mutex_lock(pcap_netif->lock);
//         if(nbuf_alloc(&nbuf, pcap_netif->header->len) == NET_ERR_OK){
//             if(nbuf_write(nbuf, (void *)pcap_netif->packet_data, pcap_netif->header->len) == NET_ERR_NOK){
//                 plat_printf("why pcap_netif->header->len changed?!\r\n");
//             }
//             sys_mutex_unlock(pcap_netif->lock);
//             nbuf_acc_reset(nbuf);
//         }
//         else{
//             sys_mutex_unlock(pcap_netif->lock);
//             plat_printf("recv and nbuf alloc err!\r\n");
//             continue;
//         }
//         // sys_sem_release(pcap_netif->ready_recv);
        
//         /* 下面两行可以用netif_in_nbuf代替 */
//         netif_recv_queue_post(netif, nbuf, 0xffffffff);
//         netif_in(netif);
//     }
//     return 1;
// }

// net_err_t sys_netif_open(netif_t * netif, void * args)
// {
//     struct pcap_netif_t * pcap_netif = malloc(sizeof(struct pcap_netif_t));
//     if( pcap_netif == NULL )
//         return NET_ERR_NOK;

//     pcap_netif->sem = sys_sem_create(0);
//     if( pcap_netif->sem == NULL ){
//         free(pcap_netif);
//         return NET_ERR_NOK;
//     }

//     pcap_netif->lock = sys_mutex_create();
//     if( pcap_netif->lock == NULL ){
//         sys_sem_destroy(pcap_netif->sem);
//         free(pcap_netif);
//         return NET_ERR_NOK;
//     }

//     pcap_netif->ready_recv = sys_sem_create(1);
//     if( pcap_netif->ready_recv == NULL ){
//         sys_sem_destroy(pcap_netif->sem);
//         sys_mutex_destroy(pcap_netif->lock);
//         free(pcap_netif);
//         return NET_ERR_NOK;
//     }

//     pcap_netif->netif = sys_netif_Initialise();
//     if(pcap_netif->netif == NULL){
//         sys_sem_destroy(pcap_netif->sem);
//         sys_sem_destroy(pcap_netif->ready_recv);
//         sys_mutex_destroy(pcap_netif->lock);
//         free(pcap_netif);
//         return NET_ERR_NOK;
//     }

//     netif->priv_args = (void *)pcap_netif;
//     return NET_ERR_OK;
// }
// net_err_t ether5_send(netif_t * netif, void * data_buff, uint16_t len)
// {
//     struct pcap_t * pacap_netif = ((struct pcap_netif_t *)(netif->priv_args))->netif;
//     pcap_sendpacket((pcap_t *)pacap_netif, (const u_char *)data_buff, (int)len);
//     return NET_ERR_OK;
// }

// netif_ll_ops_t netif_ether5_ll_ops = {
//     .open = sys_netif_open,
//     .send = ether5_send,
// };

// void net_example(void)
// {
//     /* NET */
//     net_start();
//     ip4addr_t gateway;
//     ipv4_str2ipaddr("192.168.1.1", &gateway);
//     netif_eth0 = netif_add("if0", "192.168.1.5", "255.255.255.0", &gateway, LINKER_TYPE_ETHER, &netif_ether5_ll_ops);
//     ip4addr_t gateway1;
//     ipv4_str2ipaddr("192.168.0.1", &gateway1);//路由器的WAN口IP，本机和WAN口构成小型局域网
//     // netif_eth0 = netif_add("if0", "192.168.0.105", "255.255.255.0", &gateway, LINKER_TYPE_ETHER, &netif_ether5_ll_ops);//改gateway
//     sys_thread_t handle1 = sys_thread_create(thread1, NULL);
//     sys_thread_t handle2 = sys_thread_create(thread2, NULL);
//     sys_thread_t handle3 = sys_thread_create(netif0_packet_recv, (void *)netif_eth0);
//     sys_thread_t handle4 = sys_thread_create(netif0_packet_handler, (void *)netif_eth0);
//     sys_thread_t handle5 = sys_thread_create(netif0_packet_handler2, (void *)netif_eth0);
//     sys_thread_t handle6 = sys_thread_create(netif0_packet_handler3, (void *)netif_eth0);
//     Sleep(500);
//     tcp_cb_t tcp;
//     memset(&tcp, 0, sizeof(tcp_cb_t));
//     ip4addr_t serv_ip;ipv4_str2ipaddr("192.168.1.3", &serv_ip);
//     tcp_connect_to(&tcp, &serv_ip, 8080);
//     sys_thread_t handle[6] = { handle1, handle2, handle3, handle4, handle5, handle6 };
//     WaitForMultipleObjects(6, handle, TRUE, INFINITE);
// }