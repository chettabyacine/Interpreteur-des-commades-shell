#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <strings.h>


char** getPathEtoile(char* curr, char** path, int pathCount);
char** getSousPath(char* curr, char** path, int pathCount);
char** getSousPathEtoile(char* curr, char** prefixe, int pathCount);

char** separerRep(char* path){
  if(path==NULL){
    return NULL;
  }
  char** pathList=malloc(sizeof(char*)*126);
  if(pathList==NULL){
    return NULL;
  }
    
  char* dup=malloc(sizeof(char*)*(strlen(path)+1));
  if(dup==NULL){
    free(pathList);
    return NULL;
  }
  int prof=0;
  if(path[0]=='/'){
    strcpy(dup,&path[1]);
    pathList[0]=malloc(2*sizeof(char));
    pathList[0][0]='\0';
    strcat(pathList[0],"/");
    prof++;
  }
  else{
    strcpy(dup,path);
  }
    
  char* iter=strtok(dup,"/");
  
  while(iter!=NULL){
    pathList[prof]=malloc(sizeof(char)*(126+1));
    if(pathList[prof]==NULL){
      for(int i=0;i<prof;i++){
	free(pathList[i]);
      }
      free(pathList);
    }
   // printf("%d : %s\n",prof,iter);
    strcpy(pathList[prof],iter);
    prof++;
    iter=strtok(NULL,"/");
  }
  pathList[prof]=(char*)NULL;
  free(dup);

  return pathList;
}

char** getSousPath(char* curr, char** path, int pathCount){
  if(curr==NULL || pathCount<=0){
    return NULL;
  }
  int currlen=strlen(curr);
  int pathlen=strlen(path[0]);
    
  char* pathFichier=malloc(sizeof(char)*(currlen+pathlen+2));
  if(pathFichier==NULL){
    return NULL;
  }
  strcpy(pathFichier,curr);
  //if(curr[0]=='/'){}
   strcat(pathFichier,"/");
  strcat(pathFichier,path[0]);
  // printf("%d,pathFic:%s\n",pathCount,pathFichier);
  char* pathAbsolue=realpath(pathFichier,NULL);
  if(pathAbsolue==NULL){
  //  printf("fichier %s invalide\n",path[0]);
    free(pathFichier);
    return NULL;
  }
  if(pathCount==1){

    char** ret=malloc(sizeof(char*)*2);
    if(ret==NULL){
      return NULL;
    }
  /*  if(curr[0]=='/'){
     ret[0]=malloc(sizeof(char)*(pathlen+2));
     strcat(ret[0],"/"); 
    }else{*/ ret[0]=malloc(sizeof(char)*(pathlen+1));//}

    if(ret[0]==NULL){
      free(pathFichier);
      free(pathAbsolue);
      free(ret);
      return NULL;
    }
    strcpy(ret[0],path[0]);
    ret[1]=(char*)NULL;
      
    
    /*  printf("fichier trouvÃ©:%s\n",ret[0]);
      int i=0;
      while(ret[i]!=(char*)NULL){
      printf("ss %d : %s\n",i,ret[i]);
      i++;
      }
    */

    free(pathFichier);
    free(pathAbsolue);
    return ret;
  }
  else{
    char** sret=getPathEtoile(pathFichier,&(path[1]),(pathCount-1));
    if(sret==NULL){
      free(pathFichier);
      free(pathAbsolue);
      free(sret);
      return NULL;
    }
    int sp=0;
    while(sret[sp]!=(char*)NULL){
      sp++;
    }

    char** ret=malloc(sizeof(char*)*(sp+1));
    for(int i=0;i<sp;i++){
     /* if(curr[0]=='/'){
      ret[i]=malloc(sizeof(char)*(pathlen+strlen(sret[i])+3));
      strcat(ret[i],"/");
      }else{ */ret[i]=malloc(sizeof(char)*(pathlen+strlen(sret[i])+2));
   //    }
      if(ret[i]==NULL){
	free(pathFichier);
	free(pathAbsolue);
	for(int j=0;j<i;j++){
	  free(ret[j]);
	}
	for(int j=0;j<sp;j++){
	  free(sret[j]);
	}
	free(sret);
	free(ret);
	return NULL;
      }

      strcpy(ret[i],path[0]);
      if(strcmp(path[0],"/")!=0)strcat(ret[i],"/");
      strcat(ret[i],sret[i]);
   //   printf("ret %d:%s\n",i,ret[i]);
    }
    ret[sp]=(char*)NULL;
    free(pathFichier);
    free(pathAbsolue);
    for(int j=0;j<sp;j++){
      free(sret[j]);
    }
    free(sret);
    return ret;
  }
}

