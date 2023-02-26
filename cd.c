#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#define MAX_ARGS_NUMBER 4096  //Nb d'arguments maximale pour une ligne entré par l'utilisateur
#define MAX_ARGS_STRLEN 4096  //Nb de caractères maximale pour un argument dans la ligne entré par l'utilisateur
#define MAX_FICH_NUMBER 4096  //Nb de fichier maximale dans un repertoire

#define MAX_PATH_PRINTLEN 41  //Nb de caractères maximale pour le prompt défaut (n caractères affichables + 10 pour changment de couleurs).
#define MAX_PATH_STRLEN 4096  //Nb de caractères maximale pour stocker le chemin absolu du  répertoire courant.



char* getPath(char* target, char* current_dir){
  if(current_dir==NULL || target==NULL || strlen(target)==0){
    return NULL;
  }
  
  
  //printf("gP : %s\n",current_dir);
  char** ens=malloc(4096/2);
  if(ens==NULL){
    return NULL;
  }
  char* curr=malloc(MAX_PATH_STRLEN*sizeof(char*));
  if(curr==NULL){
    free(ens);
    return NULL;
  }
  if(curr!=NULL){
    if(target[0]=='~'){
      strcpy(curr,getenv("HOME"));
    }
    else{
      if(target[0]=='/'){
        curr[0]='\0';
      }
      else{strcpy(curr,current_dir);}
      
    }
    
  }
  
  char* iter=strtok(curr,"/");
  int fic=0;
  while(iter){
    ens[fic]=malloc(sizeof(char)*100);
    if(ens[fic]==NULL){      
      return NULL;
    }
    (ens[fic])[0]='\0';
    strcat(ens[fic],iter);
    fic++;
    iter=strtok(NULL,"/");

  }
  
  int c=0;
  char* path=malloc(MAX_PATH_STRLEN*sizeof(char*));
  if(path==NULL){
    for(int i=0;i<fic;i++){
      free(ens[i]);
    }
    free(ens);
    free(curr);
    return NULL;
  }else{
    if(target[0]=='~'){
      strcpy(path,&target[2]);
    }
    else{
      if(target[0]=='/'){
        strcpy(path,&target[1]);
      }else{
        strcpy(path,target);
      }      
    }
  }
  
  iter=strtok(path,"/");
  while(iter!=NULL){
    
    //printf("%d : %s\n",c,iter); 
    if(strcmp(iter,"..")==0){
      if(fic==0){
  for(int i=0;i<fic;i++){
    free(ens[i]);
  }
  free(ens);
  free(iter);
  free(curr);
  return NULL;
      }
      free(ens[fic-1]);
      fic--;
    }
    else{
      if(strcmp(iter,".")!=0){
  ens[fic]=malloc(sizeof(char)*100);
  (ens[fic])[0]='\0';
  strcat(ens[fic],iter);
  fic++;
      }
    }
    iter=strtok(NULL,"/");    
    c++;    
  }
  free(path);
  char* final=malloc(sizeof(char)*1000);
  final[0]='\0';
  for(int i=0;i<fic;i++){
    strcat(final,"/");
    strcat(final,ens[i]);
    free(ens[i]);
  }
  free(ens);
  free(curr);
  return final;
}


int cdP(char* current_dir, char* precref, char* target){
  char* iter=malloc(sizeof(char)*255);
  if(iter==NULL || strlen(target)==0){
    return 1;    
  }
  iter[0]='\0';
  strcat(iter,current_dir);
  switch(target[0]){
  case '.':
    if(iter[(strlen(iter)-1)]!='/'){
      strcat(iter,"/");
    }
    strcat(iter,target);
    break;
  case '/':
    strcpy(iter,target);
    break;
  default:
    if(iter[(strlen(iter)-1)]!='/'){
      strcat(iter,"/");
    }
    strcat(iter,target);
    break;
  }
  if(iter==NULL) return 1;
  char* final=realpath(iter,NULL);
  //printf("%d\n",chdir(NULL));

  if( final==NULL || chdir(final)!=0 ){

    free(iter);
    return 1;
  }
  strcpy(precref,current_dir);

  setenv("PWD",final,1);
  getcwd(current_dir,MAX_ARGS_STRLEN);
  free(final);
  free(iter);
  return 0;
}

