#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

void afficher_choix(int choix);
void afficher_bilan(int scoreJoueur, int scoreOrdi);
bool partie_en_cours(int manche, int scoreJoueur, int scoreOrdi);
int saisie_joueur(void);
bool joueur1_gagne(int choix1, int choix2);

int main()
{
    int scoreJoueur = 0;
    int scoreOrdi = 0;
    int manche = 1;
    int choixOrdi;
 
    printf("=== PIERRE, FEUILLE, CISEAUX, LEZARD, SPOCK (7 Manches / avantage décisif de 2) ===\n");
 
    while (partie_en_cours(manche, scoreJoueur, scoreOrdi))

    {
        printf("--- Manche %d/7 ---\n", manche);
 
        // Saisie du joueur
        int choixJoueur = saisie_joueur();
 
        // Choix aléatoire de l'ordinateur (1, 2, 3, 4 ou 5)
        choixOrdi = (rand() % 5) + 1;
        printf("L'ordinateur a choisi : ");
        afficher_choix(choixOrdi);
        printf("\n");
 
        // Détermination du gagnant de la manche
        if (choixJoueur == choixOrdi)

        {
            printf("Égalité !\n");
        }

        else if (joueur1_gagne(choixJoueur, choixOrdi))
        {
            printf("Vous gagnez cette manche !\n");
            scoreJoueur = scoreJoueur + 1;
        }

        else
        
        {
            printf("L'ordinateur gagne cette manche !\n");
            scoreOrdi = scoreOrdi + 1;
        }
 
        printf("Score actuel -> Vous : %d | Ordi : %d\n\n", scoreJoueur, scoreOrdi);
        manche = manche + 1;
    }
 
    // Bilan de la partie
    afficher_bilan(scoreJoueur, scoreOrdi);
    return 0;
}
bool partie_en_cours(int manche, int scoreJoueur, int scoreOrdi)

{
    return manche <= 7 && scoreJoueur - scoreOrdi < 2 && scoreOrdi - scoreJoueur < 2;
}
 
int saisie_joueur(void)

{
    int choix;
    bool incorrect;
    do

    {
        printf("Choix (");
        for (int i = 1; i <= 5; i++)

        {
            printf("%d = ", i);
            afficher_choix(i);
            if (i < 5)

            {
                printf(", ");

            }

        }

        printf(") : ");
 
        scanf("%d", &choix);

        incorrect = choix < 1 || 5 < choix;

        if (incorrect)

        {

            printf("Non valide, valeurs de 1 à 5 acceptées\n");

        }

    } while (incorrect);
 
    return choix;

}
 
void afficher_choix(int choix)

{

    if (choix == 1)

    {

        printf("Pierre");

    }

    else if (choix == 2)

    {

        printf("Feuille");

    }

    else if (choix == 3)

    {

        printf("Ciseaux");

    }

    else if (choix == 4)

    {

        printf("Lezard");

    }

    else if (choix == 5)

    {

        printf("Spock");

    }

}
 
void afficher_bilan(int scoreJoueur, int scoreOrdi)
{
    printf("=== FIN DE LA PARTIE ===\n");
    printf("Score final -> Vous : %d | Ordi : %d\n", scoreJoueur, scoreOrdi);
    if (scoreJoueur > scoreOrdi)

    {

        printf("Bravo, vous avez gagné la partie !\n");

    }

    else if (scoreOrdi > scoreJoueur)

    {
        printf("L'ordinateur remporte la partie...\n");
    }
    else

    {
        printf("Match nul parfait !\n");
    }
}

bool joueur1_gagne(int choix1, int choix2)
{
    return (choix1 == 1 && (choix2 == 3 || choix2 == 4)) ||
           (choix1 == 2 && (choix2 == 1 || choix2 == 5)) ||
           (choix1 == 3 && (choix2 == 2 || choix2 == 4)) ||
           (choix1 == 4 && (choix2 == 2 || choix2 == 5)) ||
           (choix1 == 5 && (choix2 == 1 || choix2 == 3));
}