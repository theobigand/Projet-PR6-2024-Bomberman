#include "serveur.h"
#include "client.h"

int init_requete_info(requete_info *req, uint16_t codereq, uint16_t id, uint16_t eq)
{
    req->codereq = codereq;
    req->id = id;
    req->eq = eq;
    return 1;
}

int init_requete_partie(requete_partie_s *req, partie_info *partie)
{
    req->info = calloc(1, sizeof(requete_info));
    if (req->info == NULL)
        return (0);
    init_requete_info(req->info, 11, 0, 0);
    req->num = htons(partie->num_mess_seconde);
    req->hauteur = partie->hauteur;
    req->largeur = partie->largeur;
    req->cases = partie->map;
    return 1;
}

int init_requete_freq_partie(requete_freq_partie *req, partie_info *partie)
{
    req->info = calloc(1, sizeof(requete_info));
    if (req->info == NULL)
        return (0);
    init_requete_info(req->info, 12, 0, 0);
    req->num = htons(partie->num_mess_freq);
    req->nb = partie->nb_freq;
    return 1;
}

int init_requete_tchat(requete_tchat_s *req, partie_info *partie, char *data, int id, int eq)
{
    req->info = calloc(1, sizeof(requete_info));
    if (req->info == NULL)
        return (0);
    init_requete_info(req->info, 13, id, eq);

    req->len = strlen(data);
    memmove(req->data, data, strlen(data));
    return 1;
}

void init_placer_joueur(partie_info *partie_inf)
{
    for (int i = 0; i < 4; i++)
    {
        partie_inf->pos_joueurs[i] = calloc(1, sizeof(pos));
        if (!partie_inf->pos_joueurs[i])
        {
            perror("Erreur lors de l'allocation de mémoire pour pos_joueurs");
            exit(EXIT_FAILURE);
        }
        partie_inf->pos_joueurs[i]->x = (i < 2) ? 1 : partie_inf->b->h - 2;
        partie_inf->pos_joueurs[i]->y = (i % 2 == 0) ? 1 : partie_inf->b->w - 2;
        set_grid(partie_inf->b, partie_inf->pos_joueurs[i]->x, partie_inf->pos_joueurs[i]->y, '5' + i);
    }
}

int init_struct_serv(partie_info *partie_inf)
{
    partie_inf->b = calloc(1, sizeof(board));
    if (!partie_inf->b)
    {
        perror("Erreur lors de l'allocation de mémoire pour le plateau");
        return 0;
    }

    setup_board(partie_inf->b);
    init_placer_joueur(partie_inf);
    partie_inf->hauteur = partie_inf->b->h;
    partie_inf->largeur = partie_inf->b->w;
    partie_inf->map = partie_inf->b->grid;

    partie_inf->nb_freq = 0;

    partie_inf->num_mess_seconde = 0;
    partie_inf->num_mess_freq = 0;

    generer_adresse_multidiffusion(1234, partie_inf->adrmdiff);

    partie_inf->portudp = generate_port_number(667);
    partie_inf->portmdiff = generate_port_number(754);

    return 1;
}

info_joueur *init_info_joueur()
{
    info_joueur *res = calloc(1, sizeof(info_joueur));
    if (res == NULL)
    {
        exit(EXIT_FAILURE);
        return NULL;
    }
    res->b = calloc(1, sizeof(board));
    res->l = calloc(1, sizeof(line));
    res->p = calloc(1, sizeof(pos));
    if (!res->b || !res->l || !res->p)
    {
        exit(EXIT_FAILURE);
        return NULL;
    }
    res->l->cursor = 0;
    res->p->x = 0;
    res->p->y = 0;
    return res;
}

int init_requete_dem_partie(requete_dem_partie_s *req, uint16_t id, uint16_t eq, partie_info *partie_inf)
{
    req->info = calloc(1, sizeof(requete_info));
    if (req->info == NULL)
        return (0);
    init_requete_info(req->info, 9, id, eq);
    int port_udp = partie_inf->portudp; // Premier numéro de port
    uint16_t p_udp_n = htons(port_udp);
    int port_multidiffusion = partie_inf->portmdiff; // Deuxième numéro de port
    uint16_t p_mdiff_n = htons(port_multidiffusion);
    req->portudp = p_udp_n;
    req->portmdiff = p_mdiff_n;

    memmove(req->adrmdiff, partie_inf->adrmdiff, 16 * sizeof(uint8_t));
    char addr_str[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, req->adrmdiff, addr_str, INET6_ADDRSTRLEN);
    printf("Adresse IPv6 de multidiffusion : %s\n", addr_str);
    printf("Port UDP : %d\n", ntohs(req->portudp));
    printf("Port de multidiffusion : %d\n", ntohs(req->portmdiff));

    return 1;
}

int init_requete_dem_partie_client(requete_info* req, uint16_t codereq, uint16_t id, uint16_t eq) {
    req->codereq = codereq;
    req->id = id;
    req->eq = eq;
    return (1);
}

int init_requete_partie_client(requete_partie* req, uint16_t codereq, uint16_t id, uint16_t eq, uint16_t num, uint16_t action) {
    req->info = calloc(1, sizeof(requete_info));
    if (req->info == NULL)
        return (0);
    init_requete_info(req->info, codereq, id, eq);
    req->num = num;
    req->action = action;
    // printf("Code de requête : %d |  Equipe :  %d  |  Id: %d\n", req->codereq, req->eq, req->id); // Conversion au format hôte
     //printf("Numéro de message : %d |  Action :  %d\n", req->num, req->action);                   // Conversion au format hôte
    return (1);
}

int init_requete_tchat_client(requete_tchat_c* req, uint16_t codereq, uint16_t id, uint16_t eq, uint16_t len, char* data) {
    // req->codereq = codereq;
    // req->id = id;
    // req->eq = eq;
    req->info = calloc(1, sizeof(requete_info));
    if (req->info == NULL)
        return (0);
    init_requete_info(req->info, codereq, id, eq);
    req->len = len;
    req->data = data;
    return (1);
}
