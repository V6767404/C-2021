#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<alloca.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<libgen.h>
#include<limits.h>
#include<errno.h>
#include<unistd.h>
#include<dirent.h>
#define BUF_SIZE 1024*1024

//ВАРИАНТ 5
//Написать программу подсчета количества слов в файлах заданного  
//каталога его подкаталогов. Пользователь задаёт имя каталога. 
//Главный процесс открывает каталоги и запускает для каждого файла 
//каталога отдельный процесс подсчета количества слов.
//Каждый процесс выводит на экран свой pid, полный путь к файлу, общее число 
//просмотренных байт и количество слов. Число одновременно работающих процессов не должно 
//превышать N (вводится пользователем). Проверить работу программы для каталога /etc. 


char *AppName;
sig_atomic_t currentCounter = 1, maxCountProcess = 0;

int handleFile(char *path){
    int file = open(path, O_RDONLY);
	if(file == -1){
		fprintf(stderr, "%s: %s. File: %s\n", AppName, strerror(errno), path);
		exit(-1);
	}
	char flag=-1;
	unsigned words=0, bytes=0;
	char buf[BUF_SIZE];
	int cnt = 0;
	while((cnt = read(file, &buf, sizeof(BUF_SIZE))) != 0){
		if(cnt == -1){
			fprintf(stderr, "%s: %s. File: %s\n", AppName, strerror(errno), path);
			exit(-1);
		}
		for(int i = 0;i<cnt;i++){
			bytes++;
			switch(buf[i]){
				case ' ':
				case '\n':
				case '\t':
				case '\r':
					flag = -1;
					break;
				default:
					if(flag==-1){ 
						flag = 1; 
						words++; 
					}
					break;
			}
		}
	}
	printf("%d %s %u %u\n",(int)getpid(), path, bytes, words);
    if(close(file) == -1){
		fprintf(stderr, "%s: %s. File: %s\n", AppName, strerror(errno), path);
		exit(-1);
	}
	exit(0);
}

void processFolder(char *curPath){
    DIR *curDir;
	curDir = opendir(curPath);
	if(curDir == NULL) {
		fprintf(stderr,"%s: %s: %s\n", AppName, curPath, strerror(errno));
		errno = 0;
		return;
	}
	struct dirent *dent;
	struct stat buf;
	char *file = alloca(strlen(curPath) + NAME_MAX + 2);
	if(file==NULL){
		fprintf(stderr,"%s: %s.", AppName, strerror(errno));
		return;
	}
	long int sum = 0;
	int count = 0;
	int maxSize = -1;
	char *maxFile = alloca(NAME_MAX);
	if(maxFile==NULL){
		fprintf(stderr,"%s: %s.", AppName, strerror(errno));
		return;
	}
	maxFile[0] = 0;
	errno = 0;
	while((dent = readdir(curDir)) != NULL){
        if(strcmp(".", dent->d_name) && strcmp("..", dent->d_name)){
			if(strcmp("/", curPath) == 0){
				curPath = "";
			}
			if(strcmp("./", curPath) == 0){
				curPath = ".";
			}
			strcpy(file, curPath);
			strcat(file, "/");
 			strcat(file, dent->d_name);
			if (lstat(file,&buf) == -1) {
				fprintf(stderr,"%s: %s: %s\n", AppName, curPath, strerror(errno));
				return;
			}
			if(S_ISDIR(buf.st_mode)){
				processFolder(file);
			}
			int c;
			if(S_ISREG(buf.st_mode)){
				if(currentCounter == maxCountProcess){
					wait(&c);
					currentCounter--;
				}
				switch(fork()){
					case (pid_t)-1:
						fprintf(stderr,"%s: %s.\n", AppName, strerror(errno));
						errno = 0;
						break;
					case (pid_t)0:
						handleFile(file);
						break;
					default:
						currentCounter++;
						break;
				}
			}
		}
    }
	if(errno!=0){
		fprintf(stderr,"%s: %s. File: %s\n", AppName, strerror(errno), curPath);
		errno = 0;
		return;
	}

	if(closedir(curDir)==-1){
		fprintf(stderr,"%s: %s. File: %s\n", AppName, strerror(errno), curPath);
		errno = 0;
		return;
	}
	return;
}

int main(int argc, char **argv){
    AppName = alloca(strlen(basename(argv[0])));
	strcpy(AppName, basename(argv[0]));
    if(argc < 2){
		fprintf(stderr, "%s: Too few arguments.\n", AppName);
		return -1;
	}
	maxCountProcess = atoi(argv[2]);
	if(maxCountProcess<=0){
		fprintf(stderr, "%s: Second argument is wrong\n", AppName);
		return -1;
	}
	char *res;
	if((res = realpath(argv[1], NULL)) == NULL) {
		fprintf(stderr, "%s: %s. File: %s.\n", AppName, strerror(errno), argv[1]);
		return -1;
	}
    maxCountProcess = atoi(argv[2]);
	processFolder(res);
	while(wait(NULL) != -1){}
    return 0;
}
