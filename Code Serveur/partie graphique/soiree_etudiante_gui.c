#include <stdio.h>      // snprintf : fabriquer le texte du titre de la fenêtre
#include <stdlib.h>     // rand, srand : nombres au hasard
#include <time.h>       // time : heure actuelle, pour changer le hasard à chaque partie
#include <SDL2/SDL.h>   // SDL2 : fenêtre, dessin, souris et clavier
#ifdef __EMSCRIPTEN__          // seulement quand on compile pour le navigateur
#include <emscripten.h>        // emscripten_set_main_loop : boucle de jeu adaptée au navigateur
#endif
 
/* ---------- Constantes ---------- */
#define L 15                   // nombre de lignes de la salle
#define C 30                   // nombre de colonnes de la salle
#define MILIEU (C / 2)         // colonne qui sépare les deux camps (parenthèses pour que le calcul reste juste)
#define HEURES 15              // durée de la soirée (nombre de tours de jeu)
#define DENSITE_DEPART 25      // % de chances d'avoir un fêtard sur une case au départ
 
#define VIDE 0                 // code d'une case vide
#define JOUEUR_1 1             // code d'un fêtard du joueur 1
#define JOUEUR_2 2             // code d'un fêtard du joueur 2
 
#define NAISSANCE 3            // règle de Conway : 3 voisins exactement = un fêtard naît
#define SURVIE_MIN 2           // règle de Conway : on survit avec au moins 2 voisins...
#define SURVIE_MAX 3           // ... et au plus 3 voisins
 
#define RAYON_POLICE 2         // la police vide 2 cases autour du centre = zone de 5x5
#define TAILLE_GROUPE 3        // le groupe de danseurs tient dans un carré de 3x3
 
#define EN_COURS -1            // résultat : la partie n'est pas finie (-1 car 0, 1 et 2 sont déjà pris)
#define EGALITE 0              // résultat : égalité
 
/* ---------- Constantes de l'affichage ---------- */
#define CASE 32                        // taille d'une case en pixels
#define HAUT_BARRE 60                  // hauteur de la barre d'infos en haut de la fenêtre
#define LARGEUR (C * CASE)             // largeur de la fenêtre
#define HAUTEUR (HAUT_BARRE + L * CASE)   // hauteur de la fenêtre
 
#define MODE_INVITER 1                 // action choisie : inviter un ami
#define MODE_RENVOYER 2                // action choisie : faire partir un fêtard adverse
#define MODE_POLICE 3                  // action choisie : appeler la police
#define MODE_MACARENA 4                // action choisie : lancer la Macarena
 
#define BOUTON_X (LARGEUR - 4 * 60)    // position du premier bouton (à droite de la barre)
#define BOUTON_PAS 60                  // écart entre deux boutons
#define BOUTON_L 50                    // largeur d'un bouton
 
/* Planeur du joueur 1 (descend vers la droite).
   Le joueur 2 le lit en miroir (descend vers la gauche). */
const int DANSEURS[TAILLE_GROUPE][TAILLE_GROUPE] = {   // const : le motif ne change jamais
    {0, 1, 0},         // ligne 1 du motif : un danseur au milieu
    {0, 0, 1},         // ligne 2 du motif : un danseur à droite
    {1, 1, 1}          // ligne 3 du motif : trois danseurs
};
 
/* Couleurs : ligne 0 = case vide, ligne 1 = joueur 1 (bleu), ligne 2 = joueur 2 (rouge) */
const int COULEURS[3][3] = {
    {50, 50, 65},
    {70, 150, 255},
    {255, 90, 90}
};
 
/* Chiffres de 0 à 9 dessinés dans une grille de 3 colonnes sur 5 lignes (1 = carré allumé) */
const char CHIFFRES_MOTIF[10][16] = {
    "111101101101111",   // 0
    "010110010010111",   // 1
    "111001111100111",   // 2
    "111001111001111",   // 3
    "101101111001001",   // 4
    "111100111001111",   // 5
    "111100111101111",   // 6
    "111001001001001",   // 7
    "111101111101111",   // 8
    "111101111001111"    // 9
};
 
/* ---------- Donnees ---------- */
int salle[L][C];                // la salle actuelle : 0 = vide, 1 = joueur 1, 2 = joueur 2
int suivante[L][C];             // brouillon de la salle à l'heure suivante (pour une mise à jour simultanée)
int special_utilise[3];         // 0 ou 1 pour chaque joueur (cases 1 et 2 utilisées, la 0 ne sert pas)
 
