
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


/*
  Retourne 0 si aucune caractères spéciaux (sauf l'espace) détecté, 1 sinon.
  Les caractères acceptés sont les lettres majuscules,minuscules,0-9 et l'espace.
*/
int aucuneSpec(char* c){
  if(((*c)>31 && (*c)<58) || ((*c)>65 && (*c)<127) || (*c)==60 || (*c)==62 ) return 1;
  return 0;
}

/*
  Découpe str en arguments, et les stockent dans cmd, et 
  retourne  :
  >0 le nb d'arguments trouvés
  
   0 si str ne contient que des espaces
  -1 si un malloc a échoué
  -2 si str contient trop d'arguments
  -3 si str est NULL ou vide
  -4 si str contient un caractère non supporté

  Si str contient un argument trop long, sépare cet argument en 2, et affiche un message.
  
*/
int verifieFormat(char* str, char** cmd){
  //Vérifie si str est vide ou null.
  if(str==NULL || strlen(str)==0) return -3;

 

  //Itérateur de chaque mot trouvé dans str
  char* mot=malloc(MAX_ARGS_STRLEN+1);
  int nbMots=0; //Nb de mots trouvés
  if(mot==NULL){
              cmd[nbMots]=NULL;
              return -1;
            }
  mot[MAX_ARGS_STRLEN]='\0';

  
  int nbChar=0; //Nb de char dans le mot courant
  for(int i=0;i<strlen(str);i++)
  { 

    /*if(str[i]=='|') { //si on est dans le cas d'un redirection
      cmd[0]=malloc(strlen(str)*sizeof(char));
      if(cmd[0]==NULL){
                  return -1;
                }
      strcpy(cmd[0],str);
      return -5;
    }*/

    //Si on a un espace, soit c'est la fin d'un argument, soit un espace inutile
    if(str[i]==' '){
            //Si la fin d'un mot, on stock l'adresse dans l'array cmd, et on initialise
            if(nbChar>0){
              	if(nbMots>=MAX_ARGS_NUMBER){
              	  free(mot);
              	  return -2;
              	}
              	mot[nbChar]='\0';
              	cmd[nbMots]=mot;
              	mot=malloc(MAX_ARGS_STRLEN+1);
              	if(mot==NULL){
                      cmd[nbMots]=NULL;
                      return -1;
                    }
              	mot[MAX_ARGS_STRLEN]='\0';
              	nbChar=0;
              	nbMots++;
            }
    }
    else{
          if(aucuneSpec(&str[i])==0)
          {
            	free(mot);
            	return -4;
          }
          else{      
              	if(nbChar>=MAX_ARGS_STRLEN){
                	  if(nbMots>=MAX_ARGS_NUMBER){
                	    free(mot);
                	    return -2;
                	  }
                	  mot[MAX_ARGS_STRLEN]='\0';
                	  cmd[nbMots]=mot;
                	  mot=malloc(MAX_ARGS_STRLEN+1);
                	  if(mot==NULL){
                      cmd[nbMots]=NULL;
                	    return -1;
                	  }
                	  mot[MAX_ARGS_STRLEN]='\0';
                	  nbChar=0;
                	  nbMots++;
              	}
              	else{
              	  mot[nbChar]=str[i];
              	  nbChar++;
              	}
    	
               }
          }
  }

  if(nbChar>0){
    if(nbMots>=MAX_ARGS_NUMBER){
      free(mot);
      return -2;
    }
    mot[nbChar]='\0';
    cmd[nbMots]=mot;
    nbMots++;
  }
  else free(mot);
  cmd[nbMots]=NULL;

  return nbMots;
}

/*
  free les nb arguments stockés dans cmd sauf le premier.
*/
void freeCMD(char** cmd, int nb){
   
  for (int i = 0; i < nb; ++i)
  {
    if(cmd[i]!=NULL)
    {
      free(cmd[i]);
    
    }
  }

  free(cmd);
}

