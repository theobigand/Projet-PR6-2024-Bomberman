#include "client.h"
#include "pthread.h"

#define SIZE_MESS 1024

void envoi_mess_client(int sock_client, char* buf_send, int is_udp, info_joueur* info) { // le client annonce qu'il est pret à intégrer une partie
    int r = send(sock_client, buf_send, sizeof(buf_send), 0);
    if (r < 0) {
        perror("erreur d'envoi\n");
        close(sock_client);
        exit(EXIT_FAILURE);
    }
}

void send_action_to_server(info_joueur* info, int action) {
    // envoi d'un action
    char buffer_req_partie[4];
    memset(buffer_req_partie, 0, 4);
    requete_partie* req_trois = calloc(1, sizeof(requete_partie));
    init_requete_partie_client(req_trois, 5, info->id, info->eq, 0, action);
    convert_requete_partie(req_trois, buffer_req_partie);
    // printf("%ld\n", sizeof(buffer_req_partie));
    int sock_udp = socket(PF_INET6, SOCK_DGRAM, 0);
    if (sock_udp < 0)
        perror("error\n");
    // adresse de destination
    struct sockaddr_in6 servadr;
    memset(&servadr, 0, sizeof(servadr));
    servadr.sin6_family = AF_INET6;
    servadr.sin6_addr = in6addr_any;
    servadr.sin6_port = htons(info->portudp);
    socklen_t s = sizeof(servadr);
    int r = sendto(sock_udp, buffer_req_partie, sizeof(buffer_req_partie), 0, (struct sockaddr*)&servadr, s);
    if (r == -1) {
        perror("erreur d'envoi\n");
        close(sock_udp);
        exit(EXIT_FAILURE);
    }

    free(req_trois);
}

void update_all_board(board* b, char* buf) {
    // print_board(b);
    memmove(b->grid, buf + 6, 400);
}

void send_chat_to_server(info_joueur *info)
{
    char buf[BUF_SIZE] = {0};
    requete_tchat_c *req = calloc(1, sizeof(requete_tchat_c));
    init_requete_tchat_client(req, 7, info->id, info->eq, info->l->cursor, info->l->data);
    convert_requete_tchat_client(req, buf);
    // printf("%d--------------------------------------\n", info->sock_tcp);
    envoi_mess_client(info->sock_tcp, buf, 0, info);
}

void recv_changed_case(info_joueur* info, int sock_mdif, int taille) {
    char buf[BUF_SIZE] = { 0 };
    memset(buf, 0, sizeof(buf));
    struct sockaddr_in6 diffadr;
    int recu;
    socklen_t difflen = sizeof(diffadr);
    for (int i = 0; i < taille; i++) {
        memset(buf, 0, sizeof(buf));
        if ((recu = recvfrom(sock_mdif, buf, sizeof(buf) - 1, 0, (struct sockaddr*)&diffadr, &difflen)) < 0) {
            perror("echec de recvfrom.");
            exit(EXIT_FAILURE);
        }
        unsigned char ligne = buf[0];
        unsigned char colonne = buf[1];
        unsigned char contenu = buf[2];
        set_grid(info->b, ligne, colonne, contenu);
    }
}

void manage_chat(info_joueur *info)
{
    ACTION a = control_chat(info->l);
    int ret = manage_action(a);

    if (a == CHAT)
        send_chat_to_server(info);
    if (ret== QUIT_GAME)
    {
        
    }
    refresh_game(info->b, info->l);
}

