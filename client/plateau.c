#include "stdlib.h"
#include "stdio.h"

#include "plateau.h"

#include "string.h"

/**
 * Initialise et affiche le plateau de jeu.
 * @param plateau le plateau de jeu
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param desc_plat description des murs du plateau de jeu, de la forme : (3,4,H)(4,5,G)....
 */
void init_plateau(plateau_t* plateau, fenetre_t *win_plateau, char* desc_plat){
    int i, j,k,l;
    int col_mur, ligne_mur;
    char pos_mur;

    wclear(win_plateau->interieur);

    /* Creation et affichage des cases */
    for(i=0; i < LARG_PLAT; i++){
        for(j=0; j < HAUT_PLAT; j++){
           init_case(win_plateau, &(plateau->cases[i][j]), i*(LARG_CASE+2),
                                                               j*(HAUT_CASE+1)); 
        }
    }
 
    /* Recuperation et affichage des murs */
    desc_plat = strchr(desc_plat,'(');
    while( desc_plat != NULL ){
        sscanf(desc_plat,"(%d,%d,%c)", &col_mur, &ligne_mur, &pos_mur);
        aff_mur(win_plateau, plateau, col_mur, ligne_mur, pos_mur);
        desc_plat = strchr(desc_plat,')');
        desc_plat = strchr(desc_plat,'(');
    }

    fenetre_maj(win_plateau);
}

/**
 * Initialise et affiche une case du plateau de jeu.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param c la case a initialiser
 * @param posx la position x de la case dans la fenetre du plateau
 * @param posx la position x de la case dans la fenetre du plateau
 */
void init_case(fenetre_t *win_plateau, case_t* c, int posx, int posy){
    int i, j;
       
    c->posx = posx;
    c->posy = posy;
    c->murH = 0;
    c->murB = 0;
    c->murG = 0;
    c->murD = 0;
    c->robot = 0;
    c->cible = 0;
    
    /* Affichage de la case */
    for(i=posx; i < posx+LARG_CASE; i++){
        for(j=posy; j < posy+HAUT_CASE; j++){
                fenetre_setpos(win_plateau,i,j);
                fenetre_affcar(win_plateau,' ',COL_CASE);
        }
    }
}

/**
 * Initialise et demande l'affichage d'un robot sur le plateau de jeu.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param plat le plateau de jeu
 * @param x la colonne de la case sur laquelle se trouvera le robot
 * @param y la ligne de la case sur laquelle se trouvera le robot
 * @param couleur la couleur d'affichage du robot (identifie le type du robot)
 */
void init_robot(fenetre_t *win_plateau, plateau_t* plat, int x, int y, int couleur){
    switch(couleur){
        case COL_RR:
            plat->robots[ROUGE].posx =  x;
            plat->robots[ROUGE].posy =  y;
            break;
        case COL_RB:
            plat->robots[BLEU].posx =  x;
            plat->robots[BLEU].posy =  y;
            break;
        case COL_RJ:
            plat->robots[JAUNE].posx =  x;
            plat->robots[JAUNE].posy =  y;
            break;
        case COL_RV:
            plat->robots[VERT].posx =  x;
            plat->robots[VERT].posy =  y;
            break;
    }       
    plat->cases[x][y].robot = 1;
    aff_robot(win_plateau, &(plat->cases[x][y]), couleur); 
}

/**
 * Affiche un robot sur la case specifiee.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param c la case sur laquelle le robot doit etre affiche
 * @param couleur la couleur d'affichage du robot (identifie le type du robot)
 */
void aff_robot(fenetre_t *win_plateau, case_t* c, int couleur){
    int i, j;
    
    for(i=c->posx; i < c->posx+LARG_CASE; i++){
         for(j=c->posy; j< c->posy+HAUT_CASE; j++){
            fenetre_setpos(win_plateau, i, j);  
            switch(couleur){
                case COL_RR:
                    fenetre_affcar(win_plateau, 'R', 4);
                    break;
                case COL_RB:
                    fenetre_affcar(win_plateau, 'B', 5);
                    break;
                case COL_RJ:
                    fenetre_affcar(win_plateau, 'J', 6);
                    break;
                case COL_RV:
                    fenetre_affcar(win_plateau, 'V', 7);
                    break;
            }
        }
    }
}

/**
 * Supprime un robot d'une case.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param c la case de laquelle on souhaite supprimer le robot
 */
void sup_robot(fenetre_t *win_plateau, case_t* c){
    int i, j;
    
    c->robot = 0;
    for(i=c->posx; i < c->posx+LARG_CASE; i++){
         for(j=c->posy; j< c->posy+HAUT_CASE; j++){
            fenetre_setpos(win_plateau, i, j);  
            fenetre_affcar(win_plateau,' ',COL_CASE);
        }   
    }
}

/**
 * Supprime tout les robots du plateau de jeu.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param plat le plateau de jeu
 */