/*
  Mets à jour le prompt à afficher en prenant en argument le répertoire courant curr comme char* et la valeur de retour int* avec NULL le cas où on a reçu un signal.
*/
void majCWDPrint(char* curr, int* ret, char* print){
  if(curr==NULL || print==NULL) return;
  
  char* retStr=malloc(13); //12 chiffres max pour un int, +1 pour la signe négatif.
  if(retStr==NULL){
    print[0]='\0';
    return;
  }

  //Mets au début '\0' pour que les strcat marchent.
  print[0]='\0';
  int i=0; //Compteur de caractères affichables.

  //Si signal reçu, on change l'affichage à bleu pour le [SIG]
  if(*ret==999){
    
    strcat(print,"\033[34m[");
    strcat(print,"SIG");
    strcat(print,"]\033[00m");
    i=i+5;
  }

  //Sinon, on change la couleur en fonction du valeur de retour : vert pour 0 et rouge pour les autres.
  else{
    
    if(*ret==0) strcat(print,"\033[32m[");
    else strcat(print,"\033[91m[");
    for(int i=0;i<MAX_PATH_PRINTLEN;i++){
      if(print[i]=='\0'){
	break;
      }
    }
    if(sprintf(retStr,"%d",*ret)<=0){
      //printf("majCWDPrint : sprintf");
      print[0]='\0';
      free(retStr);
      return;
    }
    strcat(print,retStr);
    strcat(print,"]\033[00m");
    i=i+strlen(retStr)+2;
  }
  free(retStr);

  int len=strlen(curr);
  int occ=16; //Places déjà prises : -6 pour "$ " + "..." + '\0' à la fin et -10 pour les caractères de couleurs.
  /*    
    Si l'affichage avec le chemin complet est trop longue, découpe l'affichage, et ajouter "..." .
    -i pour les crochets et la valeur de retour
    -occ pour les "$ ","...",-10 pour les changements de couleurs et -1 pour '\0' à la fin
  */
  if(len>MAX_PATH_PRINTLEN-i-occ){
    strcat(print,"...");
    strcat(print,curr+(len-(MAX_PATH_PRINTLEN-i-occ)));
    strcat(print,"$ ");
  }
  //Sinon, ajoute le chemin en entier pour l'affichage.
  else{
    //printf("print non suffisant");
    strcat(print,curr);
    strcat(print,"$ ");
    print[(i+len+12)]='\0';
  }
}

//Affiche l'échec de malloc avec le nom du variable nomv et exit avec valeur ex si nomv est NULL.
void mallocFail(char* nomv, int ex){
  if(nomv==NULL){
    //printf("slash : malloc %s",nomv);
    exit(ex);
  }
}

int* str2int(char* str){
  if(str==NULL) return NULL;
  int len=strlen(str);
  if(len==0) return NULL;
  int* val=malloc(sizeof (int));
  if(val==NULL){
    return NULL;
  }
  *val=atoi(str);
  return val;
  
  
}

/*
  Provoque exit avec la valeur dans ret. exit(0) si aucun arguments supplémentaires sont présents.
*/
int* exitSl(char** cmd, int nbArgs, int* ret, int* boucle){
  if(nbArgs>=2){
    char* retStr=malloc(13);
    if(retStr==NULL){
      return NULL;
    }
    int* val=str2int(cmd[1]);
    if(val==NULL){
      return NULL;
    }
    free(retStr);
    *boucle=0;
    *ret=*val;
    free(val);
    return ret;
  }
  *boucle=0;

  return ret;
}

  /* 
    int nombre_Rep(char *path) -> retourne le nombre de repertoire dans path
  */
  int nombre_Rep(char *path){
  
   char* curr;
    if(path[0]=='/') curr=realpath("/",NULL);
    else  curr=realpath(".",NULL);
  

    char** reps=separerRep(path);
    int prof=0;  
    while(reps[prof]!=NULL){
      prof++;
    }
    char** refs=getPathEtoile(curr,reps,prof);
    prof=0;
    free(curr);
    while(reps[prof]!=NULL){
      free(reps[prof]);
      prof++;
    }
    free(reps);
    if(refs==(char**)NULL){
      return 0;
    }

    int nbRepertoire=0;
    while(refs[nbRepertoire]!=(char*)NULL){
      free(refs[nbRepertoire]);
      nbRepertoire++;

    } 
    free(refs);
  return nbRepertoire;
}