int s_abonner_multidiffusion(info_joueur* info_j) {
    int sock;
    /* créer la socket */
    if ((sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("echec de socket");
        exit(EXIT_FAILURE);
    }

    /* SO_REUSEADDR permet d'avoir plusieurs instances locales de cette application  */
    /* ecoutant sur le port multicast et recevant chacune les differents paquets       */
    int ok = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0) {
        perror("echec de SO_REUSEADDR");
        close(sock);
        exit(EXIT_FAILURE);
    }

    /* Initialisation de l'adresse de reception */
    struct sockaddr_in6 adr;
    memset(&adr, 0, sizeof(adr));
    adr.sin6_family = AF_INET6;
    adr.sin6_addr = in6addr_any;
    adr.sin6_port = htons(info_j->portmdiff);

    if (bind(sock, (struct sockaddr*)&adr, sizeof(adr))) {
        perror("echec de bind");
        close(sock);
        exit(EXIT_FAILURE);
    }
    /* initialisation de l'interface locale autorisant le multicast IPv6 */
    int ifindex = if_nametoindex("eth0");
    if (ifindex == 0)
        perror("if_nametoindex");

    /* s'abonner au groupe multicast */
    struct ipv6_mreq group;
    inet_pton(AF_INET6, info_j->adrmdiff, &group.ipv6mr_multiaddr.s6_addr);
    group.ipv6mr_interface = ifindex;

    if (setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof group) < 0) {
        perror("echec de abonnement groupe");
        close(sock);
        exit(EXIT_FAILURE);
    }
    info_j->adr = adr;
    return sock;
}

void config_debut_parti(int sock_client, info_joueur* info_j) {
    char buffer[16 * sizeof(u_int8_t) + 3 * sizeof(u_int16_t)]; // Taille du buffer pour stocker les données reçues
    ssize_t bytes_received = recv(sock_client, buffer, sizeof(buffer), 0);
    if (bytes_received == -1) {
        perror("Erreur lors de la réception des données du serveur");
        exit(EXIT_FAILURE);
    }

    // Extraire les deux premiers octets du buffer pour obtenir le code de requête, l'id et eq
    u_int16_t info, infor;
    memmove(&infor, buffer, 2 * sizeof(u_int8_t));
    info = ntohs(infor);
    info_j->id = (info >> 13) & 0x3;
    info_j->eq = (info >> 15) & 0x1;

    // Extraire les deux numéros de port du buffer
    int port_udp, port_multidiffusion;
    memmove(&port_udp, buffer + 2, sizeof(u_int16_t));
    memmove(&port_multidiffusion, buffer + 4, sizeof(u_int16_t));
    info_j->portudp = ntohs(port_udp);
    info_j->portmdiff = ntohs(port_multidiffusion);

    // Extraire l'adresse IPv6 du buffer
    uint8_t ipv6_buffer[16];
    memmove(ipv6_buffer, buffer + 6, 16 * sizeof(u_int8_t));

    char addr_str[40];
    inet_ntop(AF_INET6, ipv6_buffer, addr_str, 40);
    memmove(info_j->adrmdiff, addr_str, 40);

    // Afficher les données reçues
    /*printf("Code de requête : %d\n", codereq); // Conversion au format hôte
    printf("Identifiant : %d\n", info_j->id);  // Conversion au format hôte
    printf("Equipe : %d\n", info_j->eq);       // Conversion au format hôte
    printf("Adresse IPv6 de multidiffusion : %s\n", info_j->adrmdiff);
    printf("Port UDP : %d\n", info_j->portudp);                 // Conversion au format hôte
    printf("Port de multidiffusion : %d\n", info_j->portmdiff); // Conversion au format hôte*/
}

void* manage_board(void* arg) {
    struct args_action* args = (struct args_action*)arg;
    info_joueur* info = args->info;
    // memmove(info->b->grid, buf + 6, 400);
    ACTION a = control(info->l);
    int ret = manage_action(a);
    // Vérifier si l'action est de quitter en appuyant sur la touche "q"
    if (a != -1) {
        switch (ret) {

        case MOVE_NORTH: case MOVE_EAST: case MOVE_SOUTH: case MOVE_WEST:
            send_action_to_server(info, ret);
            break;
        case QUIT_GAME:
            free_info_joueur(info);
            end_print();
            exit(0);
        default:
            break;
        }
        refresh_game(info->b, info->l);
    } else {
        end_print();
        free_info_joueur(info);
        exit(0);
    }
    return NULL;
}

