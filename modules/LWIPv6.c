#include <stdio.h>
#include <vsnlib.h>
#include <lwipv6.h>


struct response* adddeladdr(struct config* cfg){
  struct ip_addr addr;
  struct ip_addr mask;
  uint16_t* p_mask = NULL;

  if(cfg->family == AF_INET6){
    uint16_t mask_app[8]={0,0,0,0,0,0,0,0};
    struct in6_addr ip6;

    uint16_t* p = (uint16_t*)(&ip6);
    p_mask = (uint16_t*)(&mask_app);

    for(int i=0; i<8; i++){
      *p=0;
      p++;
    }
    p = (uint16_t*)(&ip6);
    inet_pton(cfg->family, cfg->addr, &ip6);

    while(cfg->mask > 0){
      *p_mask = 0xffff;
      cfg->mask-= 16;
      p_mask++;
    }

    p_mask = (uint16_t*)(&mask_app);

    printf("ipv6 addr:  %04X::%04X::%04X::%04X::%04X::%04X::%04X::%04X\n",htons(*(p)),htons(*(p+1)),htons(*(p+2)),htons(*(p+3)),htons(*(p+4)),htons(*(p+5)),htons(*(p+6)),htons(*(p+7)));
    IP6_ADDR(&addr,htons(*(p)),htons(*(p+1)),htons(*(p+2)),htons(*(p+3)),htons(*(p+4)),htons(*(p+5)),htons(*(p+6)),htons(*(p+7)));
    printf("mask:  %04X::%04X::%04X::%04X::%04X::%04X::%04X::%04X\n",*(p_mask),*(p_mask+1),*(p_mask+2),*(p_mask+3),*(p_mask+4),*(p_mask+5),*(p_mask+6),*(p_mask+7));
    IP6_ADDR(&mask,*(p_mask),*(p_mask+1),*(p_mask+2),*(p_mask+3),*(p_mask+4),*(p_mask+5),*(p_mask+6),*(p_mask+7));
  }
  else{
    struct in_addr ip4;
    uint16_t lwip_mask4[4] = {0,0,0,0};
    p_mask = (uint16_t*)(&lwip_mask4);
    while(cfg->mask > 0){
      *p_mask=255;
      cfg->mask -= 8;
      p_mask++;
    }
    inet_aton(cfg->addr, &ip4);
    unsigned char ip4_dot[4];
    int j=0;
    for(int i=0; i<4; i++){
      ip4_dot[i]=(ip4.s_addr >> j) & 0xFF;
      j=j+8;
    }
    printf("ipv4 address: %d.%d.%d.%d\n",ip4_dot[0],ip4_dot[1],ip4_dot[2],ip4_dot[3]);
    IP64_ADDR(&addr,ip4_dot[0],ip4_dot[1],ip4_dot[2],ip4_dot[3]);
    printf("ipv4 mask: %d.%d.%d.%d\n",lwip_mask4[0],lwip_mask4[1],lwip_mask4[2],lwip_mask4[3]);
    IP64_MASKADDR(&mask,lwip_mask4[0],lwip_mask4[1],lwip_mask4[2],lwip_mask4[3]);
  }
  if(cfg->operation == RTM_NEWADDR){
    printf("error add addr lwipv6: %d\n", lwip_add_addr((struct netif*)(cfg->interface),&addr,&mask));
  }
  else if (cfg->operation == RTM_DELADDR) {
    printf("error del addr lwipv6: %d\n", lwip_del_addr((struct netif*)(cfg->interface),&addr,&mask));
  }
}

struct response* getaddr(struct config* cfg){

}

struct response* adddellink(struct config* cfg){
  printf("adddellink\n");
  if(cfg->operation == 1)
    printf("ifup: %d\n", lwip_ifup((struct netif*)(cfg->interface)));
  else
    printf("ifdown: %d\n", lwip_ifdown((struct netif*)(cfg->interface)));
}

struct response* getsetlink(struct config* cfg){

}

struct response* adddelroute(struct config* cfg){
  struct ip_addr addr;

  if(cfg->family==AF_INET6){
    struct in6_addr address;
    uint16_t* p = (uint16_t*)(&address);
    p = (uint16_t*)(&address);
    inet_pton(cfg->family, cfg->addr, &address);
    printf("ipv6 addr:  %04X::%04X::%04X::%04X::%04X::%04X::%04X::%04X\n",htons(*(p)),htons(*(p+1)),htons(*(p+2)),htons(*(p+3)),htons(*(p+4)),htons(*(p+5)),htons(*(p+6)),htons(*(p+7)));
    IP6_ADDR(&addr,htons(*(p)),htons(*(p+1)),htons(*(p+2)),htons(*(p+3)),htons(*(p+4)),htons(*(p+5)),htons(*(p+6)),htons(*(p+7)));
  }
  else{
    struct in_addr ip4;
    inet_aton(cfg->addr, &ip4);
    unsigned char ip4_dot[4];
    int j=0;
    for(int i=0; i<4; i++){
      ip4_dot[i]=(ip4.s_addr >> j) & 0xFF;
      j=j+8;
    }
    printf("ipv4 route: %d.%d.%d.%d\n",ip4_dot[0],ip4_dot[1],ip4_dot[2],ip4_dot[3] );
    IP64_ADDR(&addr,ip4_dot[0],ip4_dot[1],ip4_dot[2],ip4_dot[3]);
  }

  if(cfg->operation==RTM_NEWROUTE){
    printf("error add route lwipv6 %d\n", lwip_add_route((struct stack*)cfg->stack, IP_ADDR_ANY, IP_ADDR_ANY, &addr, (struct netif*)(cfg->interface), 0));
  }
  else if(cfg->operation==RTM_DELROUTE){
    printf("error del route lwipv6: %d\n", lwip_del_route((struct stack*)cfg->stack, IP_ADDR_ANY, IP_ADDR_ANY, &addr, (struct netif*)(cfg->interface), 0));
  }
}

struct response* getroute(struct config* cfg){

}
