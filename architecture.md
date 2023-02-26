## 1. Architecture logicielle

L'application "interpreteur-de-commandes" comprend six fichiers en C et un fichier makefile pour la compilation. En exécutant make à la racine du dépôt, l'exécutable `slash` sera créé dans ce même répertoire et peut être lancé avec `./slash`. La commande `make cleanall` effacera tous les fichiers générés.

Le fichier principal du programme est `slash.c`. Le fichier `lib.c` contient des fonctions utilisées pour gérer les fuites de mémoire ainsi que les tableaux de caractères et les structures. Les autres fichiers sont utilisés par "slash" pour traiter différentes commandes:   
- La commande cd est gérée dans `cd.c`   
- La commande pwd est gérée dans `pwd.c`   
- Les commandes externes et exit sont gérées dans `lib.c`   
- L'utilisation de l'étoile est gérée dans `etoile.c`   
- Les signaux sont gérés dans `slash.c`   
- Les redirections sont gérées dans `redirection.c`

## 2. Structures de données

Aucune structure de données personnalisée n'a été créée pour ce projet. Nous avons utilisé uniquement des structures standards comme:    
- Les tableaux et les tableaux de caractères   
- La structure `Dir *` pour ouvrir et fermer des répertoires,   
- La structure `Dirent` pour parcourir l'arborescence d'un répertoire,    
- La structure `stat` pour obtenir des informations sur les entrées de répertoire   
- La structure `sigaction` pour gérer les signaux reçus.   

## 3. Algorithmes implémentés

Après que la ligne de commande ait été récupérée par readline, la fonction `interprete()` dans `slash.c` va traiter les différents cas possibles de la ligne de commande écrite :    
- Une commande interne pwd, cd et exit   
- Une commande externe   
- Une redirection
- Une commande avec une étoile simple, ou double étoile
     
Voici quelques algorithmes implémentés:     
- La fonction `pwd()` dans `pwd.c` traite le cas de la commande interne `pwd [-L | -P]` (affichage du chemin du répertoire courant),    
- La fonction `cd()` dans `cd.c` traite le cas de la commande interne `cd [-L | -P]` (changement du répertoire du travail),     
- Pour les commandes externes, `cmdExtrn()` dans `lib.c` crée un nouveau processus fils qui exécute la commande donnée par l’utilisateur et oblige le père à attendre la terminaison de son fils pour reprendre la boucle principale,   
- Pour les redirections, la fonction `redirection()` dans le fichier `redirection.c` commence par effectuer une simple redirection du descripteur correspondant avec `dup()` dans le processus fils et puis lance récursivement la fonction `interprete()`    
- Pour l’implémentation de l’étoile simple, si l'étoile est un intermédiaire dans le chemin (dans le cas de la forme: `.../*/...`) nous regarderons tous les dossiers sur ce point dans le chemin, et si c'est à la fin (sous la forme: `.../*`), nous prenons tous les éléments (dossiers, fichiers, etc..).   
- Pour l'implémentation de l'étoile double :
    - si l'étoile double est suivi d'un élément non étoile, il retourne tous les fichiers dans l'arborescence dont le répertoire courant est la racine, 
        qui respecte le chemin donné après le double étoile (par ex `**/el1...` où el1 n'est pas `*` ou `*suffixe`)
    - si l'étoile double est tout seul (`**/` ou `**`), retourne tout éléments possibles dans l'arborescence qui ne sont pas des liens symboliques, et pas des dossier cachés.