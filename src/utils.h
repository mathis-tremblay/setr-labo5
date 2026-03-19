#ifndef UTILS_H
#define UTILS_H
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

// Fonction permettant de récupérer le temps courant sous forme double
double get_time();

// Structure permettant de passer les informations requises au thread
// ecrivant les caracteres sur le bus USB
struct infoThreadClavier{
    FILE* pointeurClavier;                                  // Pointeur vers le fichier virtuel sur lequel ecrire
    unsigned int tempsTraitementParCaractereMicroSecondes;  // Nombre de microsecondes a attendre apres l'envoi de chaque paquet
    pthread_barrier_t* barriere;                            // Barriere a utiliser pour synchroniser le demarrage des 2 threads
};

// Structure permettant de passer les informations requises au thread
// lisant les caracteres depuis le named pipe
struct infoThreadLecture{
    int pipeFd;                     // Descripteur de fichier du pipe
    pthread_barrier_t* barriere;    // Barriere a utiliser pour synchroniser le demarrage des 2 threads
};

// Structure contenant les statistiques que vous devez calculer
// Ces statistiques doivent etre calculees sur une periode de 2 secondes
// (donc, par exemple, le nombre de requetes traitees est le nombre de
// caracteres traites _dans les 2 dernieres secondes_) ou, si vous preferez,
// depuis le dernier affichage de ces statistiques (qui se fait a toutes
// les 2 secondes).
// La plupart des champs ont des noms evocateurs, voyez les notes de cours sur
// les files d'attente si la signification de lambda, mu et rho n'est pas claire.
struct statistiques{
    unsigned int nombreRequetesEnAttente;
    unsigned int nombreRequetesTraitees;
    unsigned int nombreRequetesPerdues;
    double tempsTraitementMoyen;
    double lambda;
    double mu;
    double rho;
};

// Fonction affichant les statistiques sur la console, a partir d'une structure contenant
// ces informations. Elle est deja implementee pour vous.
// L'argument nbrSecondesDepuisDemarrage est simplement la valeur entiere du nombre de
// secondes qui se sont ecoulees depuis le lancement du programme.
// Notez que cette fonction va _effacer_ le terminal a chaque fois. Ne l'utilisez donc
// pas si vous voulez faire des print avec des informations de debogage.
void afficherStats(unsigned int nbrSecondesDepuisDemarrage, struct statistiques *stats);

#endif
