#include "serveur.h"
#include "client.h"

void convert_requete_partie(requete_partie* req, char* buf_send) {
    convert_req_info(req->info, buf_send);
    int pos = 2;
    u_int16_t value = req->num;
    value |= req->action << 13;
    u_int16_t aux = htons(value);
    memmove(&buf_send[pos], &aux, sizeof(u_int16_t));
}

void convert_requete_tchat_client(requete_tchat_c* req, char* buf_send) {
    convert_req_info(req->info, buf_send);
    int pos = 2;
    u_int16_t value = req->len;
    memmove(&buf_send[pos], &value, sizeof(u_int16_t));
    memmove(&buf_send[pos + sizeof(u_int16_t)], req->data, req->len);
}

void convert_req_info(requete_info *req, char *buf_send)
{
    u_int16_t value = req->codereq;
    value |= req->id << 13;
    value |= req->eq << 15;
    u_int16_t aux = htons(value);
    memmove(&buf_send[0], &aux, sizeof(u_int16_t));
}

void convert_req_dem_partie(requete_dem_partie_s *req, char *buf_send)
{
    convert_req_info(req->info, buf_send);
    int pos = 2;

    u_int16_t value_portudp = req->portudp;
    memmove(&buf_send[pos], &value_portudp, sizeof(u_int16_t));

    u_int16_t value_portmdiff = req->portmdiff;
    memmove(&buf_send[pos + sizeof(u_int16_t)], &value_portmdiff, sizeof(u_int16_t));

    memmove(&buf_send[pos + 2 * sizeof(u_int16_t)], req->adrmdiff, 16 * sizeof(u_int8_t));
}

void convert_requete_partie_(requete_partie_s *req, char *buf_send)
{
    convert_req_info(req->info, buf_send);
    int pos = 2;

    u_int16_t value_num = req->num;
    memmove(&buf_send[pos], &value_num, sizeof(u_int16_t));

    u_int16_t value_hauteur_largeur = req->hauteur;
    value_hauteur_largeur |= req->largeur << 8;
    memmove(&buf_send[pos + sizeof(u_int16_t)], &value_hauteur_largeur, sizeof(u_int16_t));

    memmove(&buf_send[pos + 2 * sizeof(u_int16_t)], req->cases, req->hauteur * req->largeur);
}

void convert_requete_freq_partie(requete_freq_partie *req, char *buf_send)
{
    convert_req_info(req->info, buf_send);
    int pos = 2;

    u_int16_t value_num = req->num;
    memmove(&buf_send[pos], &value_num, sizeof(u_int16_t));

    u_int16_t value_nb = req->nb;
    memmove(&buf_send[pos + sizeof(u_int16_t)], &value_nb, sizeof(u_int8_t));
}

void convert_requete_tchat_s(requete_tchat_s *req, char *buf_send)
{
    convert_req_info(req->info, buf_send);
    int pos = 2;

    u_int16_t value_len = req->len;
    memmove(&buf_send[pos], &value_len, sizeof(u_int16_t));

    memmove(&buf_send[pos + sizeof(u_int16_t)], req->data, req->len);
}

void convert_requete_fin_partie(requete_fin_partie *req, char *buf_send)
{
    convert_req_info(req->info, buf_send);
}



// void convert_requete_partie(requete_partie* req, char* buf_send) ;
// void convert_requete_tchat_client(requete_tchat* req, char* buf_send);
// void convert_req_info(requete_info *req, char *buf_send);
// void convert_req_dem_partie(requete_dem_partie_s *req, char *buf_send);
// void convert_requete_partie_(requete_partie_s *req, char *buf_send);
// void convert_requete_freq_partie(requete_freq_partie *req, char *buf_send);
// void convert_requete_tchat_s(requete_tchat_s *req, char *buf_send);
// void convert_requete_fin_partie(requete_fin_partie *req, char *buf_send);