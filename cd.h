extern char* getPath(char* target, char* current_dir);
extern int cdP(char* current_dir, char* precref, char* target);
extern int cdL(char* current_dir, char* precref, char* target);
extern int* cd(int argc, char * argv[], char * nomRepartoirePrecedent, char* current_dir, int* ret);
