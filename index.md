---
title: "Laboratoire 5 : Émulation d'un périphérique USB et files d'attente"
---

## 1. Objectifs

Ce travail pratique vise les objectifs suivants :

1. Comprendre le fonctionnement d'un protocole de bas niveau tel que le *report protocol* du USB-HID;
2. Configurer un système embarqué pour émuler un périphérique tel qu'un clavier;
3. Utiliser une file d'attente pour gérer la charge sur un système embarqué;
4. Mesurer divers paramètres reliés à cette file d'attente et prédire le comportement futur du système;
5. Récapituler les notions de primitives de synchronisation vues dans le cours;

## 2. Présentation générale

Dans ce laboratoire, vous devrez faire en sorte que votre Raspberry Pi Zero soit reconnu comme un clavier par un autre ordinateur. En d'autres termes, vous devriez pouvoir brancher votre Raspberry Pi sur n'importe quel ordinateur supportant l'USB et vous faire passer pour un clavier. Les touches de clavier qui seront simulées proviennent de différents textes déjà dans la mémoire du Raspberry Pi. Vous devrez implémenter une file d'attente pour accommoder l'arrivée des messages qui peut être plus rapide que la vitesse à laquelle vous pouvez les transférer via le protocole de communication pour clavier.

## 3. Préparation matérielle

Dans ce laboratoire, nous utiliserons exceptionnellement le _second_ port USB, car celui habituellement utilisé ne peut servir qu'à alimenter le Raspberry Pi. Vous devrez donc brancher votre Raspberry Pi sur un ordinateur en utilisant *uniquement* le port étiquetté `USB` et **non** `PWR`. Le port `USB` peut _également_ être utilisé pour fournir l'alimentation au Raspberry Pi. Ainsi, l'ordinateur auquel vous connecterez votre Raspberry Pi fournira le courant nécessaire à son opération, comme il le ferait pour un clavier "normal". Vous aurez pour cela besoin d'un câble USB à USB-micro, que vous pouvez obtenir au service technique du 0103 (les câbles ne peuvent cependant pas sortir du laboratoire; si vous voulez expérimenter à la maison, utilisez votre propre câble).

> **Attention** : ne **jamais** brancher les 2 prises (`USB` et `PWR`) simultanément à deux appareils distincts (par exemple un ordinateur dans `USB` et l'adapteur d'alimentation du Raspberry Pi dans `PWR`). Les 5V et GND des deux ports sont communs sur le PCB du Raspberry Pi et vous risqueriez un court-circuit pouvant endommager votre Raspberry Pi et/ou votre ordinateur! De manière générale, vous ne devriez jamais avoir à brancher les deux prises simultanément, puisque le port étiquetté `USB` peut également alimenter le Raspberry Pi.

> Remarque : certains câbles USB-micro ne connectent _que_ les fils d'alimentation (5V et GND). Comme les broches D+ et D- (qui transportent les données) ne sont pas connectées, il est impossible de les utiliser pour ce laboratoire. Assurez-vous que votre ordinateur est capable de _voir_ le Raspberry Pi, tel qu'expliqué dans la section suivante, avant de tenter de transférer des caractères.


## 4. Préparation logicielle