char** getSousPathEtoile(char* curr, char** path, int pathCount){
  if(curr==NULL || path==NULL || pathCount<=0){
    return NULL;
  }
  
  char* currPath=realpath(curr,NULL);

  if(currPath==NULL){
    return NULL;
  }

  int prefixe=0;
  int lenprefixe=0;
  if(strcmp("*",path[0])!=0){    
    prefixe=1;
    lenprefixe=strlen(&(path[0])[1]);
  }
    
  DIR* dir=opendir(currPath);
  int INIT_NB_REPS=128;    
  struct dirent* iter=readdir(dir);
  int nbRep=0;
  int cap=INIT_NB_REPS;
    // printf("%d : path:%s\n",prefixe,&(path[0])[1]); 
  if(pathCount==1){
    char** files=malloc(sizeof(char*)*(INIT_NB_REPS+1));
    while(iter!=NULL){
      if(nbRep>=cap){
	files=realloc(files,(sizeof(char*)*(2*nbRep+1)));
	if(files==NULL){
	  for(int i=0;i<nbRep;i++){
	    free(files[i]);
	  }
	  closedir(dir);
	  free(currPath);
	  free(files);
	  return NULL;
	}

	cap=cap*2;
      }
      if(strncmp(iter->d_name,"..",2)!=0 && strncmp(iter->d_name,".",1)!=0){
	int lenname=strlen(iter->d_name);
	if(lenname<lenprefixe) lenname=-1;
	else{
	  lenname=lenname-lenprefixe;
	  //     printf("lenname : %s\n",&(iter->d_name)[lenname]);
	}

	if(prefixe==0 || (prefixe==1 && lenname>=0 && strcmp(&(iter->d_name)[lenname],&(path[0])[1])==0)){
	  files[nbRep]=malloc(sizeof(char*)*(strlen(iter->d_name)+1));
	  if(files[nbRep]==NULL){
	    for(int i=0;i<nbRep;i++){
	      free(files[i]);
	    }
	    free(files);
	    closedir(dir);
	    free(currPath);
	    return NULL;
	  }
	  strcpy(files[nbRep],iter->d_name);
	  nbRep++;
	}
	/*
    int lenname=strlen(iter->d_name);
    if(lenname<lenprefixe) lenname=-1;
    else{
      lenname=lenname-lenprefixe;
     //    printf("lenname : %s\n",&(iter->d_name)[lenname]);
    }

    if(prefixe==0 || (prefixe==1 && lenname>=0 && strcmp(&(iter->d_name)[lenname],&(path[0])[1])==0)){
   //   printf("fichier trouvee : %s\n",iter->d_name);
      
        if(curr[0]=='/'){
        files[nbRep]=malloc(sizeof(char*)*(strlen(iter->d_name)+2));
        strcat(files[nbRep],"/");
      }else{
        files[nbRep]=malloc(sizeof(char*)*(strlen(iter->d_name)+1));
       }
	
      if(files[nbRep]==NULL){
        for(int i=0;i<nbRep;i++){
          free(files[i]);
        }
        free(files);
        closedir(dir);
        free(currPath);
        return NULL;
      }
      strcpy(files[nbRep],iter->d_name);
      nbRep++;
    }
      */
      }
      iter=readdir(dir);
    }
    files[nbRep]=(char*)NULL;
    if(nbRep==0){
 //     printf("Aucun Ã©lÃ©ment trouvÃ© dans %s\n",path[0]);
      closedir(dir);
      free(currPath);
      free(files);
      return NULL;
    }
      
      
    //    printf("dir trouvÃ©s:\n");
    // for(int i=0;i<nbRep;i++){
    //       printf("%d:%s\n",i,(files[i]));
    //   }
      
      
    closedir(dir);
    free(currPath);

    return files;
  }
  else{
    char** reps=malloc(sizeof(char*)*(INIT_NB_REPS+1));
    if(reps==NULL){
      free(currPath);
      closedir(dir);
      return NULL;
    }
    while(iter!=NULL){
      if(nbRep>=cap){
	reps=realloc(reps,(sizeof(char*)*(2*nbRep+1)));
	if(reps==NULL){
	  closedir(dir);
	  for(int i=0;i<nbRep;i++){
	    free(reps[i]);
	  }
	  free(currPath);
	  free(reps);
	  return NULL;
	}
	cap=cap*2;
      }
      int lniter=strlen(iter->d_name);
      int lenname=-1;
      if(lniter>=lenprefixe){
	lenname=lniter-lenprefixe;
	//printf("lenname : %s\n",&(iter->d_name)[lenname]);
      }      
        
      if(strncmp(iter->d_name,"..",2)!=0 && strncmp(iter->d_name,".",1)!=0 && /*iter->d_type==DT_DIR &&*/ (prefixe==0 || (prefixe==1 && lenname>=0 && strcmp(&(iter->d_name)[lenname],&(path[0])[1])==0))){
	if(iter->d_type==DT_DIR){
	  reps[nbRep]=malloc(sizeof(char)*(strlen(iter->d_name)+1));
	  if(reps[nbRep]==NULL){
	    closedir(dir);
	    for(int i=0;i<nbRep;i++){
	      free(reps[i]);
	    }
	    free(reps);
	    free(currPath);
	    return NULL;
	  }
	  strcpy(reps[nbRep],iter->d_name);
	  nbRep++;    

	}

	if(iter->d_type==DT_LNK){
	  struct stat st;
	  char * lnpath=malloc(strlen(currPath)+lniter+2);
	  if(lnpath==NULL){

	    closedir(dir);
	    for(int i=0;i<nbRep;i++){
	      free(reps[i]);
	    }
	    free(reps);
	    free(currPath);
                          
	    return NULL;


	  }
	  strcpy(lnpath,currPath);
	  strcat(lnpath,"/");
	  strcat(lnpath,iter->d_name);
	  if(stat(lnpath,&st)!=0){
	    closedir(dir);
	    for(int i=0;i<nbRep;i++){
	      free(reps[i]);
	    }
	    free(reps);
	    free(currPath);
	    free(lnpath);
	    return NULL;
	  }
	  if((st.st_mode & S_IFMT) ==(S_IFDIR)){ 

	    reps[nbRep]=malloc(sizeof(char)*(strlen(iter->d_name)+1));

	    if(reps[nbRep]==NULL){
	      closedir(dir);
	      for(int i=0;i<nbRep;i++){
		free(reps[i]);
	      }
	      free(reps);
	      free(currPath);
	      free(lnpath);
	      return NULL;
	    }
	    strcpy(reps[nbRep],iter->d_name);
	    nbRep++;      
	  }
	  free(lnpath);
	}
      }
      iter=readdir(dir);
    }
    reps[nbRep]=(char*)NULL;
      


    if(nbRep==0){
      closedir(dir);
      free(currPath);
      free(reps);
 //     printf("Aucun rÃ©pertoire trouvÃ©s pour %s\n",path[0]);
      return NULL;
    }
 //     printf("%s dirent* %d trouvÃ©s:\n",path[0],nbRep);
  /*  for(int i=0;i<nbRep;i++){
  //       printf("%d:%s\n",i,reps[i]);
    }*/ 
    char*** sret=malloc(sizeof(char**)*nbRep);
    char** ret=malloc(sizeof(char*)*cap);
    int nbSRep=0;
    int currlen=strlen(curr);
    int replen=0;
    char* srep;
    for(int i=0;i<nbRep;i++){
      replen=strlen(reps[i]);
      srep=malloc(sizeof(char)*(currlen+replen+2));
      if(srep==NULL){
	for(int j=0;j<nbRep;j++){
	  free(reps[j]);
	}
	free(reps);
	for(int j=0;j<i;j++){
	  int z=0;
	  while((sret[j])[z]!=NULL){
	    free((sret[j])[z]);
	    z++;
	  }
	  free(sret[j]);
	}
	free(sret);
	free(currPath);
	closedir(dir);
	return NULL;
      }
      strcpy(srep,curr);
      strcat(srep,"/");
      strcat(srep,reps[i]);

      sret[i]=getPathEtoile(srep,&path[1],(pathCount-1));
      free(srep);
      if(sret[i]!=NULL){
	//printf("%s:sret[%d]:\n",srep,i);
	if(nbSRep>=cap){
	  ret=realloc(ret,(cap*2+1));
	  if(ret==NULL){
	    for(int j=0;j<nbSRep;j++){
	      free(ret[j]);
	    }
	    free(ret);
	    for(int j=0;j<=i;j++){
	      if(sret[j]!=NULL){
		int z=0;
		while((sret[j])[z]!=NULL){
		  free((sret[j])[z]);
		}
		free(sret[j]);
	      }
	    }
	    free(currPath);
	    closedir(dir);
	    return NULL;
	  }
	  cap=cap*2;      
	}

	int j=0;
	while((sret[i])[j]!=NULL){
	  ret[nbSRep]=malloc(sizeof(char)*(replen+strlen((sret[i])[j])+2));
	  //Catch malloc fail
	  strcpy(ret[nbSRep],reps[i]);
	  strcat(ret[nbSRep],"/");
	  strcat(ret[nbSRep],(sret[i])[j]);
	  //printf("rett[%d]:%s\n",nbSRep,ret[nbSRep]);
	  nbSRep++;
	  j++;
	}
      }
    }
    ret[nbSRep]=(char*)NULL;
    if(nbSRep==0){

      for (int i = 0; i < nbRep; ++i)
	{
	  free(reps[i]);
	} 
      free(reps);
      free(sret);
      closedir(dir);
      free(ret);
      /*
	for(int j=0;j<nbRep;j++){
	free(reps[j]);
	}
	free(reps);
      */
      free(currPath);
      // free(srep);
      return NULL;
    }
        
    //printf("%s nbSRep : %d\n",curr,nbSRep);
     
     
    for (int i = 0; i < nbRep; ++i){  //printf("i=%d\n",i);
      free(reps[i]);
      if(sret[i]!=NULL) 
	{  
	  int j=0;
	  while((sret[i])[j]) {
	    free((sret[i])[j]);
	    j++;
	  }
	  free(sret[i]);
	}
    }
    free(sret);
    free(reps);
    free(currPath);
    // free(srep);
    closedir(dir);
    return ret;
  }
}

