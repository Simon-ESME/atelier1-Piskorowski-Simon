# La soirée étudiante

Jeu à deux joueurs en C, inspiré du jeu de la vie de Conway.

## Contenu du dossier

| Fichier | Rôle |
|---|---|
| `soiree_etudiante.c` | Version terminal. **Version principale**, elle ne demande que `gcc`. |
| `soiree_etudiante_gui.c` | Version avec interface graphique (SDL2). Mêmes règles, affichage différent. |
| `Makefile` | Compilation des différentes versions. |

## Règles

Deux joueurs se partagent une salle de 15 lignes sur 30 colonnes. Chaque joueur a un camp (le joueur 1 à gauche, le joueur 2 à droite) et des fêtards placés au hasard au départ, de façon symétrique.

La soirée dure 15 heures. À chaque heure :

1. le joueur 1 joue une action ;
2. le joueur 2 joue une action ;
3. la salle évolue d'une génération selon les règles du jeu de la vie.

**Actions possibles** (une par tour) :

- **Inviter un ami** : poser un fêtard sur une case vide.
- **Faire partir un fêtard adverse** : retirer un fêtard du camp adverse.
- **Événement spécial** (une seule fois par joueur et par partie) :
  - *la police* vide une zone de 5×5 autour de la case choisie ;
  - *la Macarena* pose un groupe de 5 danseurs (un planeur) dans un carré de 3×3 ; celui du joueur 2 est lu en miroir.

**Évolution** (règles de Conway) : un fêtard survit s'il a 2 ou 3 voisins, sinon il part. Une case vide avec exactement 3 voisins voit naître un fêtard, qui appartient au joueur ayant le plus de voisins autour de cette case.

**Victoire** :

- un joueur qui n'a plus aucun fêtard perd immédiatement ;
- sinon, après les 15 heures, celui qui a le plus de fêtards gagne (égalité possible).

## Compiler et jouer

### Version terminal

```
make
./jeu
```

Sans `make` : `gcc soiree_etudiante.c -o jeu`

On choisit l'action avec un numéro, puis on tape la ligne et la colonne de la case visée. Les joueurs sont `X` (joueur 1) et `O` (joueur 2).

### Version graphique (SDL2)

Il faut d'abord installer SDL2 :

- Linux (Debian/Ubuntu) : `sudo apt install build-essential libsdl2-dev`
- Mac : `brew install sdl2`
- Windows : installer MSYS2, puis dans le terminal « MSYS2 MINGW64 » :
  `pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 make`

Puis :

```
make gui
./jeu_gui
```

Sans `make` : `gcc soiree_etudiante_gui.c -o jeu_gui $(sdl2-config --cflags --libs)`

**Commandes** :

- touches `1` à `4` (ou clic sur les boutons en haut à droite) : inviter, renvoyer, police, Macarena ;
- clic gauche sur une case : jouer l'action choisie (un aperçu s'affiche sous la souris) ;
- `N` : nouvelle partie, `Echap` : quitter.

La pastille en haut à gauche indique le joueur qui doit jouer, les 15 petits carrés montrent l'avancement de la soirée, et les deux nombres (bleu et rouge) sont les scores. Le fêtard du joueur 1 est bleu, celui du joueur 2 est rouge.

La Macarena est centrée sur la case cliquée (dans la version terminal, on indique son coin en haut à gauche).

### Version navigateur (facultatif)

Le même fichier `soiree_etudiante_gui.c` peut être compilé en WebAssembly avec Emscripten, pour jouer dans un navigateur :

```
make web
emrun --port 8080 web/index.html
```

Cela demande d'avoir installé Emscripten (<https://emscripten.org>) et de l'avoir activé dans le terminal (`source ./emsdk_env.sh`).

## Organisation du code

Le code est séparé en deux parties.

**La logique** (identique dans les deux versions) ne contient ni `printf` ni `scanf` :

- `inviter_ami`, `renvoyer_fetard`, `appeler_police`, `lancer_macarena` : les actions des joueurs ;
- `compter_voisins`, `evoluer` : une heure de jeu de la vie, calculée dans le tableau `suivante` pour que tous les fêtards changent en même temps ;
- `statut_partie`, `statut_final` : détection d'un vainqueur ;
- `compter_fetards`, `initialiser_salle`, `adversaire`, `dans_la_salle` : fonctions utilitaires.

**Les entrées et sorties** changent selon la version :

- *terminal* : `afficher`, `lire_nombre`, `tour`, `evenement_special` et la boucle de `main` ;
- *graphique* : `dessiner`, `clic`, `cliquer_case`, `fin_action` et la boucle `une_image`. SDL2 fournit la fenêtre, les rectangles de couleur et les événements souris et clavier. Un clic en pixels est converti en case avec `(y - HAUT_BARRE) / CASE` pour la ligne et `x / CASE` pour la colonne.

Dans la version graphique, `fin_action` reprend le rôle de la boucle de `main` de la version terminal : passer au joueur suivant, faire évoluer la salle après le joueur 2, puis compter les heures et désigner le gagnant.