int heure;                      // heure actuelle de la soirée (0 à HEURES)
int joueur;                     // joueur qui doit jouer
int statut;                     // EN_COURS, EGALITE, ou numéro du gagnant
int mode;                       // action choisie par le joueur (MODE_...)
 
SDL_Window *fenetre;            // la fenêtre du jeu
SDL_Renderer *rendu;            // l'outil qui dessine dans la fenêtre
 
/* ================= LOGIQUE (pas de printf / scanf) ================= */
 
int adversaire(int joueur) {             // donne le numéro de l'autre joueur
    if (joueur == JOUEUR_1)              // si on est le joueur 1...
        return JOUEUR_2;                 // ... l'adversaire est le 2
    return JOUEUR_1;                     // sinon l'adversaire est le 1
}
 
int dans_la_salle(int l, int c) {                      // dit si la case (l, c) existe
    return l >= 0 && l < L && c >= 0 && c < C;         // 1 si ligne et colonne sont dans les limites, 0 sinon
}
 
int compter_fetards(int joueur) {                      // compte les fêtards d'un joueur
    int total = 0;                                     // compteur, on part de 0
    for (int l = 0; l < L; l++)                        // on parcourt chaque ligne...
        for (int c = 0; c < C; c++)                    // ... et chaque colonne de la ligne
            if (salle[l][c] == joueur)                 // si la case appartient à ce joueur...
                total++;                               // ... on ajoute 1
    return total;                                      // on renvoie le résultat
}
 
void initialiser_salle(void) {                         // place les fêtards au départ
    for (int l = 0; l < L; l++) {                      // chaque ligne
        for (int c = 0; c < MILIEU; c++) {             // seulement la moitié gauche
            if (rand() % 100 < DENSITE_DEPART) {       // nombre entre 0 et 99, inférieur à 25 = 25 % de chances
                salle[l][c] = JOUEUR_1;                // fêtard du joueur 1 à gauche
                salle[l][C - 1 - c] = JOUEUR_2;        // fêtard du joueur 2 sur la case miroir à droite
            }
        }
    }
}
 
/* Renvoie 1 si l'action a ete faite, 0 sinon */
int inviter_ami(int joueur, int l, int c) {            // pose un fêtard du joueur sur la case
    if (salle[l][c] != VIDE)                           // si la case est déjà occupée...
        return 0;                                      // ... échec
    salle[l][c] = joueur;                              // sinon on pose le fêtard
    return 1;                                          // réussi
}
 
int renvoyer_fetard(int joueur, int l, int c) {        // retire un fêtard adverse
    if (salle[l][c] != adversaire(joueur))             // si la case n'a pas de fêtard adverse...
        return 0;                                      // ... échec
    salle[l][c] = VIDE;                                // sinon on vide la case
    return 1;                                          // réussi
}
 
void appeler_police(int l, int c) {                                    // vide une zone de 5x5 centrée en (l, c)
    for (int i = -RAYON_POLICE; i <= RAYON_POLICE; i++)                // décalage en ligne de -2 à +2
        for (int j = -RAYON_POLICE; j <= RAYON_POLICE; j++)            // décalage en colonne de -2 à +2
            if (dans_la_salle(l + i, c + j))                           // on ignore ce qui sort de la salle
                salle[l + i][c + j] = VIDE;                            // on vide la case
}
 
void lancer_macarena(int joueur, int l, int c) {                       // pose le groupe de danseurs, coin haut gauche en (l, c)
    for (int i = 0; i < TAILLE_GROUPE; i++) {                          // chaque ligne du motif
        for (int j = 0; j < TAILLE_GROUPE; j++) {                      // chaque colonne du motif
            int colonne_motif = j;                                     // colonne lue dans le motif : la même par défaut
            if (joueur == JOUEUR_2)                                    // pour le joueur 2...
                colonne_motif = TAILLE_GROUPE - 1 - j;                 // ... on lit le motif à l'envers (miroir)
            if (DANSEURS[i][colonne_motif] && salle[l + i][c + j] == VIDE)   // danseur à cet endroit ET case libre ?
                salle[l + i][c + j] = joueur;                          // alors on pose le danseur
        }
    }
}
 
