#include "server_command_handler.h"
#include "game_logic.h"
#include "server_client_manager.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Fonction pour envoyer un message d'aide
void send_help_message(Client *client)
{
  const char *help_message =
      "Available commands:\n"
      "/list_users                       - List all active users\n"
      "/start_duel <user>                - Challenge a user to a duel\n"
      "/list_games                       - List all active games\n"
      "/make_move <pit>                  - Make a move in your game\n"
      "/watch_player <user>              - Watch a player's game\n"
      "/stop_watch                       - Stop watching a player's game\n"
      "/change_bio <bio>                 - Update your biography\n"
      "/show_bio <user>                  - Show the biography of a user\n"
      "/add_friend <user>                - Send a friend request\n"
      "/list_friends                     - List your friends\n"
      "/toggle_private                   - Toggle private mode\n"
      "/leave_game                       - Leave your current game\n"
      "/game_history                     - View your game history\n"
      "/replay_game <id>                 - Replay a game by ID\n"
      "/private_message <user> <message> - Send a private message\n"
      "/ranking                          - Show the ranking\n"
      "/help                             - Show this help message\n";

  // Envoi du message d'aide au client
  send_message_to_client(client->socket, help_message);
}

// Fonction pour obtenir la liste des clients actifs
void send_active_clients_list(ActiveClients active_clients, Client *client,
                              char *message)
{
  Client *client_iterator = active_clients.first;
  message[0] = '\0';
  strcat(message, "Active users:\n");
  while (client_iterator)
  {
    strcat(message, client_iterator->username);
    if (!strcmp(client_iterator->username, client->username))
    {
      strcat(message, " (you)");
    }
    if (client_iterator->priv)
    {
      strcat(message, " (private)");
    }
    if (client_iterator != active_clients.last)
    {
      strcat(message, "\n");
    }
    client_iterator = client_iterator->next;
  }
  send_message_to_client(client->socket, message);
}

// Fonction pour comparer les scores (pour le tri)
int compare_scores(const void *a, const void *b)
{
  return ((PlayerScore *)b)->points - ((PlayerScore *)a)->points;
}

// Fonction pour ajouter des points à un joueur
void add_points_to_player(PlayerScore *scores, int *player_count, const char *player, int points)
{
  if (player == NULL || strlen(player) == 0)
  {
    return; // Ignorer les joueurs non valides
  }

  for (int i = 0; i < *player_count; i++)
  {
    if (strcmp(scores[i].username, player) == 0)
    {
      scores[i].points += points;
      return;
    }
  }

  // Nouveau joueur
  strcpy(scores[*player_count].username, player);
  scores[*player_count].points = points;
  (*player_count)++;
}

// Fonction pour calculer le classement
void send_ranking_to_client(Client *client)
{
  FILE *file = fopen("game_data.csv", "r");
  if (!file)
  {
    send_message_to_client(client->socket, "Error: Could not read game_data.csv\n");
    return;
  }

  PlayerScore scores[100];
  int player_count = 0;

  char line[256];
  while (fgets(line, sizeof(line), file))
  {
    // Ignorer les lignes vides ou malformées
    if (strlen(line) < 2)
    {
      continue;
    }

    // Vérification de 'end' pour passer au jeu suivant
    if (strstr(line, "end"))
    {
      continue;
    }

    // Parse la ligne
    char *game_id = strtok(line, ",");
    char *player1 = strtok(NULL, ",");
    char *player2 = strtok(NULL, ",");
    strtok(NULL, ","); // Skip score1
    strtok(NULL, ","); // Skip score2
    strtok(NULL, ","); // Skip rotation
    char *winner = strtok(NULL, ",");

    if (player1 == NULL || player2 == NULL)
    {
      continue; // Ignorer les entrées mal formées
    }

    if (winner != NULL)
    {
      // Nettoyer et vérifier les gagnants
      winner[strcspn(winner, "\n")] = '\0'; // Retirer '\n'

      if (strlen(winner) > 0)
      {
        add_points_to_player(scores, &player_count, winner, 3);
        continue;
      }
    }

    // Si pas de gagnant explicite : +1 à chaque joueur
    add_points_to_player(scores, &player_count, player1, 1);
    add_points_to_player(scores, &player_count, player2, 1);
  }

  fclose(file);

  // Trier les résultats et envoyer au client
  qsort(scores, player_count, sizeof(PlayerScore), compare_scores);

  char message[1024] = "Ranking:\n";
  for (int i = 0; i < player_count; i++)
  {
    char entry[100];
    snprintf(entry, sizeof(entry), "%d. %s - %d points\n",
             i + 1, scores[i].username, scores[i].points);
    strcat(message, entry);
  }

  send_message_to_client(client->socket, message);
}

