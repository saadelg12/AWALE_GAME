#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../options.h"

void strip_newline(char *str)
{
  str[strcspn(str, "\n")] = '\0'; // Remplace le '\n' par '\0'
}

int handle_client_input(char *buffer)
{
  char *it = buffer;      // Pointeur pour parcourir le buffer
  char command[BUF_SIZE]; // Stocke la commande extraite
  int i = 0;

  // Vérifie si le message ne commence pas par '/'
  if (*it++ != '/')
  {
    char *p = strstr(buffer, "\n"); // Trouve la fin de ligne
    if (p)
    {
      *p = '\0'; // Remplace par un terminateur nul
    }
    else
    {
      buffer[BUF_SIZE - 1] = '\0'; // Assure la fin de chaîne
    }
    return 1; // Message standard
  }

  // Lecture de la commande jusqu'à un espace ou fin de chaîne
  while (*it && !isspace(*it) && i < BUF_SIZE - 1)
  {
    command[i++] = *it++;
  }
  command[i] = '\0'; // Ajoute le terminateur nul

  // Sauter les espaces avant les arguments de la commande
  while (*it && isspace(*it))
    it++;

  // Logique de traitement des commandes
  if (!strcmp(command, "list_users"))
  {
    snprintf(buffer, BUF_SIZE, "/000");
    return 1;
  }
  else if (!strcmp(command, "start_duel"))
  {
    if (*it == '\0')
    {
      puts("Expected cmd /start_duel <username>\n");
      return 0;
    }
    strip_newline(it);
    snprintf(buffer, BUF_SIZE, "/001 %s", it);
    return 1;
  }
  else if (!strcmp(command, "list_games"))
  {
    snprintf(buffer, BUF_SIZE, "/002");
    return 1;
  }
  else if (!strcmp(command, "make_move"))
  {
    int pit_num;
    if (sscanf(it, "%d", &pit_num) == 1 && pit_num >= 1 && pit_num <= 6)
    {
      strip_newline(it);
      snprintf(buffer, BUF_SIZE, "/003 %d", pit_num);
      return 1;
    }
    else
    {
      puts("Invalid move. Usage: /make_move <pit_num>\n");
      return 0;
    }
  }
  else if (!strcmp(command, "watch_player"))
  {
    if (*it == '\0')
    {
      puts("Expected cmd /watch_player <username>\n");
      return 0;
    }
    strip_newline(it);
    snprintf(buffer, BUF_SIZE, "/004 %s", it);
    return 1;
  }
  else if (!strcmp(command, "change_bio"))
  {
    if (*it == '\0')
    {
      puts("Expected cmd /change_bio <new_bio>\n");
      return 0;
    }

    if (strlen(it) >= BIO_SIZE)
    {
      puts("Bio's length exceeds maximum allowed size.\n");
      return 0;
    }

    strip_newline(it);
    snprintf(buffer, BUF_SIZE, "/005 %s", it);
    return 1;
  }
  else if (!strcmp(command, "show_bio"))
  {
    if (*it == '\0')
    {
      puts("Expected cmd /show_bio <username>\n");
      return 0;
    }
    strip_newline(it);
    snprintf(buffer, BUF_SIZE, "/006 %s", it);
    return 1;
  }
  else if (!strcmp(command, "add_friend"))
  {
    if (*it == '\0')
    {
      puts("Expected cmd /add_friend <username>\n");
      return 0;
    }
    strip_newline(it);
    snprintf(buffer, BUF_SIZE, "/007 %s", it);
    return 1;
  }
  else if (!strcmp(command, "list_friends"))
  {
    snprintf(buffer, BUF_SIZE, "/008");
    return 1;
  }
  else if (!strcmp(command, "toggle_private"))
  {
    snprintf(buffer, BUF_SIZE, "/009");
    return 1;
  }
  else if (!strcmp(command, "leave_game"))
  {
    snprintf(buffer, BUF_SIZE, "/010");
    return 1;
  }
  else if (!strcmp(command, "game_history"))
  {
    snprintf(buffer, BUF_SIZE, "/011");
    return 1;
  }
  else if (!strcmp(command, "replay_game"))
  {
    int game_id;
    if (sscanf(it, "%d", &game_id) == 1)
    {
      strip_newline(it);
      snprintf(buffer, BUF_SIZE, "/012 %d", game_id);
      return 1;
    }
    else
    {
      puts("Invalid command. Usage: /replay_game <game_id>\n");
      return 0;
    }
  }
  else if (!strcmp(command, "help"))
  {
    snprintf(buffer, BUF_SIZE, "/013");
    return 1;
  }
  else if (!strcmp(command, "private_message"))
  {
    char recipient[USERNAME_SIZE];
    if (sscanf(it, "%s", recipient) != 1)
    {
      puts("Usage: /private_message <username> <message>\n");
      return 0;
    }

    char *message = strchr(it, ' ');
    if (!message)
    {
      puts("Please provide a message.\n");
      return 0;
    }

    message++;
    strip_newline(message);
    snprintf(buffer, BUF_SIZE, "/014 %s %s", recipient, message);
    return 1;
  }
  else if (!strcmp(command, "stop_watch"))
  {
    snprintf(buffer, BUF_SIZE, "/015");
    return 1;
  }
  else if (!strcmp(command, "ranking"))
  {
    snprintf(buffer, BUF_SIZE, "/016");
    return 1;
  }
  else
  {
    puts("Unknown command. Type /help for a list of available commands.\n");
    return 0;
  }
}