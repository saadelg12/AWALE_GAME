#ifndef CLIENT_H
#define CLIENT_H

#include "../options.h"
#include <arpa/inet.h>
#include <netdb.h> /* gethostbyname */
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> /* close */

/**
 * @brief Lance l'application client.
 *
 * Cette fonction initialise la connexion au serveur et gère la communication avec celui-ci.
 *
 * @param address Adresse IP du serveur.
 * @param name Nom de l'utilisateur.
 */
static void app(const char *address, const char *name);

/**
 * @brief Initialise la connexion au serveur.
 *
 * Cette fonction crée une socket et se connecte au serveur spécifié par l'adresse IP.
 *
 * @param address Adresse IP du serveur.
 * @return int Retourne la socket créée ou -1 en cas d'erreur.
 */
static int init_connection(const char *address);

/**
 * @brief Termine la connexion avec le serveur.
 *
 * Cette fonction ferme la socket spécifiée.
 *
 * @param sock La socket à fermer.
 */
static void end_connection(int sock);

/**
 * @brief Lit les données envoyées par le serveur.
 *
 * Cette fonction lit les données de la socket du serveur et les stocke dans le buffer.
 *
 * @param sock La socket du serveur.
 * @param buffer Le buffer où les données lues seront stockées.
 * @return int Retourne le nombre d'octets lus ou -1 en cas d'erreur.
 */
static int read_server(SOCKET sock, char *buffer);

/**
 * @brief Envoie des données au serveur.
 *
 * Cette fonction écrit les données du buffer dans la socket du serveur.
 *
 * @param sock La socket du serveur.
 * @param buffer Le buffer contenant les données à envoyer.
 */
static void write_server(SOCKET sock, const char *buffer);

#endif 

