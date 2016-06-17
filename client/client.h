#include "plateau.h"

/* Phase dans laquelle se trouve la partie */
#define ENATTENTE  0
#define REFLEXION  1
#define ENCHERE    2
#define RESOLUTION 3

/* Dimension minimales du terminal pour pouvoir afficher le jeu */
#define HAUTEUR_TERM 35
#define LARGEUR_TERM 118

/**
 * Verifie que le terminal est assez grand pour pouvoir joueur.
 */
void verif_dim_terminal();

/**
 * Initialise l'interface (ncurses) du jeu.
 */
void creation_interface();

/**
 * Initialise et confirme la connexion avec le serveur.
 */
void init_connexion();

/**
 * Informe le serveur de la deconnexion du client.
 */
void stop_connexion();

/**
 * Lance la phase de resolution.
 * @param tmp contient les informations pour la phase (joueur actif, nb de coups)
 */
void init_phase_resolution(char* tmp);

/**
 * Relance une nouvelle phase de resolution.
 * @param tmp contient les informations pour la phase (joueur actif, nb de coups)
 */
void new_phase_resolution(char* tmp);

/**
 * Affiche et active la fenetre d'interaction pour la resolution
 */
void active_resolution();

/**
 * Pour ajouter un nouveau caractere/element a une solution.
 * @param solution la solution en cours de redaction
 * @param car le caractere a ajouter a la solution
 */
void add_car_solution(char* solution, char car);

/**
 * Envoie la solution au serveur.
 * @param solution la solution a envoyer
 */
void envoie_solution(char* solution);

/**
 * Lance la phase d'enchere
 */
void init_phase_enchere();

/**
 * Envoie une enchere au serveur.
 * @param enchere l'enchere a envoyer
 */
void envoie_enchere(char* enchere);

/**
 * Lance la phase de reflexion.
 * @param messRecv contient les informations pour la phase (bilan et enigme)
 */
void init_phase_reflexion(char* messRecv);

/**
 * Affiche le bilan.
 * @param bilan le bilan (tour, nom des joueurs et scores)
 */
void affiche_bilan(char* bilan);

/**
 * Affiche un message dans la fenetre d'info et replace le curseur dans la fenetre de selection.
 * @param message le message a afficher
 */
void aff_mess_info(char* message);

/**
 * Routine de thread qui lit et traite les messages du serveur.
 * @param args les arguments passes au thread lors de sa creation
 */
void *routine_lecteur(void *arg);