char** getSousPath2Etoile(char* curr, char** path, int pathCount, int etoile, int prefixe, char* root){
  if(curr==NULL || pathCount<0 || (pathCount>0 && (path==NULL || path[0]==NULL))){
    return NULL;
  }

  /*
  if(path!=NULL) printf("getSousPath2Etoile(%s,%s,%d,%d,%d,%s)\n",curr,path[0],pathCount,etoile,prefixe,root);
  else   printf("getSousPath2Etoile(%s,NULL,%d,%d,%d,%s)\n",curr,pathCount,etoile,prefixe,root);
  */
  
  char* pathAbsolue;

  if(pathCount>0){
    int point=0;
    if(strcmp(path[0],"..")==0){
      point=2;
    }
    else if(strcmp(path[0],".")==0) point=1;

    if(point>0){
      return NULL;
      /*
	if(pathCount<=1) return NULL;
	char* currIter=malloc(strlen(curr)+point+2);
	strcpy(currIter,curr);
	strcat(currIter,"/");
	strcat(currIter,path[0]);

	pathAbsolue=realpath(currIter,NULL);
	if(pathAbsolue==NULL){
	free(currIter);
	free(pathAbsolue);
	return NULL;
	}
	int pref=0;
	int hasEtoile=0;
	char* etoilePointer=strchr(path[2],'*');    
	if(etoilePointer!=NULL){
	hasEtoile=1;
	if(strlen(path[2])>1) pref=1;
	}
	char** ret=getSousPath2Etoile(currIter,&path[2],pathCount-1,hasEtoile,pref,root);
	free(currIter);
	return ret;
      */
    }
  }

  pathAbsolue=realpath(curr,NULL);
  if(pathAbsolue==NULL){
    //printf("fichier %s invalide\n",path[0]);
    free(pathAbsolue);
    return NULL;
  }
  char* dirCurr;
  if(strcmp(pathAbsolue,"/")==0){
    dirCurr=pathAbsolue;
  }
  else{
    dirCurr=&(rindex(pathAbsolue,'/')[1]);
  }
  //printf("dirCurr : %s\n",dirCurr);
  int lenCurr=strlen(dirCurr);

  DIR* dir=opendir(pathAbsolue);
  int INIT_NB_REPS=128;    
  struct dirent* iter=readdir(dir);
  int nbRep=0;
  int cap=INIT_NB_REPS;

  /*
  int lenprefixe=0;
  if(etoile==1 && prefixe==1 && strcmp("*",path[0])!=0){    
    lenprefixe=strlen(&(path[0])[1]);
  }
  */

  char** files=malloc(sizeof(char*)*(INIT_NB_REPS+1));
  char** sret;
  if(pathCount==0){
    char** sret;
    while(iter!=NULL){     
      if(nbRep>=cap/2){
	files=realloc(files,(sizeof(char*)*(2*nbRep+1)));
	if(files==NULL){
	  for(int i=0;i<nbRep;i++){
	    free(files[i]);
	  }
	  closedir(dir);
	  free(pathAbsolue);
	  free(files);
	  return NULL;
	}
	cap=cap*2;
      }

      
      if(strcmp(iter->d_name,"..")!=0 && strncmp(iter->d_name,".",1)!=0 && iter->d_type!=DT_LNK){
	int isRoot=strcmp(dirCurr,root);
	
	if(iter->d_type==DT_DIR){
	  
	  if(isRoot!=0) files[nbRep]=malloc(strlen(iter->d_name)+lenCurr+2);
	  else files[nbRep]=malloc(strlen(iter->d_name)+1);
	  if(files==NULL){
	    for(int i=0;i<nbRep;i++){
	      free(files[i]);
	    }		
	    closedir(dir);
	    free(pathAbsolue);
	    free(files);
	    return NULL;
	  }
	  files[nbRep][0]='\0';
	  
	  if(isRoot){
	    strcat(files[nbRep],dirCurr);
	    strcat(files[nbRep],"/");
	  }
	  strcat(files[nbRep],iter->d_name);
	  //printf("files[%d]: %s\n",nbRep,files[nbRep]);
	  nbRep++;
	  
	  
	  char* lnpath=malloc(strlen(pathAbsolue)+strlen(iter->d_name)+2);
	  strcpy(lnpath,pathAbsolue);
	  strcat(lnpath,"/");
	  strcat(lnpath,iter->d_name);
	
	  sret=getSousPath2Etoile(lnpath,path,0,etoile,prefixe,root);
	  if(sret==NULL){
	    closedir(dir);
	    for(int i=0;i<nbRep;i++){
	      free(files[i]);
	    }
	    free(files);
	    free(pathAbsolue);
	    free(lnpath);
	    return NULL;
	  }
	  free(lnpath);

	  int nbsrep=0;
	  while(sret[nbsrep]!=NULL){
	    //printf("sret[%d]=%s\n",nbsrep,sret[nbsrep]);
	    nbsrep++;
	  }	  

	  //printf("2etoile traitement sret\n");
	  while(nbRep+nbsrep>=cap-1){
	    files=realloc(files,(sizeof(char*)*(2*nbRep+1)));
	    if(files==NULL){
	      for(int i=0;i<nbRep;i++){
		free(files[i]);
	      }		
	      closedir(dir);
	      free(pathAbsolue);
	      free(files);
	      for(int i=0;i<nbsrep;i++){
		free(sret[i]);
	      }
	      free(sret);
	      return NULL;
	    }
	    cap=cap*2;
	  }
	  for(int i=0;i<nbsrep;i++){
	    if(isRoot!=0) files[nbRep]=malloc(strlen(sret[i])+lenCurr+2);
	    else files[nbRep]=malloc(strlen(sret[i])+1);
	    if(files==NULL){
	      for(int i=0;i<nbRep;i++){
		free(files[i]);
	      }		
	      closedir(dir);
	      free(pathAbsolue);
	      free(files);
	      for(int i=0;i<nbsrep;i++){
		free(sret[i]);
	      }
	      free(sret);
	      return NULL;
	    }
				
	    files[nbRep][0]='\0';
	    if(isRoot!=0){
	      strcat(files[nbRep],dirCurr);
	      strcat(files[nbRep],"/");
	    }
	    strcat(files[nbRep],sret[i]);
	    
	    free(sret[i]);
	    //printf("files[%d]: %s\n",nbRep,files[nbRep]);
	    nbRep++;
	  }
	  free(sret);
	}
	else{
	  if(isRoot!=0) files[nbRep]=malloc(strlen(iter->d_name)+lenCurr+2);
	  else files[nbRep]=malloc(strlen(iter->d_name)+1);
	  if(files==NULL){
	    for(int i=0;i<nbRep;i++){
	      free(files[i]);
	    }		
	    closedir(dir);
	    free(pathAbsolue);
	    free(files);
	    return NULL;
	  }

	  files[nbRep][0]='\0';
	  if(isRoot!=0){
	    strcat(files[nbRep],dirCurr);	    
	    strcat(files[nbRep],"/");
	  }
	  strcat(files[nbRep],iter->d_name);
	  //printf("files[%d]: %s\n",nbRep,files[nbRep]);
	  nbRep++;
	}
      }
      iter=readdir(dir);
    }
    files[nbRep]=(char*)NULL;
    closedir(dir);
    free(pathAbsolue);
    if(nbRep==0){
      //printf("Aucun element trouve dans %s\n",path[0]);
    }
    return files;
  }
  else{
    //printf("2Etoile: else(%d,%d)\n",etoile,prefixe);
    if(path==NULL || path[0]==NULL){
      //printf("path null ou path[0] null\n");
      return NULL;
    }

    int isRoot=strcmp(dirCurr,root);
    
    while(iter!=NULL){     
      if(nbRep>=cap-1){
	files=realloc(files,(sizeof(char*)*(2*nbRep+1)));
	if(files==NULL){
	  for(int i=0;i<nbRep;i++){
	    free(files[i]);
	  }
	  closedir(dir);
	  free(pathAbsolue);
	  free(files);
	  return NULL;
	}
	cap=cap*2;
      }
      //printf("iter:%s\n",iter->d_name);
      if(etoile==0){	
	if(iter->d_type==DT_DIR && (strncmp(iter->d_name,"..",2)!=0 && strncmp(iter->d_name,".",1)!=0) ){
	  char* lnpath=malloc(strlen(curr)+strlen(iter->d_name)+2);
	  strcpy(lnpath,curr);
	  strcat(lnpath,"/");
	  strcat(lnpath,iter->d_name);
	  //printf("lnpath:%s\n",lnpath);

	  if(strcmp(path[0],iter->d_name)==0){
	    //printf("pathCount>1:%s,%s,%d\n\n",lnpath,(&path[1])[0],pathCount);
	    if(pathCount>1){
	      sret=getPathEtoile(lnpath,&path[1],pathCount-1);
	    }
	    else{
	      //printf("final\n");
	      sret=malloc(sizeof(char*)*2);
	      if(sret==NULL){
		for(int i=0;i<nbRep;i++){
		  free(files[i]);
		}
		closedir(dir);
		free(pathAbsolue);
		free(files);
		free(lnpath);
		return NULL;
	      }
	      sret[0]=malloc(strlen(iter->d_name)+1);
	      if(sret[0]==NULL){
		for(int i=0;i<nbRep;i++){
		  free(files[i]);
		}
		closedir(dir);
		free(pathAbsolue);
		free(files);
		free(lnpath);
		free(sret);
		return NULL;
	      }
	      strcpy(sret[0],iter->d_name);
	      sret[1]=(char*)NULL;
	    }
	  }
	  else sret=getSousPath2Etoile(lnpath,path,pathCount,etoile,prefixe,root);	  
	  free(lnpath);

	  //printf("2etoile traitement sret %d\n",nbRep);

	  int nbsrep=0;
	  if(sret!=NULL){
	    while(sret[nbsrep]!=NULL){
	      //printf("sret %d : %s\n",nbsrep,sret[nbsrep]);
	      nbsrep++;
	    }
	  }

	  while(nbRep+nbsrep>=cap-1){
	    files=realloc(files,(sizeof(char*)*(2*nbRep+1)));
	    if(files==NULL){
	      for(int i=0;i<nbRep;i++){
		free(files[i]);
	      }		
	      closedir(dir);
	      free(pathAbsolue);
	      free(files);
	      return NULL;
	    }
	    cap=cap*2;
	  }	  
	  
	  for(int i=0;i<nbsrep;i++){
	    //int added=1;
	    //printf("pathCountTSret:%d\n",pathCount);
	    
	    //if(strstr(sret[i],path[0])==NULL) added=0;
	    
	    if(isRoot!=0){
	      if(pathCount>1) files[nbRep]=malloc(lenCurr+strlen(iter->d_name)+strlen(sret[i])+3);
	      else files[nbRep]=malloc(lenCurr+strlen(sret[i])+2);
	    }
	    else{
	      files[nbRep]=malloc(strlen(sret[i])+1);
	    }

	    if(files==NULL){
	      for(int i=0;i<nbRep;i++){
		free(files[i]);
	      }		
	      closedir(dir);
	      free(pathAbsolue);
	      free(files);
	      for(int i=0;i<nbsrep;i++){
		free(sret[i]);
	      }
	      free(sret);
	      return NULL;
	    }
	    files[nbRep][0]='\0';
	    //printf("rpC %d %d\n",isRoot,pathCount);
	    if(isRoot!=0){
	      strcat(files[nbRep],dirCurr);
	      strcat(files[nbRep],"/");
	      if(pathCount>1){
		//printf("dC:%s\n",iter->d_name);
		strcat(files[nbRep],iter->d_name);
		strcat(files[nbRep],"/");
	      }
	      strcat(files[nbRep],sret[i]);
	    }
	    else{
	      strcat(files[nbRep],sret[i]);	      
	    }
	    
	  
	    //printf(">1files[%d]: %s\n",nbRep,files[nbRep]);
	    nbRep++;
	  }
	  free(sret);	  
	}
	else{	  
	  if(strcmp(path[0],iter->d_name)==0){
	    if(isRoot!=0) files[nbRep]=malloc(strlen(iter->d_name)+lenCurr+2);
	    else files[nbRep]=malloc(strlen(iter->d_name)+1);
	    if(files==NULL){
	      for(int i=0;i<nbRep;i++){
		free(files[i]);
	      }		
	      closedir(dir);
	      free(pathAbsolue);
	      free(files);
	      return NULL;
	    }
	    files[nbRep][0]='\0';
	    if(isRoot!=0){
	      strcat(files[nbRep],dirCurr);
	      strcat(files[nbRep],"/");
	    }
	    strcat(files[nbRep],iter->d_name);
	    //printf("1files[%d]: %s\n",nbRep,files[nbRep]);
	    nbRep++;
	  }
	}
      }
      else{
        for(int i=0;i<nbRep;i++){
	  free(files[i]);
	}		
	closedir(dir);
	free(pathAbsolue);
	free(files);
	return NULL;
      }
      iter=readdir(dir);
    }
    if(nbRep>0){
      //printf("Aucun réponse\n");
      files[nbRep]=(char*)NULL;
    }
    else{
      return NULL;
    }

    /*
    if(path!=NULL) printf("\n\nRendu final pour %s, %s, %d\n",curr,path[0],pathCount);
    for(int i=0;i<nbRep;i++){
      printf("rf[%d]=%s\n",i,files[i]);
    }
    printf("\n\n");
    */


    return files;
  }
}

