#include "serveur.h"
#include "grille.h"

int nb_joueur_partie = 0;
pthread_mutex_t lock_deplacement = PTHREAD_MUTEX_INITIALIZER;

void ajouterCase(partie_info *partie_inf, int ligne, int colonne, unsigned char contenu)
{
    // Augmenter la taille du tableau
    partie_inf->freq = realloc(partie_inf->freq, (partie_inf->nb_freq + 1) * sizeof(char *));
    if (partie_inf->freq == NULL)
    {
        fprintf(stderr, "Erreur de réallocation de mémoire\n");
        exit(EXIT_FAILURE);
    }

    // Allouer de la mémoire pour une nouvelle case
    partie_inf->freq[partie_inf->nb_freq] = calloc(3, sizeof(char));
    if (partie_inf->freq[partie_inf->nb_freq] == NULL)
    {
        fprintf(stderr, "Erreur d'allocation de mémoire\n");
        exit(EXIT_FAILURE);
    }

    // Ajouter les valeurs à la nouvelle case
    partie_inf->freq[partie_inf->nb_freq][0] = (char)ligne;
    partie_inf->freq[partie_inf->nb_freq][1] = (char)colonne;
    partie_inf->freq[partie_inf->nb_freq][2] = (char)contenu;

    // Incrémenter la taille du tableau
    partie_inf->nb_freq++;
}

int init_partie_info(int sock, partie_info *partie)
{

    pthread_t *tpthread = calloc(4, sizeof(pthread_t));
    if (!tpthread)
    {
        perror("calloc tpthread");
        exit(-1);
    }

    while (nb_joueur_partie < 4)
    {
        struct sockaddr_in6 adrclient;
        memset(&adrclient, 0, sizeof(adrclient));
        socklen_t size = sizeof(adrclient);
        int client_sock = accept(sock, (struct sockaddr *)&adrclient, &size);
        if (client_sock < 0)
        {
            perror("Erreur de accept");
            continue;
        }

        partie->socket_joueurs[nb_joueur_partie] = client_sock;

        int *var_sock = calloc(1, sizeof(int));
        *var_sock = client_sock;
        int *var_id = calloc(1, sizeof(int));
        *var_id = nb_joueur_partie;

        void **args = calloc(2, sizeof(void *)); // Allouer un tableau pour les arguments de chaque thread
        args[0] = partie;
        args[1] = var_id;

        if (pthread_create(&tpthread[nb_joueur_partie], NULL, receive_fts_snd_message, args) != 0)
        {
            perror("Erreur de pthread_create pour receive_initial_thread");
            free(args);
            close(client_sock);
            continue;
        }

        nb_joueur_partie++;
    }

    for (int i = 0; i < nb_joueur_partie; i++)
    {
        pthread_join(tpthread[i], NULL);
    }

    return 1;
}

// Fonction pour générer une adresse IPv6 de multidiffusion à partir d'une graine (seed)
void *receive_fts_snd_message(void *arg)
{
    // pthread_mutex_lock(&lock);
    void **args = (void **)arg;
    partie_info *partie_inf = (partie_info *)args[0];
    int *var_id = (int *)args[1];
    int id = *var_id;
    // pthread_mutex_unlock(&lock);

    char message[SIZE_MESS] = {0};
    ssize_t bytes_received;
    bytes_received = recv(partie_inf->socket_joueurs[id], message, SIZE_MESS, 0);
    if (bytes_received <= 0)
    {
        perror("Erreur de réception du message initial du client");
        return NULL ;
    }
    else
    {
        u_int16_t value;
        memmove(&value, &message[0], sizeof(u_int16_t));
        u_int16_t aux = ntohs(value);
        uint16_t code_req = aux & 0x1FFF;

        partie_inf->en_equipe = code_req - 1;

        requete_dem_partie_s *req = calloc(1, sizeof(requete_dem_partie_s));
        if (partie_inf->en_equipe == 0)
            init_requete_dem_partie(req, id, 0, partie_inf);
        else
            init_requete_dem_partie(req, 0, id % 2, partie_inf);

        char buffer[22] = {0};
        convert_req_dem_partie(req, buffer);

        // Envoyer les données au client
        ssize_t bytes_sent = send(partie_inf->socket_joueurs[id], buffer, sizeof(buffer), 0);
        if (bytes_sent == -1)
        {
            perror("Erreur lors de l'envoi des données au client");
            exit(EXIT_FAILURE);
        }
        // Afficher les informations
        printf("Code de requête : %d |  Equipe :  %d  |  ont eté envoyé au joueur n° %d\n", req->info->codereq, req->info->eq, req->info->id); // Conversion au format hôte
    }

    memset(message, 0, SIZE_MESS);
    bytes_received = recv(partie_inf->socket_joueurs[id], message, SIZE_MESS, 0);
    if (bytes_received <= 0)
    {
        perror("Erreur de réception du deuxième message du client");
        return NULL ;
    }
    else
    {
        u_int16_t value;
        memmove(&value, &message[0], sizeof(u_int16_t));
        u_int16_t aux = ntohs(value);

        // Extraire les informations de l'en-tête
        uint16_t code_req = aux & 0x1FFF;
        uint16_t id = (aux >> 13) & 0x3;
        uint16_t eq = (aux >> 15) & 0x1;
        printf("Code de requête : %d\n", code_req);
        printf("Identifiant : %d\n", id);
        printf("Equipe : %d\n", eq);
        printf("Le deuxieme send du client n° %d a été recv\n", id);
    }

    return NULL;
}

