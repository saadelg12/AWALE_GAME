#include <stdlib.h> // Inclusion de la bibliothèque standard
#include <string.h> // Inclusion de la bibliothèque pour les opérations sur les chaînes de caractères

#include "server_command_handler.h" // Inclusion du fichier d'en-tête pour les commandes
#include "server_client_manager.h"  // Inclusion du fichier d'en-tête pour les clients du serveur
#include "server_command_router.h"  // Inclusion du fichier d'en-tête pour les paquets du serveur

// Fonction pour gérer les paquets entrants
void dispatch_client_command(const ActiveClients clients, Client *client,
                             char *message, Games *games, int *current_gm_id)
{
  char *it = message;  // Pointeur pour parcourir le message
  char *it2 = message; // Deuxième pointeur pour parcourir le message
  char command2[6];    // Tableau pour stocker une commande
  int game_id;         // Identifiant du jeu
  if (*(it++) != '/')
  { // Si le message ne commence pas par '/', envoyer le message à tous les clients
    broadcast_to_all_clients(clients, *client, message, 0);
  }
  else
  {
    char command_str[4];             // Tableau pour stocker la commande encodée sur 3 caractères
    strncpy(command_str, it, 3);     // Copier les 3 premiers caractères de la commande
    command_str[3] = '\0';           // Ajouter le caractère de fin de chaîne
    int command = atoi(command_str); // Convertir la commande en entier
    switch (command)
    { // Exécuter l'action en fonction de la commande
    case 0:
      send_active_clients_list(clients, client, message); // Obtenir la liste des clients
      break;
    case 1:
      it += 4;                                                             // Avancer le pointeur de 4 caractères
      char username[USERNAME_SIZE];                                        // Tableau pour stocker le nom d'utilisateur
      strncpy(username, it, USERNAME_SIZE);                                // Copier le nom d'utilisateur
      send_game_invite(clients, client, username, message, current_gm_id); // Envoyer une invitation
      break;
    case 2:
      send_active_games_list(clients, client, message); // Obtenir la liste des jeux
      break;
    case 3:
      it += 4;                               // Avancer le pointeur de 4 caractères
      int num = atoi(it);                    // Convertir la chaîne en entier
      process_game_move(client, num, games); // Jouer au jeu
      break;
    case 4:
      it += 4;                                                            // Avancer le pointeur de 4 caractères
      char username_to_watch[USERNAME_SIZE];                              // Tableau pour stocker le nom d'utilisateur à surveiller
      strncpy(username_to_watch, it, USERNAME_SIZE);                      // Copier le nom d'utilisateur
      start_watching_player(clients, client, username_to_watch, message); // Surveiller l'utilisateur
      break;
    case 5:
      it += 4;                        // Avancer le pointeur de 4 caractères
      char bio[BIO_SIZE];             // Tableau pour stocker la biographie
      strncpy(bio, it, BIO_SIZE);     // Copier la biographie
      update_client_bio(client, bio); // Changer la biographie
      break;
    case 6:
      it += 4;                                                  // Avancer le pointeur de 4 caractères
      char username_to_get_bio_from[USERNAME_SIZE];             // Tableau pour stocker le nom d'utilisateur dont on veut obtenir la biographie
      strncpy(username_to_get_bio_from, it, USERNAME_SIZE);     // Copier le nom d'utilisateur
      send_user_bio(client, username_to_get_bio_from); // Obtenir la biographie
      break;
    case 7:
      it += 4;                                       // Avancer le pointeur de 4 caractères
      strncpy(username, it, USERNAME_SIZE);          // Copier le nom d'utilisateur
      send_friend_invite(clients, client, username); // Envoyer une demande d'ami
      break;
    case 8:
      send_friends_list(client); // Envoyer la liste d'amis
      break;
    case 9:
      toggle_private_mode(client); // Basculer en mode privé
      break;
    case 10:
      leave_current_game(client, games); // Quitter le jeu
      break;
    case 11:
      send_games_history(client, games, message); // Obtenir l'historique des jeux
      break;
    case 12:
      sscanf(it2, "%s %d", command2, &game_id);             // Lire la commande et l'identifiant du jeu
      replay_game_history(client, games, game_id, message); // Revoir le jeu
      break;
    case 13:
      send_help_message(client); // Envoyer l'aide
      break;
    case 14:
    {
      char recipient[USERNAME_SIZE];
      char buffer[BUF_SIZE];
      char *message_start = strchr(it + 4, ' ');
      if (message_start)
      {
        sscanf(it + 4, "%s", recipient);
        message_start++; // Skip the space.
        Client *target = find_active_client_by_username(clients, recipient);
        if (target)
        {
          snprintf(buffer, BUF_SIZE, "%s : %s", client->username, message_start);
          send_message_to_client(target->socket, buffer);
        }
        else
        {
          snprintf(buffer, BUF_SIZE, "User %s not found.\n", recipient);
          send_message_to_client(client->socket, buffer);
        }
      }
      else
      {
        send_message_to_client(client->socket, "Usage: /private_message <username> <message>\n");
      }
      break;
    }
    case 15:
      stop_watching_player(client);
      break;
    case 16:
      send_ranking_to_client(client);
      break;
    default:
      break; // Ne rien faire pour les commandes inconnues
    }
  }
}
