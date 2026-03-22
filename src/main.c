/******************************************************************************
 * Laboratoire 5
 * GIF-3004 Systèmes embarqués temps réel
 * Hiver 2026
 * Marc-André Gardner
 * 
 * Fichier principal
 ******************************************************************************/

#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "utils.h"
#include "emulateurClavier.h"
#include "tamponCirculaire.h"



static void* threadFonctionClavier(void* args){
    // Implementez ici votre fonction de thread pour l'ecriture sur le bus USB
    // La premiere des choses est de recuperer les arguments (deja fait pour vous)
    struct infoThreadClavier *infos = (struct infoThreadClavier *)args;

    // Vous devez ensuite attendre sur la barriere passee dans les arguments
    // pour etre certain de commencer au meme moment que le thread lecteur

    // TODO

    // Finalement, ecrivez dans cette boucle la logique du thread, qui doit:
    // 1) Tenter d'obtenir une requete depuis le tampon circulaire avec consommerDonnee()
    // 2) S'il n'y en a pas, attendre un cours laps de temps (par exemple usleep(500))
    // 3) S'il y en a une, appeler ecrireCaracteres avec les informations requises
    // 4) Liberer la memoire du champ data de la requete avec la fonction free(), puisque
    //      la requete est maintenant terminee

    while(1){
       // TODO
    }
    return NULL;
}

static void* threadFonctionLecture(void *args){

    // Implementez ici votre fonction de thread pour la lecture sur le named pipe
    // La premiere des choses est de recuperer les arguments (deja fait pour vous)
    struct infoThreadLecture *infos = (struct infoThreadLecture *)args;
    
    // Ces champs vous seront utiles pour l'appel a select()
    fd_set setFd;
    int nfds = infos->pipeFd + 1;

    // Vous devez ensuite attendre sur la barriere passee dans les arguments
    // pour etre certain de commencer au meme moment que le thread lecteur

    // TODO

    // Finalement, ecrivez dans cette boucle la logique du thread, qui doit:
    // 1) Remplir setFd en utilisant FD_ZERO et FD_SET correctement, pour faire en sorte
    //      d'attendre sur infos->pipeFd
    // 2) Appeler select(), sans timeout, avec setFd comme argument de lecture (on veut bien
    //      lire sur le pipe)
    // 3) Lire les valeurs sur le named pipe
    // 4) Si une de ses valeurs est le caracteres ASCII EOT (0x4), alors c'est la fin d'un
    //      message. Vous creez alors une nouvelle requete et utilisez insererDonnee() pour
    //      l'inserer dans le tampon circulaire. Notez que le caractere EOT ne doit PAS se
    //      retrouver dans le champ data de la requete! N'oubliez pas egalement de donner
    //      la bonne valeur aux champs taille et tempsReception.

    while(1){
        // TODO
    }
    return NULL;
}

int main(int argc, char* argv[]){
    if(argc < 4){
        printf("Pas assez d'arguments! Attendu : ./emulateurClavier cheminPipe tempsAttenteParPaquet tailleTamponCirculaire\n");
    }

    // A ce stade, vous pouvez consider que:
    // argv[1] contient un chemin valide vers un named pipe
    // argv[2] contient un entier valide (que vous pouvez convertir avec atoi()) representant le nombre de microsecondes a
    //      attendre a chaque envoi de paquet
    // argv[3] contient un entier valide (que vous pouvez convertir avec atoi()) contenant la taille voulue pour le tampon
    //      circulaire

    // Vous avez plusieurs taches d'initialisation a faire :
    //
    // 1) Ouvrir le named pipe

    // TODO : test
    const char *chemin_pipe = argv[1];
    int fd = open(chemin_pipe, O_RDONLY); // TODO: confirmer si ouvrir en écriture nécessaire
    if (fd == -1){
        perror("Erreur d'ouverture du pipe");
        return -1;
    }

    // création objet FILE pour passer en argument au thread clavier
    FILE* file_fd = fdopen(fd, "r");
    if (file_fd == NULL){
        perror("Erreur de conversion du descripteur de fichier en FILE*");
        close(fd);
        return -1;
    }

    // 2) Declarer et initialiser la barriere
    
    // TODO : test
    pthread_barrier_t barriere;
    int res = pthread_barrier_init(barriere, NULL, 2); // count=2 (thread clavier et lecteur)
    if (res != 0){
        perror("Erreur d'initialisation de la barrière");
        close(fd);
        return -1;
    }

    // 3) Initialiser le tampon circulaire avec la bonne taille

    // TODO : test
    size_t taille_tampon = atoi(argv[3]);
    res = initTamponCirculaire(taille_tampon);
    if (res == -1){
        perror("Erreur d'initialisation du tampon");
        close(fd);
        pthread_barrier_destroy(&barriere);
        return -1;
    }
    // 4) Creer et lancer les threads clavier et lecteur, en leur passant les bons arguments dans leur struct de configuration respective
    
    // TODO : test
    pthread_t clavier;
    struct infoThreadClavier *infosClavier;
    infosClavier->barriere = &barriere;
    infosClavier->pointeurClavier = file_fd;
    infosClavier->tempsTraitementParCaractereMicroSecondes = atoi(argv[2]);
    res = pthread_create(&clavier, NULL, threadFonctionClavier, (void *)infosClavier);
    if (res == 0){
        pthread_join(clavier, NULL);
    }
    else {
        fprintf(stderr, "Erreur de création du thread clavier: %s\n", strerror(res));
        close(fd);
        pthread_barrier_destroy(&barriere);
        freeMemoireTampon();
        return -1;
    }

    pthread_t lecteur;
    struct infoThreadLecture *infosLecteur;
    infosLecteur->barriere = &barriere;
    infosLecteur->pipeFd = fd;
    res = pthread_create(&lecteur, NULL, threadFonctionLecture, (void *)infosLecteur);
    if (res == 0){
        pthread_join(lecteur, NULL);
    }
    else {
        fprintf(stderr, "Erreur de création du thread lecteur: %s\n", strerror(res));
        close(fd);
        pthread_barrier_destroy(&barriere);
        freeMemoireTampon();
        pthread_cancel(clavier);
        return -1;
    }

    // La boucle de traitement est deja implementee pour vous. Toutefois, si vous voulez eviter l'affichage des statistiques
    // (qui efface le terminal a chaque fois), vous pouvez commenter la ligne afficherStats().
    struct statistiques stats;
    double tempsDebut = get_time();
    while(1){
        // Affichage des statistiques toutes les 2 secondes
        calculeStats(&stats);
        afficherStats((unsigned int)(round(get_time() - tempsDebut)), &stats);
        resetStats();
        usleep(2e6);
    }
    return 0;
}