int compter_voisins(int joueur, int l, int c) {                        // compte les voisins d'un joueur autour de (l, c)
    int total = 0;                                                     // compteur
    for (int i = -1; i <= 1; i++)                                      // ligne au-dessus, la même, ligne en dessous
        for (int j = -1; j <= 1; j++)                                  // colonne à gauche, la même, colonne à droite
            if ((i != 0 || j != 0) && dans_la_salle(l + i, c + j))     // pas la case elle-même, et qui existe
                if (salle[l + i][c + j] == joueur)                     // si cette voisine est à ce joueur...
                    total++;                                           // ... on la compte
    return total;                                                      // nombre de voisins trouvés
}
 
/* Une heure passe : regles classiques du jeu de la vie */
void evoluer(void) {
    for (int l = 0; l < L; l++) {                                      // chaque ligne
        for (int c = 0; c < C; c++) {                                  // chaque colonne
            int n1 = compter_voisins(JOUEUR_1, l, c);                  // voisins du joueur 1
            int n2 = compter_voisins(JOUEUR_2, l, c);                  // voisins du joueur 2
            int voisins = n1 + n2;                                     // total de voisins, tous camps confondus
 
            suivante[l][c] = VIDE;                                     // par défaut la case sera vide
            if (salle[l][c] != VIDE) {                                 // il y a un fêtard sur la case
                if (voisins >= SURVIE_MIN && voisins <= SURVIE_MAX)    // 2 ou 3 voisins ?
                    suivante[l][c] = salle[l][c];                      // il survit et garde son camp
            } else if (voisins == NAISSANCE) {                         // case vide avec exactement 3 voisins
                if (n1 > n2)                                           // le joueur 1 a plus de voisins...
                    suivante[l][c] = JOUEUR_1;                         // ... le nouveau fêtard est à lui
                else
                    suivante[l][c] = JOUEUR_2;                         // sinon il est au joueur 2
            }
        }
    }
    for (int l = 0; l < L; l++)                                        // on recopie la nouvelle salle...
        for (int c = 0; c < C; c++)
            salle[l][c] = suivante[l][c];                              // ... dans la salle actuelle
}
 
/* Renvoie le gagnant (1 ou 2), EGALITE ou EN_COURS */
int statut_partie(void) {                              // vérifie si un joueur a été éliminé
    int a = compter_fetards(JOUEUR_1);                 // fêtards du joueur 1
    int b = compter_fetards(JOUEUR_2);                 // fêtards du joueur 2
    if (a == 0 && b == 0)                              // plus personne
        return EGALITE;
    if (a == 0)                                        // le joueur 1 n'a plus personne
        return JOUEUR_2;                               // le joueur 2 gagne
    if (b == 0)                                        // le joueur 2 n'a plus personne
        return JOUEUR_1;                               // le joueur 1 gagne
    return EN_COURS;                                   // personne n'est éliminé
}
 
/* Fin de la soiree : le plus de fetards gagne */
int statut_final(void) {                               // désigne le gagnant après les 15 heures
    int a = compter_fetards(JOUEUR_1);                 // score du joueur 1
    int b = compter_fetards(JOUEUR_2);                 // score du joueur 2
    if (a > b)                                         // le joueur 1 en a plus...
        return JOUEUR_1;                               // ... il gagne
    if (b > a)                                         // le joueur 2 en a plus...
        return JOUEUR_2;                               // ... il gagne
    return EGALITE;                                    // même score
}
 
/* ================= DEROULEMENT DE LA PARTIE ================= */
 
void nouvelle_partie(void) {                           // remet tout à zéro et lance une partie
    for (int l = 0; l < L; l++)                        // on vide la salle...
        for (int c = 0; c < C; c++)
            salle[l][c] = VIDE;
    special_utilise[JOUEUR_1] = 0;                     // les événements spéciaux sont de nouveau disponibles
    special_utilise[JOUEUR_2] = 0;
    initialiser_salle();                               // place les fêtards de départ
    heure = 0;                                         // la soirée commence
    joueur = JOUEUR_1;                                 // le joueur 1 commence
    statut = EN_COURS;                                 // la partie n'est pas finie
    mode = MODE_INVITER;                               // action choisie par défaut
}
 
