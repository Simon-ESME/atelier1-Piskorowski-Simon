#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ---------- Constantes ---------- */
#define L 15
#define C 30
#define MILIEU (C / 2)
#define HEURES 15
#define DENSITE_DEPART 25      /* % de fetards au depart dans chaque camp */
#define CHIFFRES 10            /* regle graduee de l'affichage */

#define VIDE 0
#define JOUEUR_1 1
#define JOUEUR_2 2

#define NAISSANCE 3            /* regles de Conway */
#define SURVIE_MIN 2
#define SURVIE_MAX 3

#define RAYON_POLICE 2         /* zone de 5x5 */
#define TAILLE_GROUPE 3        /* danseurs : 3x3 */

#define EN_COURS -1
#define EGALITE 0

/* Planeur du joueur 1 (descend vers la droite).
   Le joueur 2 le lit en miroir (descend vers la gauche). */
const int DANSEURS[TAILLE_GROUPE][TAILLE_GROUPE] = {
    {0, 1, 0},
    {0, 0, 1},
    {1, 1, 1}
};

/* ---------- Donnees ---------- */
int salle[L][C];
int suivante[L][C];
int special_utilise[3];   /* cases 1 et 2 utilisees */

/* ================= LOGIQUE (pas de printf / scanf) ================= */

int adversaire(int joueur) {
    if (joueur == JOUEUR_1)
        return JOUEUR_2;
    return JOUEUR_1;
}

int dans_la_salle(int l, int c) {
    return l >= 0 && l < L && c >= 0 && c < C;
}

int compter_fetards(int joueur) {
    int total = 0;
    for (int l = 0; l < L; l++)
        for (int c = 0; c < C; c++)
            if (salle[l][c] == joueur)
                total++;
    return total;
}

void initialiser_salle(void) {
    for (int l = 0; l < L; l++) {
        for (int c = 0; c < MILIEU; c++) {
            if (rand() % 100 < DENSITE_DEPART) {
                salle[l][c] = JOUEUR_1;
                salle[l][C - 1 - c] = JOUEUR_2;
            }
        }
    }
}

/* Renvoie 1 si l'action a ete faite, 0 sinon */
int inviter_ami(int joueur, int l, int c) {
    if (salle[l][c] != VIDE)
        return 0;
    salle[l][c] = joueur;
    return 1;
}

int renvoyer_fetard(int joueur, int l, int c) {
    if (salle[l][c] != adversaire(joueur))
        return 0;
    salle[l][c] = VIDE;
    return 1;
}

void appeler_police(int l, int c) {
    for (int i = -RAYON_POLICE; i <= RAYON_POLICE; i++)
        for (int j = -RAYON_POLICE; j <= RAYON_POLICE; j++)
            if (dans_la_salle(l + i, c + j))
                salle[l + i][c + j] = VIDE;
}

void lancer_macarena(int joueur, int l, int c) {
    for (int i = 0; i < TAILLE_GROUPE; i++) {
        for (int j = 0; j < TAILLE_GROUPE; j++) {
            int colonne_motif = j;
            if (joueur == JOUEUR_2)
                colonne_motif = TAILLE_GROUPE - 1 - j;
            if (DANSEURS[i][colonne_motif] && salle[l + i][c + j] == VIDE)
                salle[l + i][c + j] = joueur;
        }
    }
}

int compter_voisins(int joueur, int l, int c) {
    int total = 0;
    for (int i = -1; i <= 1; i++)
        for (int j = -1; j <= 1; j++)
            if ((i != 0 || j != 0) && dans_la_salle(l + i, c + j))
                if (salle[l + i][c + j] == joueur)
                    total++;
    return total;
}

/* Une heure passe : regles classiques du jeu de la vie */
void evoluer(void) {
    for (int l = 0; l < L; l++) {
        for (int c = 0; c < C; c++) {
            int n1 = compter_voisins(JOUEUR_1, l, c);
            int n2 = compter_voisins(JOUEUR_2, l, c);
            int voisins = n1 + n2;

            suivante[l][c] = VIDE;
            if (salle[l][c] != VIDE) {
                if (voisins >= SURVIE_MIN && voisins <= SURVIE_MAX)
                    suivante[l][c] = salle[l][c];
            } else if (voisins == NAISSANCE) {
                if (n1 > n2)
                    suivante[l][c] = JOUEUR_1;
                else
                    suivante[l][c] = JOUEUR_2;
            }
        }
    }
    for (int l = 0; l < L; l++)
        for (int c = 0; c < C; c++)
            salle[l][c] = suivante[l][c];
}

/* Renvoie le gagnant (1 ou 2), EGALITE ou EN_COURS */
int statut_partie(void) {
    int a = compter_fetards(JOUEUR_1);
    int b = compter_fetards(JOUEUR_2);
    if (a == 0 && b == 0)
        return EGALITE;
    if (a == 0)
        return JOUEUR_2;
    if (b == 0)
        return JOUEUR_1;
    return EN_COURS;
}

