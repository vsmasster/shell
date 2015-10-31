#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <pwd.h>
#include "builtins.h"

int lecho(char*[]),lexit(char*[]),lcd(char*[]),lkill(char*[]),lls(char*[]);
int undefined(char *[]);

builtin_pair builtins_table[]={
	{"lexit",	&lexit},
	{"lecho",	&lecho},
	{"lcd",		&lcd},
	{"lkill",	&lkill},
	{"lls",		&lls},
	{NULL,NULL}
};

int lecho(char * argv[]){
	int i =1;
	if (argv[i]) printf("%s", argv[i++]);
	while  (argv[i])
		printf(" %s", argv[i++]);

	printf("\n");
	fflush(stdout);
	return 0;
}

int lexit(char * argv[]){
	exit(0);
	return 2;
}

int lcd(char * argv[]){
	chdir(argv[1] != NULL?argv[1]:getenv("HOME"));
	return 0;
}

int lkill(char * argv[]){
	errno = 0;
	if(argv[2] != NULL){
		int pid = strtol(argv[2],NULL,10);
		int sid = strtol(argv[1],NULL,10);
		if(errno == EINVAL)return 1;
		return kill(pid,sid);
	}
	
	int pid = strtol(argv[1],NULL,10);
	if(errno == EINVAL)return 1;
	return kill(pid,SIGTERM);
}

int lls(char * argv[]){
	DIR *mydir;
	struct dirent *myfile;
	char cwd[1024];
	getcwd(cwd,sizeof(cwd));
	mydir = opendir(cwd);
	while((myfile = readdir(mydir)) != NULL){
		if(myfile->d_name[0] != '.')
		printf(" %s\n",myfile->d_name);
	}
	closedir(mydir);
	
	return 0;
}

int undefined(char * argv[]){
	fprintf(stderr, "Command %s undefined.\n", argv[0]);
	return BUILTIN_ERROR;
}