/* Appelée quand un joueur a réussi son action : passe au joueur suivant ou à l'heure suivante */
void fin_action(void) {
    statut = statut_partie();                          // regarde s'il y a un vainqueur
    if (statut != EN_COURS)                            // partie finie : on s'arrête là
        return;
    if (joueur == JOUEUR_1) {                          // le joueur 1 vient de jouer...
        joueur = JOUEUR_2;                             // ... c'est au joueur 2
    } else {                                           // le joueur 2 vient de jouer...
        evoluer();                                     // ... une heure passe
        statut = statut_partie();                      // on revérifie
        joueur = JOUEUR_1;                             // le joueur 1 rejoue
        heure++;                                       // une heure de plus
        if (statut == EN_COURS && heure == HEURES)     // les 15 heures sont écoulées sans vainqueur
            statut = statut_final();                   // on compare les scores
    }
    if (mode >= MODE_POLICE && special_utilise[joueur])   // si l'action choisie n'est plus disponible...
        mode = MODE_INVITER;                           // ... on revient à "inviter un ami"
}
 
/* Position du groupe de danseurs : centré sur la case visée, sans sortir de la salle */
int ligne_groupe(int l) {
    int debut = l - 1;                                 // on centre le groupe de 3 sur la case
    if (debut < 0) debut = 0;                          // pas au-dessus de la salle
    if (debut > L - TAILLE_GROUPE) debut = L - TAILLE_GROUPE;   // pas en dessous
    return debut;
}
 
int colonne_groupe(int c) {
    int debut = c - 1;                                 // on centre le groupe de 3 sur la case
    if (debut < 0) debut = 0;                          // pas à gauche de la salle
    if (debut > C - TAILLE_GROUPE) debut = C - TAILLE_GROUPE;   // pas à droite
    return debut;
}
 
void cliquer_case(int l, int c) {                      // le joueur clique sur la case (l, c)
    int fait = 0;                                      // reste à 0 tant qu'aucune action n'a réussi
 
    if (mode == MODE_INVITER) {                        // inviter un ami
        fait = inviter_ami(joueur, l, c);
    } else if (mode == MODE_RENVOYER) {                // faire partir un fêtard adverse
        fait = renvoyer_fetard(joueur, l, c);
    } else if (mode == MODE_POLICE && !special_utilise[joueur]) {   // la police, si pas encore utilisée
        appeler_police(l, c);
        special_utilise[joueur] = 1;                   // on note que l'événement est consommé
        fait = 1;
    } else if (mode == MODE_MACARENA && !special_utilise[joueur]) { // la Macarena, si pas encore utilisée
        lancer_macarena(joueur, ligne_groupe(l), colonne_groupe(c));
        special_utilise[joueur] = 1;                   // on note que l'événement est consommé
        fait = 1;
    }
 
    if (fait)                                          // si l'action a réussi...
        fin_action();                                  // ... on passe la main
}
 
void choisir_mode(int nouveau) {                       // change l'action choisie (touches 1 à 4 ou clic sur un bouton)
    if (statut != EN_COURS)                            // partie finie : rien à choisir
        return;
    if (nouveau >= MODE_POLICE && special_utilise[joueur])   // événement spécial déjà utilisé
        return;
    mode = nouveau;
}
 
void clic(int x, int y) {                              // gère un clic de souris à la position (x, y) en pixels
    if (y >= HAUT_BARRE) {                             // clic dans la salle
        if (statut == EN_COURS)
            cliquer_case((y - HAUT_BARRE) / CASE, x / CASE);   // pixels -> numéro de ligne et de colonne
        return;
    }
    for (int n = 1; n <= 4; n++) {                     // clic dans la barre : on regarde chaque bouton
        int debut = BOUTON_X + (n - 1) * BOUTON_PAS;   // bord gauche du bouton n
        if (x >= debut && x < debut + BOUTON_L && y >= 10 && y < 50)
            choisir_mode(n);
    }
}
 
/* ================= AFFICHAGE ================= */
 
void rectangle(int x, int y, int w, int h, int r, int g, int b, int a) {   // dessine un rectangle plein
    SDL_Rect forme = {x, y, w, h};                     // position et taille
    SDL_SetRenderDrawColor(rendu, r, g, b, a);         // couleur (a = transparence, 255 = opaque)
    SDL_RenderFillRect(rendu, &forme);                 // on remplit
}
 
void dessiner_nombre(int x, int y, int n, int t) {     // écrit un nombre avec des carrés de t pixels
    char texte[16];                                    // le nombre écrit en texte
    snprintf(texte, sizeof(texte), "%d", n);           // 42 devient "42"
    for (int k = 0; texte[k] != '\0'; k++) {           // pour chaque chiffre
        const char *motif = CHIFFRES_MOTIF[texte[k] - '0'];   // le dessin du chiffre
        for (int i = 0; i < 5; i++)                    // 5 lignes
            for (int j = 0; j < 3; j++)                // 3 colonnes
                if (motif[i * 3 + j] == '1')           // carré allumé ?
                    rectangle(x + (k * 4 + j) * t, y + i * t, t, t, 255, 255, 255, 255);
    }
}
 