Le Raspberry Pi Zero a ceci de particulier que son port USB peut à la fois servir comme *host* (à savoir qu'on y connecte des périphériques, comme vous l'avez fait avec un clavier au début de la session pour le configurer) ou comme *device* (à savoir qu'il attend les instructions d'un hôte, comme le ferait n'importe quel périphérique USB). Il faut cependant le configurer pour lui indiquer la configuration à adopter (le défaut est *host*). Pour ce faire, suivez les étapes suivantes :

1. Modifiez le fichier `/boot/config.txt` (vous pouvez par exemple utiliser la commande `sudo nano /boot/config.txt`). Juste **après** la ligne `[all]`, ajoutez : `dtoverlay=dwc2` sur une ligne distincte.
2. Modifiez le fichier `/etc/modules`. Après les lignes commençant par `#`, ajoutez `dwc2` sur une ligne distincte, puis `libcomposite` sur une autre ligne (autrement dit, n'écrivez pas `dwc2 libcomposite` sur la même ligne).
3. Redémarrez votre Raspberry Pi et assurez-vous qu'il est connecté correctement à un autre ordinateur (voir section 3).
4. Exécutez le fichier `activeUSB` présent dans votre répertoire `/home/pi` de l'image par défaut du cours. Si, pour une raison ou une autre, ce fichier n'est pas présent sur votre Raspberry Pi, vous pouvez le récupérer dans le dépôt Git. Cette exécution requiert les droits d'administrateur (donc `sudo ~/activeUSB`).
5. Validez que votre Raspberry Pi est maintenant détecté comme un périphérique USB. Sur Windows, vous pouvez consulter le Gestionnaire de périphériques. MacOS possède un outil similaire. Sur Linux, vous pouvez utiliser la commande `lsusb`. L'identifiant à chercher est "Linux Foundation Multifunction Composite Gadget". Si ce n'est pas le cas, vérifiez que vous avez bien effectué les étapes précédentes.

> Note : il se peut que Windows se plaigne de l'absence de pilote pour ce périphérique ou indique que "l'installation n'a pas pu être complétée". Ce n'est **pas** un problème pour nous.

L'exécution de `activeUSB` doit être _répétée_ à chaque redémarrage du Raspberry Pi. Si vous voulez éviter le risque de l'oublier, vous pouvez l'ajouter à votre fichier `/etc/rc.local`, comme vous avez fait pour DuckDNS lors du laboratoire 1. N'oubliez pas également que `activeUSB` doit être exécutable pour que cette procédure fonctionne. C'est le cas dans l'image par défaut du cours, mais au besoin vous pouvez utiliser la commande `chmod +x activeUSB` pour le rendre exécutable.

> Remarque : après ces modifications, vous ne pourrez _PAS_ utiliser le port USB de votre Raspberry Pi pour y brancher, par exemple, un clavier. Si vous voulez revenir au comportement par défaut, retirez simplement les lignes susmentionnées.

## 5. Énoncé

Une fois ces étapes de configuration complétées, vous pouvez commencer le laboratoire à proprement parler. Le code de base et le squelette de projet (incluant la configuration CMake) sont disponibles sur ce dépôt Git : [https://github.com/setr-ulaval/labo5-h26](https://github.com/setr-ulaval/labo5-h26). L'architecture générale du laboratoire est présentée à la figure suivante :

<img src="img/architecture.png" style="width:1162px"/>

Vous devrez donc coder les modules suivants :

- Dans le fichier `tamponCirculaire.c`, votre implémentation des fonctions déclarées dans `tamponCirculaire.h`. Votre tampon circulaire doit supporter des opérations _concourantes_ (provenant de plusieurs threads) et gérer les cas où l'écriture "rattrape" la lecture (en écrasant les données les plus vieilles). Il doit également produire certaines statistiques, en collaboration avec les autres modules. Voyez la description de chaque fonction dans `tamponCirculaire.h` pour plus de détails.
- Dans le fichier `emulateurClavier.c`, la fonction `ecrireCaracteres`, telle que déclarée dans `emulateurClavier.h`. Cette fonction doit accepter un pointeur de caractères (ASCII) en entrée et l'écrire sur le bus USB, pour que ces caractères soient lus par l'ordinateur qui considère le Raspberry Pi comme un clavier. Voyez la section 5.1 pour avoir plus de détails.
- Dans le fichier `main.c`, la fonction `main()`, responsable de l'initialisation et de la boucle d'affichage des statistiques. Par ailleurs, vous devez implémenter les fonctions `threadFonctionClavier()` et `threadFonctionLecture()`, qui, comme leurs noms l'indiquent, sont respectivement responsables de l'écriture (envoyer les données sur le bus USB) et de l'obtention des données. Voyez les sections 5.2 et 5.3 pour avoir plus de détails.


### 5.1 Conversion ASCII->USB-HID et écriture sur le bus

Les données que vous recevez sont en format ASCII (des chaînes de caractères standards). Elle ne peuvent contenir **que** :

- Les caractères alphabétiques (A à Z), majuscules ou minuscules
- Les chiffres (0 à 9)
- Les caractères de ponctuation suivants : la virgule (,) et le point (.)
- Le caractère d'espacement ( ) et le retour à la ligne (`\n`)

Tous les autres caractères *peuvent être ignorés* (et ne seront de toute façon pas produits si vous utilisez les données du cours).

Étant donné que les touches du clavier ne sont pas forcément associées à un seul caractère ASCII, le protocole USB encode ces données différemment, sous forme d'un _paquet_ de _8 octets_.

| Position | Octet 1 | Octet 2 | Octet 3 | Octet 4 | Octet 5 | Octet 6 | Octet 7 | Octet 8|
|---|---|---|---|---|---|---|---|---|
| **Fonction** | Touches *modificatrices* | Toujours 0 | 1ere touche pressée | 2e touche pressée | 3e touche pressée | 4e touche pressée | 5e touche pressée | 6e touche pressée |

Le premier octet contient l'information sur les touches modifiant les autres (Shift, Ctrl, Alt, etc.). Dans votre cas, le seul élément qui vous importe est que `0` représente "aucun modificateur" et que `2` représente "Left Shift enfoncé". Ces deux valeurs vous seront suffisantes pour afficher tous les caractères demandés. Le second octet n'est jamais utilisé dans ce laboratoire et *sa valeur doit toujours être fixée à `0`*.

Les autres octets encodent jusqu'à *6 touches* simultanément. Cet encodage se fait selon une table spéciale, que [vous pouvez trouver ici](https://www.win.tue.nl/~aeb/linux/kbd/scancodes-14.html). Vous devez donc _traduire_ le code ASCII des caractères que vous recevez en code USB-HID. Par exemple, si vous voulez envoyer le caractère `a`, il faut envoyer `4` dans un de ces octets représentant les pressions des touches. Si vous voulez envoyer `A` (majuscule), il faut envoyer *le même code*, mais utiliser le modificateur `2` dans le premier octet. Dans les octets 2 à 8, la valeur `0` est spéciale et signale "aucune touche". Cela permet d'envoyer un paquet "incomplet", ce qui peut parfois être utile : par exemple, si vous avez la chaîne de caractères `Aa` à envoyer, il est impossible de le faire avec un seul paquet, parce que le modificateur s'applique à _toutes_ les touches indiquées dans le paquet. Il est donc uniquement possible de mettre en commun des caractères qui ont la même _casse_ (majuscules vs minuscules et chiffres).

> **Note importante** : le paquet décrit plus haut sert à indiquer à l'ordinateur que des touches sont _pressées_. Il faut ensuite les _relâcher_, sinon l'ordinateur va considérer que la touche est encore pressée et les répéter à l'infini. Pour les relâcher, il suffit d'envoyer un paquet avec _tous les octets à 0_.

> **Note** : l'ordre des lettres dans le paquet a de l'importance! L'ordinateur considère le contenu du paquet comme une liste ordonnée. Si vous voulez envoyer la chaîne `abcde` par exemple, vous devez envoyer `a` à l'octet 2, `b` à l'octet 3, et ainsi de suite. Inverser l'ordre inversera aussi la façon dont l'ordinateur recevra les caractères!

Pour écrire sur le bus USB, il suffit d'utiliser la fonction `fwrite` en utilisant le pointeur de fichier `periphClavier`, que vous recevez en argument. Ce fichier est ouvert pour vous par la fonction `initClavier`, déjà codée pour vous. Vous devez toutefois appeler cette fonction dans votre `main()` et passer correctement sa valeur de retour comme argument à `ecrireCaracteres`.

> À savoir : contrairement à `write()`, `fwrite()` vous _garantit_ l'écriture des 8 octets d'un coup. Vous n'avez donc pas à faire de boucle autour de cette fonction (vous devriez tout de même vérifier sa valeur de retour, pour être certain que l'appel n'échoue pas).

#### Délai d'envoi

La vitesse du bus USB est en pratique suffisament élevée pour qu'il ne soit jamais complètement occupé par un simple clavier. Toutefois, dans le but de tester différents scénarios de files d'attente, **vous devez ajouter un délai artificiel lors de l'envoi de chaque paquet** (pas chaque caractère, chaque paquet). Vous devrez pour cela appeler la fonction `usleep` dans `ecrireCaracteres`, en lui fournissant le délai artificiel qu'elle doit produire (il vous sera indiqué sur la ligne de commande, comme nous le verrons dans la section 5.5). N'oubliez pas que cette fonction doit être appelée à chaque fois que vous envoyez un _paquet_ : vous avez donc tout avantage à "remplir" autant que possible ces paquets.


### 5.2 Thread préparant l'écriture des données sur le bus USB

Ce thread est implémenté dans le fichier `main.c`, dans la fonction `threadFonctionClavier`. Voyez les commentaires dans la fonction pour une explication détaillée des étapes à suivre, mais globalement, vous devez :

1. Récupérer les arguments que le thread principal vous fournit, sous forme d'un `void*` qui est en fait un pointeur vers une `struct infoThreadClavier`.
2. Attendre sur la *barrière* fournit en argument, pour assurer que le thread clavier et le thread lecteur commencent leur boucle critique en même temps.
3. Boucler sans fin en testant si une requête est disponible dans le tampon circulaire (en utilisant `consommerDonnee()`). Si oui, appeler `ecrireCaracteres` avec les bons paramètres (y compris le délai artificiel à ajouter par paquet). Si non, attendez une durée courte, mais arbitraire, en utilisant `usleep` (par exemple 500 microsecondes).
4. Ce thread n'a pas besoin de gérer une éventuelle fin du programme principal.

### 5.3 Thread récupérant les données depuis le pipe nommé

Ce thread est implémenté dans le fichier `main.c`, dans la fonction `threadFonctionLecteur`. Celle-ci doit lire les données depuis un *pipe nommé* (named pipe). De votre point de vue, cela ressemble fortement à un fichier, qui se remplit au fur et à mesure. C'est un autre programme, `createurRequetes`, qui se charge de créer le pipe nommé et de le remplir au fur et à mesure. Voyez la section 5.6 à ce sujet. Vous pouvez consulter les commentaires dans la fonction pour une explication détaillée des étapes à suivre, mais globalement, vous devez :

1. Récupérer les arguments que le thread principal vous fournit, sous forme d'un `void*` qui est en fait un pointeur vers une `struct infoThreadLecteur`.
2. Attendre sur la *barrière* fournit en argument, pour assurer que le thread clavier et le thread lecteur commencent leur boucle critique en même temps.
3. Utiliser `select()` pour éviter d'avoir à faire une attente active sur le pipe nommé.
4. Lorsque des données sont disponibles, *lire et accumuler les données (des caractères ASCII) jusqu'à ce que vous receviez le caractère ASCII EOT* (End of Transmission). Ce caractère signifie la fin d'un message. Vous devez alors empaqueter ce message dans une `struct requete` et insérer cette requête dans le tampon circulaire (en utilisant `insererDonnee()`).
5. Ce thread n'a pas besoin de gérer une éventuelle fin du programme principal.

### 5.4 Programme principal, statistiques et affichage

Le programme principal doit d'abord initialiser les différentes structures et lancer les threads lecteur et clavier. Voyez les commentaires dans la fonction `main()` à ce sujet. Une fois cela fait, vous devez entrer dans une boucle infinie qui va afficher des *statistiques* sur le système et l'état de sa file d'attente. La fonction d'affichage est déjà codée pour vous (dans `utils.c`, `afficherStats()`). Vous devez cependant calculer les dites statistiques et les mettre dans une `struct statistiques` que vous passerez par pointeur à cette fonction d'affichage. La plupart des variables nécessaires au calcul de ces statistiques seront en fait modifiées par les fonctions d'accès au tampon circulaire. Voyez les variables globales définies en haut du fichier `tamponCirculaire.c` pour vous donner une idée de ce que vous devriez conserver comme information.

La plupart des statistiques ont des noms parlants, mais n'hésitez pas à vous référer aux notes de cours sur les files d'attente pour vous rafraîchir la mémoire sur ce que sont mu, lambda et rho.


### 5.5 Arguments et ligne de commande

Votre programme doit accepter trois arguments sur la ligne de commande :

- `argv[1]` contient le chemin vers le fichier virtuel du pipe nommé à utiliser.
- `argv[2]` contient le nombre de micro-secondes (valeur entière) à attendre lors de l'envoi de chaque paquet (voir section 5.1). Vous pouvez utiliser la fonction `atoi()` pour la convertir en un nombre.
- `argv[3]` est la taille du tampon circulaire à utiliser (valeur entière). Vous pouvez utiliser la fonction `atoi()` pour la convertir en un nombre.


### 5.6 Créateur de requêtes

Vous aurez remarqué que nous parlons beaucoup de "requêtes", mais sans avoir défini ce qui les générait : que doit-on connecter à l'autre bout du pipe nommé? Nous vous fournissons un _exécutable_ capable de le faire. Cet exécutable, que vous pouvez trouver dans `src/createurRequetes/createurRequetes`, se lance à l'aide de la ligne de commande et requiert *7 arguments* :

```
./createurRequetes chemin_named_pipe freq_min freq_max nbr_carac_min nbr_carac_max mode verbose
```

Dans le détail:

1. `chemin_named_pipe` contient le chemin où l'exécutable doit créer le fichier virtuel du pipe nommé. C'est ce même argument qui devra être passé en première position à votre programme.
2. `freq_min` et `freq_max` indiquent respectivement fréquence _minimale_ et _maximale_ des messages en *messages par minutes*. Par exemple, si `freq_min=40` et `freq_max=80`, le `createurRequetes` générera entre 40 et 80 messages à chaque minute. Le nombre peut varier car une partie du programme est probabiliste (à chaque message envoyé, il prend une pause qui lui permet de rester *en moyenne* dans la plage de fréquence demandée, mais cette pause n'est pas toujours égale).
3. `nbr_carac_min` et `nbr_carac_max` indiquent le nombre _minimum_ et _maximum_ de caractères par message (nombre de caractère avant le `EOT`). Comme pour la fréquence, ce nombre est probabiliste.
4. `mode` permet de choisir quel type de texte recevoir. Vous pouvez sélectionner un mode en utilisant un chiffre entre 0 et 3 (voir le tableau plus bas).
5. `verbose` peut prendre la valeur `0` ou `1`. Si `1`, des messages seront affichés sur la console pour vous indiquer ce qui vient d'être envoyé. Cela peut vous être utile pour le débogage.

| Mode | Valeur |
|---|---|
| 0 | Texte réel choisi au hasard parmi une sélection de courriels importants dans l'histoire de votre système Linux.|
| 1 | Test simple : `abcdefghijklmnopqrstuvwxyz\nABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ,.\n`|
| 2 | Test difficile : `AbCdEfGhIjKlMnOpQrStUvWxYzA1B2C3D4E5F6G7H8I9J0K,L.M NO\naBCDeFGHiJKLmNOPqrsTuvwXyZ9,8,7.6.5.4,3,2 1\n`|
| 3 | Recopier l'entrée standard (vous pouvez écrire dans le terminal et presser Enter pour envoyer le texte)|

Nous vous suggérons de commencer par tester votre programme avec le mode `1`, qui est le plus "facile" et aussi celui où une erreur est le plus aisément détectable. Par la suite, vous pouvez essayer le mode `0` et, quand vous considérez votre code suffisament robuste, le mode `2`. Le mode `3` est fourni pour votre amusement et vous ne serez normalement pas évalué avec celui-ci : n'oubliez pas toutefois de vous limiter aux caractères autorisés!

> Note : le créateur de requête quitte automatiquement lorsque l'autre processus ferme le *named pipe*. N'oubliez pas de le relancer si votre programme plante et que vous lancez vos programmes manuellement en console (les scripts de débogage le font déjà pour vous).

### 5.7 Exécution et débogage

Comme pour les laboratoires 1 à 3, vous pouvez utiliser le débogage de VScode pour tester votre code de manière plus pratique. Sachez toutefois que le débogueur est limité dans son support des threads : il vous est suggéré de commencer par implémenter une version _série_ (sans thread, où les opérations de lecture et d'écriture se font en alternance) afin de valider et déboguer la logique de votre code, _puis_ de passer en mode multi-threads. Le script de lancement du débogage lance automatiquement `createurRequetes` avec votre programme, avec des paramètres définis dans `syncAndStartGDB.sh`. Si vous voulez, vous pouvez également lancer les programmes séparément, en vous connectant par SSH à votre Raspberry Pi et en lançant `createurRequetes` vous-mêmes.

> **Attention** : le programme principal (`emulateurClavier`) requiert les droits administrateurs pour écrire sur le bus USB. Nos scripts le lancent déjà ainsi, mais si vous le lancez vous-mêmes sur la ligne de commande, utilisez `sudo`!


## 6. Modalités d'évaluation

Ce travail doit être réalisé **en équipe de deux**, la charge de travail étant à répartir équitablement entre les deux membres de l'équipe. Aucun rapport n'est à remettre, mais vous devez soumettre votre code source dans monPortail avant le **25 mars 2026, 23h59**. Ensuite, lors de la séance de laboratoire du **27 mars 2026, 9h30**, les **deux** équipiers doivent être présents pour l'évaluation individuelle de 30 minutes. Si vous ne pouvez pas vous y présenter, contactez l'équipe pédagogique du cours dans les plus brefs délais afin de convenir d'une date d'évaluation alternative. Ce travail compte pour **12%** de la note totale du cours.

Notre évaluation se fera sur le Raspberry Pi de l'enseignant ou de l'assistant et comprendra notamment les éléments suivants:

  1. La sortie de compilation d'un *CMake: Clean Rebuild*;
  2. Le lancement de votre programme avec un flot de requête "simple" (par exemple `./createurRequetes fichierComm 50 70 10 20 1 0` et `./emulateurClavier fichierComm 1000 10`), où tous les caractères devraient être affichés sans problème;
  3. Le lancement de votre programme avec un flot de requête "limite" (par exemple `./createurRequetes fichierComm 240 300 20 30 2 0` et `./emulateurClavier fichierComm 10000 10`), où tous les caractères devraient également être affichés, mais avec un `rho` nettement plus près de 1.0;
  4. Le lancement de votre programme avec un flot de requête "excessif" (par exemple `./createurRequetes fichierComm 300 360 20 30 2 0` et `./emulateurClavier fichierComm 20000 10`), où le tampon circulaire devrait finir par se remplir puis par "manquer" des caractères, mais sans que votre programme n'en soit autrement affecté.

### 6.1. Barème d'évaluation

Le barême d'évaluation détaillé sera le suivant (laboratoire noté sur 20 points) :

#### 6.1.1. Qualité du code remis (8 points)

* (4 pts) Le code C est valide, complet et ne contient pas d'erreurs empêchant le bon déroulement des programmes.
* (1 pts) La compilation se fait sans avertissement (*warning*) de la part du compilateur.
* (3 pts) La synchronisation entre les différents threads est correctement implémentée et correspond à ce qui est demandé (barrière à l'entrée des threads, mutex pour protéger le tampon circulaire, etc.)

#### 6.1.2. Validité de la solution (12 points)

> **Attention** : un programme ne compilant pas obtient automatiquement une note de **zéro** pour cette section.

* (3 pts) Votre programme est en mesure d'envoyer des caractères sur l'ordinateur sur lequel le Raspberry Pi est branché
* (4 pts) Les caractères affichés sont corrects (pas de coupure, mauvais caractère, ou caractère manquant -- sauf en cas de dépassement de la capacité du tampon circulaire, bien sûr)
* (3 pts) Les statistiques sont affichées correctement dans le terminal, à toutes les 2 secondes, et correspondent aux valeurs attendues
* (2 pts) L'envoi des caractères sur le bus USB est efficace (utilisation maximisée des paquets)

#### 6.1.3. Évaluation individuelle

Une évaluation individuelle écrite portant sur le laboratoire sera tenue, *en personne*, à la séance d'atelier du 27 mars 2026, à 9h30. La note obtenue à cette évaluation deviendra un facteur multiplicatif appliqué individuellement sur la note d'équipe. Par exemple, une note de 75% à l'évaluation individuelle combinée à une note de 90% pour le code remis résultera en une note de 0.75*0.90 = 67.5%. Une absence non-motivée à cette évaluation entraîne une note (et donc un facteur multiplicatif) de 0.

#### 6.1.4. Questionnaire sur l'utilisation de l'IA

Votre remise _doit_ inclure le fichier `UTILISATION_IA.txt` dûment complété. Les réponses ne sont pas évaluées en tant que telles, mais ne pas remettre ce fichier (ou le remettre dans son état initial, sans modifications et réponses aux questions) entraîne une pénalité automatique de 10% sur la note d'équipe.

## 7. Ressources et lectures connexes

* [Le dépôt Git contenant les fichiers de base](https://github.com/setr-ulaval/labo5-h26)
* [La table des caractères USB-HID](https://www.win.tue.nl/~aeb/linux/kbd/scancodes-14.html)
* [La table ASCII](https://www.asciitable.com/)
* [Un article expliquant comment transformer le Raspberry Pi Zero en clavier, source d'inspiration pour ce laboratoire](https://randomnerdtutorials.com/raspberry-pi-zero-usb-keyboard-hid/)


