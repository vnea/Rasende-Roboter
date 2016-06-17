#ifndef _PLATEAU_
#define _PLATEAU_

#include "interface.h"

#define HAUT_PLAT 16  /* Nombre de lignes de cases. */
#define LARG_PLAT 16  /* Nombre de colonnes de cases. */
#define HAUT_CASE 1   /* Hauteur d'une case en pixel ncurses. */
#define LARG_CASE 2   /* Largeur d'une case en pixel ncurses. */

#define NB_ROBOTS 4
/* Definition de l'indice de chaque robot dans le tableau de robots du plateau */
#define ROUGE 0
#define BLEU  1
#define JAUNE 2
#define VERT  3

/* Definition des couleurs des differents elements du plateau de jeu */
#define COL_MUR  	1
#define COL_CASE 	3
#define COL_RR	    4
#define COL_RB	    5
#define COL_RJ	    6
#define COL_RV	    7
#define COL_CIBLE	8

/**
 * Structure representant une case du plateau de jeu.
 */
typedef struct {
	int posx; /* Position x de la case dans la fenetre du plateau. */
    int posy;
    int murH; /* 1 si un mur est present sur le haut de la case, 0 sinon. */
    int murB;
    int murG;
    int murD;
    int robot; /* 1 si un robot est présent sur la case, 0 sinon. */
    int cible; /* 1 si la case est la cible, 0 sinon. */
}case_t;

/**
 * Structure representant un robot.
 */
typedef struct {
    int posx; /* colonne de la case sur laquelle se trouvera le robot (entre 0 et LARG_PLAT). */
    int posy; /* ligne de la case sur laquelle se trouvera le robot (entre 0 et HAUT_PLAT). */
}robot_t;

/**
 * Structure representant un robot.
 */
typedef struct {
    int posx; /* colonne de la case sur laquelle se trouvera la cible (entre 0 et LARG_PLAT). */
    int posy; /* ligne de la case sur laquelle se trouvera la cible (entre 0 et HAUT_PLAT). */
}cible_t;



/**
 * Structure representant le plateau de jeu.
 */
typedef struct {
	case_t cases [HAUT_PLAT][LARG_PLAT]; 
    robot_t robots [4];
	cible_t cible;
}plateau_t;

/**
 * Initialise et affiche le plateau de jeu.
 * @param plateau le plateau de jeu
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param desc_plat description des murs du plateau de jeu, de la forme : (3,4,H)(4,5,G)....
 */
void init_plateau(plateau_t* plat, fenetre_t *win_plateau, char* desc_plat);

/**
 * Initialise et affiche une case du plateau de jeu.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param c la case a initialiser
 * @param posx la position x de la case dans la fenetre du plateau
 * @param posy la position y de la case dans la fenetre du plateau
 */
void init_case(fenetre_t *win_plateau, case_t* c, int posx, int posy);

/**
 * Initialise et demande l'affichage d'un robot sur le plateau de jeu.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param plat le plateau de jeu
 * @param x la colonne de la case sur laquelle se trouvera le robot
 * @param y la ligne de la case sur laquelle se trouvera le robot
 * @param couleur la couleur d'affichage du robot (identifie le type du robot)
 */
void init_robot(fenetre_t *win_plateau, plateau_t* plat, int x, int y, int couleur);

/**
 * Affiche un robot sur la case specifiee.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param c la case sur laquelle le robot doit etre affiche
 * @param couleur la couleur d'affichage du robot (identifie le type du robot)
 */
void aff_robot(fenetre_t *win_plateau, case_t* c, int couleur);

/**
 * Supprime un robot d'une case.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param c la case de laquelle on souhaite supprimer le robot
 */
void sup_robot(fenetre_t *win_plateau, case_t* c);

/**
 * Supprime tout les robots du plateau de jeu.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param plat le plateau de jeu
 */
void sup_robots(fenetre_t *win_plateau, plateau_t *plat);

/**
 * Initialise la cible du plateau de jeu.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param plat le plateau de jeu
 * @param x la colonne de la case sur laquelle se trouvera la cible
 * @param y la ligne de la case sur laquelle se trouvera la cible
 */
void init_cible(fenetre_t *win_plateau, plateau_t* plat, int x, int y);

/**
 * Affiche la cible du plateau de jeu.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param plat le plateau de jeu
 * @param x la colonne de la case sur laquelle se trouvera la cible
 * @param y la ligne de la case sur laquelle se trouvera la cible
 */
void aff_cible(fenetre_t *win_plateau, plateau_t* plat, int x, int y);

/**
 * Supprime la cible du plateau de jeu.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param plat le plateau de jeu
 */
void sup_cible(fenetre_t *win_plateau, plateau_t* plat);

/**
 * Affiche la cible du plateau de jeu.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param plat le plateau de jeu
 * @param col_mur la colonne de la case sur laquelle se trouvera le mur
 * @param ligne_mur la ligne de la case sur laquelle se trouvera le mur
 * @param pos_mur la position du mur par pa rapport a la case (haut, bas, ...)
 */
void aff_mur(fenetre_t* win_plateau, plateau_t* plat, int col_mur, int ligne_mur, 
                                                                  char pos_mur);
                                                                 
/**
 * Anime une solution proposee pour resoudre une enigme.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param plat le plateau de jeu
 * @param solution la solution proposee
 */
void anim_solution(fenetre_t *win_plateau, plateau_t *plat, char* solution);

/**
 * Affiche un deplacement d'un robot.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param plat le plateau de jeu
 * @param robot le robot a deplacer
 * @param dep le deplacement a effectuer
 * @param couleur la couleir du rebot a deplacer
 */
void aff_dep_robot(fenetre_t *win_plateau, plateau_t *plat, int robot, char dep, int couleur);

#endif
