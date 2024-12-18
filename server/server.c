#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server_command_handler.h"
#include "server.h"
#include "server_client_manager.h"
#include "server_command_router.h"

// Fonction principale de l'application
static void server_main_loop(void)
{
  SOCKET sock = init_connection(); // On récupère le FD de la socket du serveur.

  char buffer[BUF_SIZE]; // Buffer pour les messages.

  int max = sock; // Le descripteur de fichier avec le plus grand numéro, nécessaire pour l'appel à select()

  ActiveClients clients; // Structure pour gérer les clients actifs

  clients.first = NULL;
  clients.last = NULL;
  clients.nb = 0;

  Games games; // Structure pour gérer les jeux
  games.first = NULL;
  games.last = NULL;
  load_games(&games);                      // Charger les jeux existants
  int current_gm_id = get_last_id(&games); // Obtenir le dernier ID de jeu

  fd_set rdfs; // Ensemble de descripteurs de fichiers. Avant select : tous les descripteurs de fichiers. Après select : seulement les descripteurs de fichiers prêts à être lus.

  while (1)
  {
    Client *client_iterator = clients.first;
    FD_ZERO(&rdfs); // Effacer l'ensemble

    FD_SET(STDIN_FILENO, &rdfs); // Ajouter STDIN à l'ensemble

    FD_SET(sock, &rdfs); // Ajouter la socket du serveur à l'ensemble

    while (client_iterator)
    { // Ajouter la socket de chaque client
      FD_SET(client_iterator->socket, &rdfs);
      client_iterator = client_iterator->next;
    }

    if (select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
    { // Bloquer jusqu'à ce qu'il y ait une socket lisible, effacer toutes les sockets de l'ensemble sauf celles lisibles
      perror("select()");
      exit(errno);
    }

    if (FD_ISSET(STDIN_FILENO, &rdfs))
    { // Si stdin est lisible, on arrête l'application
      break;
    }
    else if (FD_ISSET(sock, &rdfs))
    { // Si la socket du serveur est lisible, il y a un nouveau client
      SOCKADDR_IN csin = {0};
      size_t sinsize = sizeof csin;
      int csock = accept(sock, (SOCKADDR *)&csin, (socklen_t *)&sinsize); // Accepter la connexion du nouveau client
      if (csock == SOCKET_ERROR)
      {
        perror("accept()");
        continue;
      }

      if (read_client(csock, buffer) == 0)
      {
        continue; // Le client s'est déconnecté
      }

      char username[USERNAME_SIZE];
      char password[PASSWORD_SIZE];
      strncpy(username, buffer, USERNAME_SIZE);

      if (is_already_connected(clients, username))
      {
        send_message_to_client(csock, "You are already connected");
        closesocket(csock);
        continue;
      }
      else
      {
        if (is_username_already_used(clients, username))
        {
          char greetings[BUF_SIZE] = "Welcome back, ";
          strcat(greetings, username);

          send_message_to_client(csock, greetings);
          send_message_to_client(csock, "\nPlease provide your password");

          if (read_client(csock, buffer) == 0)
          {
            continue; // Le client s'est déconnecté
          }
          strncpy(password, buffer, PASSWORD_SIZE);

          if (!login_client(username, password))
          {
            send_message_to_client(csock, "Invalid password");
            closesocket(csock);
            continue;
          }
        }
        else
        { // Registration
          char greetings[BUF_SIZE] = "Welcome new user, ";
          strcat(greetings, username);

          send_message_to_client(csock, greetings);
          send_message_to_client(csock, "\nPlease provide a password");

          if (read_client(csock, buffer) == 0)
          {
            continue; // Le client s'est déconnecté
          }
          strncpy(password, buffer, PASSWORD_SIZE);
          register_client(username, password);
        }
      }

      Client *c = malloc(sizeof(Client));
      Invites *invites = malloc(sizeof(Invites));
      Invites *friend_requests_sent = malloc(sizeof(Invites));
      Observers *observers = malloc(sizeof(Observers));
      FriendList *friendList = malloc(sizeof(FriendList));
      c->socket = csock;
      c->game = NULL;
      c->opponent = NULL;
      c->connected = 0;
      c->next = NULL;
      c->previous = NULL;
      c->watching = NULL;
      c->invites = invites;
      c->invites->first = NULL;
      c->friend_requests_sent = friend_requests_sent;
      c->friend_requests_sent->first = NULL;
      c->observers = observers;
      c->observers->first = NULL;
      c->observers->last = NULL;

      c->friends = friendList;
      c->friends->first = NULL;
      c->friends->last = NULL;
      c->priv = 0;

      strcpy(c->username, username);

      init_client(c);

      if (!add_active_client(&clients, c))
      {
        continue;
      }

      max = csock > max ? csock : max; // Mettre à jour la valeur max si nécessaire

      FD_SET(csock, &rdfs); // Ajouter la socket du client à l'ensemble

      // Ajouter un message de bienvenue après avoir validé l'utilisateur
      char welcome_message[BUF_SIZE] = "Welcome to AWALE game. Type /help for documentation";
      send_message_to_client(csock, welcome_message); // Envoi du message de bienvenue
    }
    else
    { // Dans ce cas, au moins une socket client est lisible.
      client_iterator = clients.first;
      while (client_iterator)
      {
        if (FD_ISSET(client_iterator->socket, &rdfs))
        {
          int c = read_client(client_iterator->socket, buffer);
          if (c == 0)
          { // Le client s'est déconnecté
            printf("[INFO] Client %s disconnected.\n", client_iterator->username);

            // Fermer la socket
            closesocket(client_iterator->socket);

            // Si le client était dans une partie
            if (client_iterator->game)
            {
              snprintf(client_iterator->game->winner, sizeof(client_iterator->game->winner), "%s", client_iterator->opponent->username); // Définir l'adversaire comme gagnant
              char message[BUF_SIZE];
              snprintf(message, sizeof(message), "Your opponent %s disconnected. You won the game!", client_iterator->username);
              send_message_to_client(client_iterator->opponent->socket, message); // Informer l'adversaire
              finalize_game_session(client_iterator, &games); // Finaliser la session de jeu
              if (client_iterator->opponent) { 
                client_iterator->opponent->opponent = NULL; // Réinitialiser l'adversaire
                client_iterator->opponent->game = NULL; // Réinitialiser la partie
              }
            }
            /** 
              // Informer l'adversaire
              if (client_iterator->opponent)
              {
                char message[BUF_SIZE];
                snprintf(message, sizeof(message),
                         "Your opponent %s disconnected. You won the game!",
                         client_iterator->username);
                send_message_to_client(client_iterator->opponent->socket, message);
              }

              // Finaliser la session de jeu
              finalize_game_session(client_iterator, &games);
            }
            **/

            // Diffuser la déconnexion
            strncpy(buffer, client_iterator->username, BUF_SIZE - 1);
            strncat(buffer, " Disconnected!", BUF_SIZE - strlen(buffer) - 1);
            broadcast_to_all_clients(clients, *client_iterator, buffer, 1);

            // Supprimer le client de la liste des actifs
            remove_active_client(&clients, client_iterator);
          }
          else
          { // INFO: C'est ici que nous allons dans dispatch_incomming_package();
            dispatch_client_command(clients, client_iterator, buffer, &games, &current_gm_id);
          }
          break; // ISSUE: Cela peut causer une famine si le premier processus continue de parler ?
        }
        client_iterator = client_iterator->next;
      }
    }
  }

  clear_clients(&clients); // Nettoyer les clients
  end_connection(sock);    // Terminer la connexion
}

// Fonction pour nettoyer les clients
static void clear_clients(ActiveClients *clients)
{
  clients->nb = 0;
  Client *client_iterator = clients->first;
  while (client_iterator)
  {
    Invite *invite_it = client_iterator->invites->first;
    while (invite_it)
    {
      Invite *previous = invite_it;
      invite_it = invite_it->next;
      free(previous);
    }
    closesocket(client_iterator->socket);
    Client *previous = client_iterator;
    client_iterator = client_iterator->next;
    free(previous);
  }
}

// Fonction pour initialiser la connexion
static int init_connection(void)
{
  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0); // Socket IPV4 TCP
  SOCKADDR_IN sin = {0};

  if (sock == INVALID_SOCKET)
  {
    perror("socket()");
    exit(errno);
  }

  sin.sin_addr.s_addr = htonl(INADDR_ANY); // Toutes les interfaces du PC
  sin.sin_port = htons(PORT);              // PORT défini dans server.h
  sin.sin_family = AF_INET;                // IPV4

  if (bind(sock, (SOCKADDR *)&sin, sizeof sin) == SOCKET_ERROR)
  { // Lier la socket aux paramètres définis.
    perror("bind()");
    exit(errno);
  }

  if (listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
  { // Marquer la socket comme socket passive. Cet appel est non bloquant
    perror("listen()");
    exit(errno);
  }

  return sock;
}

// Fonction pour terminer la connexion
static void end_connection(int sock)
{
  closesocket(sock);
}

// Fonction pour lire les données du client
static int read_client(SOCKET sock, char *buffer)
{
  int n = 0;

  if ((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
  {
    perror("recv()");
    n = 0;
  }

  buffer[n] = '\0';

  return n;
}

// Fonction pour charger les jeux
void load_games(Games *games)
{
  FILE *file = fopen("game_data.csv", "r");

  fseek(file, 0, SEEK_END);
  // Vérifier si le fichier est vide
  long taille = ftell(file);
  if (taille == 0)
  {
    return;
  }
  fseek(file, 0, SEEK_SET);
  while (!feof(file))
  {
    Game *game = import_game_from_csv(file);
    if (game == NULL)
    {
      break;
    }
    if (!games->first)
    {
      games->first = game;
      games->last = game;
    }
    else
    {
      games->last->next = game;
      game->previous = games->last;
      games->last = game;
    }
    game->next = NULL;
  }
}

// Fonction pour obtenir le dernier ID de jeu
int get_last_id(Games *games)
{
  if (games->first == NULL)
  {
    return 0;
  }
  int id = 0;
  Game *current = games->first;
  while (current != NULL)
  {
    if (current->game_id > id)
    {
      id = current->game_id;
    }
    current = current->next;
  }
  id++;
  return id;
}

// Fonction principale
int main(int argc, char **argv)
{
  server_main_loop();  // Appeler la fonction principale de l'application
  return EXIT_SUCCESS; // Retourner le succès
}
