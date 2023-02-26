#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#define MAX_ARGS_STRLEN 4096  //Nb de caractères maximale pour un argument dans la ligne entré par l'utilisateur


int* pwd(int argc, char** argv, char* curr, int* ret){
  if(argv==NULL){
    *ret=1;
    return ret;
  }
  char pwd[MAX_ARGS_STRLEN+1];
  switch (argc){    
  case 2 :
    //Cas chemin physique
    if(strcmp(argv[1],"-P")==0){      
      if(getcwd(pwd,sizeof(pwd))==NULL){
	*ret=1;
	return ret;
      }
      printf("%s\n",pwd);
      
    }
    else{
      //Cas chemin logique
      if(strcmp(argv[1],"-L")==0){
	printf("%s\n",getenv("PWD"));
      }
    }
    break;
    
  case 1 ://Cas par defaut chemin logique
    printf("%s\n",getenv("PWD"));
    break;
  }
  *ret=0;
  return ret;

}
