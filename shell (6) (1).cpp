#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <regex>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>


#define GetCurrentDir getcwd

void exec_prog(char* cmd){
	char* stkRetorno;
	char* cmds[10];
	stkRetorno = strtok(cmd," ");
	cmds[0] = stkRetorno + '\0';
	int i = 1;
	while(stkRetorno != NULL){
		stkRetorno = strtok(NULL," ");
		cmds[i] = stkRetorno + '\0';
		i++;
	}
	int saida = execvp(cmds[0],cmds);
	if(saida < 0) std::cout<<"Comando não encontrado!\n";
}

void exec_prog_pipe(char* cmd){

	int pipefd[2];
	char* stkRetorno;
	char* cmds[2];
	char* args1[10];
	char* args2[10];
	pid_t pid1,pid2;
	int i;
	cmds[0] = strtok(cmd,"|") + '\0';
	cmds[1] =  strtok(NULL," ") + '\0';

	stkRetorno = strtok(cmds[0]," ");
	args1[0] = stkRetorno + '\0';
	i = 1;

	while(stkRetorno != NULL){
		stkRetorno = strtok(NULL," ");
		args1[i] = stkRetorno + '\0';
		i++;
	}

	stkRetorno = strtok(cmds[1]," ");
	args2[0] = stkRetorno + '\0';
	i = 1;
	while(stkRetorno != NULL){
		stkRetorno = strtok(NULL," ");
		args2[i] = stkRetorno + '\0';
		i++;
	}

	pipe(pipefd);
	pid1 = fork();

	if(pid1 == 0){
		dup2(pipefd[1],STDOUT_FILENO);
		close(pipefd[0]);
		execvp(args1[0],args1);
		perror("exec");
		exit(1);
	}

	pid2 = fork();

	if(pid2 == 0){

		dup2(pipefd[0],STDIN_FILENO);
		close(pipefd[1]);
		execvp(args2[0],args2);
		perror("exec");
		exit(1);

	}

	close(pipefd[0]);
	close(pipefd[1]);
	int status1,status2;
	
	do{
			waitpid(pid1,&status1,WUNTRACED);
			waitpid(pid2,&status2,WUNTRACED);

		}while(!WIFEXITED(status1) && !WIFSIGNALED(status1) && !WIFEXITED(status2) && !WIFSIGNALED(status2));

}
void cd(std::string path){

	int saida = chdir(const_cast<char*>(path.substr(3,path.size()).c_str()));
	if(saida) std::cout<<"Diretorio não encontrado";
}

void ctrlC(int sinal){

	char op;
	std::cout<<"\nTem certeza que deseja encerrar o shell? (S/N) ";
	std::cin>>op;

	if(op == 'S'){

		exit(0);

	}else{

		(void) signal(SIGINT, ctrlC);
		return;

	}
}


int main(){

	(void) signal(SIGINT, ctrlC);
	std::string cmd = "";
	char caminho_Atual[FILENAME_MAX];
	pid_t c_pid;
	int status;
	int pid;

	while(1){
		GetCurrentDir(caminho_Atual,FILENAME_MAX);
		std::cout<<caminho_Atual<<" >> ";
		std::cin.clear();
		cmd = "";
		while(cmd == ""){
		   std::getline(std::cin,cmd);		
		}
		
		if(cmd == "exit") exit(0);

		else if(cmd.substr(0,2) == "cd") cd(cmd);

		else if(cmd.substr(0,4) == "exec"){

			exec_prog(const_cast<char*>(cmd.substr(5,cmd.size()).c_str()));
			exit(0);
		}
		else {
			pid = fork();
			switch(pid){
				case -1:
			        perror("fork failed");
			        exit(1);
			        break;

			    case 0:
			    	if(cmd.find("|") != std::string::npos) exec_prog_pipe(const_cast<char*>(cmd.c_str()));
			    	else exec_prog(const_cast<char*>(cmd.c_str()));
					exit(0);
			        break;
			    default:
			    	do{
			    		c_pid = waitpid(pid,&status,WUNTRACED);
			    	}while(!WIFEXITED(status) && !WIFSIGNALED(status));
			    	break;
			       
			}
		}
		
	}
}
