#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "../options.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @struct Move
 * @brief Représente un mouvement dans le jeu.
 *
 * Cette structure contient les informations sur un mouvement effectué par un joueur.
 * 
 * @var Move::player
 * Nom du joueur qui a effectué le mouvement.
 * 
 * @var Move::value
 * Valeur associée au mouvement.
 * 
 * @var Move::next
 * Pointeur vers le mouvement suivant dans la liste.
 * 
 * @var Move::previous
 * Pointeur vers le mouvement précédent dans la liste.
 */
typedef struct Move {
  char player[100];
  int value;
  struct Move *next;
  struct Move *previous;
} Move;

/**
 * @struct Moves
 * @brief Représente une liste de mouvements.
 *
 * Cette structure contient une liste de mouvements effectués dans le jeu.
 * 
 * @var Moves::first
 * Pointeur vers le premier mouvement de la liste.
 * 
 * @var Moves::size
 * Taille de la liste de mouvements.
 * 
 * @var Moves::last
 * Pointeur vers le dernier mouvement de la liste.
 */
typedef struct Moves {
  Move *first;
  int size;
  Move *last;
} Moves;


/**
 * @struct Game
 * @brief Structure représentant une partie de jeu.
 * 
 * @var Game::board
 * Pointeur vers le tableau représentant le plateau de jeu.
 * 
 * @var Game::game_id
 * Identifiant unique de la partie.
 * 
 * @var Game::moves
 * Pointeur vers la structure Moves contenant les mouvements effectués dans la partie.
 * 
 * @var Game::next
 * Pointeur vers la partie suivante dans une liste chaînée de parties.
 * 
 * @var Game::previous
 * Pointeur vers la partie précédente dans une liste chaînée de parties.
 * 
 * @var Game::player1
 * Nom du joueur 1.
 * 
 * @var Game::player2
 * Nom du joueur 2.
 * 
 * @var Game::score_player1
 * Score du joueur 1.
 * 
 * @var Game::score_player2
 * Score du joueur 2.
 * 
 * @var Game::rotation_sens
 * Sens de rotation du jeu.
 * 
 * @var Game::winner
 * Nom du gagnant de la partie.
 */
typedef struct Game {
  int *board;
  int game_id;
  Moves *moves;
  struct Game *next;
  struct Game *previous;
  char player1[USERNAME_SIZE];
  char player2[USERNAME_SIZE];
  int score_player1;
  int score_player2;
  int rotation_sens;
  char winner[10];
} Game;

/**
 * Initialise une nouvelle partie avec les noms d'utilisateur fournis.
 * 
 * @param username1 Nom d'utilisateur du premier joueur.
 * @param username2 Nom d'utilisateur du deuxième joueur.
 * @param game_id Pointeur vers l'identifiant du jeu.
 * @return Pointeur vers la structure de jeu initialisée.
 */
Game * initialize_game(char *username1, char *username2, int *game_id);

/**
 * Crée une représentation textuelle du plateau de jeu pour le joueur spécifié.
 * 
 * @param g Pointeur vers la structure de jeu.
 * @param player Numéro du joueur (1 ou 2).
 * @return Chaîne de caractères représentant le plateau de jeu.
 */
char* generate_board_representation(Game *g, int player);

/**
 * Copie le contenu d'un plateau de jeu vers un autre.
 * 
 * @param board1 Pointeur vers le plateau de destination.
 * @param board2 Pointeur vers le plateau source.
 */
void copy_board(int *board1, int *board2);

/**
 * Vérifie si une case parcourue appartient au côté du joueur.
 * 
 * @param joueur Numéro du joueur (1 ou 2).
 * @param case_parcourue Numéro de la case parcourue.
 * @return true si la case appartient au côté du joueur, false sinon.
 */
bool is_in_player_side(int joueur, int case_parcourue);

/**
 * Effectue un mouvement dans le jeu.
 * 
 * @param g Pointeur vers la structure de jeu.
 * @param case_jeu Numéro de la case à jouer.
 * @param joueur Numéro du joueur (1 ou 2).
 * @return Résultat du mouvement (dépend de l'implémentation).
 */
int execute_move(Game *g, int case_jeu, int joueur);

/**
 * Affiche le score actuel des joueurs.
 * 
 * @param g Pointeur vers la structure de jeu.
 */
void display_score(Game * g);

/**
 * Vérifie si le joueur a des graines dans son camp.
 * 
 * @param g Pointeur vers la structure de jeu.
 * @param player Numéro du joueur (1 ou 2).
 * @return true si le joueur a des graines, false sinon.
 */
bool player_has_seeds(Game * g, int player);

/**
 * Calcule le nombre de trous à partir d'une case sélectionnée pour un joueur.
 * 
 * @param selected_pit Numéro de la case sélectionnée.
 * @param player Numéro du joueur (1 ou 2).
 * @param game Pointeur vers la structure de jeu.
 * @return Nombre de trous.
 */
int get_pits_count(int selected_pit, int player, Game *game);

/**
 * Teste si la partie est terminée pour un joueur.
 * 
 * @param g Pointeur vers la structure de jeu.
 * @param player Numéro du joueur (1 ou 2).
 * @return Résultat du test (dépend de l'implémentation).
 */
int check_game_end(Game *g, int player);

/**
 * Supprime une partie et libère la mémoire associée.
 * 
 * @param g Pointeur vers la structure de jeu.
 */
void delete_game(Game * g);

/**
 * Calcule la somme des graines restantes pour un joueur.
 * 
 * @param g Pointeur vers la structure de jeu.
 * @param player Numéro du joueur (1 ou 2).
 * @return Somme des graines restantes.
 */
int calculate_remaining_seeds(Game *g, int player);

/**
 * Ajoute un mouvement à l'historique des mouvements du jeu.
 * 
 * @param g Pointeur vers la structure de jeu.
 * @param player Numéro du joueur (1 ou 2).
 * @param selected_pit Numéro de la case sélectionnée.
 */
void log_move(Game *g, int player, int selected_pit);

/**
 * Rejoue la partie à partir de l'historique des mouvements.
 * 
 * @param g Pointeur vers la structure de jeu.
 * @return Chaîne de caractères représentant les mouvements rejoués.
 */
char* replay_game(Game *g);

/**
 * Vérifie si un joueur doit nourrir son adversaire.
 * 
 * @param jeu Pointeur vers la structure de jeu.
 * @param coups_possibles Tableau des coups possibles.
 * @param index_coups Pointeur vers l'index des coups possibles.
 * @param joueur Numéro du joueur (1 ou 2).
 * @return true si le joueur doit nourrir son adversaire, false sinon.
 */
bool enforce_feeding(Game *jeu, int *coups_possibles, int *index_coups, int joueur);


/**
 * Exporte l'état du jeu dans un fichier CSV.
 * 
 * @param game Pointeur vers la structure de jeu.
 * @param filename Nom du fichier CSV.
 */
void export_game_to_csv(Game *game, const char *filename);

/**
 * Importe l'état du jeu à partir d'un fichier CSV.
 * 
 * @param file Pointeur vers le fichier CSV.
 * @return Pointeur vers la structure de jeu importée.
 */
Game* import_game_from_csv(FILE *file);


#endif