void* recv_game_info(void* arg) {
    struct args_recevoir* args = (struct args_recevoir*)arg;
    info_joueur* info = args->info;
    int sock_mdif = args->sock;

    /* recevoir les messages */
    char buf[BUF_SIZE] = { 0 };
    memset(buf, 0, sizeof(buf));

    // ou avec recv ou recvfrom
    struct sockaddr_in6 diffadr;
    int recu;
    socklen_t difflen = sizeof(diffadr);

    memset(buf, 0, sizeof(buf));
    if ((recu = recvfrom(sock_mdif, buf, sizeof(buf), 0, (struct sockaddr*)&diffadr, &difflen)) < 0) {
        perror("echec de recvfrom.");
        exit(EXIT_FAILURE);
    }

    int codereq = extract_coderec(buf);
    uint8_t taille;
    switch (codereq) {
    case 11:
        update_all_board(info->b, buf);
        break;
    case 12:
        // TODO: gerer code req 12 (freq)
        memmove(&taille, buf + 4, sizeof(u_int8_t));
        recv_changed_case(info, sock_mdif, taille);
        break;
    default:
        fprintf(stderr, "Unknown message type received: %d\n", codereq);
        break;
    }

    return NULL;
}

// la dedans créer une socket udp qui va stocker les informations reçu du server et s'abonner à l'adresse de multidiffusion
int main(void) {
    int sock_client = socket(PF_INET6, SOCK_STREAM, 0);
    if (sock_client == -1) {
        perror("creation socket");
        exit(1);
    }
    struct sockaddr_in6 adrsock;
    memset(&adrsock, 0, sizeof(adrsock));
    adrsock.sin6_family = AF_INET6;
    adrsock.sin6_port = htons(8080);
    inet_pton(AF_INET6, "localhost", &adrsock.sin6_addr);
    int r = connect(sock_client, (struct sockaddr*)&adrsock, sizeof(adrsock));
    if (r < 0) {
        perror("erreur de connect");
        close(sock_client);
        exit(1);
    }
    info_joueur* info_j = init_info_joueur();
    if (!info_j)
        return (1);

    requete_info* req_un = calloc(1, sizeof(requete_info));

    init_requete_dem_partie_client(req_un, 1, 0, 0);
    char buffer[2] = {0};
    convert_req_info(req_un, buffer);
    // convert_req_dem_partie(req_un, buffer);
    envoi_mess_client(sock_client, buffer, 0, NULL);
    //printf("Code de requête : %d |  Equipe :  %d  |  Id: %d\n", req_un->codereq, req_un->eq, req_un->id); // Conversion au format hôte
    config_debut_parti(sock_client, info_j);
    free(req_un);

    requete_info* req_deux = calloc(1, sizeof(requete_info));
    if (!req_deux)
        return 1;
    memset(buffer, 0, 2);
    init_requete_dem_partie_client(req_deux, 3, info_j->id, info_j->eq);
    convert_req_info(req_deux, buffer);
    // convert_req_dem_partie(req_deux, buffer);
    envoi_mess_client(sock_client, buffer, 0, NULL);
    // printf("Code de requête : %d |  Equipe :  %d  |  Id: %d\n", req_deux->codereq, req_deux->eq, req_deux->id); // Conversion au format hôte
    free(req_deux);

    init_print();
    setup_board(info_j->b);

    int sock_mult = s_abonner_multidiffusion(info_j);
    struct args_action *arg_action = calloc(1, sizeof(arg_action));
    if (!arg_action)
        return 1;
    arg_action->info = info_j;

    struct args_recevoir *arg_recv = calloc(1, sizeof(arg_recv));
    if (!arg_recv)
    {
        free(arg_action);
        return 1;
    }
    arg_recv->info = info_j;
    arg_recv->sock = sock_mult;


    while (1) {
        pthread_t thread_action, thread_recv;

        if (pthread_create(&thread_action, NULL, manage_board, arg_action) != 0) {
            perror("erreur de création du thread manage_board");
            exit(EXIT_FAILURE);
        }

        if (pthread_create(&thread_recv, NULL, recv_game_info, arg_recv) != 0) {
            perror("erreur de création du thread recv_game_info");
            exit(EXIT_FAILURE);
        }

        if (pthread_join(thread_action, NULL) != 0) {
            perror("erreur de join du thread manage_board");
            exit(EXIT_FAILURE);
        }

        if (pthread_join(thread_recv, NULL) != 0) {
            perror("erreur de join du thread recv_game_info");
            exit(EXIT_FAILURE);
        }
    }
    free_info_joueur(info_j);
    end_print();
    close(sock_client);
    return (0);
}