char** getPathEtoile(char* curr, char** path, int pathCount){
  if(curr==NULL || pathCount<=0){
    return NULL;
  }

  int etoile=strncmp(path[0],"*",1);
  //printf("getPathEtoile(%s,%s,%d)\n",curr,path[0],pathCount);
  if(etoile!=0){
    //printf("getSousPath\n");
    return getSousPath(curr,path,pathCount);
  }
  else{    
    if(strcmp(path[0],"**")==0){
      int prefixe=0;
      etoile=0;
      if(pathCount>1){
	etoile=strncmp(path[0],"*",1);
	if(strcmp("*",path[0])!=0){
	  prefixe=1;
	}
      }
      //printf("getSousPath2Etoile\n");
      if(strcmp(curr,"/")==0){
	return getSousPath2Etoile(curr,&path[1],pathCount-1,etoile,prefixe,"/");
      }
      else{
	char* iter=rindex(curr,'/');
	int liter=strlen(iter);
	char* iter2=strdup(curr);
	if(liter==1){	  
	  iter2[strlen(iter2)-1]='\0';
	  iter=rindex(iter2,'/');
	  if(iter==NULL){
	    iter=iter2;
	  }
	  else{
	    iter=iter+1;
	  }
	}
	else iter=iter+1;
	char** ret=getSousPath2Etoile(curr,&path[1],pathCount-1,etoile,prefixe,iter);
	free(iter2);
	return ret;
      }
    }
    //printf("getSousPathEtoile\n");
    return getSousPathEtoile(curr,path,pathCount);
  }
}

