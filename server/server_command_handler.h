#ifndef SERVER_COMMAND_HANDLER_H
#define SERVER_COMMAND_HANDLER_H
#include "server_client_manager.h"
#include "game_logic.h"


/**
 * @brief Structure pour stocker le score d'une joueur
 */
typedef struct {
    char username[USERNAME_SIZE];
    int points;
} PlayerScore;


/**
 * @brief Envoyer la liste des clients actifs à un client spécifique
 * 
 * @param clients Liste des clients actifs.
 * @param client Pointeur vers le client.
 * @param buffer Tampon pour stocker la liste des clients.
 */
void send_active_clients_list(ActiveClients, Client *, char *);

/**
 * @brief Envoie une invitation à un autre client.
 * 
 * @param clients Liste des clients actifs.
 * @param client Pointeur vers le client.
 * @param recipient_username Nom d'utilisateur du destinataire.
 * @param buffer Tampon pour stocker l'invitation.
 * @param status Pointeur vers le statut de l'invitation.
 */
void send_game_invite(ActiveClients, Client *, const char *, char *, int *);

/**
 * @brief Joue un mouvement.
 * 
 * @param client Pointeur vers le client.
 * @param move Mouvement à jouer.
 * @param games Liste des jeux.
 */
void process_game_move(Client *, int, Games * games);

/**
 * @brief Récupère la liste des jeux disponibles.
 * 
 * @param clients Liste des clients actifs.
 * @param client Pointeur vers le client.
 * @param buffer Tampon pour stocker la liste des jeux.
 */
void send_active_games_list(ActiveClients clients, Client *client, char *buffer);

/**
 * @brief Regarde un utilisateur.
 * 
 * @param clients Liste des clients actifs.
 * @param client Pointeur vers le client.
 * @param username Nom d'utilisateur à regarder.
 * @param buffer Tampon pour stocker les informations de l'utilisateur.
 */
void start_watching_player(ActiveClients clients, Client *client, char *username, char *buffer);

/**
 * @brief Change la biographie du client.
 * 
 * @param client Pointeur vers le client.
 * @param bio Nouvelle biographie.
 */
void update_client_bio(Client *client, char *bio);

/**
 * @brief Récupère la biographie d'un utilisateur.
 * 
 * @param client Pointeur vers le client.
 * @param username Nom d'utilisateur dont on veut la biographie.
 */
void send_user_bio(Client* client, char *username);

/**
 * @brief Envoie une demande d'ami à un autre client.
 * 
 * @param clients Liste des clients actifs.
 * @param client Pointeur vers le client.
 * @param recipient_username Nom d'utilisateur du destinataire.
 */
void send_friend_invite(ActiveClients clients, Client *client, const char *recipient_username);

/**
 * @brief Envoie la liste d'amis du client.
 * 
 * @param client Pointeur vers le client.
 */
void send_friends_list(Client *client);

/**
 * @brief Active ou désactive le mode privé du client.
 * 
 * @param client Pointeur vers le client.
 */
void toggle_private_mode(Client *client);

/**
 * @brief Quitte une partie de jeu.
 * 
 * @param client Pointeur vers le client.
 * @param games Liste des jeux.
 */
void leave_current_game(Client *client, Games * games);

/**
 * @brief Termine une partie de jeu.
 * 
 * @param client Pointeur vers le client.
 * @param games Liste des jeux.
 */
void finalize_game_session(Client * client, Games * games);

/**
 * @brief Récupère l'historique des jeux du client.
 * 
 * @param client Pointeur vers le client.
 * @param games Liste des jeux.
 * @param buffer Tampon pour stocker l'historique des jeux.
 */
void send_games_history(Client * client, Games *games, char *buffer);

/**
 * @brief Trouve un jeu par son identifiant.
 * 
 * @param games Liste des jeux.
 * @param game_id Identifiant du jeu.
 * @return Pointeur vers le jeu trouvé.
 */
Game * get_game_by_id(Games *games, int game_id);

/**
 * @brief Revoir une partie de jeu.
 * 
 * @param client Pointeur vers le client.
 * @param games Liste des jeux.
 * @param game_id Identifiant du jeu à revoir.
 * @param buffer Tampon pour stocker les informations du jeu.
 */
void replay_game_history(Client * client, Games *games,int game_id, char *buffer);

/**
 * @brief Envoie un message d'aide au client.
 * 
 * @param client Pointeur vers le client.
 */
void send_help_message(Client *client);

/**
 * @brief Compare les scores des joueurs.
 * 
 * @param a Pointeur vers le premier score.
 * @param b Pointeur vers le deuxième score.
 * @return int Résultat de la comparaison.
 */
void send_ranking_to_client(Client *client);

void add_points_to_player(PlayerScore *scores, int *player_count, const char *player, int points);

#endif