int cdL(char* current_dir, char* precref, char* target){
  if(target==NULL || current_dir==NULL || strlen(target)==0){
    return 1;
  }
  
  
  char* final=getPath(target,current_dir);
  if(final==NULL){
    return 1;
  }
   //if(strcmp(final,"/tmp/tmp.lOeqJvmKQX/main-dir/x/y/z/d/..")==0) printf("final=%s\n",final);/////////////////////////////////////////////////

 
  //Obtenir le nouveau chemin à sauvegarder, sans les ".." et "." éventuels.
  if(chdir(final)!=0){
    
    free(final);
    return cdP(current_dir,precref,target);
  }
  //Copier le chemin précédent
  if(precref!=NULL) strcpy(precref, current_dir);  
    
  //Mettre à jour PWD et current_dir
  /*char* env=malloc(PATH_MAX*sizeof(char));
  env=realpath(final,NULL);
  if(env==NULL){
    return 1;
    }
  if(setenv("PWD",env,1)==-1) return 1;
  free(env);*/
  setenv("PWD",final,1);
  strcpy(current_dir,final);
  
  free(final);
 // char* test=malloc(4096);
  //getcwd(test,4096);

  return 0;
}

int* cd(int argc, char * argv[], char * nomRepartoirePrecedent, char* current_dir, int* ret){
  // nomRepartoirePrecedent contient le path du répartoire précedent, et sera modifiée par le nouveau path (actuel pour l'état futur)

 
  if(current_dir==NULL){
    return NULL;
  }
  switch(argc){

  case 1: 
    //TODO: cd -> executer cd -L $HOME, maj nomRepartoirePrecedent !!
    
    if(cdL(current_dir,nomRepartoirePrecedent,getenv("HOME"))!=0){
      goto erreur;
    }
    break;
  case 2:
     
    // cd avec une seule option
    if(strcmp(argv[1],"-L")==0 || strcmp(argv[1], "-P")==0){ 
      //cd -P -> executer cd -P $HOME , maj nomRepartoirePrecedent
      if (strcmp(argv[1], "-P")==0){
	if(cdP(current_dir,nomRepartoirePrecedent,getenv("HOME"))!=0) {goto erreur;}
      }
      else if (strcmp(argv[1], "-L")==0){
	//TODO: cd -L -> executer cd -L $HOME , maj nomRepartoirePrecedent
	if(cdL(current_dir,nomRepartoirePrecedent,getenv("HOME"))!=0){
	  goto erreur;
	}
      }
	  
    }
        
    else{

      char* ref=malloc(sizeof(char*)*255);
      if(ref==NULL){
	goto erreur;
      }
      
      if(strcmp(argv[1], "-")==0){
	// cd - -> executer cd nomRepartoirePrecedent , maj nomRepartoirePrecedent
	
	strcpy(ref,nomRepartoirePrecedent);
	if(cdL(current_dir,nomRepartoirePrecedent,ref)!=0){
	  free(ref);
	  goto erreur;
	}
      }
      else {
	strcpy(ref,argv[1]);
	//TODO: lire la réf puis executer cd -L ref
	if(cdL(current_dir,nomRepartoirePrecedent,ref)!=0){
	  free(ref);
	  goto erreur;
	}
      }
      free(ref);
    }
    break;
	
  case 3:
    //TODO: lire les deux arguments executer cd mode ref, maj nomRepartoirePrecedent
    if(strcmp(argv[1], "-P")==0){
      // le cas général: cd -P ref
      if(cdP(current_dir,nomRepartoirePrecedent,argv[2])!=0){
	goto erreur;
      }
	
    }
    else{
      if(strcmp(argv[1], "-L")==0){
	//TODO: le cas général: cd -L ref
	if(cdL(current_dir,nomRepartoirePrecedent,argv[2])!=0){
	  goto erreur;
	}
      }
      else{
	goto erreur;
      }
    }
    break;
      
  default: 
    goto erreur;
      
  }
    
  
  
  *ret=0;
  return ret;
    
 erreur: perror("cd");
  *ret=1;
  return ret;
}

