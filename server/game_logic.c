#define PITS_NB 12

#include "game_logic.h"
#include <stdlib.h>
#include <string.h>

Game *initialize_game(char *joueur1, char *joueur2, int *id_jeu_courant)
{
  Game *jeu = (Game *)malloc(sizeof(Game));
  jeu->board = (int *)malloc(sizeof(int) * PITS_NB);
  jeu->game_id = *id_jeu_courant;
  *id_jeu_courant += 1;

  strncpy(jeu->player1, joueur1, USERNAME_SIZE - 1);
  jeu->player1[USERNAME_SIZE - 1] = '\0';

  strncpy(jeu->player2, joueur2, USERNAME_SIZE - 1);
  jeu->player2[USERNAME_SIZE - 1] = '\0';

  jeu->moves = (Moves *)malloc(sizeof(Moves));
  jeu->moves->first = NULL;
  jeu->moves->last = NULL;
  jeu->moves->size = 0;

  for (int i = 0; i < PITS_NB; ++i)
  {
    jeu->board[i] = 4; // Initialisation des cases avec 4 graines chacune
  }

  jeu->score_player1 = 0;
  jeu->score_player2 = 0;
  jeu->rotation_sens = -1; // Sens de rotation par défaut

  return jeu;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_BUFFER_SIZE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_BUFFER_SIZE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_BUFFER_SIZE 500

char *generate_board_representation(Game *jeu, int joueur)
{
    int ligne_haute, ligne_basse;
    char *adversaire, *nom_joueur;
    ligne_haute = PITS_NB / 2 * (2 - joueur);
    ligne_basse = PITS_NB / 2 * joueur - 1;

    if (joueur == 1)
    {
        adversaire = jeu->player2;
        nom_joueur = jeu->player1;
    }
    else
    {
        adversaire = jeu->player1;
        nom_joueur = jeu->player2;
    }

    // Allouer un tampon initial
    size_t buffer_size = INITIAL_BUFFER_SIZE;
    char *plateau_affichage = (char *)malloc(buffer_size);
    if (!plateau_affichage)
    {
        return NULL; // Échec de l'allocation
    }

    size_t longueur_utilisee = 0;

    // Fonction pour assurer l'espace restant
    #define ENSURE_SPACE(size_needed) \
        if (longueur_utilisee + (size_needed) >= buffer_size) { \
            buffer_size *= 2; \
            char *new_buffer = realloc(plateau_affichage, buffer_size); \
            if (!new_buffer) { free(plateau_affichage); return NULL; } \
            plateau_affichage = new_buffer; \
        }

    // Ajouter les informations de l'adversaire
    ENSURE_SPACE(50);
    longueur_utilisee += snprintf(plateau_affichage + longueur_utilisee, buffer_size - longueur_utilisee, "%s's board \n\n", adversaire);

    for (int i = ligne_haute; i < ligne_haute + 6; ++i)
    {
        ENSURE_SPACE(10);
        longueur_utilisee += snprintf(plateau_affichage + longueur_utilisee, buffer_size - longueur_utilisee, " | %d", jeu->board[i]);
    }
    ENSURE_SPACE(10);
    longueur_utilisee += snprintf(plateau_affichage + longueur_utilisee, buffer_size - longueur_utilisee, " |\n");

    // Ligne de séparation
    for (int i = 0; i < PITS_NB / 2; ++i)
    {
        ENSURE_SPACE(10);
        longueur_utilisee += snprintf(plateau_affichage + longueur_utilisee, buffer_size - longueur_utilisee, "-----");
    }
    ENSURE_SPACE(10);
    longueur_utilisee += snprintf(plateau_affichage + longueur_utilisee, buffer_size - longueur_utilisee, "\n");

    for (int i = ligne_basse; i > ligne_basse - 6; --i)
    {
        ENSURE_SPACE(10);
        longueur_utilisee += snprintf(plateau_affichage + longueur_utilisee, buffer_size - longueur_utilisee, " | %d", jeu->board[i]);
    }
    ENSURE_SPACE(10);
    longueur_utilisee += snprintf(plateau_affichage + longueur_utilisee, buffer_size - longueur_utilisee, " |\n\n%s's board \n\n", nom_joueur);

    for (int j = 1; j <= 6; ++j)
    {
        ENSURE_SPACE(10);
        longueur_utilisee += snprintf(plateau_affichage + longueur_utilisee, buffer_size - longueur_utilisee, " | %d", j);
    }
    ENSURE_SPACE(10);
    longueur_utilisee += snprintf(plateau_affichage + longueur_utilisee, buffer_size - longueur_utilisee, " |\n\n");

    // Ajouter les scores capturés
    ENSURE_SPACE(100);
    longueur_utilisee += snprintf(plateau_affichage + longueur_utilisee, buffer_size - longueur_utilisee, "Captured seeds:\n");
    longueur_utilisee += snprintf(plateau_affichage + longueur_utilisee, buffer_size - longueur_utilisee, "%s: %d seeds\n", jeu->player1, jeu->score_player1);
    longueur_utilisee += snprintf(plateau_affichage + longueur_utilisee, buffer_size - longueur_utilisee, "%s: %d seeds\n", jeu->player2, jeu->score_player2);

    return plateau_affichage;
}

void copy_board(int *source, int *destination)
{
  for (int i = 0; i < PITS_NB; ++i)
  {
    destination[i] = source[i];
  }
}

void clean_board(Game *jeu)
{
  for (int i = 0; i < PITS_NB; ++i)
  {
    jeu->board[i] = 0; // Vide toutes les cases du plateau
  }
}

bool is_in_player_side(int joueur, int case_visitee)
{
  return (PITS_NB / 2 * joueur > case_visitee) &&
         (PITS_NB / 2 * (joueur - 1) <= case_visitee);
}

int execute_move(Game *jeu, int case_choisie, int joueur)
{
  int graines_restantes;
  int resultat = 1;
  int case_visitee, case_depart;
  int *score_joueur = (joueur == 1) ? (&jeu->score_player1) : (&jeu->score_player2);
  int score_temporaire = *score_joueur;
  int *plateau_temp1;
  int *plateau_temp2;
  int adversaire = (joueur == 1) ? (2) : (1);

  plateau_temp2 = (int *)malloc(PITS_NB * sizeof(int));
  copy_board(jeu->board, plateau_temp2);

  case_depart = (PITS_NB / 2 * joueur) - case_choisie;
  case_visitee = case_depart;

  graines_restantes = jeu->board[case_visitee];
  jeu->board[case_visitee] = 0;

  if (!graines_restantes)
  {
    return -1; // Case vide, mouvement invalide
  }

  while (graines_restantes > 0)
  {
    case_visitee = (case_visitee + jeu->rotation_sens) % 12;

    if (case_visitee == -1)
    {
      case_visitee = 11;
    }
    else if (case_visitee == 12)
    {
      case_visitee = 0;
    }

    if (case_visitee == case_depart)
    {
      continue;
    }

    jeu->board[case_visitee]++;
    graines_restantes--;
  }

  if (!player_has_seeds(jeu, adversaire))
  {
    copy_board(plateau_temp2, jeu->board);
    return 0; // Le joueur doit nourrir l'adversaire
  }

  plateau_temp1 = (int *)malloc(PITS_NB * sizeof(int));
  copy_board(jeu->board, plateau_temp1);

  while (!is_in_player_side(joueur, case_visitee) &&
         (jeu->board[case_visitee] == 2 || jeu->board[case_visitee] == 3))
  {
    *score_joueur += jeu->board[case_visitee];
    jeu->board[case_visitee] = 0;
    case_visitee -= jeu->rotation_sens;

    if (!player_has_seeds(jeu, adversaire))
    {
      copy_board(plateau_temp1, jeu->board);
      *score_joueur = score_temporaire;
      resultat = 2;
    }
  }

  log_move(jeu, joueur, case_choisie);
  return resultat;
}

void display_score(Game *jeu)
{
  printf("Score du joueur 1 : %d\n", jeu->score_player1);
  printf("Score du joueur 2 : %d\n", jeu->score_player2);
}

bool player_has_seeds(Game *jeu, int joueur)
{
  int case_actuelle = PITS_NB / 2 * (joueur - 1);
  while (jeu->board[case_actuelle] == 0 && case_actuelle < PITS_NB / 2 * joueur)
  {
    case_actuelle++;
  }
  return (case_actuelle == PITS_NB / 2 * joueur) ? false : true;
}

int nb_of_tiles(Game *jeu, int case_selectionnee, int joueur)
{
  if (jeu->rotation_sens == 1)
  {
    return (PITS_NB / 2 * joueur - case_selectionnee);
  }
  else
  {
    return case_selectionnee + 1 - 6 * (joueur - 1);
  }
}

bool can_feed(Game *jeu, int joueur)
{
  int case_actuelle = PITS_NB / 2 * (joueur - 1);
  while (jeu->board[case_actuelle] < nb_of_tiles(jeu, case_actuelle, joueur))
  {
    case_actuelle++;
  }
  return (case_actuelle == PITS_NB / 2 * joueur) ? false : true;
}

int get_pits_count(int selected_pit, int player, Game *game)
{
  if (game->rotation_sens == 1)
  {
    return (PITS_NB / 2 * (player)-selected_pit);
  }
  else
  {
    return selected_pit + 1 - 6 * (player - 1);
  }
}

bool enforce_feeding(Game *jeu, int *coups_possibles, int *index_coups, int joueur)
{
  if (player_has_seeds(jeu, joueur))
  {
    return false; // Pas besoin de nourrir
  }

  int case_actuelle = PITS_NB / 2 * (joueur - 1);
  *index_coups = 0;
  while (case_actuelle < PITS_NB / 2 * joueur)
  {
    if (jeu->board[case_actuelle] < nb_of_tiles(jeu, case_actuelle, joueur))
    {
      coups_possibles[*index_coups] = case_actuelle % (PITS_NB / 2);
      (*index_coups)++;
    }
    case_actuelle++;
  }

  return true; // Le joueur doit nourrir
}

int check_game_end(Game *jeu, int joueur)
{
  int adversaire = (joueur == 1) ? 2 : 1;
  int *score_joueur = (joueur == 1) ? &jeu->score_player1 : &jeu->score_player2;
  int *score_adversaire = (joueur == 1) ? &jeu->score_player2 : &jeu->score_player1;

  if (jeu->score_player1 >= 25)
  {
    return 1; // Joueur 1 gagne
  }

  if (jeu->score_player2 >= 25)
  {
    return 2; // Joueur 2 gagne
  }

  if (!player_has_seeds(jeu, joueur))
  {
    int coups_possibles[PITS_NB / 2];
    int index_coups = 0;

    if (enforce_feeding(jeu, coups_possibles, &index_coups, adversaire))
    {
      return 0; // La partie continue, l'adversaire doit nourrir
    }
    else
    {
      *score_adversaire += calculate_remaining_seeds(jeu, adversaire);
      return (*score_adversaire > *score_joueur) ? adversaire : joueur;
    }
  }

  return 0; // Partie non terminée
}

int calculate_remaining_seeds(Game *jeu, int joueur)
{
  int debut, fin;
  int somme = 0;

  if (joueur == 1)
  {
    debut = 0;
    fin = 5;
  }
  else
  {
    debut = 6;
    fin = 11;
  }

  for (int i = debut; i <= fin; i++)
  {
    somme += jeu->board[i];
  }

  return somme; // Retourne le total des graines restantes
}

void log_move(Game *jeu, int joueur, int case_selectionnee)
{
  Move *nouveau_coup = (Move *)malloc(sizeof(Move));
  char *nom_joueur = (joueur == 1) ? jeu->player1 : jeu->player2;

  strncpy(nouveau_coup->player, nom_joueur, USERNAME_SIZE - 1);
  nouveau_coup->value = case_selectionnee;

  if (jeu->moves->first == NULL)
  {
    jeu->moves->first = nouveau_coup;
    jeu->moves->last = nouveau_coup;
  }
  else
  {
    jeu->moves->last->next = nouveau_coup;
    nouveau_coup->previous = jeu->moves->last;
    jeu->moves->last = nouveau_coup;
  }

  nouveau_coup->next = NULL;
  jeu->moves->size++;
}

char *replay_game(Game *jeu)
{
  int id_temp = 5;
  Game *jeu_temp = initialize_game(jeu->player1, jeu->player2, &id_temp);
  jeu_temp->rotation_sens = jeu->rotation_sens;

  char *historique_plateau = (char *)malloc((200 * (jeu->moves->size + 1)) * sizeof(char));
  int perspective = 1;

  strcat(historique_plateau, generate_board_representation(jeu_temp, perspective));
  Move *coup_actuel = jeu->moves->first;

  while (coup_actuel != NULL)
  {
    int joueur = (!strcmp(coup_actuel->player, jeu->player1)) ? 1 : 2;
    execute_move(jeu_temp, coup_actuel->value, joueur);
    strcat(historique_plateau, generate_board_representation(jeu_temp, perspective));
    coup_actuel = coup_actuel->next;
  }

  strcat(historique_plateau, "\n\nLe gagnant est : ");
  strcat(historique_plateau, jeu->winner);
  strcat(historique_plateau, "\n\n");

  return historique_plateau;
}

void export_game_to_csv(Game *jeu, const char *nom_fichier)
{
  FILE *fichier = fopen(nom_fichier, "a");
  if (fichier == NULL)
  {
    printf("Erreur lors de l'ouverture du fichier.\n");
    return;
  }

  fprintf(fichier, "%d,%s,%s,%d,%d,%d,%s\n",
          jeu->game_id, jeu->player1, jeu->player2,
          jeu->score_player1, jeu->score_player2,
          jeu->rotation_sens, jeu->winner);

  Move *coup = jeu->moves->first;
  while (coup != NULL)
  {
    fprintf(fichier, "%s,%d\n", coup->player, coup->value);
    coup = coup->next;
  }

  fprintf(fichier, "end\n");
  fclose(fichier);
}

Game *import_game_from_csv(FILE *fichier)
{
  if (fichier == NULL)
  {
    printf("Erreur lors de l'ouverture du fichier.\n");
    return NULL;
  }

  Game *jeu = (Game *)malloc(sizeof(Game));
  char buffer[1024];

  fgets(buffer, sizeof(buffer), fichier);
  if (strstr(buffer, "end"))
  {
    fgets(buffer, sizeof(buffer), fichier);
  }

  if (!strcmp(buffer, "\n") || feof(fichier))
  {
    return NULL;
  }

  sscanf(buffer, "%d,%[^,],%[^,],%d,%d,%d,%s\n",
         &jeu->game_id, jeu->player1, jeu->player2,
         &jeu->score_player1, &jeu->score_player2,
         &jeu->rotation_sens, jeu->winner);

  jeu->moves = (Moves *)malloc(sizeof(Moves));
  jeu->moves->first = NULL;
  jeu->moves->last = NULL;
  jeu->moves->size = 0;

  char joueur[USERNAME_SIZE];
  int coup;

  while (strcmp(fgets(buffer, sizeof(buffer), fichier), "end"))
  {
    if (strstr(buffer, "end"))
    {
      break;
    }

    sscanf(buffer, "%[^,],%d\n", joueur, &coup);
    if (!strcmp(jeu->player1, joueur))
    {
      log_move(jeu, 1, coup);
    }
    else
    {
      log_move(jeu, 2, coup);
    }
  }

  return jeu;
}

void delete_game(Game *jeu)
{
  free(jeu->board);

  Move *coup_actuel = jeu->moves->first;
  while (coup_actuel != NULL)
  {
    Move *temp = coup_actuel;
    coup_actuel = coup_actuel->next;
    free(temp);
  }

  free(jeu->moves);
  free(jeu);
}
