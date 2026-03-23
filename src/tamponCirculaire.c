/******************************************************************************
 * Laboratoire 5
 * GIF-3004 Systèmes embarqués temps réel
 * Hiver 2026
 * Marc-André Gardner
 * 
 * Fichier implémentant les fonctions de gestion du tampon circulaire
 ******************************************************************************/

#include "tamponCirculaire.h"
//#define _POSIX_C_SOURCE 199309L
//#include <time.h>
// Plusieurs variables globales statiques (pour qu'elles ne soient accessible que dans les
// fonctions de ce fichier) sont declarees ici. Elle servent a conserver l'etat du tampon
// circulaire ainsi qu'a mesurer certains elements utiles au calcul des statistiques.
// Vous etes libres d'en creer d'autres si vous en voyez le besoin.

// Pointe vers la memoire allouee pour le tampon circulaire
static char* memoire;

// Taile du tampon circulaire (en nombre d'elements de type struct requete)
static size_t memoireTaille;

// Positions de lecture et d'ecriture, et longueur actuelle du tampon circulaire
static unsigned int posLecture, posEcriture, longueurCourante;

// Mutex permettant de proteger les acces au tampon circulaire
// N'oubliez pas que _deux_ threads vont tenter de faire des operations en parallele!
pthread_mutex_t mutexTampon;
static int mutexTamponInitialise = 0;

// Pour les statistiques
static unsigned int nombreRequetesRecues, nombreRequetesTraitees, nombreRequetesPerdues;

// Le tempsDebutPeriode permet de se rappeler du debut de la periode ou les statistiques sont mesurees
// sommeTempsService contient la somme des temps d'ecriture reellement mesures
// par le thread clavier pour les requetes traitees.
static double tempsDebutPeriode, sommeTempsService;

int initTamponCirculaire(size_t taille){
    // Initialisez ici:
    // La memoire, en utilisant malloc ou calloc (rappelez-vous que votre tampon circulaire doit
    // pouvoir contenir _taille_ fois la taille d'une struct requete)
    //
    // Les positions de lecture, d'ecriture et de longueur courante.
    //
    // Le mutex
    //
    // Les variables de statistiques

    // TODO (DONE)
    // Mémoire du tampon circulaire
    if (taille == 0) {
        return -1;
    }
    memoire = (char*)calloc(taille, sizeof(struct requete));
    // on regarde si on renvoie une valeur null (donc le calloc s'est pas fait correctement)
    if (memoire == NULL) {
        return -1;
    }
    posLecture = 0;
    posEcriture = 0;
    longueurCourante = 0;
    memoireTaille = taille;

    // Initialiser le mutex
    if (pthread_mutex_init(&mutexTampon, NULL) != 0) {
        freeMemoireTampon();
        memoire = NULL;
        return -1;
    }
    mutexTamponInitialise = 1;

    // Initialiser les statistiques
    nombreRequetesRecues = 0;
    nombreRequetesTraitees = 0;
    nombreRequetesPerdues = 0;

    tempsDebutPeriode = get_time();
    sommeTempsService = 0.0;
    return 0;

}

void resetStats(){
    // TODO (DONE)
    // Reinitialise les variables de statistique
    pthread_mutex_lock(&mutexTampon);
    nombreRequetesRecues = 0;
    nombreRequetesTraitees = 0;
    nombreRequetesPerdues = 0;
    sommeTempsService = 0.0;
    tempsDebutPeriode = get_time();
    pthread_mutex_unlock(&mutexTampon);
}

void calculeStats(struct statistiques *stats){
    // TODO
    // Structure contenant les statistiques que vous devez calculer
// Ces statistiques doivent etre calculees sur une periode de 2 secondes
// (donc, par exemple, le nombre de requetes traitees est le nombre de
// caracteres traites _dans les 2 dernieres secondes_) ou, si vous preferez,
// depuis le dernier affichage de ces statistiques (qui se fait a toutes
// les 2 secondes).
// La plupart des champs ont des noms evocateurs, voyez les notes de cours sur
// les files d'attente si la signification de lambda, mu et rho n'est pas claire.
/* struct statistiques{
    unsigned int nombreRequetesEnAttente;
    unsigned int nombreRequetesTraitees;
    unsigned int nombreRequetesPerdues;
    double tempsTraitementMoyen;
    double lambda;
    double mu;
    double rho;
};
*/
    double tempsActuel;
    double deltaT;

    pthread_mutex_lock(&mutexTampon);

    // Aller chercher le temps actuel
    tempsActuel = get_time();

    // Calculer la durée de la période
    deltaT = tempsActuel - tempsDebutPeriode;
    if (deltaT <= 0.0) {
        deltaT = 1e-9;  // éviter une division par 0
    }

    // Remplir la structure de statistiques
    stats->nombreRequetesEnAttente = longueurCourante;
    stats->nombreRequetesTraitees = nombreRequetesTraitees;
    stats->nombreRequetesPerdues = nombreRequetesPerdues;

    if (nombreRequetesTraitees > 0) {
        stats->tempsTraitementMoyen = sommeTempsService / nombreRequetesTraitees;
    } else {
        stats->tempsTraitementMoyen = 0.0;
    }

    stats->lambda = (double)nombreRequetesRecues / deltaT;
    if (sommeTempsService > 0.0) {
        stats->mu = 1/(stats->tempsTraitementMoyen);
    } else {
        stats->mu = 0.0;
    }

    if (stats->mu > 0.0) {
        stats->rho = stats->lambda / stats->mu;
    } else {
        stats->rho = 0.0;
    }

    pthread_mutex_unlock(&mutexTampon);

}

