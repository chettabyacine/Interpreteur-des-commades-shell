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
  Regarde si cmd est une commande interne, et ensuite lance les fonctions appropriés.
*/
int* interprete(char** cmd, int nbArgs, int* ret, int* boucle,char* precref, char* curr){ 
int ifredirection=0; 
   
  // printf("interprete cmd[0] = %s \n",cmd[0]);
//  printf("interprete   cmd[nbArgs-1]=%s\n",cmd[nbArgs-1]);
  
 /* if(nbArgs==-5){//cas de redirections
    
   //je coupe en argument et j'appele la fonctions pipe_2
    char** cmdPipe=malloc((MAX_ARGS_STRLEN+1)*(sizeof (char*)));
    if(cmdPipe==NULL){
    perror("slash : malloc cmd");
    exit(-1);
    }
    cmdPipe=decoupCmd(cmd[0]);
    pipe_2(cmdPipe);
    
    return 0;
 // }*/
  if(strcmp(cmd[0],"exit")==0){
    if(exitSl(cmd,nbArgs,ret,boucle)==NULL){
      *ret=1;
    }
    return ret;
  }
  if(strcmp(cmd[0],"cd")==0){
    if(cd(nbArgs,cmd,precref,curr,ret)==NULL){
      *ret=1;
    }
    return ret;
  }
  if((strcmp(cmd[0],"pwd")==0 ) && ((nbArgs==1) || ( ( strcmp(cmd[1],"-P")==0 || strcmp(cmd[1],"-L")==0 ) && (nbArgs<= 2) ) )  ) {
    return pwd(nbArgs,cmd,curr,ret);
  }
  if(strcmp(cmd[0],"oldpwd")==0){
    printf("%s\n",precref);
    return ret;
  }

  int index;//l'indice du symbole de redirection dans cmd

  for (index=nbArgs-1; index >= 0; --index)
  { 
  
    if( (cmd[index][0]=='>' && cmd[index][1]=='>' && cmd[index][2]=='\0')  
         || (cmd[index][0]=='>' && cmd[index][1]=='|' && cmd[index][2]=='\0') 
         || (cmd[index][0]=='>' &&  cmd[index][1]=='\0')  
         ||(cmd[index][0]=='2'  && cmd[index][1]=='>' && cmd[index][2]=='\0' )
         || (cmd[index][0]=='2'  && cmd[index][1]=='>' && cmd[index][2]=='|' &&  cmd[index][3]=='\0' )
         || (cmd[index][0]=='2'  && cmd[index][1]=='>' && cmd[index][2]=='>' &&  cmd[index][3]=='\0' )
         || (cmd[index][0]=='<' && cmd[index][1]=='\0' )
      ){  
         ifredirection=1;
        
         break;
       }
       
  }


  if(ifredirection){
   
     if(cmd[index][0]=='<'){
      if(cmd[index][1]=='\0'){*ret=redirection(0,0,cmd,index,nbArgs,ret,boucle,precref,curr); return ret;}
       else {*ret=1; return ret;}
     }else
     if(cmd[index][0]=='>'){
       
       if(cmd[index][1]=='>'){ *ret=redirection(1,3,cmd,index,nbArgs,ret,boucle,precref,curr);return ret;}
       else if(cmd[index][1]=='|'){*ret=redirection(1,2,cmd,index,nbArgs,ret,boucle,precref,curr);return ret;}
       else if(cmd[index][1]=='\0'){*ret=redirection(1,1,cmd,index,nbArgs,ret,boucle,precref,curr);return ret;}
     }else
     if(cmd[index][0]=='2'){

       if(cmd[index][1]=='>'){

          if(cmd[index][2]=='>'){*ret=redirection(2,3,cmd,index,nbArgs,ret,boucle,precref,curr);return ret;}
          else if(cmd[index][2]=='|'){
            *ret=redirection(2,2,cmd,index,nbArgs,ret,boucle,precref,curr);return ret;}
          else if(cmd[index][2]=='\0'){*ret=redirection(2,1,cmd,index,nbArgs,ret,boucle,precref,curr);return ret;}
       }
       
     }
  }

        
   int nombre_Reptoire=0;
   int isnot_cmdrxtrn=0;

   for (int i = 0; i < nbArgs; ++i)
    {
       
      nombre_Reptoire=nombre_Reptoire+nombre_Rep(cmd[i]);
      isnot_cmdrxtrn=isnot_cmdrxtrn+ifEtoile(cmd[i]);
    }

    char **commande=NULL;;
 if(isnot_cmdrxtrn!=0)
 {  
 
  commande=malloc((nbArgs+nombre_Reptoire)*(sizeof (char*)));
  //1)Ajouter la commande a commande[0]
  char* cmdetoile;
  if(ifEtoile(cmd[0])){//si cmd[0] contien etoile en cherche la commande et on l'ajoute a commande[0]
      
      char* curr;
    if(cmd[0][0]=='/') curr=realpath("/",NULL);
    else  curr=realpath(".",NULL);
  

    char** reps=separerRep(cmd[0]);
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
      return (int*)1;
    }

    if(refs[1]!=NULL){
      prof=0;
      while(refs[prof]!=NULL) {
        free(refs[prof]);
        prof++;
      }
      free(refs);
      return (int*)1;
    }    
       cmdetoile=refs[0];
       
        commande[0]=malloc((strlen(cmdetoile)+1)*sizeof (char*))  ;
        strcpy(commande[0],cmdetoile);
      
        int irefs=0;
         while(refs[irefs]!=NULL){
         free(refs[irefs]);
         irefs++;
          }

        free(refs); 
            
    }else{
   
       commande[0]=malloc((strlen(cmd[0])+1)*sizeof (char*))  ;
       strcpy(commande[0],cmd[0]);

     
    }
    

  //2)Ajouter le reste des arguments dans commande[i]
  int idx_cmd =1;
  int idx_commande=1;
  while(idx_commande<=nbArgs+nombre_Reptoire-1 && idx_cmd<nbArgs)  
  { 
  
    if(!ifEtoile(cmd[idx_cmd])) {//si nonetoile ajouter directement
      commande[idx_commande]=malloc((strlen(cmd[idx_cmd])+1)*sizeof (char*))  ;
      strcpy(commande[idx_commande],cmd[idx_cmd]);
      idx_commande++;
      idx_cmd++;
    }else{
    char* curr;
    if(cmd[idx_cmd][0]=='/') curr=realpath("/",NULL);
    else  curr=realpath(".",NULL);
    char** reps=separerRep(cmd[idx_cmd]);
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
      
      *ret=1;
      return (int*)1;
    }
     
    int parcourRefs=0;
    while(refs[parcourRefs]!=NULL){

      commande[idx_commande]=malloc((strlen(refs[parcourRefs])+1)*sizeof (char*))  ;
      strcpy(commande[idx_commande],refs[parcourRefs]);
      free(refs[parcourRefs]);
      parcourRefs++;
      idx_commande++;
      
    }
    free(refs);
     idx_cmd++;
    }
       
  }
  commande[idx_commande]=NULL;
 
  idx_commande=0;
  
   if(strcmp(commande[0],"exit")==0) {
    exitSl(commande,nbArgs,ret,boucle);
   }else{

   cmdExtrn(commande,ret,boucle);}

  
 }else{

  if(strcmp(cmd[0],"exit")==0) {
    exitSl(cmd,nbArgs,ret,boucle);
   }else{
   
   cmdExtrn(cmd,ret,boucle);}



 }
 

   if(commande!=NULL){
    
      int k=0;
      while(commande[k]!=NULL){
       free(commande[k]);
       k++;
      }
      free(commande);
      
   }
             
  return ret;
}