// Fonction pour envoyer une invitation
void send_game_invite(ActiveClients clients, Client *sender,
                      const char *recipient_username, char *message,
                      int *current_gm_id)
{
  if (sender->opponent)
  {
    strcpy(message, "You can't send an invite while in a game");
    send_message_to_client(sender->socket, message);
    return;
  }
  Client *recipient = find_active_client_by_username(clients, recipient_username);

  if (recipient == NULL)
  {
    strcpy(message, "The user you asked for is not connected.");
    send_message_to_client(sender->socket, message);
  }
  else if (recipient->opponent != NULL)
  {
    strcpy(message, "The user you asked for is currently in a game.");
    send_message_to_client(sender->socket, message);
  }
  else if (!strcmp(recipient->username, sender->username))
  {
    strcpy(message, "You cannot challenge yourself");
    send_message_to_client(sender->socket, message);
  }
  else if (!is_client_in_invite_list(recipient->invites, sender))
  {
    if (add_invite_to_list(sender->invites, recipient))
    {
      strcpy(message, "You got an invite for a game against ");
      strcat(message, sender->username);
      strcat(message, " type /start_duel ");
      strcat(message, sender->username);
      strcat(message, " to accept the request");
      send_message_to_client(recipient->socket, message);
      strcpy(message, "Invite sent to ");
      strcat(message, recipient->username);
      send_message_to_client(sender->socket, message);
    }
    else
    {
      strcpy(message, "You already sent an invite to this player");
      send_message_to_client(sender->socket, message);
    }
  }
  else
  {
    Game *game =
        initialize_game(sender->username, recipient->username, current_gm_id);

    sender->opponent = recipient;
    sender->game = game;
    // WARNING: BOTH PLAYERS SHARE THE SAME GAME OBJECT
    recipient->game = game;
    recipient->opponent = sender;

    clear_client_invites(recipient);
    clear_client_invites(sender);
    if (recipient->watching)
    {
      remove_observer_from_list(recipient->watching->observers, recipient);
      recipient->watching = NULL;
    }
    if (sender->watching)
    {
      remove_observer_from_list(sender->watching->observers, sender);
      sender->watching = NULL;
    }

    strcpy(message, "The game against ");
    strcat(message, sender->username);
    strcat(message, " has started.\n");
    send_message_to_client(recipient->socket, message);

    strcpy(message, "The game against ");
    strcat(message, recipient->username);
    strcat(message, " has started.\n");
    send_message_to_client(sender->socket, message);

    send_message_to_client(sender->socket, generate_board_representation(game, 1));

    send_message_to_client(recipient->socket, generate_board_representation(game, 2));

    srand(time(NULL));
    int first = (rand() % 2) + 1;
    if (first == 1)
    {
      send_message_to_client(sender->socket, "You start, type /make_move <pit_num>");
      sender->turn = 1;
    }
    else
    {
      send_message_to_client(recipient->socket, "You start, type /make_move <pit_num>");
      recipient->turn = 1;
    }
  }
}

// Fonction pour traiter un mouvement de jeu effectué par un client
void process_game_move(Client *sender, int num, Games *games)
{
  if (!sender->turn)
  {
    send_message_to_client(sender->socket, "It's not your turn.");
    return;
  }

  Game *game = sender->game;
  int player = (!strcmp(sender->username, game->player1)) ? (1) : (2);
  int opp = (player == 1) ? (2) : (1);
  int check = execute_move(game, num, player);

  if (!check)
  {
    send_message_to_client(sender->socket, "Illegal move; you must feed your opponent.\n");
    return;
  }

  if (check == -1)
  {
    send_message_to_client(sender->socket, "Illegal move; you must choose a pit with seeds.\n");
    return;
  }

  sender->turn = 0;
  sender->opponent->turn = 1;

  char board[500];
  snprintf(board, sizeof(board), "%s", generate_board_representation(game, player));
  send_message_to_client(sender->socket, board);
  broadcast_to_observers(sender->observers, board);

  snprintf(board, sizeof(board), "Opponent's play :\n%s", generate_board_representation(game, opp));
  send_message_to_client(sender->opponent->socket, board);
  broadcast_to_observers(sender->opponent->observers, board);

  if (check_game_end(game, player))
  {
    send_message_to_client(sender->socket, "End of the game.\n");
    send_message_to_client(sender->opponent->socket, "End of the game.\n");
    broadcast_to_observers(sender->opponent->observers, "End of the game.\n");
    finalize_game_session(sender, games);
  }
}

