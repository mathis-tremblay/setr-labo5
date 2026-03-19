#ifndef TAMPONCIRCULAIRE_H
#define TAMPONCIRCULAIRE_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "utils.h"

// Structure contenant l'information sur une requete a envoyer sur le bus USB
struct requete{
    double tempsReception;  // Moment de la reception de la requete (lorsque le thread lecteur la lit sur le named pipe)
    size_t taille;          // Nombre de caracteres dans le buffer data
    char* data;             // Buffer contenant les donnees (alloue avec malloc() par le thread lecteur lors de la
                            // creation de la requete, et libere avec free() par le thread clavier une fois son
                            // envoi acheve)
};

// Initialise le tampon circulaire pour son utilisation. Son unique argument est sa taille (par exemple, 10
// indique qu'on doit creer un tampon circulaire de 10 elements). Chaque element doit etre de taille
// struct requete. Retourne 0 si effectue avec succes, -1 sinon.
int initTamponCirculaire(size_t taille);

// Calcule les statistiques et insere les resultats dans la struct passee par pointeur
void calculeStats(struct statistiques *stats);

// Reinitialise les compteurs internes permettant de calculer les statistiques
void resetStats();

// Insere une donnee (une requete req) en la copiant dans le tampon circulaire. 
// Retourne 0 si effectue avec succes, -1 sinon.
// N'oubliez pas de proteger la ou les sections critiques de cette fonction avec
// un mutex!
int insererDonnee(struct requete *req);

// Consomme et retire la prochaine donnee du tampon circulaire et la copie dans la
// struct requete passee en argument.
// Si aucune donnee n'est disponible, cette fonction retourne 0. Si une requete
// a ete copiee, alors cette fonction retourne 1. En cas d'erreur, elle retourne -1.
// N'oubliez pas de proteger la ou les sections critiques de cette fonction avec
// un mutex!
int consommerDonnee(struct requete *req);

// Retourne la longueur courante de la file d'attente (le nombre de requetes en attente)
unsigned int longueurFile();

#endif