int deplacement_possible(board *b, pos *p, int direction)
{
    int dx[] = {0, 1, 0, -1};  // Déplacements en x pour les directions 0, 1, 2, 3
    int dy[] = {-1, 0, +1, 0}; // Déplacements en y pour les directions 0, 1, 2, 3

    return (get_grid(b, p->x + dx[direction], p->y + dy[direction]) == '0');
}

void deplacer_joueur(partie_info *partie_inf, int id, int direction)
{
    if (deplacement_possible(partie_inf->b, partie_inf->pos_joueurs[id], direction))
    {
        set_grid(partie_inf->b, partie_inf->pos_joueurs[id]->x, partie_inf->pos_joueurs[id]->y, '0');
        ajouterCase(partie_inf, partie_inf->pos_joueurs[id]->x, partie_inf->pos_joueurs[id]->y, '0');
        int dx[] = {0, 1, 0, -1};  // Déplacements en x pour les directions 0, 1, 2, 3
        int dy[] = {-1, 0, +1, 0}; // Déplacements en y pour les directions 0, 1, 2, 3
        partie_inf->pos_joueurs[id]->x += dx[direction];
        partie_inf->pos_joueurs[id]->y += dy[direction];
        set_grid(partie_inf->b, partie_inf->pos_joueurs[id]->x, partie_inf->pos_joueurs[id]->y, '5' + id);
        ajouterCase(partie_inf, partie_inf->pos_joueurs[id]->x, partie_inf->pos_joueurs[id]->y, '5' + id);
    }
}

int envoi_seconde(partie_info *partie_inf, int sock, struct sockaddr_in6 gradr)
{
    requete_partie_s *req_partie = calloc(1, sizeof(requete_partie_s));
    if (!req_partie)
    {
        perror("Erreur lors de l'allocation de mémoire pour req_partie");
        pthread_exit(NULL);
    }
    init_requete_partie(req_partie, partie_inf);
    char buffer[6 + partie_inf->hauteur * partie_inf->largeur];
    memset(buffer, 0, sizeof(buffer));
    convert_requete_partie_(req_partie, buffer);

    ssize_t bytes_sent = sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&gradr, sizeof(gradr));
    if (bytes_sent == -1)
    {
        perror("Erreur lors de l'envoi des données du plateau (seconde) au client");
        free(req_partie);
        return -1;
    }
    free(req_partie);
    for (size_t i = 0; i < partie_inf->nb_freq; i++)
    {
        free(partie_inf->freq[i]);
    }
    partie_inf->nb_freq = 0;
    return 0;
}

int envoi_freq(partie_info *partie_inf, int sock, struct sockaddr_in6 gradr)
{
    requete_freq_partie *req = calloc(1, sizeof(requete_freq_partie));
    if (!req)
    {
        perror("Erreur lors de l'allocation de mémoire pour req");
        exit(EXIT_FAILURE);
    }
    init_requete_freq_partie(req, partie_inf);
    char buffer[24];
    memset(buffer, 0, sizeof(buffer));
    convert_requete_freq_partie(req, buffer);

    ssize_t bytes_sent = sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&gradr, sizeof(gradr));
    if (bytes_sent == -1)
    {
        perror("Erreur lors de l'envoi de l'entête (freq) au client");
        return -1;
    }
    partie_inf->num_mess_freq++;

    for (int i = 0; i < partie_inf->nb_freq; i++)
    {
        bytes_sent = sendto(sock, partie_inf->freq[i], 3, 0, (struct sockaddr *)&gradr, sizeof(gradr));
        if (bytes_sent == -1)
        {
            perror("Erreur lors de l'envoi des données du plateau (freq) au client");
            return -1;
        }
    }

    for (size_t i = 0; i < partie_inf->nb_freq; i++)
    {
        free(partie_inf->freq[i]);
    }
    free(req);
    partie_inf->nb_freq = 0;
    return 0;
}

