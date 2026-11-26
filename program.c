#include <stdio.h>      // printf, scanf, getchar : afficher et lire au clavier
#include <stdlib.h>     // rand, srand : nombres au hasard
#include <time.h>       // time : heure actuelle, pour changer le hasard à chaque partie

/* ---------- Constantes ---------- */
#define L 15                   // nombre de lignes de la salle
#define C 30                   // nombre de colonnes de la salle
#define MILIEU (C / 2)         // colonne qui sépare les deux camps (parenthèses pour que le calcul reste juste)
#define HEURES 15              // durée de la soirée (nombre de tours de jeu)
#define DENSITE_DEPART 25      // % de chances d'avoir un fêtard sur une case au départ
#define CHIFFRES 10            // sert à n'afficher que le dernier chiffre des colonnes (c % 10)

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

/* Planeur du joueur 1 (descend vers la droite).
   Le joueur 2 le lit en miroir (descend vers la gauche). */
const int DANSEURS[TAILLE_GROUPE][TAILLE_GROUPE] = {   // const : le motif ne change jamais
    {0, 1, 0},         // ligne 1 du motif : un danseur au milieu
    {0, 0, 1},         // ligne 2 du motif : un danseur à droite
    {1, 1, 1}          // ligne 3 du motif : trois danseurs
};

/* ---------- Donnees ---------- */
int salle[L][C];                // la salle actuelle : 0 = vide, 1 = joueur 1, 2 = joueur 2
int suivante[L][C];             // brouillon de la salle à l'heure suivante (pour une mise à jour simultanée)
int special_utilise[3];         // 0 ou 1 pour chaque joueur (cases 1 et 2 utilisées, la 0 ne sert pas)

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

/* ================= ENTREES / SORTIES ================= */

void afficher(int heure) {                                             // dessine la salle à l'écran
    printf("\n--- Heure %d sur %d ---\n", heure, HEURES);              // titre avec l'heure
    printf("    ");                                                    // espace pour aligner avec les lignes
    for (int c = 0; c < C; c++)                                        // pour chaque colonne...
        printf("%d", c % CHIFFRES);                                    // ... on affiche son dernier chiffre
    printf("\n");                                                      // retour à la ligne
    for (int l = 0; l < L; l++) {                                      // pour chaque ligne de la salle
        printf("%2d  ", l);                                            // numéro de ligne sur 2 caractères
        for (int c = 0; c < C; c++) {                                  // pour chaque case de la ligne
            if (salle[l][c] == JOUEUR_1) printf("X");                  // joueur 1 = X
            else if (salle[l][c] == JOUEUR_2) printf("O");             // joueur 2 = O
            else printf(".");                                          // case vide = point
        }
        printf("\n");                                                  // fin de la ligne
    }
    printf("Joueur 1 (X) : %d fetards | Joueur 2 (O) : %d fetards\n",  // affiche le score des deux joueurs
           compter_fetards(JOUEUR_1), compter_fetards(JOUEUR_2));
}

/* Lit un entier entre min et max, redemande sinon */
int lire_nombre(int min, int max) {
    int n, ch;                                                         // n = nombre lu, ch = caractère à jeter
    int valide;                                                        // 1 si la saisie est correcte, 0 sinon
    do {                                                               // on lit au moins une fois
        valide = (scanf("%d", &n) == 1);                               // scanf renvoie 1 s'il a bien lu un entier
        if (!valide) {                                                 // le joueur a tapé autre chose qu'un nombre
            while ((ch = getchar()) != '\n' && ch != EOF);             // on vide le reste de la ligne tapée
            printf("  entre un nombre : ");                            // on redemande
        } else if (n < min || n > max) {                               // c'est un nombre mais hors limites
            printf("  entre une valeur entre %d et %d : ", min, max);  // on explique les limites
            valide = 0;                                                // on doit redemander
        }
    } while (!valide);                                                 // on recommence tant que ce n'est pas valide
    return n;                                                          // on renvoie le nombre correct
}

/* Remplacent lire_case (qui utilisait des pointeurs) */
int lire_ligne(int max) {                      // demande un numéro de ligne
    printf("  ligne : ");                      // question
    return lire_nombre(0, max);                // renvoie la ligne, entre 0 et max
}

int lire_colonne(int max) {                    // demande un numéro de colonne
    printf("  colonne : ");                    // question
    return lire_nombre(0, max);                // renvoie la colonne, entre 0 et max
}

