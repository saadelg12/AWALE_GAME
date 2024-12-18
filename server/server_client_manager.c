#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server_client_manager.h"
#include "util/libbcrypt/bcrypt.h"

// Ajoute un client à la liste des clients actifs
int add_active_client(ActiveClients *clients, Client *client)
{
  if (clients->nb == MAX_CLIENTS)
  {
    return 0;
  }
  if (!clients->first)
  {
    clients->first = client;
    clients->last = client;
    ++clients->nb;
    return 1;
  }
  else
  {
    clients->last->next = client;
    client->previous = clients->last;
    clients->last = client;
    ++clients->nb;
    return 1;
  }
}

// Supprime un client de la liste des clients actifs
void remove_active_client(ActiveClients *clients, Client *client) {
    if (!clients || !client) {
        printf("[DEBUG] ActiveClients or client is NULL.\n");
        return;
    }

    // Supprimer le client de la liste des actifs
    if (clients->first == client) {
        clients->first = client->next;
    } else if (clients->last == client) {
        clients->last = client->previous;
        if (clients->last) {
            clients->last->next = NULL;
        }
    } else if (client->previous && client->next) {
        client->previous->next = client->next;
        client->next->previous = client->previous;
    }
    --clients->nb;

    printf("[DEBUG] Client %s removed from active clients list.\n", client->username);

    clear_client_invites(client);

    // Nettoyer les observateurs
    Observer *observer = client->observers->first;
    while (observer) {
        Observer *temp = observer;
        observer = observer->next;

        if (temp->watcher) {
            temp->watcher->watching = NULL;
            send_message_to_client(temp->watcher->socket,
                                   "The client you were watching went offline, you are no longer watching anyone.");
        }

        printf("[DEBUG] Freeing observer: %p\n", (void *)temp);
        free(temp);
    }
    client->observers->first = NULL;
    client->observers->last = NULL;

    // Nettoyer la liste des amis
    Friend *friend_it = client->friends->first;
    while (friend_it) {
        Friend *temp = friend_it;
        friend_it = friend_it->next;
        free(temp);
    }
    client->friends->first = NULL;
    client->friends->last = NULL;

    // Libération des ressources
    free(client->friends);
    free(client->observers);
    free(client);
    printf("[DEBUG] Client resources cleaned up.\n");
}

// Ajoute un observateur à la liste des observateurs
int add_observer_to_list(Observers *observers, Observer *observer)
{
  if (!observers->first)
  {
    observers->first = observer;
    observers->last = observer;
    return 1;
  }
  else
  {
    observers->last->next = observer;
    observer->previous = observers->last;
    observers->last = observer;
    return 1;
  }
}

// Supprime un observateur de la liste des observateurs
void remove_observer_from_list(Observers *observers, Client *client) {
    if (!observers || !client) {
        printf("[DEBUG] Observers list or client is NULL.\n");
        return;
    }

    Observer *observer = observers->first;
    while (observer) {
        if (observer->watcher && strcmp(observer->watcher->username, client->username) == 0) {
            // Retirer de la liste
            if (observer->previous) {
                observer->previous->next = observer->next;
            } else {
                observers->first = observer->next; // Mise à jour de la tête
            }

            if (observer->next) {
                observer->next->previous = observer->previous;
            } else {
                observers->last = observer->previous; // Mise à jour de la queue
            }

            printf("[DEBUG] Freeing observer: %p (username: %s)\n", (void *)observer, client->username);

            // Libération sécurisée
            observer->watcher = NULL; // Évite les pointeurs invalides
            free(observer);
            return;
        }

        observer = observer->next;
    }

    printf("[DEBUG] Observer %s not found in the list.\n", client->username);
}

// Ajoute un ami à la liste des amis
int add_friend_to_list(FriendList *friends, Friend *new_friend)
{
  if (!friends->first)
  {
    friends->first = new_friend;
    friends->last = new_friend;
    return 1;
  }
  else
  {
    friends->last->next = new_friend;
    new_friend->previous = friends->last;
    friends->last = new_friend;
    return 1;
  }
}

// Ajoute une invitation à la liste des invitations
int add_invite_to_list(Invites *invites, Client *recipient)
{
  if (invites->first == NULL)
  {
    Invite *invite = malloc(sizeof(Invite));
    invites->first = invite;
    invite->recipient = recipient;
    invite->next = NULL;
    return 1;
  }
  else
  {
    Invite *it_invite = invites->first;
    while (it_invite->next != NULL)
    {
      if (!strcmp(it_invite->recipient->username, recipient->username))
      {
        return 0;
      }
      it_invite = it_invite->next;
    }
    if (!strcmp(it_invite->recipient->username, recipient->username))
    {
      return 0;
    }
    Invite *invite = malloc(sizeof(Invite));
    it_invite->next = invite;
    invite->recipient = recipient;
    invite->next = NULL;
    return 1;
  }
}

