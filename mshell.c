#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "config.h"
#include "siparse.h"
#include "utils.h"
#include "builtins.h"



int findCommand(char* name, char** argv){
	int it = 0;
	while(builtins_table[it].name != NULL){
		if(!strcmp(builtins_table[it].name,name)){
			(builtins_table[it].fun)(argv);
			return 1;
		}
		it++;
	}
	
	return 0;
}

int check_status(){
	switch(errno){
							case ENOENT:
										fprintf(stderr,"%s: no such file or directory!\n",com->redirs[0]->filename);
										break;
									case ENOTDIR:
										fprintf(stderr,"%s: no such file or directory!\n",com->redirs[0]->filename);
										break;
									case EACCES:
										fprintf(stderr,"%s: permision denied\n",com->redirs[0]->filename);
										break;
								}
}

int main(int argc, char *argv[]){
	char buff[MAX_LINE_LENGTH+1],next[MAX_LINE_LENGTH+1];
	int if_next = 0, parse_next = 0;
	int next_size = 0;
	struct stat buffer;
	int status = fstat(0,&buffer);

	while(1){
		if(S_ISCHR(buffer.st_mode))write(1,PROMPT_STR,2);
		
		int c = read(0,buff,MAX_LINE_LENGTH);
		if(c == -1)return -1;
		if(c == 0)return 0;
		buff[c] = 0;
		int i = 0;
		if(if_next){
            while(buff[i] != 10)next[next_size++] = buff[i++];
            next[next_size] = 0;
            parse_next = 1;
            if_next = 0;
        }
		if(buff[c-1] != 10){
            i = c-1;
            while(buff[i] != 10)i--;
            int k = 0;
            for(i=i+1;i<c;i++)next[k++] = buff[i];
            next_size = k;
            if_next = 1;
        }
		i=0;
		
		while(i<c){

			char temp[MAX_LINE_LENGTH+1];
			if(parse_next){
			    int it = 0;
			    while(it < next_size)temp[it] = next[it++];
                temp[it] = 0;
                parse_next = 0;
            }else{
			    int it = 0; 
			    while(i<c && buff[i] != 10)temp[it++] = buff[i++];
		    	temp[it] = 0;
			    i++;
            }
			
			line* ln = parseline(temp);
			if(ln == NULL){
				fprintf(stderr,"%s\n",SYNTAX_ERROR_STR);
				continue;
			}
			command* com = pickfirstcommand(ln);
			if(findCommand(com->argv[0],com->argv) == 0){
				int child_pid = fork();
				if(child_pid == 0){
					int i = 0;
					while(com->redirs[i] != NULL){
						if(i==0 && IS_RIN(com->redirs[0]->flags)){
							close(0);
							int status = open(com->redirs[0]->filename,O_RDONLY);
							if(status == -1){
								
								
								exit(EXEC_FAILURE);
							}else if(status != 0){
								fprintf(stderr,"Wrong descriptor!\n");
								exit(EXEC_FAILURE);
							}
							
						}
						
						if(i==1 && (IS_ROUT(com->redirs[1]->flags) || IS_RAPPEND(com->redirs[1]->flags))){
							close(1);
							int status;
							if(IS_RAPPEND(com->redirs[1]->flags))
								status = open(com->redirs[1]->filename,O_CREAT|O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
							else status = open(com->redirs[1]->filename,O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR);
							if(status == -1){
								switch(errno){
									case ENOENT:
										fprintf(stderr,"%s: no such file or directory!\n",com->redirs[1]->filename);
										break;
									case ENOTDIR:
										fprintf(stderr,"%s: no such file or directory!\n",com->redirs[1]->filename);
										break;
									case EACCES:
										fprintf(stderr,"%s: permision denied\n",com->redirs[1]->filename);
										break;
								}
								
								exit(EXEC_FAILURE);
							}else if(status != 1){
								fprintf(stderr,"Wrong descriptor!\n");
								exit(EXEC_FAILURE);
							}
						}
						
						i++;
					}
					
					if(execvp(com->argv[0],com->argv) == -1){
						switch(errno){
							case ENOENT:
								fprintf(stderr,"%s: no such file or directory!\n",com->argv[0]);
								break;
							case EACCES:
								fprintf(stderr,"%s: permision denied\n",com->argv[0]);
								break;
							default:
								fprintf(stderr,"%s: exec error\n",com->argv[0]);
								break;
						}
						exit(EXEC_FAILURE);
					}
					
					close(0);close(1);
				}else waitpid(child_pid,NULL,0);	
			}
		}
	}

	return 0;
}
