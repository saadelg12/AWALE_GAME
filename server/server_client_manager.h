
#ifndef SERVER_CLIENT_MANAGER_H
#define SERVER_CLIENT_MANAGER_H

#include "game_logic.h"

/**
 * @brief Structure représentant un client.
 */
typedef struct Client Client;

/**
 * @brief Structure représentant les clients actifs.
 */
typedef struct ActiveClients ActiveClients;

/**
 * @brief Structure représentant un ami.
 */
typedef struct Friend {
  struct Friend *next;        /**< Pointeur vers l'ami suivant. */
  struct Friend *previous;    /**< Pointeur vers l'ami précédent. */
  Client *friend_of_client;   /**< Pointeur vers le client ami. */
} Friend;

/**
 * @brief Structure représentant une liste de parties.
 */
typedef struct Games {
  Game *first; /**< Premier jeu de la liste. */
  Game *last;  /**< Dernier jeu de la liste. */
} Games;

/**
 * @brief Structure représentant une liste d'amis.
 */
typedef struct FriendList {
  Friend *first; /**< Premier ami de la liste. */
  Friend *last;  /**< Dernier ami de la liste. */
} FriendList;

/**
 * @brief Structure représentant un observateur.
 */
typedef struct Observer {
  struct Observer *next;      /**< Pointeur vers l'observateur suivant. */
  struct Observer *previous;  /**< Pointeur vers l'observateur précédent. */
  Client *watcher;            /**< Pointeur vers le client observateur. */
} Observer;

/**
 * @brief Structure représentant une liste d'observateurs.
 */
typedef struct Observers {
  Observer *first; /**< Premier observateur de la liste. */
  Observer *last;  /**< Dernier observateur de la liste. */
} Observers;

/**
 * @brief Structure représentant une invitation.
 */
typedef struct Invite {
  Client *recipient; /**< Pointeur vers le client destinataire de l'invitation. */
  struct Invite *next; /**< Pointeur vers l'invitation suivante. */
} Invite;

/**
 * @brief Structure représentant une liste d'invitations.
 */
typedef struct Invites {
  Invite *first; /**< Première invitation de la liste. */
} Invites;

/**
 * @brief Structure représentant un client.
 */
struct Client {
  int socket; /**< Socket du client. */
  char username[USERNAME_SIZE]; /**< Nom d'utilisateur du client. */
  char bio[BIO_SIZE]; /**< Biographie du client. */
  Game *game; /**< Pointeur vers le jeu du client. */
  Invites *invites; /**< Pointeur vers les invitations reçues par le client. */
  Invites *friend_requests_sent; /**< Pointeur vers les demandes d'amis envoyées par le client. */
  struct Client *opponent; /**< Pointeur vers l'adversaire du client. */
  struct Client *next; /**< Pointeur vers le client suivant. */
  struct Client *previous; /**< Pointeur vers le client précédent. */
  struct Client *watching; /**< Pointeur vers le client observé. */
  Observers *observers; /**< Pointeur vers la liste des observateurs du client. */
  FriendList *friends; /**< Pointeur vers la liste des amis du client. */
  int connected; /**< État de connexion du client (-1 si déconnecté, 0 si nouveau nom d'utilisateur, 1 si mot de passe requis, 2 si connecté). */
  char expected_password[20]; /**< Mot de passe attendu (haché) du client s'il est dans la base de données. */
  int turn; /**< Tour du client. */
  int priv; /**< Privilèges du client. */
};

/**
 * @brief Structure représentant les clients actifs.
 */
typedef struct ActiveClients {
  Client *first; /**< Premier client actif. */
  Client *last; /**< Dernier client actif. */
  int nb; /**< Nombre de clients actifs. */
} ActiveClients;

/**
 * @brief Ajoute un client à la liste des clients actifs.
 * 
 * @param active_clients Pointeur vers la liste des clients actifs.
 * @param client Pointeur vers le client à ajouter.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 */
int add_active_client(ActiveClients *, Client *);

/**
 * @brief Supprime un client de la liste des clients actifs.
 * 
 * @param active_clients Pointeur vers la liste des clients actifs.
 * @param client Pointeur vers le client à supprimer.
 */
void remove_active_client(ActiveClients *, Client *);

/**
 * @brief Ajoute un observateur à la liste des observateurs.
 * 
 * @param observers Pointeur vers la liste des observateurs.
 * @param observer Pointeur vers l'observateur à ajouter.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 */
int add_observer_to_list(Observers *observers, Observer *observer);

/**
 * @brief Supprime un observateur de la liste des observateurs.
 * 
 * @param observers Pointeur vers la liste des observateurs.
 * @param client Pointeur vers le client observateur à supprimer.
 */
void remove_observer_from_list(Observers *observers, Client *client);

/**
 * @brief Ajoute un ami à la liste des amis.
 * 
 * @param friends Pointeur vers la liste des amis.
 * @param new_friend Pointeur vers le nouvel ami à ajouter.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 */
int add_friend_to_list(FriendList *friends, Friend *new_friend);


/**
 * @brief Écrit un message à un client via son socket.
 * 
 * @param sock Socket du client.
 * @param buffer Message à envoyer.
 */
