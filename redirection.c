#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include "pwd.h"
#include "etoile.h"
#include "cd.h"
#include "lib.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



#define MAX_ARGS_NUMBER 4096  //Nb d'arguments maximale pour une ligne entré par l'utilisateur
#define MAX_ARGS_STRLEN 4096  //Nb de caractères maximale pour un argument dans la ligne entré par l'utilisateur
#define MAX_FICH_NUMBER 4096  //Nb de fichier maximale dans un repertoire

#define MAX_PATH_PRINTLEN 41  //Nb de caractères maximale pour le prompt défaut (n caractères affichables + 10 pour changment de couleurs).
#define MAX_PATH_STRLEN 4096  //Nb de caractères maximale pour stocker le chemin absolu du  répertoire courant.

int redirection(int dsc,int flags,char **cmd,int index, int nbArgs, int* ret, int* boucle,char* precref, char* curr){
  //  printf("dans redirecton\n");


   char **commande=malloc((index+1)*(sizeof (char*)));//alouer un tableau de taille(index+2) pour stocker les arguments qu'on va passer a execvp
      for (int i = 0; i < index; ++i)
      {
        commande[i]=malloc((strlen(cmd[i]))*sizeof (char*))  ;
        strcpy(commande[i],cmd[i]);
     // printf("commande[i]=%s\n",commande[i]);
      }
   commande[index]=NULL;
    

  
  int fd; // Descripteur pour fic
  int pid; // PID fils
  int status;

  char** refs;
  char * fic;
  if(ifEtoile(cmd[index+1])){

  char* curr;
    if(cmd[index+1][0]=='/') curr=realpath("/",NULL);
    else  curr=realpath(".",NULL);
  

    char** reps=separerRep(cmd[index+1]);
    int prof=0;  
    while(reps[prof]!=NULL){
      prof++;
    }
    refs=getPathEtoile(curr,reps,prof);
    prof=0;
    free(curr);
    while(reps[prof]!=NULL){
      free(reps[prof]);
      prof++;
    }
    free(reps);
    if(refs==(char**)NULL){
      return 1;
    }

    if(refs[1]!=NULL){
      prof=0;
      while(refs[prof]!=NULL) {
        free(refs[prof]);
        prof++;
      }
      free(refs);
      return 1;
    }

    
   fic=refs[0];



  }else{


    fic=cmd[index+1];
  }

      switch ( flags )
    {
       case 0://cmd < fic
            // On ouvre le fichier fic en lecture
            if ((fd = open(fic, O_RDONLY)) == -1) {
              //perror("main : open");
              return 1;
              for (int i = 0; i < index; ++i)
                {
                  free(commande[i]);
                }
                free(commande);
            }
            break;
        case 1:  //cmd > fic
            // On ouvre le fichier fic en écriture, sans écrasement
            if ((fd = open(fic, O_WRONLY | O_CREAT | O_EXCL,0666)) == -1) {
              //perror("main : open");
              for (int i = 0; i < index; ++i)
                {
                  free(commande[i]);
                }
                free(commande);
              return 1;
            }
            break;
        case 2://cmd >| fic
          // On ouvre le fichier fic en écriture, sans écrasement
          if ((fd = open(fic, O_WRONLY | O_CREAT | O_TRUNC,0666)) == -1) {
            //perror("main : open");
            for (int i = 0; i < index; ++i)
                {
                  free(commande[i]);
                }
                free(commande);
            return 1;
          }
            
            break;
        case 3://cmd >> fic
            // On ouvre le fichier fic en écriture, en positionnant le curseur à la fin du fichier
            if ((fd = open(fic, O_WRONLY | O_APPEND | O_CREAT,0666)) == -1) {
              for (int i = 0; i < index; ++i)
                {
                  free(commande[i]);
                }
                free(commande);
              return 1;
            }
            break;
                    
        default:
          for (int i = 0; i < index; ++i)
                {
                  free(commande[i]);
                }
                free(commande);
          return 1;

    }

  
  // On crée un processus fils
  if ((pid = fork()) < 0) {
    for (int i = 0; i < index; ++i)
      {
        free(commande[i]);
      }
     free(commande);
    return 1;
  }
  
  // Si on est dans le fils
  if (pid == 0) {
    // Rediretion
       switch ( dsc )
    {
        case 0:
            dup2(fd, 0);
            break;
        case 1:
            dup2(fd, 1);
            break;
         case 2:
            dup2(fd, 2);
            break;    
        default:
            return 1;
    }
    
    // On exécute la commande cmd
    char ** cmdnw=TabModif(cmd);
     ret= interprete(cmdnw,nbArgs-2,ret,boucle,precref,curr);//lazem tbadale cmd par cmd-{les deux dernier argument}
     //return *ret;
     exit(0);
    //execvp(commande[0], commande);
    //perror("main : execlp");
    return 1;
  }
  
  // On ferme le descripteur de fichier du fichier fic
  close(fd);
  
  // On attend la fin de l'exécution du fils
  while (wait(&status) != pid);
  
  for (int i = 0; i < index; ++i)
      {
        free(commande[i]);
      }
  free(commande);

  return 0;


}