int insererDonnee(struct requete *req){
    // Dans cette fonction, vous devez :
    //
    // Determiner a quel endroit copier la requete req dans le tampon circulaire
    //
    // Copier celle-ci
    //
    // Mettre a jour posEcriture et longueurCourante (toujours) et possiblement
    // posLecture (si vous vous etes "mordu la queue" et que vous etes revenu au
    // debut de votre tampon circulaire, il faut aussi repousser le pointeur de lecture
    // pour que le prochain element lu soit le plus ancien!)
    //
    // Mettre a jour les variables necessaires aux statistiques (comme nombreRequetesRecues, par exemple)
    //
    // N'oubliez pas de proteger les operations qui le necessitent par un mutex!
   
    // TODO (DONE)
    struct requete *destination;
    pthread_mutex_lock(&mutexTampon);

    // Trouver l'adresse de la case ou écrire
    destination = (struct requete *)(memoire + posEcriture * sizeof(struct requete));

    // Si on ecrase une entree existante avec un buffer alloue, on le libere d'abord
    if (longueurCourante == memoireTaille && destination->data != NULL) {
        free(destination->data);
        destination->data = NULL;
    }

    // Copier la requête dans le tampon
    *destination = *req;

    // Pour stats
    nombreRequetesRecues++;
    // Mise a jour variable global
    posEcriture = (posEcriture + 1) % memoireTaille;
    // Cas d'un tampon plein
    if (memoireTaille == longueurCourante) {
        // On peut pas traité la requete, elle est perdu
        nombreRequetesPerdues++;
        // On avance aussi le ptr de lecture, pcq lancien plus vieux est perdu
        posLecture = (posLecture + 1) % memoireTaille;
    } 
    else { // Le tampon est pas plein
        // On peut juste continuer d'increementer longueurCourante
        longueurCourante++;
    }

    pthread_mutex_unlock(&mutexTampon);

    return 0;
}

int consommerDonnee(struct requete *req){
    struct requete *source;

    pthread_mutex_lock(&mutexTampon);

    // Vérifier si une requête est disponible
    if (longueurCourante == 0) {
        pthread_mutex_unlock(&mutexTampon);
        return 0;
    }

    // Trouver la requête la plus ancienne
    source = (struct requete *)(memoire + posLecture * sizeof(struct requete));

    // Copier la requête vers la structure fournie
    *req = *source;

    // Mettre à jour le tampon circulaire
    posLecture = (posLecture + 1) % memoireTaille;
    longueurCourante--;

    pthread_mutex_unlock(&mutexTampon);

    return 1;
    
}

void enregistrerTempsService(double tempsService){
    if(tempsService <= 0.0){
        return;
    }

    pthread_mutex_lock(&mutexTampon);
    sommeTempsService += tempsService;
    nombreRequetesTraitees++;
    pthread_mutex_unlock(&mutexTampon);
}

unsigned int longueurFile(){
    // Retourne la longueur courante de la file contenue dans votre tampon circulaire.
    
    return longueurCourante;
}

void freeMemoireTampon(){
    if (mutexTamponInitialise) {
        pthread_mutex_lock(&mutexTampon);
    }

    // Liberer les buffers encore presents dans le tampon
    for (unsigned int i = 0; i < longueurCourante; i++) {
        unsigned int idx = (posLecture + i) % memoireTaille;
        struct requete *r = (struct requete *)(memoire + idx * sizeof(struct requete));
        free(r->data);
        r->data = NULL;
    }

    free(memoire);
    memoire = NULL;
    memoireTaille = 0;
    posLecture = 0;
    posEcriture = 0;
    longueurCourante = 0;

    if (mutexTamponInitialise) {
        pthread_mutex_unlock(&mutexTampon);
    }
}
