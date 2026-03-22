/******************************************************************************
 * Laboratoire 5
 * GIF-3004 Systèmes embarqués temps réel
 * Hiver 2026
 * Marc-André Gardner
 * 
 * Fichier implémentant les fonctions de l'emulateur de clavier
 ******************************************************************************/

#include "emulateurClavier.h"

FILE* initClavier(){
    // Deja implementee pour vous
    FILE* f = fopen(FICHIER_CLAVIER_VIRTUEL, "wb");
    setbuf(f, NULL);        // On desactive le buffering pour eviter tout delai
    return f;
}

// Fonction custom ajouter pour la conversion ASCII vers HID
// sa rend ca plus simple a ecrire (fonction moins grosse)
/*
Params :
    c : caractere que on veut envoyer sur le clavier
    *modificateur : modificateur associer au clavier (On utilise 0 pour normal pis 2 si char en MAJ)
    *code : code de la conversion HID

    return : 0 si ok, -1 si sa chie
*/
static int asciiVersHid(char c, unsigned char *modificateur, unsigned char *code){
    *modificateur = 0;

    // Lettres minuscules
    if (c >= 'a' && c <= 'z') {
        *code = 4 + (c - 'a'); // Itere du code 4 a 29
        return 0;
    }

    // Lettres majuscules
    if (c >= 'A' && c <= 'Z') {
        *code = 4 + (c - 'A'); // Itere du code 4 a 29
        *modificateur = 2;   // Left Shift
        return 0;
    }

    // Chiffres
    if (c >= '1' && c <= '9') {
        *code = 30 + (c - '1');
        return 0;
    }
    // 0 est apres le 9 dans la table
    if (c == '0') {
        *code = 39;
        return 0;
    }

    // Enter
    if (c == '\n') {
        *code = 40;
        return 0;
    }

    // Space
    if (c == ' ') {
        *code = 44;
        return 0;
    }

    // Virgule
    if (c == ',') {
        *code = 54;
        return 0;
    }

    // Point
    if (c == '.') {
        *code = 55;
        return 0;
    }

    // Car que on traite pas
    return -1;
}

int ecrireCaracteres(FILE* periphClavier, const char* caracteres, size_t len, unsigned int tempsTraitementParPaquetMicroSecondes){
    // TODO ecrivez votre code ici. Voyez les explications dans l'enonce et dans
    // emulateurClavier.h
    
    // Sert à parcourir le buffer caracteres (Indice)
    size_t i;

    // Compteur de resultats (au debut on a rien ecrit)
    int nbCaracteresEcrits = 0;

    // Si on a pas de peripherique ou pas de caracteres a ecrire
    if (periphClavier == NULL || caracteres == NULL) {
        return -1;
    }

    // On regarde pour chaque character dans caracteres
    while(i < len) {
        // Cree 2 tableaux de 8 octets (tous a 0)
        unsigned char paquet[LONGUEUR_USB_PAQUET] = {0};
        unsigned char paquetVide[LONGUEUR_USB_PAQUET] = {0};

        // Nombre de touches qu'on a dans le paquet
        int nbTouchesDansPaquet = 0;

        // Variable pour asciiVersHid
        unsigned char modificateurPaquet;
        unsigned char code;
        // Securite de conversion
        if (asciiVersHid(caracteres[i], &modificateurPaquet, &code) != 0) {
            return -1;
        }

        paquet[0] = modificateurPaquet;  // 0 ou 2
        paquet[1] = 0;             // toujours 0
        paquet[2] = code;          // une touche envoyée
        // Tant que on a encore des char a envoyer et que y reste de la place dans le paquet
        // 8 octets, 2 sont deja utilise de base, on a de place pour 6 du meme type
        while (i < len && nbTouchesDansPaquet < 6) {
            unsigned char modificateurCourant;
            unsigned char codeCourant;

            // Conversion avec securite
            if (asciiVersHid(caracteres[i], &modificateurCourant, &codeCourant) != 0) {
                return -1;
            }

            // On break le while si on a un autre modificateur que celui du paquet
            // Sinon on va pas ecrire les bons char (ex min a la place de MAJ)
            if (modificateurCourant != modificateurPaquet) {
                break;
            }

            // On place le code courant dans le paquet
            paquet[2 + nbTouchesDansPaquet] = codeCourant;
            nbTouchesDansPaquet++;
            i++;
            nbCaracteresEcrits++;
        }

        // On regarde si les ecritures ont marche (fwrite renvoit 1 en cas de success)

        // Ecriture (touche enfoncee)
        if (fwrite(paquet, LONGUEUR_USB_PAQUET, 1, periphClavier) != 1) {
            return -1;
        }
        // Ecriture (touche relachee)
        if (fwrite(paquetVide, LONGUEUR_USB_PAQUET, 1, periphClavier) != 1) {
            return -1;
        }
        // Temps de traitement simululer
        usleep(tempsTraitementParPaquetMicroSecondes);
    }

    return nbCaracteresEcrits;
}