void sup_robots(fenetre_t *win_plateau, plateau_t *plat){
    sup_robot(win_plateau, &(plat->cases[plat->robots[ROUGE].posx][plat->robots[ROUGE].posy]));
    sup_robot(win_plateau, &(plat->cases[plat->robots[BLEU].posx][plat->robots[BLEU].posy]));
    sup_robot(win_plateau, &(plat->cases[plat->robots[JAUNE].posx][plat->robots[JAUNE].posy]));
    sup_robot(win_plateau, &(plat->cases[plat->robots[VERT].posx][plat->robots[VERT].posy]));
}

/**
 * Initialise la cible du plateau de jeu.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param plat le plateau de jeu
 * @param x la colonne de la case sur laquelle se trouvera la cible
 * @param y la ligne de la case sur laquelle se trouvera la cible
 */
void init_cible(fenetre_t *win_plateau, plateau_t* plat, int x, int y){
    plat->cible.posx = x;
    plat->cible.posy = y;
    aff_cible(win_plateau, plat, x, y);
}

/**
 * Affiche la cible du plateau de jeu.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param plat le plateau de jeu
 * @param x la colonne de la case sur laquelle se trouvera la cible
 * @param y la ligne de la case sur laquelle se trouvera la cible
 */
void aff_cible(fenetre_t *win_plateau, plateau_t* plat, int x, int y){
    int i, j;
    case_t c = plat->cases[x][y]; 
    
    c.cible = 1;
    for(i=c.posx; i < c.posx+LARG_CASE; i++){
         for(j=c.posy; j< c.posy+HAUT_CASE; j++){
            fenetre_setpos(win_plateau, i, j);  
            fenetre_affcar(win_plateau,'C',COL_CIBLE);
        }   
    }
    fenetre_maj(win_plateau); 
}

/**
 * Supprime la cible du plateau de jeu.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param plat le plateau de jeu
 */
void sup_cible(fenetre_t *win_plateau, plateau_t* plat){
    int i, j;
    case_t c = plat->cases[plat->cible.posx][plat->cible.posy]; 
   
    for(i=c.posx; i < c.posx+LARG_CASE; i++){
         for(j=c.posy; j< c.posy+HAUT_CASE; j++){
            fenetre_setpos(win_plateau, i, j);  
            fenetre_affcar(win_plateau,' ',COL_CASE);
        }   
    }
    fenetre_maj(win_plateau); 
}

/**
 * Affiche la cible du plateau de jeu.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param plat le plateau de jeu
 * @param col_mur la colonne de la case sur laquelle se trouvera le mur
 * @param ligne_mur la ligne de la case sur laquelle se trouvera le mur
 * @param pos_mur la position du mur par pa rapport a la case (haut, bas, ...)
 */
void aff_mur(fenetre_t* win_plateau, plateau_t* plat, int col_mur, int ligne_mur, 
                                                                  char pos_mur){
    int i;
    case_t c;

    c = plat->cases[col_mur][ligne_mur];
    switch(pos_mur){
        case 'H':
            c.murH = 1;
            /* On affiche le mur s'il n'est pas sur le bord du plateau
               Et on le marque egalement sur la case voisine */
            if( ligne_mur != 0 ){ 
                plat->cases[col_mur][ligne_mur-1].murB = 1;
                for(i=0; i < LARG_CASE; i++){
                    fenetre_setpos(win_plateau, c.posx+i, c.posy-1); 
                    fenetre_affcar(win_plateau,' ',COL_MUR); 
                }
            }
            break;
        case 'B':
            c.murB = 1;
            if( ligne_mur != HAUT_PLAT-1 ){
                plat->cases[col_mur][ligne_mur+1].murH = 1;
                for(i=0; i < LARG_CASE; i++){
                    fenetre_setpos(win_plateau, c.posx+i, c.posy+HAUT_CASE);  
                    fenetre_affcar(win_plateau,' ',COL_MUR); 
                }
            }
            break;
        case 'D':
            c.murD = 1;
            if( col_mur != LARG_PLAT-1 ){
                plat->cases[col_mur+1][ligne_mur].murG = 1;
                for(i=0; i < HAUT_CASE; i++){
                    fenetre_setpos(win_plateau, c.posx+LARG_CASE, c.posy+i);  
                    fenetre_affcar(win_plateau,' ',COL_MUR); 
                    fenetre_setpos(win_plateau, c.posx+LARG_CASE+1, c.posy+i);
                    fenetre_affcar(win_plateau,' ',COL_MUR); 
                }
            }
            break;
        case 'G':
            c.murG = 1;
            if( col_mur != 0 ){
                plat->cases[col_mur-1][ligne_mur].murD = 1;
                for(i=0; i < HAUT_CASE; i++){
                    fenetre_setpos(win_plateau, c.posx-1, c.posy+i);  
                    fenetre_affcar(win_plateau,' ',COL_MUR); 
                    fenetre_setpos(win_plateau, c.posx-2, c.posy+i); 
                    fenetre_affcar(win_plateau,' ',COL_MUR); 
                }
            }
            break;
    }
}