// Fonction pour obtenir la liste des jeux en cours
void send_active_games_list(ActiveClients clients, Client *client, char *buffer)
{
  // printf("Beginning\n");
  Client *client_iterator = clients.first;
  ActiveClients clients_already_dealt_with;
  clients_already_dealt_with.first = NULL;
  clients_already_dealt_with.last = NULL;
  strcpy(buffer, "List of the games currently going on:");
  while (client_iterator)
  {
    if (client_iterator->game != NULL)
    {
      if (!is_client_in_active_list(client_iterator, clients_already_dealt_with))
      {
        Client *cli = malloc(sizeof(Client));
        Client *opponent = malloc(sizeof(Client));
        strcpy(cli->username, client_iterator->username);
        strcpy(opponent->username, client_iterator->opponent->username);
        add_active_client(&clients_already_dealt_with, cli);
        add_active_client(&clients_already_dealt_with, opponent);
        strcat(buffer, "\n");
        strcat(buffer, client_iterator->username);
        strcat(buffer, " vs ");
        strcat(buffer, client_iterator->opponent->username);
      }
    }
    client_iterator = client_iterator->next;
  }
  if (clients_already_dealt_with.first == NULL)
  {
    send_message_to_client(client->socket, "No games going on currently");
  }
  else
  {
    send_message_to_client(client->socket, buffer);
  }
}

// Fonction pour regarder une partie de jeu
void start_watching_player(ActiveClients clients, Client *client, char *username, char *buffer)
{
  if (!client || !username)
  {
    snprintf(buffer, BUF_SIZE, "Invalid client or username.");
    send_message_to_client(client->socket, buffer);
    return;
  }

  if (!strcmp(client->username, username))
  {
    if (client->watching)
    {
      if (client->watching->observers)
      {
        remove_observer_from_list(client->watching->observers, client);
      }
      client->watching = NULL;
    }
    strcpy(buffer, "You are not watching anyone anymore");
    send_message_to_client(client->socket, buffer);
    return;
  }

  if (client->game)
  {
    strcpy(buffer, "You can't watch someone while you're in a game");
    send_message_to_client(client->socket, buffer);
    return;
  }

  Client *client_to_watch = find_active_client_by_username(clients, username);
  if (!client_to_watch)
  {
    snprintf(buffer, BUF_SIZE, "%s is not connected", username);
    send_message_to_client(client->socket, buffer);
    return;
  }

  if (client_to_watch->priv && !are_clients_friends(client, client_to_watch))
  {
    snprintf(buffer, BUF_SIZE, "%s is in private mode and you are not friends with them", username);
    send_message_to_client(client->socket, buffer);
    return;
  }

  if (client->watching)
  {
    if (client->watching->observers)
    {
      remove_observer_from_list(client->watching->observers, client);
    }
  }

  client->watching = client_to_watch;

  Observer *observer = malloc(sizeof(Observer));
  if (!observer)
  {
    perror("malloc()");
    strcpy(buffer, "Failed to allocate memory for observer");
    send_message_to_client(client->socket, buffer);
    return;
  }

  observer->watcher = client;
  observer->previous = NULL;
  observer->next = NULL;

  if (!add_observer_to_list(client_to_watch->observers, observer))
  {
    free(observer);
    strcpy(buffer, "Failed to add observer to the list");
    send_message_to_client(client->socket, buffer);
    return;
  }

  snprintf(buffer, BUF_SIZE, "You are now watching %s", username);
  send_message_to_client(client->socket, buffer);
}