void send_message_to_client(SOCKET sock, const char *buffer);

/**
 * @brief Envoie un message à tous les clients actifs.
 * 
 * @param clients Liste des clients actifs.
 * @param client Client envoyant le message.
 * @param buffer Message à envoyer.
 * @param from_server Indicateur si le message provient du serveur.
 */
void broadcast_to_all_clients(ActiveClients clients, Client client, const char *buffer, char from_server);

/**
 * @brief Envoie un message à tous les observateurs.
 * 
 * @param observers Liste des observateurs.
 * @param message Message à envoyer.
 */
void broadcast_to_observers(Observers *observers, const char *message);

/**
 * @brief Trouve un client par son nom d'utilisateur.
 * 
 * @param active_clients Liste des clients actifs.
 * @param username Nom d'utilisateur à rechercher.
 * @return Client* Pointeur vers le client trouvé, NULL si non trouvé.
 */
Client *find_active_client_by_username(const ActiveClients, const char *username);

/**
 * @brief Ajoute une invitation à la liste des invitations.
 * 
 * @param invites Pointeur vers la liste des invitations.
 * @param client Pointeur vers le client destinataire de l'invitation.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 */
int add_invite_to_list(Invites *, Client *);

/**
 * @brief Supprime une invitation après acceptation d'une nouvelle amitié.
 * 
 * @param client Pointeur vers le client ayant reçu l'invitation.
 * @param new_friend_client Pointeur vers le nouveau client ami.
 */
void remove_invite_after_friendship(Client *client, Client *new_friend_client);

/**
 * @brief Supprime une invitation de la liste des invitations.
 * 
 * @param invites Pointeur vers la liste des invitations.
 * @param invite Pointeur vers l'invitation à supprimer.
 */
void remove_invite_from_list(Invites *invites, Invite *);

/**
 * @brief Supprime toutes les invitations envoyées par un client.
 * 
 * @param client Pointeur vers le client.
 */
void clear_client_invites(Client *client);

/**
 * @brief Vérifie si un client est dans la liste des invitations.
 * 
 * @param invites Pointeur vers la liste des invitations.
 * @param recipient Pointeur vers le client destinataire.
 * @return int 1 si le client est dans la liste, 0 sinon.
 */
int is_client_in_invite_list(const Invites *invites, const Client *recipient);

/**
 * @brief Vérifie si un client est dans la liste des clients actifs.
 * 
 * @param client Pointeur vers le client.
 * @param list_of_clients Liste des clients actifs.
 * @return int 1 si le client est dans la liste, 0 sinon.
 */
int is_client_in_active_list(Client *client, ActiveClients list_of_clients);

/**
 * @brief Vérifie si deux clients sont amis.
 * 
 * @param client1 Pointeur vers le premier client.
 * @param client2 Pointeur vers le deuxième client.
 * @return int 1 si les clients sont amis, 0 sinon.
 */
int are_clients_friends(Client *client1, Client *client2);

/**
 * @brief Vérifie si un client est déjà connecté.
 * 
 * @param clients Liste des clients actifs.
 * @param username Nom
 * @return int 1 si le client est déjà connecté, 0 sinon.
 */
int is_already_connected(ActiveClients clients, char *username);

/**
 * @brief Vérifie si un nom d'utilisateur est déjà utilisé dans le users.csv
 * 
 * @param clients Liste des clients actifs.
 * @param username Nom d'utilisateur à vérifier.
 * @return int 1 si le nom d'utilisateur est déjà utilisé, 0 sinon.
 */

int is_username_already_used(ActiveClients clients, char *username);

/**
 * @brief Fonction pour enregistrer un client.
 * 
 * @param username Nom d'utilisateur du client.
 * @param password Mot de passe du client.
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 */
int register_client(char *username, char *password);

/**
 * @brief Fonction pour connecter un client.
 * 
 * @param username Nom d'utilisateur du client.
 * @param password Mot de passe du client.
 * @return int 1 en cas de succès, 0 en cas d'erreur.
 */
int login_client(char *username, char *password);

/**
 * @brief Fonction pour arrêter d'observer un jouer.
 * 
 * @param client Pointeur vers le client à déconnecter.
 */
void stop_watching_player(Client *client);

/**
 * @brief Fonction pour initialiser un client.
 * 
 * @param client Pointeur vers le client
 * @return int 0 en cas de succès, -1 en cas d'erreur.
 */
int init_client(Client *client);


/**
 * @brief Fonction pour persister ami avec le client
 * 
 * @param client 
 * @param new_friend_client 
 * @return int -1 en cas d'erreur, 0 en cas de succès
 */
int persist_friend_client(Client *client, Client *new_friend_client);


/**
 * @brief Fonction pour persister bio avec le client
 * 
 * @param client 
 * @param bio 
 * @return int -1 en cas d'erreur, 0 en cas de succès
 */
int persist_bio_client(Client *client, char *bio);

/**
 * @brief Fonction pour lire la biographie du client
 * 
 * @param username Nom d'utilisateur du client
 * @param bio Pointeur vers la biographie 
 * @return int -1 en cas d'erreur, 0 en cas de succès
 */
int read_bio_client(char *username, char *bio);

#endif