/**
 * Anime une solution proposee pour resoudre une enigme.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param plat le plateau de jeu
 * @param solution la solution proposee
 */
void anim_solution(fenetre_t *win_plateau, plateau_t *plat, char* solution){
    char robot, dep;
    int i = 0, j;
    robot_t robotsInit [4];
    
    /* Sauvegarde des positions initiales des robots */
    
    for(j=0; j < NB_ROBOTS; j++){
        robotsInit[j].posx = plat->robots[j].posx;
        robotsInit[j].posy = plat->robots[j].posy;
    }
    
    solution = strchr(solution, '/');
    solution = &solution[1];

    /* Affichage des deplacements */
    while( solution[i] != '\n' ){
        robot = solution[i];
        dep = solution[i+1];
        switch(robot) {
            case 'R':
                aff_dep_robot(win_plateau, plat, ROUGE, dep, COL_RR);
                break;
            case 'B':
                aff_dep_robot(win_plateau, plat, BLEU, dep, COL_RB);
                break;
            case 'J':
                aff_dep_robot(win_plateau, plat, JAUNE, dep, COL_RJ);
                break;
            case 'V':
                aff_dep_robot(win_plateau, plat, VERT, dep, COL_RV);
                break;
        }
        usleep(300000);
        i = i+2;
    }

    /* Restauration des positions initiales des robots */
    sup_robots(win_plateau, plat);
    aff_robot(win_plateau, &(plat->cases[robotsInit[0].posx][robotsInit[0].posy]) ,COL_RR);
    aff_robot(win_plateau, &(plat->cases[robotsInit[1].posx][robotsInit[1].posy]) ,COL_RB);
    aff_robot(win_plateau, &(plat->cases[robotsInit[2].posx][robotsInit[2].posy]) ,COL_RJ);
    aff_robot(win_plateau, &(plat->cases[robotsInit[3].posx][robotsInit[3].posy]) ,COL_RV);

    fenetre_maj(win_plateau); 
}

/**
 * Affiche un deplacement d'un robot.
 * @param win_plateau la fenetre d'affichage du plateau de jeu
 * @param plat le plateau de jeu
 * @param robot le robot a deplacer
 * @param dep le deplacement a effectuer
 * @param couleur la couleir du rebot a deplacer
 */
void aff_dep_robot(fenetre_t *win_plateau, plateau_t *plat, int robot, char dep, int couleur){
    int x, y;
    
    /* Recuperation de la position du robot */
    x = plat->robots[robot].posx;
    y = plat->robots[robot].posy;

    switch(dep) {
        case 'H':
            y = y-1;
            while( y >= 0  &&  plat->cases[x][y].robot == 0  &&  plat->cases[x][y].murB == 0){
                sup_robot(win_plateau, &(plat->cases[x][y+1]));
                if(plat->cases[x][y+1].cible == 1)
                    aff_cible(win_plateau, plat, x, y);
                aff_robot(win_plateau, &(plat->cases[x][y]), couleur);
                plat->robots[robot].posy = y;
                fenetre_maj(win_plateau);
                usleep(150000);
                y--;
            }
            break;
        case 'B':
            y = y+1;
            while( y <= HAUT_PLAT  &&  plat->cases[x][y].robot == 0  &&  plat->cases[x][y].murH == 0){
                sup_robot(win_plateau, &(plat->cases[x][y-1]));
                if(plat->cases[x][y-1].cible == 1)
                    aff_cible(win_plateau, plat, x, y);
                aff_robot(win_plateau, &(plat->cases[x][y]), couleur);
                plat->robots[robot].posy = y;
                fenetre_maj(win_plateau); 
                usleep(150000);
                y++;
            }
            break;
        case 'D':
            x = x+1;
            while( x <= LARG_PLAT  &&  plat->cases[x][y].robot == 0  &&  plat->cases[x][y].murG == 0){
                sup_robot(win_plateau, &(plat->cases[x-1][y]));
                if(plat->cases[x-1][y].cible == 1)
                    aff_cible(win_plateau, plat, x, y);
                aff_robot(win_plateau, &(plat->cases[x][y]), couleur);
                plat->robots[robot].posx = x;
                fenetre_maj(win_plateau); 
                usleep(150000);
                x++;
            }
            break;
        case 'G':
            x = x-1;
            while( x >= 0  &&  plat->cases[x][y].robot == 0  &&  plat->cases[x][y].murD == 0){
                sup_robot(win_plateau, &(plat->cases[x+1][y]));
                if(plat->cases[x+1][y].cible == 1)
                    aff_cible(win_plateau, plat, x, y);
                aff_robot(win_plateau, &(plat->cases[x][y]), couleur);
                plat->robots[robot].posx = x;
                fenetre_maj(win_plateau);
                usleep(150000);
                x--;
            }
            break;
    } 
}











