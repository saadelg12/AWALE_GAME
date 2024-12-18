
#ifndef SERVER_COMMAND_ROUTER_H
#define SERVER_COMMAND_ROUTER_H

#include "server_client_manager.h"

/**
 * @brief Gère un paquet de données entrant.
 * 
 * @param clients Liste des clients actifs.
 * @param client Pointeur vers le client qui a envoyé le message.
 * @param message Message reçu du client.
 * @param games Liste des jeux en cours.
 * @param current_gm_id Pointeur vers l'identifiant du jeu en cours.
 */
void dispatch_client_command(const ActiveClients clients, Client *client,
                              char *message, Games *games, int *current_gm_id);

#endif