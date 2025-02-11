#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <net/if.h>
#include "grille.h"
#define BUF_SIZE 1024

typedef struct s_info_joueur {
  uint16_t id;
  uint16_t eq;
  uint16_t portudp;
  uint16_t portmdiff;
  //int sock_udp;
  struct sockaddr_in6 adr;
  char adrmdiff[40];
  int en_equipe;
  board* b;
  line* l;
  pos* p;
  int sock_tcp;
} info_joueur;

struct args_action {
    info_joueur *info;
};

struct args_recevoir{
    info_joueur *info;
    int sock;  
};

typedef struct requete_partie {
  requete_info *info;
  uint16_t num;
  uint16_t action;
} requete_partie;

typedef struct requete_tchat_c{
  requete_info *info;
  uint16_t len;
  char* data;
} requete_tchat_c;


void convert_requete_partie(struct requete_partie* req, char* buf_send);

void envoi_mess_client(int sock_client, char* buf_send, int is_udp, info_joueur * info);

int init_requete_info(requete_info *req, uint16_t codereq, uint16_t id, uint16_t eq);
int init_requete_dem_partie_client(requete_info* req, uint16_t codereq, uint16_t id, uint16_t eq);
int init_requete_partie_client(requete_partie* req, uint16_t codereq, uint16_t id, uint16_t eq, uint16_t num, uint16_t action);
int init_requete_tchat_client(requete_tchat_c* req, uint16_t codereq, uint16_t id, uint16_t eq, uint16_t len, char* data);

void convert_requete_tchat_client(requete_tchat_c* req, char* buf_send);
info_joueur *init_info_joueur();
void convert_req_info(requete_info *req, char *buf_send);
void free_info_joueur(info_joueur *info);
int extract_action(char *buf);
int extract_coderec(char* buf);