/* Fin de la soiree : le plus de fetards gagne */
int statut_final(void) {
    int a = compter_fetards(JOUEUR_1);
    int b = compter_fetards(JOUEUR_2);
    if (a > b)
        return JOUEUR_1;
    if (b > a)
        return JOUEUR_2;
    return EGALITE;
}

/* ================= ENTREES / SORTIES ================= */

void afficher(int heure) {
    printf("\n--- Heure %d sur %d ---\n", heure, HEURES);
    printf("    ");
    for (int c = 0; c < C; c++)
        printf("%d", c % CHIFFRES);
    printf("\n");
    for (int l = 0; l < L; l++) {
        printf("%2d  ", l);
        for (int c = 0; c < C; c++) {
            if (salle[l][c] == JOUEUR_1) printf("X");
            else if (salle[l][c] == JOUEUR_2) printf("O");
            else printf(".");
        }
        printf("\n");
    }
    printf("Joueur 1 (X) : %d fetards | Joueur 2 (O) : %d fetards\n",
           compter_fetards(JOUEUR_1), compter_fetards(JOUEUR_2));
}

/* Lit un entier entre min et max, redemande sinon */
int lire_nombre(int min, int max) {
    int n, ch;
    int valide;
    do {
        valide = (scanf("%d", &n) == 1);
        if (!valide) {
            while ((ch = getchar()) != '\n' && ch != EOF);
            printf("  entre un nombre : ");
        } else if (n < min || n > max) {
            printf("  entre une valeur entre %d et %d : ", min, max);
            valide = 0;
        }
    } while (!valide);
    return n;
}

/* Remplacent lire_case (qui utilisait des pointeurs) */
int lire_ligne(int max) {
    printf("  ligne : ");
    return lire_nombre(0, max);
}

int lire_colonne(int max) {
    printf("  colonne : ");
    return lire_nombre(0, max);
}

/* Renvoie 1 si l'evenement a bien ete joue, 0 si le joueur annule */
int evenement_special(int joueur) {
    int l, c;
    printf("1) Le voisin appelle la police (vide une zone de 5x5)\n");
    printf("2) La Macarena (un groupe de danseurs traverse la piste en diagonale)\n");
    printf("0) Retour\n");
    printf("  ton choix : ");
    int choix = lire_nombre(0, 2);

    if (choix == 1) {
        printf("Centre de la zone :\n");
        l = lire_ligne(L - 1);
        c = lire_colonne(C - 1);
        appeler_police(l, c);
    } else if (choix == 2) {
        printf("Coin en haut a gauche du groupe (il prend 3x3 cases) :\n");
        l = lire_ligne(L - TAILLE_GROUPE);
        c = lire_colonne(C - TAILLE_GROUPE);
        lancer_macarena(joueur, l, c);
    } else {
        return 0;
    }
    special_utilise[joueur] = 1;
    return 1;
}

void tour(int joueur) {
    int l, c, choix;
    int fait = 0;

    printf("\nJoueur %d, que fais-tu ?\n", joueur);
    printf("1) Inviter un ami\n");
    printf("2) Faire partir un fetard adverse\n");
    if (!special_utilise[joueur])
        printf("3) Evenement special (une seule fois)\n");

    do {
        printf("Ton choix : ");
        choix = lire_nombre(1, 3);
        if (choix == 1) {
            l = lire_ligne(L - 1);
            c = lire_colonne(C - 1);
            fait = inviter_ami(joueur, l, c);
            if (!fait)
                printf("  la case est deja occupee\n");
        } else if (choix == 2) {
            l = lire_ligne(L - 1);
            c = lire_colonne(C - 1);
            fait = renvoyer_fetard(joueur, l, c);
            if (!fait)
                printf("  il n'y a pas de fetard adverse ici\n");
        } else if (!special_utilise[joueur]) {
            fait = evenement_special(joueur);
        } else {
            printf("  evenement special deja utilise\n");
        }
    } while (!fait);
}

void afficher_resultat(int statut) {
    if (statut == EGALITE)
        printf("\nEgalite !\n");
    else
        printf("\nLe joueur %d gagne !\n", statut);
}

int main(void) {
    int heure, joueur;
    int statut = EN_COURS;

    srand(time(NULL));
    initialiser_salle();

    for (heure = 0; heure < HEURES && statut == EN_COURS; heure++) {
        for (joueur = JOUEUR_1; joueur <= JOUEUR_2 && statut == EN_COURS; joueur++) {
            afficher(heure);
            tour(joueur);
            statut = statut_partie();
        }
        if (statut == EN_COURS) {
            evoluer();
            statut = statut_partie();
        }
    }

    if (statut == EN_COURS) {
        afficher(HEURES);
        statut = statut_final();
    }
    afficher_resultat(statut);
    return 0;
}