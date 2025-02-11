#include "serveur.h"
#include "client.h"

void print_board(board* b) {
    for (int y = 0; y < b->h; y++) {
        for (int x = 0; x < b->w; x++) {
            printf("%c", b->grid[y * b->w + x]);
        }
        printf("\n");
    }
}

int extract_coderec(char* buf) {
    u_int16_t info, infor;
    memmove(&infor, buf, 2 * sizeof(u_int8_t));
    info = ntohs(infor);
    uint16_t codereq;
    codereq = info & 0x1FFF;
    return codereq;
}

void free_info_joueur(info_joueur *info)
{
    free_board(info->b);
    free(info->b);
    free(info->l);
    free(info->p);
}

int extract_action(char *buf)
{
    u_int16_t tmp;
    memmove(&tmp, &buf[2], 2);
    tmp = ntohs(tmp);
    return (tmp >> 13);
}

int extract_id(char *buf)
{
    uint16_t entete;
    memmove(&entete, &buf[0], 2);
    entete = ntohs(entete);
    return ((entete >> 13) & 0x3);
}

void generer_adresse_multidiffusion(unsigned int seed, uint8_t *buffer)
{
    srand(seed); // Initialiser le générateur de nombres pseudo-aléatoires avec la graine

    // Les parties fixes de l'adresse de multidiffusion
    const char *partie_fixe = "ff12::";

    // Générer une partie aléatoire pour l'identifiant de groupe multicast
    int identifiant_groupe = rand() % 65536; // Nombre aléatoire entre 0 et 65535

    // Convertir l'identifiant de groupe en hexadécimal
    char identifiant_groupe_hex[5];
    sprintf(identifiant_groupe_hex, "%04x", identifiant_groupe);

    // Construire l'adresse IPv6 de multidiffusion
    char adresse_ipv6[INET6_ADDRSTRLEN];
    snprintf(adresse_ipv6, sizeof(adresse_ipv6), "%s%s", partie_fixe, identifiant_groupe_hex);

    // Convertir l'adresse IPv6 en binaire
    struct in6_addr addr;
    inet_pton(AF_INET6, adresse_ipv6, &addr);

    // Copier les octets de l'adresse dans le buffer au format réseau
    memcpy(buffer, &addr, sizeof(addr));
}

int generate_port_number(int seed)
{
    // Initialiser le générateur de nombres aléatoires
    srand(seed);

    // Générer un numéro de port aléatoire entre 1024 et 49151
    return rand() % (49151 - 1024 + 1) + 1024;
}