/*  
    int ifEtoile(char * cmd) -> retourne 1 si cmd contient le caractere * 
 */

 int ifEtoile(char * cmd){
  // printf("if ifEtoile=============\n");
  for ( int i=0; i <strlen(cmd) ; i++)
      {
          if( cmd[i]=='*') {
            return 1;}
      }

  return 0; 
 }




    /* 
      execute la commande de redirection ( cmd[0] | cmd[1] )
      cmd est un tableu passé en parametre qui contient les commande a executer:
      
    */
   /* int pipe_2(char ** cmd){

      int pip_fd[2];
      int pid, pid2, status;
          


       if (pipe(pip_fd) == -1) {
          perror("pipe_2 : pipe(pip_fd)");
          return 1;
        }
       
        //creation du fils
        if ((pid = fork()) < 0) {
           perror("pipe_2 : pipe(pip_fd)");
          return 1;
        }
       
        if (pid == 0) {
        //on est dans le fils1
          close(pip_fd[1]);               
          dup2(pip_fd[0], 0);//redirection de l'entrée sur pip_fd[0]
          execlp(cmd[1],cmd[1],NULL);
          perror("pipe_2 : execlp ls"); 
          return 1;
        }
       
        else {               
        //on est dans le parent1
          if ((pid2 = fork()) < 0) {//fork pour  pas sortir de slash
             perror("pipe_2 : fork2");
            return 1;
          }
          if (pid2 == 0) {   
          // on est dans le fils2
            close(pip_fd[0]);               
            dup2(pip_fd[1], 1);//redirection de sortie sur pip_fd[1]
            execlp(cmd[0],cmd[0],NULL); //on lance le processus
            perror("pipe_2 : execlp(cmd[0])");   
            return 1;
          }
          else {     
          //on est dans le parent2
            close(pip_fd[0]); 
           // close(pip_fd[1]);  
            //close(pip_fd[1]);  
            close(pip_fd[1]);   
              while (wait(&status) != pid2);
          }
        }

         if (pipe(pip_fd) == -1) {
          perror("pipe_2 : pipe(pip_fd)");
          return 1;
        }      
                 
     return 0;
  }*/


  /*
    int *cmdExtrn(char ** buf,int* ret,int * boucle) -> exexute la commande externe passé en parametre
  */
int *cmdExtrn(char ** buf,int* ret,int * boucle){
      
      int status;
      pid_t fils=fork();
      switch(fils){

      case -1:
        //perror("slash : fork échoué");
        *ret=1;
        break;
      case 0:

        if(execvp(buf[0],buf)==-1){
         // perror("slash : fork exec\n");
          *boucle=0;
          *ret=-99;
          
          break;
          
        }
      default:    
        waitpid(fils,&status,0);
        if(WIFSIGNALED(status)){
         *ret=999;
         break;

        }
        if(WIFEXITED(status)){
          *ret=WEXITSTATUS(status);
          break;
        }
        if(WIFSIGNALED(status)){
          *ret=WTERMSIG(status);
          break;
        }
        perror("slash : cmd error encountered\n");
        *ret=1;
        break;
      }
      return ret;
  }
  
  /*
    TabModif(char ** commande) -> prend un tableau de caractere et renvoi un nouveau tableau priver de ces 2 dernier element
  */

  char** TabModif(char ** commande){
    int i=0;
    int cpt_lengt=0;
    while(commande[i]!=NULL){
     cpt_lengt+=strlen(commande[i]);
     i++;
    }


    char** newcmd=malloc((i+1)*sizeof(char*));

    for (int j = 0; j < i-2; ++j)
    {
      newcmd[j]=malloc((strlen(commande[j])+1)*sizeof(char*));
      strcpy(newcmd[j],commande[j]);
    }
    newcmd[i-2]=NULL; 
     
     i=0;
     while(newcmd[i]!=NULL){
     i++;
    }
    
    return newcmd;
    
  }

  int back(char* path){
  if(path==NULL) return 1;
  int len=strlen(path);
  for(int i=len-1;i>0;i--){
    if(path[i]=='/'){
      path[i]='\0';
      return 0;
    }
  }
  if(path[0]=='/') return 0;
  return 1;
}
  /*
   int redirection(...) -> exexucte le redirection correspondante 
  */