// Supprime une invitation de la liste des invitations
void remove_invite_from_list(Invites *invites, Invite *invite)
{
  Invite *it_invite = invites->first;
  if (it_invite == invite)
  {
    invites->first = it_invite->next;
    free(it_invite);
    return;
  }
  while (it_invite->next != invite)
  {
    it_invite = it_invite->next;
  }
  Invite *invite_to_remove = it_invite->next;
  it_invite->next = it_invite->next->next;
  free(invite_to_remove);
}

// Supprime toutes les invitations d'un client
void clear_client_invites(Client *client) {
    if (!client || !client->invites || !client->friend_requests_sent) {
        printf("[DEBUG] Client or invites list is NULL.\n");
        return;
    }

    // Libération des invitations reçues
    Invite *invite = client->invites->first;
    while (invite) {
        Invite *temp = invite;
        invite = invite->next;

        printf("[DEBUG] Freeing invite: %p\n", (void *)temp);
        free(temp); // Libérer l'invitation
    }
    client->invites->first = NULL;

    // Libération des invitations envoyées
    invite = client->friend_requests_sent->first;
    while (invite) {
        Invite *temp = invite;
        invite = invite->next;

        printf("[DEBUG] Freeing sent invite: %p\n", (void *)temp);
        free(temp); // Libérer l'invitation
    }
    client->friend_requests_sent->first = NULL;

    printf("[DEBUG] Client invites cleared.\n");
}

// Vérifie si un client est dans la liste des invitations
int is_client_in_invite_list(const Invites *invites, const Client *recipient)
{
  Invite *it_invite = invites->first;
  while (it_invite)
  {
    if (!strcmp(it_invite->recipient->username, recipient->username))
    {
      return 1;
    }
    it_invite = it_invite->next;
  }
  return 0;
}

// Trouve un client par son nom d'utilisateur
Client *find_active_client_by_username(const ActiveClients clients,
                                       const char *username)
{
  Client *client_iterator = clients.first;
  while (client_iterator != NULL)
  {
    if (!strcmp(client_iterator->username, username))
    {
      break;
    }
    client_iterator = client_iterator->next;
  }
  return client_iterator;
}

// Envoie un message à tous les clients
void broadcast_to_all_clients(ActiveClients clients, Client sender,
                              const char *buffer, char from_server)
{
  char message[BUF_SIZE];
  Client *client_iterator = clients.first;
  while (client_iterator)
  {
    message[0] = 0;
    {
      /* we don't send message to the sender */
      if (sender.socket != client_iterator->socket)
      {
        if (from_server == 0)
        {
          strncpy(message, sender.username, BUF_SIZE - 1);
          strncat(message, " : ", sizeof message - strlen(message) - 1);
        }
        strncat(message, buffer, sizeof message - strlen(message) - 1);
        send_message_to_client(client_iterator->socket, message);
      }
    }
    client_iterator = client_iterator->next;
  }
}

// Envoie un message à tous les observateurs
void broadcast_to_observers(Observers *observers, const char *message)
{
  Observer *observer_iterator = observers->first;
  while (observer_iterator)
  {
    send_message_to_client(observer_iterator->watcher->socket, message);
    observer_iterator = observer_iterator->next;
  }
}

// Écrit un message à un client
void send_message_to_client(SOCKET sock, const char *buffer)
{
  if (send(sock, buffer, strlen(buffer), 0) < 0)
  {
    perror("send()");
    exit(errno);
  }
}

// Vérifie si un client est dans la liste des clients actifs
int is_client_in_active_list(Client *client, ActiveClients list_of_clients)
{
  Client *client_iterator = list_of_clients.first;
  while (client_iterator)
  {
    if (!strcmp(client_iterator->username, client->username))
    {
      return 1;
    }
    client_iterator = client_iterator->next;
  }
  return 0;
}

// Supprime une invitation d'un nouvel ami
void remove_invite_after_friendship(Client *client, Client *new_friend_client)
{
  Invite *invite_it = client->friend_requests_sent->first;
  while (invite_it)
  {
    if (!strcmp(invite_it->recipient->username, new_friend_client->username))
    {
      break;
    }
    invite_it = invite_it->next;
  }
  if (!invite_it)
  {
    printf("Invite not found in friendlist, should not be happening\n");
    return;
  }
  remove_invite_from_list(client->friend_requests_sent, invite_it);
}

// Vérifie si deux clients sont amis
int are_clients_friends(Client *client1, Client *client2)
{
  Friend *friend = client1->friends->first;
  while (friend)
  {
    if (!strcmp(friend->friend_of_client->username, client2->username))
    {
      return 1;
    }
    friend = friend->next;
  }
  return 0;
}

int is_already_connected(ActiveClients clients, char *username)
{
  Client *client_iterator = clients.first;
  while (client_iterator)
  {
    if (!strcmp(client_iterator->username, username))
    {
      return 1;
    }
    client_iterator = client_iterator->next;
  }
  return 0;
}

// Vérifie si un nom d'utilisateur est déjà utilisé
int is_username_already_used(ActiveClients clients, char *username)
{
  FILE *file = fopen("users.csv", "r");
  if (!file)
  {
    perror("fopen()");
    return 0;
  }
  char line[100];
  while (fgets(line, 100, file))
  {
    char *token = strtok(line, ",");
    if (!strcmp(token, username))
    {
      fclose(file);
      return 1;
    }
  }
  fclose(file);
  return 0;
}

