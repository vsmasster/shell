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

int check_status(int status,char* filename){
	if(status == -1){
		switch(errno){
			case ENOENT:
				fprintf(stderr,"%s: no such file or directory!\n",filename);
				break;
			case ENOTDIR:
				fprintf(stderr,"%s: no such file or directory!\n",filename);
				break;
			case EACCES:
				fprintf(stderr,"%s: permision denied\n",filename);
				break;
			default:
				fprintf(stderr,"%s: exec error\n",filename);
				break;
		}
	}
	
	return status;
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
				while(com != NULL){
					int child_pid = fork();
					if(child_pid == 0){
						int i = 0;
						while(com->redirs[i] != NULL){
							if(IS_RIN(com->redirs[i]->flags)){
								close(0);
								if(check_status(open(com->redirs[i]->filename,O_RDONLY),com->redirs[i]->filename) != 0){
									fprintf(stderr,"Something went wrong!\n");
									exit(EXEC_FAILURE);
								}
							}
							
							if(IS_ROUT(com->redirs[i]->flags)){
								close(1);
								if(check_status(open(com->redirs[i]->filename,O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR),com->redirs[i]->filename) != 1){
									fprintf(stderr,"Something went wrong!\n");
									exit(EXEC_FAILURE);
								}
							}
							
							if(IS_RAPPEND(com->redirs[i]->flags)){
								close(1);
								if(check_status(open(com->redirs[i]->filename,O_CREAT|O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR),com->redirs[i]->filename) != 1){
									fprintf(stderr,"Something went wrong!\n");
									exit(EXEC_FAILURE);
								}
							}
							
							i++;
						}
						
						if(check_status(execvp(com->argv[0],com->argv),com->argv[0]) == -1)
							exit(EXEC_FAILURE);
						
					}else waitpid(child_pid,NULL,0);
					
					com++;
				}
			}
		}
	}

	return 0;
}
