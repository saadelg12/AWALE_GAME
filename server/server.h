#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>      // Pour les opérations de conversion d'adresses Internet
#include <netdb.h>          // Pour gethostbyname et autres fonctions de réseau
#include <netinet/in.h>     // Pour les structures de données de réseau
#include <sys/socket.h>     // Pour les opérations sur les sockets
#include <sys/types.h>      // Pour les définitions de types de données
#include <unistd.h>         // Pour les fonctions POSIX comme close

#include "server_client_manager.h"  // Inclusion du fichier d'en-tête pour les clients du serveur

/**
 * @brief Fonction principale de l'application serveur.
 */
static void client_main_loop(void);

/**
 * @brief Initialise la connexion du serveur.
 * 
 * @return int Le descripteur de socket du serveur.
 */
static int init_connection(void);

/**
 * @brief Termine la connexion du serveur.
 * 
 * @param sock Le descripteur de socket à fermer.
 */
static void end_connection(int sock);

/**
 * @brief Lit les données envoyées par un client.
 * 
 * @param sock Le descripteur de socket du client.
 * @param buffer Le tampon où les données lues seront stockées.
 * @return int Le nombre d'octets lus.
 */
static int read_client(SOCKET sock, char *buffer);

/**
 * @brief Efface les informations des clients actifs.
 * 
 * @param clients Pointeur vers la structure des clients actifs.
 */
static void clear_clients(ActiveClients *clients);

/**
 * @brief Charge les parties disponibles.
 * 
 * @param games Pointeur vers la structure des parties.
 */
void load_games(Games *games);

/**
 * @brief Obtient l'identifiant de la dernière partie.
 * 
 * @param games Pointeur vers la structure des jeux.
 * @return int L'identifiant du dernier jeu.
 */
int get_last_id(Games *games);

#endif /* SERVER_H */