int register_client(char *username, char *password)
{
  FILE *file = fopen("users.csv", "a");
  if (!file)
  {
    perror("fopen()");
    return 0;
  }

  char salt[BCRYPT_HASHSIZE];
  char hash[BCRYPT_HASHSIZE];

  int ret = bcrypt_gensalt(12, salt);

  ret = bcrypt_gensalt(12, salt);
  assert(ret == 0);
  ret = bcrypt_hashpw(password, salt, hash);
  assert(ret == 0);

  fprintf(file, "%s,%s,Aucun bio.\n", username, hash);
  fclose(file);
  return 1;
}

int login_client(char *username, char *password)
{
  FILE *file = fopen("users.csv", "r");
  if (!file)
  {
    perror("fopen()");
    return 0;
  }
  char line[100];
  while (fgets(line, 100, file))
  {
    char *token = strtok(line, ",");
    if (!strcmp(token, username))
    {
      token = strtok(NULL, ",");

      // Remove newline character
      char *pos;
      if ((pos = strchr(token, '\n')) != NULL)
      {
        *pos = '\0';
      }

      int ret = bcrypt_checkpw(password, token);
      assert(ret != -1);

      if (!ret)
      { // Passwords match
        fclose(file);
        return 1;
      }
    }
  }
  fclose(file);
  return 0;
}

int init_client(Client *client)
{
  // Init friends
  FILE *file = fopen("friends.csv", "r");
  if (!file)
  {
    perror("fopen()");
    return 0;
  }

  char *username = client->username;

  char line[450];
  while (fgets(line, 100, file))
  {
    char *token = strtok(line, ",");

    if (!strcmp(token, username))
    {
      token = strtok(NULL, ",");

      // Remove newline character
      char *pos;
      if ((pos = strchr(token, '\n')) != NULL)
      {
        *pos = '\0';
      }

      Friend *friend = malloc(sizeof(Friend));
      friend->next = NULL;

      Client *friend_client = malloc(sizeof(Client));
      strcpy(friend_client->username, token);

      friend->friend_of_client = friend_client;
      add_friend_to_list(client->friends, friend);
    }
  }
  fclose(file);

  // Init bio
  file = fopen("users.csv", "r");
  if (!file)
  {
    perror("fopen()");
    return 0;
  }

  while (fgets(line, 450, file))
  {
    char *token = strtok(line, ",");
    if (!strcmp(token, username))
    {
      token = strtok(NULL, ",");
      token = strtok(NULL, ","); // Get bio

      // Remove newline character
      char *pos;
      if ((pos = strchr(token, '\n')) != NULL)
      {
        *pos = '\0';
      }

      strcpy(client->bio, token);
    }
  }
  fclose(file);
  return 0;
}

int persist_friend_client(Client *client, Client *friend)
{
  FILE *file = fopen("friends.csv", "a");
  if (!file)
  {
    perror("fopen()");
    return -1;
  }
  fprintf(file, "%s,%s\n", client->username, friend->username);
  fclose(file);
  return 0;
}

int persist_bio_client(Client *client, char *bio)
{
  FILE *file = fopen("users.csv", "r");
  if (!file)
  {
    perror("fopen()");
    return -1;
  }

  FILE *temp = fopen("users_temp.csv", "w");
  if (!temp)
  {
    perror("fopen()");
    fclose(file);
    return -1;
  }

  char line[450];
  int updated = 0;
  while (fgets(line, sizeof(line), file))
  {
    char *username = strtok(line, ",");
    char *password = strtok(NULL, ",");
    char *current_bio = strtok(NULL, ",");

    if (username && password && current_bio && strcmp(username, client->username) == 0)
    {
      fprintf(temp, "%s,%s,%s\n", username, password, bio);
      updated = 1;
    }
    else
    {
      fprintf(temp, "%s,%s,%s", username, password, current_bio);
    }
  }

  fclose(file);
  fclose(temp);

  // Replace the original file with the temporary file
  if (updated)
  {
    remove("users.csv");
    rename("users_temp.csv", "users.csv");
  }
  else
  {
    remove("users_temp.csv");
  }

  return updated ? 0 : -1;
}

int read_bio_client(char *username, char *bio)
{
  printf("DEBUG\n");
  fflush(stdout);
  FILE *file = fopen("users.csv", "r");
  if (!file)
  {
    perror("fopen()");
    return -1;
  }

  printf("DEBUG\n");
  fflush(stdout);

  char line[450];
  while (fgets(line, 450, file))
  {
    char *token = strtok(line, ",");
    if (!strcmp(token, username))
    {
      token = strtok(NULL, ",");
      token = strtok(NULL, ","); // Get bio

      // Remove newline character
      char *pos;
      if ((pos = strchr(token, '\n')) != NULL)
      {
        *pos = '\0';
      }

      strcpy(bio, token);
      printf("Bio: %s\n", bio);
      fflush(stdout);
      return 0;
    }
  }

  fclose(file);
  return -1;
}