void tester(char* arg){
  char* curr=realpath(".",NULL);
  printf("curr:%s\n",curr);
  char** reps=separerRep(arg);
  int prof=0;  
  while(reps[prof]!=NULL){
    //printf("reps[%d]=%s\n",prof,reps[prof]);
    prof++;
  }
  //printf("-------------------\n");
  char** refs=getPathEtoile(curr,reps,prof);
  prof=0;
  free(curr);
  while(reps[prof]!=NULL){
    free(reps[prof]);
    prof++;
  }
  free(reps);
  //printf();
  if(refs==(char**)NULL){
    printf("Aucun path trouvee pour %s\n",arg);
    printf("--------------------------------\n");
    return;
  }

  printf("------ Refs trouvee pour %s-----\n",arg);
  int i=0;
  while(refs[i]!=(char*)NULL){
    printf("refs[%d] : %s\n",i,refs[i]);
    free(refs[i]);
    i++;
  }
  printf("--------------------------------\n");
  //printf("%s\n",refs[i]);
  free(refs);
}


//int main(int argc, char** argv){

//tester("F/*");
//     printf("-------------\n");
//  tester("a/*c");
//    tester("*a");
// tester("*/A");
//tester("A/*/B3");
// tester("A/*/B1");
//tester("*/*");
//tester("ttest/*");
//tester("ttest/ttest2b");
//tester("*");
//tester("ttttester");
// tester("*/*/*/*");
//tester("*test/*");
//tester("*t");

//tester("**/");
  //tester("**/ttest3");
  //tester("**/ttest2/ttest3");
  //tester("**/dqdsqds");
  //tester("**/ttest2/testt");
  
//tester("**/testt");
//tester("**/./ttest2/ttest3");
//tester("**/../ttest3");
//tester("**/../ttest2/ttest3");

/*
 return 0;
}
*/