// Fonction pour changer la biographie du joueur
void update_client_bio(Client *client, char *bio)
{
  strcpy(client->bio, bio);
  char message[200];
  strcpy(message, "Your bio was changed to: ");
  strcat(message, client->bio);
  persist_bio_client(client, bio);
  send_message_to_client(client->socket, message);
}

// Fonction pour obtenir la biographie d'un autre joueur
void send_user_bio(Client *client, char *username)
{
  char *bio = malloc(BIO_SIZE);
  if (!read_bio_client(username, bio))
  {
    send_message_to_client(client->socket, bio);
  }
  else
  {
    char message[200];
    strcpy(message, username);
    strcat(message, " was not found.");
    send_message_to_client(client->socket, message);
  }
  free(bio);
}

// Fonction pour envoyer une demande d'ami
void send_friend_invite(ActiveClients clients, Client *sender,
                        const char *recipient_username)
{
  Client *recipient = find_active_client_by_username(clients, recipient_username);
  char message[500];
  if (recipient == NULL)
  {
    strcpy(message, "The user you asked for is not connected.");
    send_message_to_client(sender->socket, message);
  }
  else if (!strcmp(recipient->username, sender->username))
  {
    strcpy(message, "You cannot send a friend request to yourself.");
    send_message_to_client(sender->socket, message);
  }
  else if (are_clients_friends(sender, recipient))

  {
    strcpy(message, "You are already friends with ");
    strcat(message, recipient->username);
    send_message_to_client(sender->socket, message);
  }
  else if (!is_client_in_invite_list(recipient->friend_requests_sent, sender))
  {
    if (add_invite_to_list(sender->friend_requests_sent, recipient))
    {
      strcpy(message, "You got a friend request from ");
      strcat(message, sender->username);
      strcat(message, " type /add_friend ");
      strcat(message, sender->username);
      strcat(message, " to accept the request");
      send_message_to_client(recipient->socket, message);
      strcpy(message, "Friend request sent to ");
      strcat(message, recipient->username);
      send_message_to_client(sender->socket, message);
    }
    else
    {
      strcpy(message, "You already sent a friend request to this player");
      send_message_to_client(sender->socket, message);
    }
  }
  else
  {
    Friend *friendship_1 = malloc(sizeof(Friend));
    friendship_1->next = NULL;
    friendship_1->friend_of_client = recipient;
    add_friend_to_list(sender->friends, friendship_1);
    strcpy(message, "You are now friends with ");
    strcat(message, recipient->username);
    send_message_to_client(sender->socket, message);

    persist_friend_client(sender, recipient);

    Friend *friendship_2 = malloc(sizeof(Friend));
    friendship_2->next = NULL;
    friendship_2->friend_of_client = sender;
    add_friend_to_list(recipient->friends, friendship_2);
    remove_invite_after_friendship(recipient, sender);
    strcpy(message, "You are now friends with ");
    strcat(message, sender->username);
    send_message_to_client(recipient->socket, message);

    persist_friend_client(recipient, sender);
  }
}

// Fonction pour envoyer la liste d'amis
void send_friends_list(Client *client)
{
  char message[1024];
  strcpy(message, "FriendList:");
  Friend *friend_it = client->friends->first;
  while (friend_it)
  {
    strcat(message, "\n");
    strcat(message, friend_it->friend_of_client->username);
    friend_it = friend_it->next;
  }
  send_message_to_client(client->socket, message);
}

// Fonction pour activer le mode privé
void toggle_private_mode(Client *client)
{
  client->priv = (client->priv + 1) % 2;
  char message[200];
  switch (client->priv)
  {
  case 0:
    strcpy(message, "Private mode off.");
    send_message_to_client(client->socket, message);
    break;
  case 1:
    strcpy(message, "Private mode on.");
    send_message_to_client(client->socket, message);
    Observer *observer = client->observers->first;
    while (observer)
    {
      if (!are_clients_friends(client, observer->watcher))
      {
        strcpy(message, client->username);
        strcat(message, " went private, only their friends can watch them");
        send_message_to_client(observer->watcher->socket, message);
        Observer *temp = observer;
        observer->watcher->watching = NULL;
        observer = observer->next;
        remove_observer_from_list(client->observers, temp->watcher);
      }
      else
      {
        observer = observer->next;
      }
    }
    break;
  }
}

