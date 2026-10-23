#include <stdio.h>
 
// Dimensions world
#define WIDTH 20
#define HEIGHT 20
 
// Tableau du monde : 0 = cellule morte, 1 = cellule vivante
static int world[HEIGHT][WIDTH] = {0};
 
int main()
{
    int i, j;
 
    // Structure stable
    world[1][1] = 1;
    world[1][2] = 1;
    world[2][1] = 1;
    world[2][2] = 1;
 
    // Oscillateur
    world[2][8] = 1;
    world[2][9] = 1;
    world[2][10] = 1;
 
    // Vaisseau
    world[10][2] = 1;
    world[10][5] = 1;
    world[11][1] = 1;
    world[12][1] = 1;
    world[12][5] = 1;
    world[13][1] = 1;
    world[13][2] = 1;
    world[12][3] = 1;
 
    // Affichage du tableau world
    for (i = 0; i < HEIGHT; i++)
    {
        for (j = 0; j < WIDTH; j++)
        {
            if (world[i][j] == 1)
            {
                printf("O ");
            }
            else
            {
                printf(". ");
            }
        }
        printf("\n");
    }
 
    return 0;
}