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
#include <errno.h>

#include "utils.h"
#include "emulateurClavier.h"
#include "tamponCirculaire.h"



static void* threadFonctionClavier(void* args){
    // Implementez ici votre fonction de thread pour l'ecriture sur le bus USB
    // La premiere des choses est de recuperer les arguments (deja fait pour vous)
    struct infoThreadClavier *infos = (struct infoThreadClavier *)args;

    // Vous devez ensuite attendre sur la barriere passee dans les arguments
    // pour etre certain de commencer au meme moment que le thread lecteur
    
    // TODO : test
    pthread_barrier_wait(infos->barriere);
    // Finalement, ecrivez dans cette boucle la logique du thread, qui doit:
    // 1) Tenter d'obtenir une requete depuis le tampon circulaire avec consommerDonnee()
    // 2) S'il n'y en a pas, attendre un cours laps de temps (par exemple usleep(500))
    // 3) S'il y en a une, appeler ecrireCaracteres avec les informations requises
    // 4) Liberer la memoire du champ data de la requete avec la fonction free(), puisque
    //      la requete est maintenant terminee

    while(1){
        struct requete req = {0};
        int ret = consommerDonnee(&req);
        if(ret == 1){
            double debutService = get_time();
            int rep = ecrireCaracteres(infos->pointeurClavier, req.data, req.taille, infos->tempsTraitementParCaractereMicroSecondes);
            double finService = get_time();

            if(rep >= 0){
                enregistrerTempsService(finService - debutService);
            }
            //printf("%i\n", rep);
            free(req.data);
        } else if(ret == 0){
            usleep(500);
        }
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

    // TODO : test
    pthread_barrier_wait(infos->barriere);
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
    char buff[100];
    char *message = malloc(256);
    size_t messageCapacite = 256;
    size_t messageTaille = 0;

    if(message == NULL){
        perror("Erreur d'allocation memoire");
        return NULL;
    }

    while(1){
        FD_ZERO(&setFd);
        FD_SET(infos->pipeFd, &setFd);
        int s = select(nfds, &setFd, NULL, NULL, NULL); // dernier NULL : pas de timeout
        if (s < 0){
            if(errno == EINTR){
                continue;
            }
            perror("Erreur select");
            break;
        }

        if (s > 0 && FD_ISSET(infos->pipeFd, &setFd)){
            ssize_t n = read(infos->pipeFd, buff, sizeof(buff));

            if (n < 0){
                if(errno == EINTR){
                    continue;
                }
                perror("Erreur read");
                break;
            }

            if (n == 0){
                usleep(500);
                continue;
            }

            for (ssize_t i = 0; i < n; i++){
                if ((unsigned char)buff[i] == 0x4){
                    struct requete req = {0};
                    req.tempsReception = get_time();
                    req.taille = messageTaille;
                    req.data = NULL;

                    if (messageTaille > 0){
                        req.data = malloc(messageTaille);
                        if(req.data == NULL){
                            perror("Erreur d'allocation memoire");
                            messageTaille = 0;
                            continue;
                        }
                        memcpy(req.data, message, messageTaille);
                        //printf("%s", message);
                    }

                    if(insererDonnee(&req) != 0){
                        free(req.data);
                    }

                    messageTaille = 0;
                } else {
                    if (messageTaille == messageCapacite){
                        size_t nouvelleCapacite = messageCapacite * 2;
                        char *nouveauMessage = realloc(message, nouvelleCapacite);
                        if(nouveauMessage == NULL){
                            perror("Erreur de reallocation memoire");
                            messageTaille = 0;
                            continue;
                        }
                        message = nouveauMessage;
                        messageCapacite = nouvelleCapacite;
                    }
                    message[messageTaille++] = buff[i];
                }
            }
        }
    }

    free(message);
    return NULL;
}

int main(int argc, char* argv[]){
    if(argc < 4){
        printf("Pas assez d'arguments! Attendu : ./emulateurClavier cheminPipe tempsAttenteParPaquet tailleTamponCirculaire\n");
        return -1;
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
    //printf("Début programme.\n");
    // TODO : test
    const char *chemin_pipe = argv[1];
    int fd = open(chemin_pipe, O_RDONLY); // TODO: confirmer si ouvrir en écriture nécessaire
    if (fd == -1){
        perror("Erreur d'ouverture du pipe");
        return -1;
    }
    //printf("Pipe ouvert avec succès.\n");

    // 2) Declarer et initialiser la barriere
    
    // TODO : test
    pthread_barrier_t barriere;
    int res = pthread_barrier_init(&barriere, NULL, 2); // count=2 (thread clavier et lecteur)
    if (res != 0){
        perror("Erreur d'initialisation de la barrière");
        close(fd);
        return -1;
    }
    //printf("Barrière initialisée avec succès.\n");

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
    //printf("Tampon circulaire initialisé avec succès.\n");
    // 4) Creer et lancer les threads clavier et lecteur, en leur passant les bons arguments dans leur struct de configuration respective
    
    // TODO : test
    // Création du pointeur vers le clavier virtuel USB
    FILE* file_fd = initClavier();
    if (file_fd == NULL){
        perror("Erreur d'ouverture du clavier virtuel");
        close(fd);
        return -1;
    }
    //printf("Clavier virtuel ouvert avec succès.\n");

    pthread_t clavier;
    struct infoThreadClavier infosClavier = {0};
    infosClavier.barriere = &barriere;
    infosClavier.pointeurClavier = file_fd;
    infosClavier.tempsTraitementParCaractereMicroSecondes = atoi(argv[2]);
    res = pthread_create(&clavier, NULL, threadFonctionClavier, (void *)&infosClavier);
    if (res != 0) {
        fprintf(stderr, "Erreur de création du thread clavier: %s\n", strerror(res));
        close(fd);
        fclose(file_fd);
        pthread_barrier_destroy(&barriere);
        freeMemoireTampon();
        return -1;
    }
    //printf("Thread clavier créé avec succès.\n");

    pthread_t lecteur;
    struct infoThreadLecture infosLecteur = {0};
    infosLecteur.barriere = &barriere;
    infosLecteur.pipeFd = fd;
    res = pthread_create(&lecteur, NULL, threadFonctionLecture, (void *)&infosLecteur);
    if (res != 0) {
        fprintf(stderr, "Erreur de création du thread lecteur: %s\n", strerror(res));
        pthread_cancel(clavier);
        pthread_join(clavier, NULL);
        close(fd);
        fclose(file_fd);
        pthread_barrier_destroy(&barriere);
        freeMemoireTampon();
        return -1;
    }
    //printf("Thread lecteur créé avec succès.\n");

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
