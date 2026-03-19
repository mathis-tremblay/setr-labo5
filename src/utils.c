/******************************************************************************
 * Laboratoire 5
 * GIF-3004 Systèmes embarqués temps réel
 * Hiver 2026
 * Marc-André Gardner
 * 
 * Fichier implémentant des fonctions utilitaires (déjà codées pour vous)
 ******************************************************************************/

#include "utils.h"

// Fonction permettant de récupérer le temps courant sous forme double
double get_time(){
    // Deja implementee pour vous
	struct timeval t;
	struct timezone tzp;
	gettimeofday(&t, &tzp);
	return (double)t.tv_sec + (double)(t.tv_usec)*1e-6;
}

void afficherStats(unsigned int nbrSecondesDepuisDemarrage, struct statistiques *stats){
    // Deja implementee pour vous
    char lineBuf[74];
    memset(lineBuf, '-', sizeof(lineBuf));
    printf("\033[H\033[J");     // Efface le terminal
    printf("Statistiques pour t=%u secondes depuis le demarrage\n", nbrSecondesDepuisDemarrage);
    printf("%s\n", lineBuf);
    printf("| %-40s| %-30s|\n", "Champ", "Valeur");
    printf("%s\n", lineBuf);
    printf("| %-40s| %-30u|\n", "Nombre de requetes en attente", stats->nombreRequetesEnAttente);
    printf("| %-40s| %-30u|\n", "Nombre de requetes traitees", stats->nombreRequetesTraitees);
    printf("| %-40s| %-30u|\n", "Nombre de requetes perdues", stats->nombreRequetesPerdues);
    printf("%s\n", lineBuf);
    printf("| %-40s| %-30f|\n", "Temps de traitement moyen", stats->tempsTraitementMoyen);
    printf("| %-40s| %-30f|\n", "Lambda", stats->lambda);
    printf("| %-40s| %-30f|\n", "Mu", stats->mu);
    printf("| %-40s| %-30f|\n", "Rho", stats->rho);
    printf("%s\n", lineBuf);
}
