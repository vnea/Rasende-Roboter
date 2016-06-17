#define _XOPEN_SOURCE 500 

#include <stdlib.h>    
#include <stdio.h>  
#include <sys/stat.h>    
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#include "client.h"

/* Variables globales */
fenetre_t win_info; /* La fenetre d'affichage des informations de la partie */
fenetre_t win_selection; /* La fenetre d'interaction avec le joueur */
fenetre_t win_enigme; /* La fenetre d'affichage de l'enigme */
fenetre_t win_plateau; /* La fenetre d'affichage du plateau de jeu */
plateau_t plateau; /* Le plateau de jeu. */
int socketfd; /* Le descripteur de la socket entre le joueur et le serveur */
char user [21]; /* Le nom du joueur */
int phase = ENATTENTE;
int actif = 0; /* 1 si le joueur est le joueur actif pour la phase de resolution */
int taille = 0; /* Nombre d'entiers deja rentres par l'utilisateur pour sa prochaine enchere
                  ou (selon phase) nombre de caracteres dans la solution du joueur */

pthread_mutex_t mutexAff = PTHREAD_MUTEX_INITIALIZER;  /* mutex pour l'affichage. */

/**
 * Verifie que le terminal est assez grand pour pouvoir joueur.
 */
void verif_dim_terminal(){
    if(ncurses_verifiedim(LARGEUR_TERM, HAUTEUR_TERM) == -1) {
	    ncurses_stopper();
	    fprintf(stderr, "\033[1m[ERREUR]\033[0m La taille du terminal n'est pas"
                            " assez grande (%d, %d).\n", COLS, LINES);
	    fprintf(stderr, "Veuillez l'aggrandir jusqu'a (%d, %d) et executez a "
                        "nouveau l'application.\n", LARGEUR_TERM, HAUTEUR_TERM);
	    exit(EXIT_FAILURE);
    }
}

/**
 * Initialise l'interface (ncurses) du jeu.
 */
void creation_interface(){
	/* Initialistaion de ncurses */
    ncurses_initialiser(); 
	ncurses_initcouleurs();
  	ncurses_initsouris(); 

    verif_dim_terminal();

	fenetre_affch(NULL, "Pressez F2 pour quitter \n");

    /* Creation des differentes fenetres */
	win_plateau = fenetre_creer(HAUT_PLAT*HAUT_CASE+HAUT_PLAT+1, 
	                               LARG_PLAT*LARG_CASE+LARG_PLAT*2+1,
	                                2,1,"Plateau",1); 
    win_info = fenetre_creer(20, 50, 5,
                             LARG_PLAT*LARG_CASE+LARG_PLAT*2+3,"Informations partie",0); 

    win_selection = fenetre_creer(10, 50, 25,
                             LARG_PLAT*LARG_CASE+LARG_PLAT*2+3,"Selection",1); 

    win_enigme = fenetre_creer(3, 50, 2,
                             LARG_PLAT*LARG_CASE+LARG_PLAT*2+3,"enigme",1);
	 
}

/**
 * Initialise et confirme la connexion avec le serveur.
 */
