#ifndef EMULATEUR_CLAVIER_H
#define EMULATEUR_CLAVIER_H

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

// Nom du fichier virtuel dans lequel vous pouvez ecrire pour envoyer vos signaux de clavier
#define FICHIER_CLAVIER_VIRTUEL "/dev/hidg0"

// Longueur de chaque paquet USB HID contenant l'information sur les touches pressees
// Cette valeur est constante (definie dans la norme USB)
#define LONGUEUR_USB_PAQUET 8

// Cette fonction initialise le clavier en ouvrant le fichier virtuel en ecriture et en retournant
// le pointeur de fichier correspondant.
// Elle est deja implementee pour vous dans emulateurClavier.c
FILE* initClavier();

/*
Fonction ecrivant les caracteres sur le bus USB. Elle recoit les arguments suivants:
    - periphClavier : pointeur de fichier virtuel USB, tel que retourne par initClavier(). C'est sur
                        ce pointeur que vous pouvez appeler fwrite() pour envoyer les donnees.
    - caracteres : pointeur vers les caracteres a copier (ASCII). Ce buffer est _garanti_ de
                        contenir uniquement les caracteres autorises (A-Za-z0-9., \n).
    - len : longueur du buffer caracteres (nombre de caracteres a envoyer, _sans compter_ un eventuel
                        caractere nul a la fin).
    - tempsTraitementParPaquetMicroSecondes : la duree que vous devez attendre (artificiellement, en
                        utilisant usleep() avec cette valeur) à chaque fois que vous envoyez un _paquet_.
                        Notez que cette valeur est la même peu-importe le nombre de caractères transmis
                        dans ce paquet (vous avez donc avantage à les remplir le plus possible!)
                        Finalement, bien que vous devez faire _2_ fwrite() pour envoyer un ensemble de
                        caracteres (un premier avec les caracteres, et le second rempli de 0 pour
                        "relacher les touches"), vous n'avez a attendre qu'une seule fois. En termes plus
                        techniques, cette portion de votre fonction d'envoi devrait ressembler a:
                            fwrite(vosdonnees, LONGUEUR_USB_PAQUET, 1, periphClavier);
                            fwrite(bufferRempliDeZeros, LONGUEUR_USB_PAQUET, 1, periphClavier);
                            usleep(tempsTraitementParPaquetMicroSecondes);
    
    Cette fonction retourne -1 si une erreur est survenue, ou le nombre de caracteres ecrits si c'est
    un success. En temps normal, son retour devrait donc etre egal a son argument len.
    
    N'oubliez pas que vous ne pouvez PAS envoyer directement les caracteres ASCII sur le bus USB, vous
    devez les convertir tel qu'explique dans l'enonce!
*/
int ecrireCaracteres(FILE* periphClavier, const char* caracteres, size_t len, unsigned int tempsTraitementParPaquetMicroSecondes);

#endif