int main(int argc, char** argv){


    struct sigaction sa = {0};
    sa.sa_handler = SIG_IGN;
    sigaction(SIGINT,&sa, NULL);
    sigaction(SIGTERM,&sa,NULL);
  //curr contient la référence absolue du répertoire courant
  char* curr=malloc(MAX_PATH_STRLEN);
  mallocFail(curr,1);
  if(getcwd(curr,MAX_PATH_STRLEN)!=curr){
    perror("slash : getcwd échoué");
    exit(1);
  }

  //precref contiendra la référence du répertoire parcouru précédemment.
  char* precref=malloc(MAX_ARGS_STRLEN * sizeof(char));
  mallocFail(precref,1);
  strcpy(precref,curr);
  //precref=NULL;
  

  int ret = 0; //Int stockant le valeur de retour du dernier commande executé. Initialisé à 0.
  
  //Tableau de string qui contiendra tous les arguments entrés.
  char** cmd=malloc((MAX_ARGS_NUMBER+1)*(sizeof (char*)));
  if(cmd==NULL){
    perror("slash : malloc cmd");
    exit(-1);
  }
  
  char* ligne=(char*) NULL; //Stockera la ligne lue par readline (ligne de commande écrit par l'utilisateur)
  
  char* prompt=malloc(MAX_PATH_PRINTLEN+1); //Stockera la message à afficher avant l'entrée (répertoire courant, en respectant la limite de caractères)
  mallocFail(prompt,1);
  
  int nbArg=0;//Stockera le nb d'arguments trouvés
  int boucle=1;//Determine si la boucle doit continuer ou pas
  rl_outstream=stderr;
  while(boucle){
    
    majCWDPrint(curr,&ret,prompt);
    //Lit la ligne entrée par l'utilisateur
    ligne=readline(prompt);
    if(ligne==NULL){ 
      rl_clear_history();
      nbArg=0;
      freeCMD(cmd,nbArg);
      free(ligne);
      free(precref);
      free(prompt);
      //free(cmd);
      free(curr);
      printf("\n");
      return ret;
    }
    //Mettre dans nbArg le nb d'arguments trouvés, et mettre les arguments dans cmd.    
    nbArg=verifieFormat(ligne,cmd);
   
    if(nbArg>0 /*|| nbArg==-5*/) {
      interprete(cmd,nbArg,&ret,&boucle,precref,curr);

    }
   
    freeCMD(cmd,nbArg);
    cmd=malloc((MAX_ARGS_NUMBER+1)*(sizeof (char*)));
    if(cmd==NULL){
    perror("slash : malloc cmd");
    exit(-1);
  }
    free(ligne);
  }

  free(precref);
  free(prompt);
  free(cmd);//Attention ----------------> c'etait en commentaire ??????
  free(curr);
  rl_clear_history();
  return ret;
}


/*
  Source/Aide trouvé sur internet

  getcwd : 
   - stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program
*/
