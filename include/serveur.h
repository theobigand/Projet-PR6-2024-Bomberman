#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/wait.h>
#include <net/if.h>
#include "grille.h"

#define SIZE_MESS 10
#define BUF_SIZE 1024

typedef struct s_partie_info
{
    board *b;
    char *map;
    pos *pos_joueurs[4];
    char **freq;
    uint8_t nb_freq;
    uint16_t hauteur;
    uint16_t largeur;
    uint16_t nb_joueurs;
    uint16_t num_mess_seconde;
    uint16_t num_mess_freq;
    int socket_joueurs[4];
    int en_equipe;
    uint8_t adrmdiff[16];
    uint16_t portudp;
    uint16_t portmdiff;
    int sock_tcp;
    int sock_mutli ;
    struct sockaddr_in6 gradr;

} partie_info;

typedef struct s_requete_dem_partie
{
    requete_info *info;
    uint16_t portudp;
    uint16_t portmdiff;
    uint8_t adrmdiff[16];
} requete_dem_partie_s;

typedef struct requete_partie_s
{
    requete_info *info;
    uint16_t num;
    uint16_t hauteur;
    uint16_t largeur;
    char *cases;
} requete_partie_s;

typedef struct requete_freq_partie
{
    requete_info *info;
    uint16_t num;
    u_int8_t nb;
} requete_freq_partie;

typedef struct requete_tchat_s
{
    requete_info *info;
    uint16_t len;
    char *data;
} requete_tchat_s;

typedef struct requete_fin_partie
{
    requete_info *info;
} requete_fin_partie;

struct args_envoyer
{
    partie_info *partie_inf;
    int sock;
    struct sockaddr_in6 gradr;
};

struct arg_recevoir
{
    partie_info *partie_inf;
    int sock;
    struct sockaddr_in6 gradr;
};


int generate_port_number(int seed);
void generer_adresse_multidiffusion(unsigned int seed, uint8_t *buffer);

int init_requete_info(requete_info *req, uint16_t codereq, uint16_t id, uint16_t eq);
int init_requete_dem_partie(requete_dem_partie_s *req, uint16_t id, uint16_t eq, partie_info *partie_inf);
int init_requete_partie(requete_partie_s *req, partie_info *partie);

void convert_req_info(requete_info *req, char *buf_send);
void convert_req_dem_partie(requete_dem_partie_s *req, char *buf_send);
void convert_requete_freq_partie(requete_freq_partie *req, char *buf_send);
void convert_requete_fin_partie(requete_fin_partie *req, char *buf_send);
void convert_requete_partie_(requete_partie_s *req, char *buf_send);
void convert_requete_tchat_s(requete_tchat_s *req, char *buf_send);

void *receive_fts_snd_message(void *arg);
int init_partie_info(int sock, partie_info *partie);
int jeu(partie_info *partie_inf, int sock_udp, int sock_multi, struct sockaddr_in6 gradr);
int envoi_freq(partie_info *partie_inf, int sock, struct sockaddr_in6 gradr);
int envoi_tchat(partie_info *partie_inf, char *data, int id, int eq, int sock, struct sockaddr_in6 gradr);

int init_requete_info(requete_info *req, uint16_t codereq, uint16_t id, uint16_t eq);
int init_requete_partie(requete_partie_s *req, partie_info *partie);
int init_requete_freq_partie(requete_freq_partie *req, partie_info *partie);
int init_requete_tchat(requete_tchat_s *req, partie_info *partie, char *data, int id, int eq);
int init_struct_serv(partie_info *partie_inf);
int init_requete_dem_partie(requete_dem_partie_s *req, uint16_t id, uint16_t eq, partie_info *partie_inf);
int init_partie_info(int sock, partie_info *partie);
int init_requete_dem_partie_client(requete_info* req, uint16_t codereq, uint16_t id, uint16_t eq);

int extract_coderec(char* buf);     
int extract_action(char *buf);
int extract_id(char *buf);
void generer_adresse_multidiffusion(unsigned int seed, uint8_t *buffer);
int generate_port_number(int seed);
