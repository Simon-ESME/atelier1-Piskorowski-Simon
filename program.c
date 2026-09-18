#include <stdio.h>

int main() {
    int alt = 2200;
    int spd  = 180;
    int angles = 10;
    int cap = 45;
    int compas = 0;
    printf("+----------------------------------------------------------+\n");
    printf("| IAS(kts)            . . . ^ . . .              ALT(ft)   |\n");
    printf("| %3d |                  \\  |  /                | %4d |   |\n", spd+90, alt+900);
    printf("| %3d |               -----%2d-----              | %4d |   |\n", spd+80,angles+10,alt+800);
    printf("| %3d |                   -----                 | %4d |   |\n", spd+70, alt+700);
    printf("| %3d |         \\                       /       | %4d |   |\n", spd+60, alt+600);
    printf("| %3d |          \\                     /        | %4d |   |\n", spd+50, alt+500);
    printf("|=====|=========================================|======|===|\n", spd+40, alt+400);
    printf("| %3d |               -----%2d-----              | %4d |   |\n", spd+30,angles, alt+300);
    printf("| %3d |                   -----                 | %4d |   |\n", spd+20, alt+200);
    printf("| %3d |                                         | %4d |   |\n", spd+10,alt+100);
    printf("|>%3d<|        |---|      [ + ]     |---|       | %4d |   |\n",spd,alt);
    printf("|-----|--------+---+--------+--------+-+--------|------|---|\n");
    printf("| %3d |        |---|                |---|       | %4d |   |\n",spd-10, alt-100);
    printf("| %3d |                                         | %4d |   |\n",spd-20, alt-200);
    printf("| %3d |                   -----                 | %4d |   |\n",spd-30,alt-300);
    printf("| %3d |               -----%2d-----              | %4d |   |\n",spd-40,angles,alt-400);
    printf("| %3d |                   -----                 | %4d |   |\n",spd-50,alt-500);
    printf("| %3d |               -----%2d-----              | %4d |   |\n",spd-60,angles+10,alt-600);
    printf("| %3d |                   -----                 | %4d |   |\n",spd-70,alt-700);
    printf("| %3d |               -----%2d-----              | %4d |   |\n",spd-80,angles+20,alt-800);
    printf("| %3d |                                         | %4d |   |\n",spd-90,alt-900);
    printf("|----------------------------------------------------------|\n");
    printf("| W        %3d°            N        %3d°       E      %3d° |\n",compas, compas+90, compas+180);
    printf("|---------------------- CAP%3d ----------------------------|\n",cap);
    printf("+----------------------------------------------------------+\n");
    return 0;
}