void surligner(int l, int c, int joueur_couleur, int transparence) {   // colore une case par-dessus (aperçu)
    rectangle(c * CASE + 1, HAUT_BARRE + l * CASE + 1, CASE - 2, CASE - 2,
              COULEURS[joueur_couleur][0], COULEURS[joueur_couleur][1], COULEURS[joueur_couleur][2], transparence);
}
 
void dessiner_apercu(int l, int c) {                   // montre ce que va faire l'action sur la case visée
    if (mode == MODE_INVITER || mode == MODE_RENVOYER) {
        surligner(l, c, joueur, 120);                  // simple case colorée
    } else if (mode == MODE_POLICE) {                  // zone de 5x5 en jaune
        for (int i = -RAYON_POLICE; i <= RAYON_POLICE; i++)
            for (int j = -RAYON_POLICE; j <= RAYON_POLICE; j++)
                if (dans_la_salle(l + i, c + j))
                    rectangle((c + j) * CASE + 1, HAUT_BARRE + (l + i) * CASE + 1, CASE - 2, CASE - 2, 255, 220, 60, 110);
    } else if (mode == MODE_MACARENA) {                // les danseurs, comme ils seront posés
        int debut_l = ligne_groupe(l);
        int debut_c = colonne_groupe(c);
        for (int i = 0; i < TAILLE_GROUPE; i++) {
            for (int j = 0; j < TAILLE_GROUPE; j++) {
                int colonne_motif = j;
                if (joueur == JOUEUR_2)
                    colonne_motif = TAILLE_GROUPE - 1 - j; // même miroir que dans lancer_macarena
                if (DANSEURS[i][colonne_motif])
                    surligner(debut_l + i, debut_c + j, joueur, 160);
            }
        }
    }
}
 
void dessiner_bouton(int n) {                          // un bouton d'action numéroté
    int x = BOUTON_X + (n - 1) * BOUTON_PAS;           // bord gauche du bouton
    int dispo = !(n >= MODE_POLICE && special_utilise[joueur]);   // 0 si l'événement spécial est déjà utilisé
    int gris = dispo ? 90 : 40;                        // gris clair si dispo, foncé sinon
    rectangle(x, 10, BOUTON_L, 40, gris, gris, gris + 10, 255);
    if (n == mode && statut == EN_COURS)               // le bouton choisi prend la couleur du joueur
        rectangle(x, 10, BOUTON_L, 40, COULEURS[joueur][0], COULEURS[joueur][1], COULEURS[joueur][2], 255);
    dessiner_nombre(x + 19, 20, n, 4);                 // le numéro, au milieu du bouton
}
 