// Fonction pour quitter une partie
void finalize_game_session(Client *client, Games *games)
{
  Game *game = client->game;
  client->game = NULL;
  client->opponent->game = NULL;
  client->opponent->opponent = NULL;
  client->opponent = NULL;
  export_game_to_csv(game, "game_data.csv");
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

// Fonction pour quitter le jeu
void leave_current_game(Client *client, Games *games) {
    char message[200];

    // Vérification si le client est dans un jeu
    if (!client->game) {
        strcpy(message, "You are not in a game.");
        send_message_to_client(client->socket, message);
        return;
    }

    Game *game = client->game;

    // Informer les observateurs que le joueur quitte
    Observer *observer = client->observers->first;
    while (observer) {
        send_message_to_client(observer->watcher->socket,
                               "The player you were watching has left the game.");
        observer->watcher->watching = NULL; // Réinitialiser leur état
        Observer *temp = observer;
        observer = observer->next;

        free(temp); // Libérer les observateurs
    }
    client->observers->first = NULL;
    client->observers->last = NULL;

    // Vérifier si l'adversaire existe
    if (client->opponent) {
        // Déclarer l'adversaire gagnant
        strcpy(game->winner, client->opponent->username);

        // Informer l'adversaire qu'il a gagné
        snprintf(message, sizeof(message), "%s left the game, you won.", client->username);
        send_message_to_client(client->opponent->socket, message);

        // Réinitialiser les références de l'adversaire
        client->opponent->opponent = NULL;
        client->opponent->game = NULL;

        // Informer les observateurs de l'adversaire
        Observer *opp_observer = client->opponent->observers->first;
        while (opp_observer) {
            snprintf(message, sizeof(message),
                     "%s left the game, %s won.", client->username, client->opponent->username);
            send_message_to_client(opp_observer->watcher->socket, message);

            opp_observer = opp_observer->next;
        }
    }

    // Finaliser et enregistrer la partie
    finalize_game_session(client, games);

    // Réinitialiser les références du joueur
    client->game = NULL;
    client->opponent = NULL;

    // Informer le joueur qu'il a quitté
    strcpy(message, "You left the game.");
    send_message_to_client(client->socket, message);
}


// Fonction pour obtenir l'historique des parties
void send_games_history(Client *client, Games *games, char *message)
{
  Game *game_iterator = games->first;
  message[0] = '\0';
  if (!game_iterator)
  {
    strcat(
        message,
        "There is no game in the record, go ahead and challenge a friend !\n");
  }
  else
  {
    strcat(message, "History of games:\n\n");
    while (game_iterator)
    {
      strcat(message, "- Game_id : ");
      char intStr[20];
      sprintf(intStr, "%d", game_iterator->game_id);
      strcat(message, intStr);
      strcat(message, " ");
      strcat(message, game_iterator->player1);
      strcat(message, " vs. ");
      strcat(message, game_iterator->player2);
      strcat(message, " and the winner was : ");
      strcat(message, game_iterator->winner);
      strcat(message, "\n");
      game_iterator = game_iterator->next;
    }
  }

  send_message_to_client(client->socket, message);
}

// Fonction pour trouver une partie par son identifiant
Game *get_game_by_id(Games *games, int game_id)
{
  Game *game_iterator = games->first;
  while (game_iterator)
  {
    if (game_id == game_iterator->game_id)
    {
      return game_iterator;
    }
    game_iterator = game_iterator->next;
  }
  return NULL;
}

// Fonction pour revoir une partie
void replay_game_history(Client *client, Games *games, int game_id, char *buffer)
{
  Game *game = get_game_by_id(games, game_id);
  if (game == NULL)
  {
    send_message_to_client(client->socket, "The game selected is not in the history");
  }
  else
  {
    char *message = replay_game(game);
    send_message_to_client(client->socket, message);
  }
}

// Fonction pour arrêter d'observer un joueur
void stop_watching_player(Client *client)
{
  if (client->watching)
  {
    remove_observer_from_list(client->watching->observers, client);
    client->watching = NULL;
    send_message_to_client(client->socket, "You have stopped watching any player.");
  }
  else
  {
    send_message_to_client(client->socket, "You are not currently watching any player.");
  }
}