int envoi_tchat(partie_info *partie_inf, char *data, int id, int eq, int sock, struct sockaddr_in6 gradr)
{
    requete_tchat_s *req = calloc(1, sizeof(requete_tchat_s));
    if (!req)
    {
        perror("Erreur lors de l'allocation de mémoire pour req");
        exit(EXIT_FAILURE);
    }
    init_requete_tchat(req, partie_inf, data, id, eq);
    char buffer[24 + req->len];
    convert_requete_tchat_s(req, buffer);

    ssize_t bytes_sent = sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&gradr, sizeof(gradr));
    if (bytes_sent == -1)
    {
        perror("Erreur lors de l'envoi de l'entête (tchat) au client");
        return -1;
    }
    free(req);
    return 0;
}

void *envoyer_messages(void *arg)
{
    struct args_envoyer *args = (struct args_envoyer *)arg;
    partie_info *partie_inf = args->partie_inf;
    int sock = args->sock;
    struct sockaddr_in6 gradr = args->gradr;

    time_t time_s = time(NULL);
    time_t time_ms = time(NULL);

    while (1)
    {
        if (time(NULL) - time_ms >= 0.005)
        {
            envoi_freq(partie_inf, sock, gradr);
            time_ms = time(NULL);
        }

        if (time(NULL) - time_s >= 1)
        {
            envoi_seconde(partie_inf, sock, gradr);
            time_s = time(NULL); // Réinitialiser le temps de début
        }
    }
    return NULL;
}

void *recevoir_messages(void *arg)
{
    struct arg_recevoir *args = (struct arg_recevoir *)arg;
    int sock = args->sock;
    partie_info *partie_inf = args->partie_inf;
    while (1)
    {
        struct sockaddr_in6 cliadr;
        socklen_t len = sizeof(cliadr);
        char buf_recv_action[BUF_SIZE] = {0};
        ssize_t bytes_received = recvfrom(sock, buf_recv_action, sizeof(buf_recv_action), 0, (struct sockaddr *)&cliadr, &len);
        if (bytes_received == -1)
        {
            perror("Erreur lors de la reception des actions du client");
            return NULL;
        }

        int a = extract_action(buf_recv_action);
        int id = extract_id(buf_recv_action);

        pthread_mutex_lock(&lock_deplacement);
        deplacer_joueur(partie_inf, id, a);
        pthread_mutex_unlock(&lock_deplacement);
    }
    return NULL;
}

int send_mess_tchat(partie_info *partie_inf, char *mess, int id, int eq, struct sockaddr_in6 gradr)
{
    uint8_t len = 0;
    memmove(&len, &mess[2], sizeof(uint8_t));
    if (!envoi_tchat(partie_inf, &mess[3], id, eq, partie_inf->sock_mutli, partie_inf->gradr))
        return 0;
    else
        return -1;
}

/*void *recevoir_mess_chat(void *arg)
{
    struct args_mess_chat *args = (struct args_mess_chat *)arg;
    int sock = args->sock_tcp;
    partie_info *partie_inf = args->partie_inf;
    while (1)
    {

        char message[SIZE_MESS];
        memset(message, 0, SIZE_MESS);
        ssize_t bytes_received;
        bytes_received = recv(sock, message, SIZE_MESS, 0);
        if (bytes_received <= 0)
        {
            perror("Erreur");
        }
        int coderec = extract_coderec(message);
        int id_j = extract_id(message);
        int eq_j = extract_eq(message);
        switch (coderec)
        {
        case 7:
            send_mess_tchat(partie_inf, message, id_j, eq_j, partie_inf->gradr);
            break;
        case 8:
            break;
        default:
            break;
        }
    }
}*/