/* Renvoie 1 si l'evenement a bien ete joue, 0 si le joueur annule */
int evenement_special(int joueur) {
    int l, c;                                                          // coordonnées choisies
    printf("1) Le voisin appelle la police (vide une zone de 5x5)\n"); // menu : choix 1
    printf("2) La Macarena (un groupe de danseurs traverse la piste en diagonale)\n");  // menu : choix 2
    printf("0) Retour\n");                                             // menu : annuler
    printf("  ton choix : ");                                          // question
    int choix = lire_nombre(0, 2);                                     // lit 0, 1 ou 2

    if (choix == 1) {                                                  // la police
        printf("Centre de la zone :\n");
        l = lire_ligne(L - 1);                                         // ligne du centre
        c = lire_colonne(C - 1);                                       // colonne du centre
        appeler_police(l, c);                                          // vide la zone
    } else if (choix == 2) {                                           // la Macarena
        printf("Coin en haut a gauche du groupe (il prend 3x3 cases) :\n");
        l = lire_ligne(L - TAILLE_GROUPE);                             // limite pour que le groupe rentre en bas
        c = lire_colonne(C - TAILLE_GROUPE);                           // limite pour que le groupe rentre à droite
        lancer_macarena(joueur, l, c);                                 // pose les danseurs
    } else {                                                           // choix 0 : retour
        return 0;                                                      // rien n'est joué, l'événement n'est pas consommé
    }
    special_utilise[joueur] = 1;                                       // on note que ce joueur a utilisé son événement
    return 1;                                                          // événement joué
}

void tour(int joueur) {                                                // déroule le tour d'un joueur
    int l, c, choix;                                                   // coordonnées et choix du joueur
    int fait = 0;                                                      // reste à 0 tant qu'aucune action n'a réussi

    printf("\nJoueur %d, que fais-tu ?\n", joueur);                    // annonce le joueur
    printf("1) Inviter un ami\n");                                     // menu : choix 1
    printf("2) Faire partir un fetard adverse\n");                     // menu : choix 2
    if (!special_utilise[joueur])                                      // si l'événement n'est pas encore utilisé...
        printf("3) Evenement special (une seule fois)\n");             // ... on propose le choix 3

    do {                                                               // on boucle jusqu'à ce qu'une action réussisse
        printf("Ton choix : ");
        choix = lire_nombre(1, 3);                                     // lit 1, 2 ou 3
        if (choix == 1) {                                              // inviter un ami
            l = lire_ligne(L - 1);                                     // ligne choisie
            c = lire_colonne(C - 1);                                   // colonne choisie
            fait = inviter_ami(joueur, l, c);                          // 1 si réussi, 0 sinon
            if (!fait)                                                 // échec : on explique pourquoi
                printf("  la case est deja occupee\n");
        } else if (choix == 2) {                                       // faire partir un fêtard adverse
            l = lire_ligne(L - 1);
            c = lire_colonne(C - 1);
            fait = renvoyer_fetard(joueur, l, c);                      // 1 si réussi, 0 sinon
            if (!fait)                                                 // échec : on explique pourquoi
                printf("  il n'y a pas de fetard adverse ici\n");
        } else if (!special_utilise[joueur]) {                         // choix 3 et événement encore disponible
            fait = evenement_special(joueur);                          // 1 si joué, 0 si annulé
        } else {                                                       // choix 3 mais déjà utilisé
            printf("  evenement special deja utilise\n");
        }
    } while (!fait);                                                   // tant que rien n'est joué, on redemande
}

void afficher_resultat(int statut) {                   // annonce le résultat de la partie
    if (statut == EGALITE)                             // égalité
        printf("\nEgalite !\n");
    else                                               // sinon le statut est le numéro du gagnant
        printf("\nLe joueur %d gagne !\n", statut);
}

int main(void) {
    int heure, joueur;                                 // compteurs de boucle
    int statut = EN_COURS;                             // la partie commence

    srand(time(NULL));                                 // initialise le hasard avec l'heure actuelle
    initialiser_salle();                               // place les fêtards de départ

    for (heure = 0; heure < HEURES && statut == EN_COURS; heure++) {                        // les heures, tant que personne n'a gagné
        for (joueur = JOUEUR_1; joueur <= JOUEUR_2 && statut == EN_COURS; joueur++) {       // le joueur 1 puis le joueur 2
            afficher(heure);                           // montre la salle
            tour(joueur);                              // le joueur joue son action
            statut = statut_partie();                  // regarde s'il y a un vainqueur
        }
        if (statut == EN_COURS) {                      // si personne n'a gagné...
            evoluer();                                 // ... une heure passe
            statut = statut_partie();                  // on revérifie
        }
    }

    if (statut == EN_COURS) {                          // les 15 heures sont écoulées sans vainqueur
        afficher(HEURES);                              // on montre la salle finale
        statut = statut_final();                       // on compare les scores
    }
    afficher_resultat(statut);                         // annonce le résultat
    return 0;                                          // fin du programme
}