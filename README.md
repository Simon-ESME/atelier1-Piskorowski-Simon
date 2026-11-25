Soirée dans la salle – Jeu de la vie à deux joueurs

- Présentation du jeu :
Soirée dans la salle est un jeu en console développé en C, inspiré du Jeu de la vie de John Conway. Deux joueurs s'affrontent au tour par tour en faisant évoluer une salle de fête représentée par une grille. Chaque case occupée représente un fêtard appartenant à l'un des deux joueurs.
L'objectif est de conserver le plus de fêtards possible jusqu'à la fin de la soirée, tout en essayant d'éliminer ceux de l'adversaire.

- Règles du jeu :
Une partie dure 15 heures. À chaque heure :
Le joueur 1 effectue une action.
Le joueur 2 effectue une action.
La grille évolue selon les règles du Jeu de la vie de Conway.

- Chaque joueur peut choisir une seule action par tour :
* Inviter un ami : ajouter un fêtard sur une case vide.
* Renvoyer un fêtard adverse : supprimer un fêtard appartenant à l'adversaire.
* Événement spécial : utiliser une capacité unique par partie.

- Deux événements spéciaux sont disponibles :
* Appeler la police : vider une zone de 5x5 cases autour de la position choisie.
* La Macarena : placer un groupe de danseurs formant un planeur de 3x3 cases, qui se déplace en diagonale selon le joueur.

Un événement spécial peut être annulé sans être consommé.

Ensuite il y a une nouvelle évolution de la grille.

- À chaque heure, les cases évoluent simultanément selon les règles de Conway :
* Naissance : une case vide devient occupée si elle possède exactement 3 voisins.
* Survie : un fêtard reste si son nombre de voisins est de 2 ou 3.
* Mort : un fêtard disparaît s'il est isolé ou en situation de surpopulation.

Les fêtards des deux joueurs comptent comme voisins. Lorsqu'un nouveau fêtard apparaît, il appartient au joueur majoritaire parmi ses 3 voisins.

- Conditions de victoire
* Victoire par élimination : si un joueur n'a plus aucun fêtard, son adversaire gagne immédiatement.
* Égalité par élimination : si les deux joueurs n'ont plus de fêtards.
* Victoire finale : après 15 heures, le joueur possédant le plus de fêtards gagne. En cas d'égalité, la partie se termine par un match nul.
* Choix de conception
* Grille : dimensions fixes de 15 lignes sur 30 colonnes.
* Bords : la grille est finie et non torique. Les bords ne sont donc pas reliés entre eux et les cases situées hors de la grille sont considérées comme mortes.
* Répartition initiale : chaque case de la moitié gauche a 25 % de chances d'être occupée par un fêtard du joueur 1. La moitié droite est le miroir exact de la gauche pour le joueur 2. Le départ est ainsi parfaitement symétrique.
* Liberté de placement : les joueurs peuvent inviter des fêtards n'importe où dans la salle, même dans la moitié adverse.
* Actions limitées : une seule action par tour et un seul événement spécial par joueur et par partie, afin de conserver un équilibre stratégique.

Mode de jeu : deux joueurs humains s'affrontent, sans adversaire contrôlé par l'ordinateur.

La salle est représentée par les symboles suivants :
X : fêtard du joueur 1.
O : fêtard du joueur 2.
. : case vide.

Les numéros des lignes et des colonnes facilitent le choix des cases. Le nombre de fêtards de chaque joueur est également affiché.