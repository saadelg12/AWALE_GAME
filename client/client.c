#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"
#include "client_command_parser.h"

// Fonction principale de l'application
static void client_main_loop(const char *address, const char *name) {
  SOCKET sock = init_connection(address); // Initialisation de la connexion
  char buffer[BUF_SIZE];

  fd_set rdfs; // ensemble de descripteurs de fichiers

  assert(strlen(name) < USERNAME_SIZE &&
         "username length must be <= USERNAME_SIZE"); // Vérification de la longueur du nom d'utilisateur

  write_server(sock, name); // Envoi du nom d'utilisateur au serveur

  while (1) {
    FD_ZERO(&rdfs); // Réinitialise l'ensemble

    FD_SET(STDIN_FILENO, &rdfs); // Ajoute stdin à l'ensemble

    FD_SET(sock, &rdfs); // Ajoute le socket à l'ensemble

    if (select(sock + 1, &rdfs, NULL, NULL, NULL) == -1) {
      perror("select()");
      exit(errno);
    }

    if (FD_ISSET(STDIN_FILENO, &rdfs)) { // Si quelque chose provient du clavier
      if (fgets(buffer, BUF_SIZE - 1, stdin)) {
        if (!handle_client_input(buffer)) {
          continue;
        }
      } else {
        break;
      }
      write_server(sock, buffer); // Envoi du message au serveur
    } else if (FD_ISSET(sock, &rdfs)) { // Si quelque chose provient du socket
      int n = read_server(sock, buffer);
      if (n == 0) { // serveur déconnecté
        printf("Server disconnected !\n");
        break;
      }
      puts(buffer); // Affiche le message reçu
    }
  }

  end_connection(sock); // Fin de la connexion
}

// Initialisation de la connexion
static int init_connection(const char *address) {
  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
  SOCKADDR_IN sin = {0};
  struct hostent *hostinfo;

  if (sock == INVALID_SOCKET) {
    perror("socket()");
    exit(errno);
  }

  hostinfo = gethostbyname(address);
  if (hostinfo == NULL) {
    fprintf(stderr, "Unknown host %s.\n", address);
    exit(EXIT_FAILURE);
  }

  sin.sin_addr = *(IN_ADDR *)hostinfo->h_addr;
  sin.sin_port = htons(PORT);
  sin.sin_family = AF_INET;

  if (connect(sock, (SOCKADDR *)&sin, sizeof(SOCKADDR)) == SOCKET_ERROR) {
    perror("connect()");
    exit(errno);
  }

  return sock;
}

// Fin de la connexion
static void end_connection(int sock) { closesocket(sock); }

// Lecture des données du serveur
static int read_server(SOCKET sock, char *buffer) {
  int n = 0;

  if ((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0) {
    perror("recv()");
    exit(errno);
  }

  buffer[n] = 0;

  return n;
}

// Envoi des données au serveur
static void write_server(SOCKET sock, const char *buffer) {
  if (send(sock, buffer, strlen(buffer), 0) < 0) {
    perror("send()");
    exit(errno);
  }
}

// Fonction principale
int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage : %s [address] [pseudo]\n", argv[0]);
    return EXIT_FAILURE;
  }

  client_main_loop(argv[1], argv[2]); // Appel de la fonction principale de l'application

  return EXIT_SUCCESS;
}
