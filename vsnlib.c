#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <dlfcn.h>

#include <vsnlib.h>
#include <vsnshared.h>


struct ret_val{
  struct nlmsghdr* res;
  struct ret_val* next;
};

char* combine_stack_handler(char* fun_name, char* stack){
    //rimuove il .so
    char* combo;
    int len_stack = strlen(stack)-1;
    combo = malloc(sizeof(char)*(strlen(fun_name)+(len_stack-1)));
    strcpy(combo, stack);
    combo[len_stack-2] = '_';
    combo[len_stack-1] = '\0';
    strcat(combo, fun_name);
    return combo;
}


int init_vsnlib(char* stack){
  const char *error;
  char* f;
  void *stack_module;

  stack_module = dlopen(stack,RTLD_LAZY);
  if(!stack_module){
    fprintf(stderr, "Cannot open stack library: %s\n",
    dlerror());
    return -1;
  }
  for(int i=0; i < API_TABLE_SIZE; i++){
    api_table[i].real_fun = dlsym(stack_module, api_table[i].fun_name);
    if ((error = dlerror())) {
      fprintf(stderr, "Couldn't find %s: %s\n", api_table[i].fun_name, error);
      exit(1);
    }
  }
  return 0; //if success
}

void handle_vsnlib(void* buf, size_t len, void* nif, void* stack){
  struct nlmsghdr* header = (struct nlmsghdr*) buf;
  struct nlmsghdr* ret_pkg;
  while (NLMSG_OK(header, len)) {
    /*int err;*/
    int type;

    if (header->nlmsg_type == NLMSG_DONE)
      break;
    type=header->nlmsg_type - RTM_BASE;
    header->nlmsg_pid=getpid();
    if (type >= 0 && vsncli_fun_table[type] != NULL) {
        struct ret_val* app = malloc(sizeof(struct ret_val));
        (vsncli_fun_table[type](header, nif, stack));
    }
    header = NLMSG_NEXT(header, len);
  }
}

struct nlmsghdr* rtm_newlink_c(struct nlmsghdr* header, void* nif, void* stack){
  struct ifinfomsg* ifi = (struct ifinfomsg*)(NLMSG_DATA(header));
  struct config generic_conf;
  struct response* generic_resp;
  generic_conf.interface = nif;
  generic_conf.operation = ifi->ifi_flags;
  generic_resp = api_table[0].real_fun(&generic_conf);
}

struct nlmsghdr* rtm_dellink_c(struct nlmsghdr* header, void* nif, void* stack){
  struct ifinfomsg* ifi = (struct ifinfomsg*)(NLMSG_DATA(header));
  struct config* generic_conf;
  struct response* generic_resp;
  generic_resp = api_table[0].real_fun(generic_conf);
}

struct nlmsghdr* rtm_getlink_c(struct nlmsghdr* header, void* nif, void* stack){
  struct ifinfomsg* ifi = (struct ifinfomsg*)(NLMSG_DATA(header));
  struct config* generic_conf;
  struct response* generic_resp;
  generic_resp = api_table[1].real_fun(generic_conf);
}

struct nlmsghdr* rtm_setlink_c(struct nlmsghdr* header, void* nif, void* stack){
  struct ifinfomsg* ifi = (struct ifinfomsg*)(NLMSG_DATA(header));
  struct config* generic_conf;
  struct response* generic_resp;
  generic_resp = api_table[1].real_fun(generic_conf);
}

struct nlmsghdr* rtm_newaddr_c(struct nlmsghdr* header, void* nif, void* stack){
  struct ifaddrmsg* ifa = (struct ifaddrmsg*)(NLMSG_DATA(header));
  struct config generic_conf;
  struct response* generic_resp;
  char str6[INET6_ADDRSTRLEN];
  char* str4;
  generic_conf.operation=header->nlmsg_type;
  generic_conf.interface=nif;
  generic_conf.mask=ifa->ifa_prefixlen;
  generic_conf.family=ifa->ifa_family;
  struct rtattr* attr = (struct rtattr*)((void*)(((char*)ifa)+ sizeof(struct ifaddrmsg)));
  if(ifa->ifa_family == AF_INET6){
    struct in6_addr* ip6_a = (struct in6_addr*)RTA_DATA(attr);
    inet_ntop(ifa->ifa_family, ip6_a, str6, INET6_ADDRSTRLEN);
    inet_pton(ifa->ifa_family, str6, ip6_a);
    generic_conf.addr=str6;
  }
  else{
    struct in_addr ip4_a = *((struct in_addr*)RTA_DATA(attr));
    str4 = inet_ntoa(ip4_a);
    generic_conf.addr=str4;
  }
  generic_conf.stack=stack;
  generic_resp = api_table[2].real_fun(&generic_conf);
}

struct nlmsghdr* rtm_deladdr_c(struct nlmsghdr* header, void* nif, void* stack){
  rtm_newaddr_c(header, nif, stack);
}

struct nlmsghdr* rtm_getaddr_c(struct nlmsghdr* header, void* nif, void* stack){
  struct ifaddrmsg* ifa = (struct ifaddrmsg*)(NLMSG_DATA(header));
  struct config* generic_conf;
  struct response* generic_resp;
  generic_resp = api_table[3].real_fun(generic_conf);
}

struct nlmsghdr* rtm_newroute_c(struct nlmsghdr* header, void* nif, void* stack){
  struct rtmsg* rtm = (struct rtmsg*)(NLMSG_DATA(header));
  struct config generic_conf;
  struct response* generic_resp;
  char str6[INET6_ADDRSTRLEN];
  char* str4;

  generic_conf.operation=header->nlmsg_type;
  generic_conf.interface=nif;
  generic_conf.family=rtm->rtm_family;
  generic_conf.stack=stack;
  struct rtattr* attr = (struct rtattr*)((void*)(((char*)rtm)+ sizeof(struct rtmsg)));
  if(rtm->rtm_family == AF_INET6){
    struct in6_addr* ip6_a = (struct in6_addr*)RTA_DATA(attr);
    inet_ntop(rtm->rtm_family, ip6_a, str6, INET6_ADDRSTRLEN);
    generic_conf.addr=str6;
  }
  else{
    struct in_addr ip4_a = *((struct in_addr*)RTA_DATA(attr));
    str4 = inet_ntoa(ip4_a);
    generic_conf.addr=str4;
  }
  generic_resp = api_table[4].real_fun(&generic_conf);
}

struct nlmsghdr* rtm_delroute_c(struct nlmsghdr* header, void* nif, void* stack){
  rtm_newroute_c(header, nif, stack);
}

struct nlmsghdr* rtm_getroute_c(struct nlmsghdr* header, void* nif, void* stack){
  struct rtmsg* rtm = (struct rtmsg*)(NLMSG_DATA(header));
  struct config* generic_conf;
  struct response* generic_resp;
  generic_resp = api_table[5].real_fun(generic_conf);
}