void init_connexion(){
    struct sockaddr_in adresseServeur;
    char messSend[50] = {'\0'};
    char messRecv[50] = {'\0'};

    /* Creation du socket */ 
    memset(&adresseServeur, 0, sizeof(adresseServeur));
    adresseServeur.sin_family = AF_INET;
    adresseServeur.sin_port = htons(2016);
    inet_pton(AF_INET, "127.0.0.1", &adresseServeur.sin_addr);
    if( (socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1 ){
		perror("Erreur connection au serveur : ");
        exit(EXIT_FAILURE);
    }
    connect(socketfd, (struct sockaddr*)&adresseServeur, sizeof(adresseServeur));

    /* Envoie du nom d'utilisateur au serveur */
    strcat(messSend, "CONNEXION/");
    strcat(messSend, user);
    strcat(messSend, "/\n");
    if(write(socketfd, &messSend, sizeof(char)*strlen(messSend)) == -1) {
		perror("Erreur envoie du nom d'utilisateur : ");
		exit(EXIT_FAILURE);
	}
    
    /* Reception confirmation de connexion */
    if(recv(socketfd, &messRecv, sizeof(char)*strlen(messSend), 0) == -1) {
		perror("Erreur confirmation connexion : ");
		exit(EXIT_FAILURE);
	}

	fenetre_affch(&win_info, "%s\n",messRecv);
	fenetre_affch(&win_info, "Attente du prochain tour...\n\n");
	fenetre_maj(&win_info);
}

/**
 * Informe le serveur de la deconnexion du client.
 */
void stop_connexion(){
    char messSend[30] = {'\0'};

    strcat(messSend, "SORT/");
    strcat(messSend, user);
    strcat(messSend, "/\n");
    if(send(socketfd, &messSend, sizeof(char)*strlen(messSend), 0) == -1) {
		perror("Erreur envoie deconnexion : ");
		exit(EXIT_FAILURE);
	}
}

/**
 * Lance la phase de resolution.
 * @param tmp contient les informations pour la phase (joueur actif, nb de coups)
 */
void init_phase_resolution(char* tmp){
    char joueur [21];
    int nbCoup;

    phase = RESOLUTION;
    curs_set(0);
    fenetre_affch(&win_info, "\n*********** RESOLUTION ***********\n");
    
    /* Affichage du joueur actif */
    sscanf(tmp,"%[^/]", joueur); 
    tmp = strchr(tmp, '/');
    sscanf(tmp,"/%d", &nbCoup); 
    fenetre_affch(&win_info, "joueur actif : %s en %d coups\n",joueur, nbCoup);
    
    /* Activation de la fenetre de selection si l'on est le joueur actif */
    if( strcmp(user,joueur) == 0 ){
        actif = 1;
        taille = 0;
        active_resolution();
    }
    else{
        actif = 0;
        wclear(win_selection.interieur);
        fenetre_maj(&win_selection);
    }
    
    fenetre_maj(&win_info);
}


/**
 * Relance une nouvelle phase de resolution.
 * @param tmp contient les informations pour la phase (joueur actif, nb de coups)
 */
void new_phase_resolution(char* tmp){
    char joueur [21];

    fenetre_affch(&win_info, "\nNouvelle phase de resolution :\n");
    sscanf(tmp,"%[^/]", joueur); 
    fenetre_affch(&win_info, "Joueur actif : %s\n", joueur);

    /* Activation de la fenetre de selection si l'on est le joueur actif */
    if( strcmp(user,joueur) == 0 ){
        actif = 1;
        taille = 0;
        active_resolution();
    }
    else{
        actif = 0;
        wclear(win_selection.interieur);
        fenetre_maj(&win_selection);
    }
    
    fenetre_maj(&win_info);
}

/**
 * Affiche la fenetre d'interaction pour la resolution
 */
void active_resolution(){
    fenetre_affch_pos(&win_selection, 0, 0,"Saisissez votre solution en choississant\n"
                                  " un robot puis sur un deplacement, etc...\n");

    fenetre_setpos(&win_selection, 0, 2);
    fenetre_affcar(&win_selection, 'R', COL_RR);
    fenetre_setpos(&win_selection, 2, 2);
    fenetre_affcar(&win_selection, 'B', COL_RB);
    fenetre_setpos(&win_selection, 4, 2);
    fenetre_affcar(&win_selection, 'J', COL_RJ);
    fenetre_setpos(&win_selection, 6, 2);
    fenetre_affcar(&win_selection, 'V', COL_RV);

    fenetre_setpos(&win_selection, 10, 2);
    fenetre_affcar(&win_selection, 'H', 9);
    fenetre_setpos(&win_selection, 12, 2);
    fenetre_affcar(&win_selection, 'B', 9);
    fenetre_setpos(&win_selection, 14, 2);
    fenetre_affcar(&win_selection, 'D', 9);
    fenetre_setpos(&win_selection, 16, 2);
    fenetre_affcar(&win_selection, 'G', 9);

    fenetre_setpos(&win_selection, 22, 2);
    fenetre_affch(&win_selection, "Correction");
    fenetre_setpos(&win_selection, 36, 2);
    fenetre_affch(&win_selection, "Envoyer");

    fenetre_maj(&win_selection);
}

/**
 * Pour ajouter un nouveau caractere/element a une solution.
 * @param solution la solution en cours de redaction
 * @param car le caractere a ajouter a la solution
 */
void add_car_solution(char* solution, char car){
    fenetre_setpos(&win_selection, taille%48, 4+(taille/48)); 
    solution[taille] = car;
    taille++;
    fenetre_affcar(&win_selection, car, 9);
    fenetre_maj(&win_selection);
}

/**
 * Envoie la solution au serveur.
 * @param solution la solution a envoyer
 */
void envoie_solution(char* solution){
    int i;    
    char messSend[50] = {'\0'};
    
    /* Envoie de la solution */
    strcat(messSend, "SOLUTION/");
    strcat(messSend, user);
    strcat(messSend, "/");
    strcat(messSend, solution);
    strcat(messSend, "/\n");
    if(write(socketfd, &messSend, sizeof(char)*strlen(messSend)) == -1) {
	    perror("Erreur envoie enchere : ");
	    exit(EXIT_FAILURE);
    }

    /* Affichage de la solution envoyee */
    fenetre_affch(&win_info, "%s envoie : %s\n", user, solution);  
    
    /* Reinitialise l'affichage de la solution */
    for(i=0; i < taille; i++){
        solution[i] = '\0';
        fenetre_setpos(&win_selection, i%48, 4+(i/48));
        fenetre_affch(&win_selection, " "); 
    }
    taille = 0;  

    fenetre_maj(&win_info);
    fenetre_maj(&win_selection);
}

/**
 * Lance la phase d'enchere
 */
void init_phase_enchere(){
    phase = ENCHERE;
    aff_mess_info("\n*********** ENCHERE ***********\n");

}

/**
 * Envoie une enchere au serveur.
 * @param enchere l'enchere a envoyer
 */
void envoie_enchere(char* enchere){
    char messSend[50] = {'\0'};

    /* Envoie de l'enchere au serveur */
    if( phase == REFLEXION ){
        strcat(messSend, "SOLUTION/");
        strcat(messSend, user);
        strcat(messSend, "/");
        strcat(messSend, enchere);
        strcat(messSend, "/\n");
        
    }
    else if( phase == ENCHERE){
        strcat(messSend, "ENCHERE/");
        strcat(messSend, user);
        strcat(messSend, "/");
        strcat(messSend, enchere);
        strcat(messSend, "/\n");
    }
    if(write(socketfd, &messSend, sizeof(char)*strlen(messSend)) == -1) {
	    perror("Erreur envoie enchere : ");
	    exit(EXIT_FAILURE);
    }

    /* Reinitialise l'affichage de l'enchere */
    taille = 0;
    enchere[0] = '\0';
    enchere[1] = '\0';
    fenetre_setpos(&win_selection, 0, 2);
    fenetre_affch(&win_selection, " ");
    fenetre_setpos(&win_selection, 1, 2);
    fenetre_affch(&win_selection, " ");
    fenetre_setpos(&win_selection, 0, 2);
    fenetre_maj(&win_selection); 
}

/**
 * Lance la phase de reflexion.
 * @param messRecv contient les informations pour la phase (bilan et enigme)
 */
void init_phase_reflexion(char* messRecv){
    char *bilan;
    int xr,yr, xb,yb, xj,yj, xv,yv, xc,yc;
    char c;      

    /* Recuperation et affichage de l'enigme */  
    sscanf(messRecv,"(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%c)",
            &xr,&yr, &xb,&yb, &xj,&yj, &xv,&yv, &xc,&yc,&c);
    init_robot(&win_plateau, &plateau, xr, yr, COL_RR);
    init_robot(&win_plateau, &plateau, xb, yb, COL_RB);
    init_robot(&win_plateau, &plateau, xj, yj, COL_RJ);
    init_robot(&win_plateau, &plateau, xv, yv, COL_RV);
    init_cible(&win_plateau, &plateau, xc, yc);
    fenetre_affch_pos(&win_enigme, 0, 0, "%c dans cible.\n", c);
    fenetre_maj(&win_enigme);
    
    /* Affichage du bilan */
    bilan = strchr(messRecv,'/');
    affiche_bilan(bilan);

    /* Affichage des informations pour le nouveau tour */
    taille = 0;
    fenetre_affch(&win_info, "*********** REFLEXION ***********\n");
    fenetre_maj(&win_info);
    fenetre_setpos(&win_selection, 0, 0);
    fenetre_affch(&win_selection, "Entrer la valeur de votre enchere :\n");
    fenetre_affch(&win_selection, "(appuyer sur entrer pour valider)\n");
    fenetre_setpos(&win_selection, 0, 2);
    curs_set(1);
    fenetre_maj(&win_selection);  
    phase = REFLEXION;  
}

/**
 * Affiche le bilan.
 * @param bilan le bilan (tour, nom des joueurs et scores)
 */
void affiche_bilan(char* bilan){
    char joueur [21];
    int tour, score;

    fenetre_affch(&win_info, "*********** SCORES ***********\n");
    /* Affichage du tour */
    bilan = strchr(bilan,'/');
    sscanf(bilan,"/%d", &tour);
    fenetre_affch(&win_info, "Tour : %d\n", tour);

    /* Affichage du score de chaque joueur */
    bilan = strchr(bilan,'(');
    while( bilan != NULL ){
        bilan = &bilan[1];
        sscanf(bilan,"%[^,]", joueur);
        bilan = strchr(bilan, ',');
        sscanf(bilan,",%d", &score);
        fenetre_affch(&win_info, "Score %s : %d\n", joueur, score);
        bilan = strchr(bilan,')');
        bilan = strchr(bilan,'(');
    }
    fenetre_affch(&win_info, "\n");
    fenetre_maj(&win_info);
}

/**
 * Affiche un message dans la fenetre d'info et replace le curseur dans la fenetre de selection.
 * @param message le message a afficher
 */
void aff_mess_info(char* message){
    fenetre_affch(&win_info, message);
    fenetre_maj(&win_info); 
    fenetre_setpos(&win_selection, taille, 2);
    fenetre_maj(&win_selection);
}

/**
 * Routine de thread qui lit et traite les messages du serveur.
 * @param args les arguments passes au thread lors de sa creation
 */
void *routine_lecteur(void *args){
    char c = '\0'; 
    char messRecv[3000];
    char* tmp; /* Informations de la commande */
    int cpt = 0;

    while(1){
        /* Reception de la commande du serveur */
        while(c != '\n'){
            if( recv(socketfd, &c, sizeof(char), 0) == -1) {
	            perror("Erreur read routine_lecteur : ");
	            exit(EXIT_FAILURE);
            } 
            messRecv[cpt] = c;
            cpt++;
        }
        messRecv[cpt] = '\0';
        cpt = 0;
        c = '\0';
       
        /* Traitement de la commande */
        tmp = strchr(messRecv,'/');
        tmp = &tmp[1];
        pthread_mutex_lock(&mutexAff);
        /* Connexion / Deconnexion */
        if( strncmp("CONNECTE", messRecv, 8) == 0 ){
            if( phase == ENCHERE  ||  phase == REFLEXION )
                aff_mess_info(messRecv);
            else{
                fenetre_affch(&win_info, messRecv);
                fenetre_maj(&win_info);
            }
        }
	    else if( strncmp("DECONNEXION", messRecv, 11) == 0 )
	        if( phase == ENCHERE  ||  phase == REFLEXION )
                aff_mess_info(messRecv);
            else{
                fenetre_affch(&win_info, messRecv);
                fenetre_maj(&win_info);
            }
        /* Debut d'une session */
        else if( strncmp("SESSION", messRecv, 7) == 0 ){
            fenetre_affch(&win_info, "*********** NEW SESSION ***********\n");
	        init_plateau(&plateau, &win_plateau, tmp);
            fenetre_maj(&win_info);
        }
        else if( strncmp("VAINQUEUR", messRecv, 9) == 0 ){
            fenetre_affch(&win_info, "\n*********** FIN SESSION ***********\n");
            affiche_bilan(messRecv);
            phase = ENATTENTE;
            fenetre_affch(&win_info, "Attente du prochain tour...\n\n");
            fenetre_maj(&win_info);
        }
        /* Phase de reflexion */
        else if( strncmp("TOUR", messRecv, 4) == 0 ){
            init_phase_reflexion(tmp);
        }
        else if( strncmp("SOLUTION", messRecv, 8) == 0 )
            aff_mess_info(messRecv);
        else if( strncmp("TUASTROUVE", messRecv, 10) == 0 ){
            aff_mess_info(messRecv);
            init_phase_enchere();
        }
        else if( strncmp("ILATROUVE", messRecv, 9) == 0 ){
            aff_mess_info(messRecv);
            init_phase_enchere();
        }
        else if( strncmp("FINREFLEXION", messRecv, 12) == 0 ){
            aff_mess_info("Fin delai imparti\n");
            init_phase_enchere();
        }
        /* Phase d'enchere  */
        else if( strncmp("VALIDATION", messRecv, 10) == 0 )
            aff_mess_info(messRecv);
        else if( strncmp("ECHEC", messRecv, 5) == 0 )
            aff_mess_info(messRecv);
        else if( strncmp("NOUVELLEENCHERE", messRecv, 15) == 0 )
            aff_mess_info(messRecv);
        else if( strncmp("FINENCHERE", messRecv, 10) == 0 ){
            init_phase_resolution(tmp);
        }
        /* Phase de resolution */
        else if( strncmp("SASOLUTION", messRecv, 10) == 0 ){
             fenetre_affch(&win_info, messRecv);
             fenetre_maj(&win_info);
            /****** ANIMATION ? ***********/
            //anim_solution(&win_plateau, &plateau, tmp);
        }
        else if( strncmp("BONNE", messRecv, 5) == 0 ){
            phase = ENATTENTE;
            wclear(win_selection.interieur);
            fenetre_maj(&win_selection);
            fenetre_affch(&win_info, "Solution bonne !!\n");
            fenetre_affch(&win_info, "\n*********** FIN TOUR ***********\n");
            fenetre_maj(&win_info);
            sup_robots(&win_plateau, &plateau);
            sup_cible(&win_plateau, &plateau);
            fenetre_maj(&win_plateau);
        }
        else if( strncmp("MAUVAISE", messRecv, 8) == 0 ){
            fenetre_affch(&win_info, "Solution fausse !!\n");
            new_phase_resolution(tmp);
        }
        else if( strncmp("FINRESO", messRecv, 7) == 0 ){
            phase = ENATTENTE;
            wclear(win_selection.interieur);
            fenetre_maj(&win_selection);
            fenetre_affch(&win_info, "\n*********** FIN TOUR ***********\n");
            fenetre_maj(&win_info);
            sup_robots(&win_plateau, &plateau);
            sup_cible(&win_plateau, &plateau);
            fenetre_maj(&win_plateau);
        }
        else if( strncmp("TROPLONG", messRecv, 8) == 0 ){
            fenetre_affch(&win_info, "Trop long !!\n");
            new_phase_resolution(tmp);
        }
        pthread_mutex_unlock(&mutexAff);
    }
    
    pthread_exit(NULL);
}

/**
 * Routine principal du programme.
 */
int main(int argc, char* argv){
    char c;
    int car; /* Caractere rentre au clavier/souris par le joueur */
    int sourisx, sourisy, x, y;
    char enchere [3] = {'\0'}; /* 99 coups max */
    char solution [100] = {'\0'}; /* 99 coups max */
    char messSend [50] = {'\0'}; /* Message a envoyer au serveur */
    pthread_t lecteur; /* Lit les messages envoyes par le serveur */

	printf("Veuillez entrer votre nom d'utilisateur (tronqué à 20 caracteres): \n");
	while(scanf("%20s", user) == EOF) {
		printf("nom d'utilisateur incorrect, recommencez : \n");
		while ((c = getchar ()) != '\n' && c != EOF);
	}
    
    /* Initialisation de l'interface */
	creation_interface();

    /* Initialisation connexion avec le serveur */
    init_connexion();
    
    /* Lancement du thread lecteur des requetes du serveur */
    if( pthread_create(&lecteur, NULL, routine_lecteur, NULL) != 0 ){
        perror("Erreur creation du thread lecteur : ");
        exit(EXIT_FAILURE);
    }
 
    while( (car = getch()) != KEY_F(2) ) {
        /* Si on est dans la phase de reflexion ou d'enchere
           alors on permet de taper un nombre a 2 chiffres dans la fenetre
           de selection. Lorsque l'on recoit "entrer", on envoie l'enchere
           au serveur.
        */
        pthread_mutex_lock(&mutexAff);
        if( phase == REFLEXION  ||  phase == ENCHERE){
            switch(car) {
                case '0':
                    if( taille != 0 ){
                        enchere[taille] = car;
                        taille++;
                        fenetre_affcar(&win_selection, car, 9);
                        fenetre_maj(&win_selection);
                    }
                    break;
                case '1' ... '9':
                    if( taille < 2 ){
                        enchere[taille] = car;
                        taille++;
                        fenetre_affcar(&win_selection, car, 9);
                        fenetre_maj(&win_selection);
                    }
                    break;             
                case KEY_BACKSPACE:
                    if(taille > 0){
                        taille--;
                        enchere[taille] = '\0';
                        fenetre_setpos(&win_selection, taille, 2);
                        fenetre_affch(&win_selection, " ");
                        fenetre_setpos(&win_selection, taille, 2);
                        fenetre_maj(&win_selection); 
                    }
                    break;
                case 10: /* Pour entrer */
                    if(taille > 0){
                        envoie_enchere(enchere);
                    } 
                    break;   
            }          
        }
        /* Executer le programme pour bien voir le mecanisme */
        else if( phase == RESOLUTION  &&  actif == 1 ){
            if( car == KEY_MOUSE ){
                if(souris_getpos(&sourisx, &sourisy) == OK) {
                    if(fenetre_getcoordonnees(&win_selection, sourisx, sourisy, 
                                                                  &x, &y) == 1){
                        if( y == 2 ){
                            switch(x) {
                                case 0:
                                    if( taille < 198  &&  taille%2 == 0 ){ // 99 coups
                                        add_car_solution(solution, 'R');
                                    }
                                    break;
                                case 2:
                                    if( taille < 198  &&  taille%2 == 0 ){ 
                                        add_car_solution(solution, 'B');
                                    }
                                    break;
                                case 4:
                                    if( taille < 198  &&  taille%2 == 0 ){ 
                                        add_car_solution(solution, 'J');
                                    }
                                    break;
                                case 6:
                                    if( taille < 198  &&  taille%2 == 0 ){ 
                                        add_car_solution(solution, 'V');
                                    }
                                    break;
                                case 10:
                                    if( taille < 198  &&  taille%2 == 1 ){ 
                                        add_car_solution(solution, 'H');
                                    }
                                    break;
                                case 12:
                                    if( taille < 198  &&  taille%2 == 1 ){ 
                                        add_car_solution(solution, 'B');
                                    }
                                    break;
                                case 14:
                                    if( taille < 198  &&  taille%2 == 1 ){ 
                                        add_car_solution(solution, 'D');
                                    }
                                    break;
                                case 16:
                                    if( taille < 198  &&  taille%2 == 1 ){ 
                                        add_car_solution(solution, 'G');
                                    }
                                    break;
                                case 22 ... 31:
                                    if(taille > 0){
                                        taille--;
                                        enchere[taille] = '\0';
                                        fenetre_setpos(&win_selection, taille%48, 4+(taille/48));
                                        fenetre_affch(&win_selection, " ");
                                        fenetre_maj(&win_selection); 
                                    }
                                    break;
                                case 36 ... 42:
                                    if( taille%2 == 0 ){
                                        envoie_solution(solution);
                                        actif = 0;
                                        /****** ANIMATION ? ***********/
                                    } 
                                    break;
                            }
                        }
                    }
                }
            }
        }
        pthread_mutex_unlock(&mutexAff);
    }

    /* Signalement de fin de connexion au serveur */
    stop_connexion();
    
    /* Destruction de l'interface */
    fenetre_detruire(&win_plateau);
    fenetre_detruire(&win_info);
    fenetre_detruire(&win_enigme);
    fenetre_detruire(&win_selection);
	ncurses_stopper();

    /* Destruction du thread lecteur */
	pthread_cancel(lecteur);
	if( pthread_join(lecteur, NULL) != 0 ){
		fprintf(stderr, "Probleme destruction du thread lecteur\n");	
		exit(EXIT_FAILURE);	
	}

    /* Fermeture de la connexion */
    close(socketfd);

	return EXIT_SUCCESS;
}

   