void dessiner(void) {                                  // redessine toute la fenêtre
    int mx, my;                                        // position de la souris en pixels
    SDL_GetMouseState(&mx, &my);
    SDL_SetRenderDrawBlendMode(rendu, SDL_BLENDMODE_BLEND);   // autorise les couleurs transparentes
    rectangle(0, 0, LARGEUR, HAUTEUR, 25, 25, 35, 255);       // fond
 
    int couleur_barre = joueur;                        // la pastille montre le joueur qui doit jouer...
    if (statut == EGALITE)
        couleur_barre = VIDE;                          // ... en cas d'égalité, elle est grise
    else if (statut != EN_COURS)
        couleur_barre = statut;                        // ... à la fin, elle montre le gagnant
    rectangle(10, 10, 40, 40, COULEURS[couleur_barre][0], COULEURS[couleur_barre][1], COULEURS[couleur_barre][2], 255);
 
    for (int h = 0; h < HEURES; h++) {                 // une petite case par heure de la soirée
        int allume = (h < heure) ? 200 : 60;           // claire si l'heure est passée
        rectangle(70 + h * 18, 22, 14, 16, allume, allume, allume, 255);
    }
 
    rectangle(360, 20, 20, 20, COULEURS[JOUEUR_1][0], COULEURS[JOUEUR_1][1], COULEURS[JOUEUR_1][2], 255);
    dessiner_nombre(390, 22, compter_fetards(JOUEUR_1), 3);   // score du joueur 1
    rectangle(470, 20, 20, 20, COULEURS[JOUEUR_2][0], COULEURS[JOUEUR_2][1], COULEURS[JOUEUR_2][2], 255);
    dessiner_nombre(500, 22, compter_fetards(JOUEUR_2), 3);   // score du joueur 2
 
    for (int n = 1; n <= 4; n++)                       // les 4 boutons d'action
        dessiner_bouton(n);
 
    for (int l = 0; l < L; l++)                        // la salle, case par case
        for (int c = 0; c < C; c++)
            surligner(l, c, salle[l][c], 255);         // la couleur dépend de qui est sur la case
 
    rectangle(MILIEU * CASE - 1, HAUT_BARRE, 2, L * CASE, 255, 255, 255, 80);   // ligne entre les deux camps
 
    if (statut == EN_COURS && my >= HAUT_BARRE)        // la souris est au-dessus de la salle
        dessiner_apercu((my - HAUT_BARRE) / CASE, mx / CASE);
 
    char titre[200];                                   // texte de la barre de titre de la fenêtre
    if (statut == EN_COURS)
        snprintf(titre, sizeof(titre), "La soiree etudiante - Joueur %d - Heure %d/%d - 1 inviter, 2 renvoyer, 3 police, 4 macarena - N : rejouer",
                 joueur, heure + 1, HEURES);
    else if (statut == EGALITE)
        snprintf(titre, sizeof(titre), "Egalite ! - N : rejouer");
    else
        snprintf(titre, sizeof(titre), "Le joueur %d gagne ! - N : rejouer", statut);
    SDL_SetWindowTitle(fenetre, titre);
 
    SDL_RenderPresent(rendu);                          // affiche le dessin à l'écran
}
 
int quitter = 0;                                       // passe à 1 pour fermer le jeu
 
void une_image(void) {                                 // traite les événements en attente, puis redessine
    SDL_Event evenement;                               // un événement : touche, clic, fermeture...
    while (SDL_PollEvent(&evenement)) {                // tant qu'il reste des événements à traiter
        if (evenement.type == SDL_QUIT) {              // clic sur la croix
            quitter = 1;
        } else if (evenement.type == SDL_KEYDOWN) {    // une touche est enfoncée
            int touche = evenement.key.keysym.sym;
#ifndef __EMSCRIPTEN__
            if (touche == SDLK_ESCAPE) quitter = 1;    // Echap ferme le jeu (inutile dans un navigateur)
#endif
            if (touche == SDLK_n) nouvelle_partie();
            else if (touche == SDLK_1) choisir_mode(MODE_INVITER);
            else if (touche == SDLK_2) choisir_mode(MODE_RENVOYER);
            else if (touche == SDLK_3) choisir_mode(MODE_POLICE);
            else if (touche == SDLK_4) choisir_mode(MODE_MACARENA);
        } else if (evenement.type == SDL_MOUSEBUTTONDOWN && evenement.button.button == SDL_BUTTON_LEFT) {
            clic(evenement.button.x, evenement.button.y);   // clic gauche
        }
    }
    dessiner();                                        // on redessine à chaque image
#ifdef __EMSCRIPTEN__
    if (quitter)                                       // fermeture demandée : on arrête la boucle du navigateur
        emscripten_cancel_main_loop();
#endif
}
 
int main(int argc, char *argv[]) {                     // argc et argv : exigés par SDL
    (void)argc;                                        // on ne s'en sert pas
    (void)argv;
 
    if (SDL_Init(SDL_INIT_VIDEO) != 0)                 // démarre la partie vidéo de SDL
        return 1;
    fenetre = SDL_CreateWindow("La soiree etudiante", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LARGEUR, HAUTEUR, 0);
    rendu = SDL_CreateRenderer(fenetre, -1, 0);        // l'outil de dessin de la fenêtre
    if (fenetre == NULL || rendu == NULL)              // problème à la création
        return 1;
 
    srand(time(NULL));                                 // initialise le hasard avec l'heure actuelle
    nouvelle_partie();                                 // première partie
 
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(une_image, 0, 1);         // le navigateur appelle une_image à chaque image
#else
    while (!quitter) {                                 // boucle de jeu classique
        une_image();
        SDL_Delay(16);                                 // petite pause : environ 60 images par seconde
    }
    SDL_DestroyRenderer(rendu);                        // on libère ce qu'on a créé
    SDL_DestroyWindow(fenetre);
    SDL_Quit();
#endif
    return 0;
}