int jeu(partie_info *partie_inf, int sock_udp, int sock_multi, struct sockaddr_in6 gradr)
{
    pthread_t thread_envoyer, thread_recevoir;

    // struct args_envoyer args_envoyer = {partie_inf, sock_multi, gradr};
    struct args_envoyer *args_envoyer = calloc(1, sizeof(struct args_envoyer));
    if (!args_envoyer)
    {
        free(partie_inf);
        return -1;
    }
    args_envoyer->partie_inf = partie_inf;
    args_envoyer->sock = sock_multi;
    args_envoyer->gradr = gradr;

    // struct args_recevoir args_recevoir = {partie_inf, sock_udp};
    struct arg_recevoir *args_recevoir = calloc(1, sizeof(struct arg_recevoir));
    if (!args_recevoir)
    {
        free(partie_inf);
        return -1;
    }
    args_recevoir->partie_inf = partie_inf;
    args_recevoir->sock = sock_udp;

    if (pthread_create(&thread_envoyer, NULL, envoyer_messages, args_envoyer) != 0)
    {
        perror("Erreur lors de la création du thread envoyer");
        return -1;
    }

    if (pthread_create(&thread_recevoir, NULL, recevoir_messages, args_recevoir) != 0)
    {
        perror("Erreur lors de la création du thread recevoir");
        return -1;
    }

    pthread_join(thread_envoyer, NULL);
    pthread_join(thread_recevoir, NULL);
    // pthread_join(thread_recevoir_messages, NULL);

    pthread_mutex_destroy(&lock_deplacement);

    return 1;
}

int main()
{
    struct sockaddr_in6 address_sock;
    address_sock.sin6_family = AF_INET6;
    address_sock.sin6_port = htons(8080);
    address_sock.sin6_addr = in6addr_any;

    //*** création de la socket ***
    int sock = socket(PF_INET6, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Création de socket");
        exit(EXIT_FAILURE);
    }

    int optval = 0;
    if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval)) < 0)
    {
        perror("Erreur connexion IPv4 impossible");
        exit(EXIT_FAILURE);
    }

    /* permet de lier et d'utiliser le même port, même si un autre socket est déjà en écoute sur ce port.
    Cependant, il est important de noter que SO_REUSEADDR ne permet pas à plusieurs sockets en écoute de recevoir des connexions simultanément sur le même port.
     Seul le premier socket en écoute recevra les connexions entrantes.*/
    optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        perror("Erreur lors de la configuration de l'option SO_REUSEADDR");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (bind(sock, (struct sockaddr *)&address_sock, sizeof(address_sock)) < 0)
    {
        perror("Erreur lors du bind");
        exit(EXIT_FAILURE);
    }

    if (listen(sock, 4) < 0)
    {
        perror("Erreur lors du listen");
        exit(EXIT_FAILURE);
    }

    partie_info *partie_inf = calloc(1, sizeof(partie_info));
    if (!partie_inf)
    {
        perror("Erreur lors de l'allocation de mémoire pour partie");
        exit(EXIT_FAILURE);
    }

    if (init_struct_serv(partie_inf) < 1)
    {
        perror("Erreur lors de l'initialisation des informations de la partie");
        exit(EXIT_FAILURE);
    }

    /* créer la socket */
    int sock_multi;
    if ((sock_multi = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
    {
        perror("erreur socket");
        return 1;
    }

    /* Initialisation de l'adresse d'abonnement */
    struct sockaddr_in6 gradr;
    memset(&gradr, 0, sizeof(gradr));
    gradr.sin6_family = AF_INET6;

    char addr_str_mdiff[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, partie_inf->adrmdiff, addr_str_mdiff, INET6_ADDRSTRLEN);

    inet_pton(AF_INET6, addr_str_mdiff, &gradr.sin6_addr);

    gradr.sin6_port = htons(partie_inf->portmdiff);

    int ifindex = if_nametoindex("eth0");
    if (ifindex == 0)
        perror("if_nametoindex");

    gradr.sin6_scope_id = ifindex; // ou 0 pour interface par défaut

    if (init_partie_info(sock, partie_inf) < 1)
    {
        perror("Erreur lors de l'initialisation des informations de la partie");
        exit(EXIT_FAILURE);
    }

    printf("IT'S SHOW TIME\n");

    int sock_udp = socket(PF_INET6, SOCK_DGRAM, 0);
    if (sock_udp < 0)
        return -1;
    // adresse de destination
    struct sockaddr_in6 servadr;
    memset(&servadr, 0, sizeof(servadr));
    servadr.sin6_family = AF_INET6;
    servadr.sin6_addr = in6addr_any;
    servadr.sin6_port = htons(partie_inf->portudp);
    optval = 1;
    if (setsockopt(sock_udp, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        perror("Erreur lors de la configuration de l'option SO_REUSEADDR");
        close(sock_udp);
        exit(EXIT_FAILURE);
    }
    if (bind(sock_udp, (struct sockaddr *)&servadr, sizeof(servadr)) < 0)
        return -1;

    jeu(partie_inf, sock_udp, sock_multi, gradr);

    close(sock);
    close(sock_udp);
    close(sock_multi);

    return 0;
}
