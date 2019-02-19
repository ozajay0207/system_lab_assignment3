#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define QUEUE_SIZE 2
#define TOTAL_ELEMENTS 100
#define BUFF_SIZE 5

sem_t *full_TA1,*empty_TA1;
sem_t *full_TA2,*empty_TA2;
sem_t *full_TA3,*empty_TA3;
sem_t *full_TA4,*empty_TA4;


void unlink_semaphores(){
	sem_unlink("/TA1_full");
	sem_unlink("/TA1_empty");
	sem_unlink("/TA2_full");
	sem_unlink("/TA2_empty");
	sem_unlink("/TA3_full");
	sem_unlink("/TA3_empty");
	sem_unlink("/TA4_full");
	sem_unlink("/TA4_empty");
}

void initialize_semaphores(){
	full_TA1 = sem_open("/TA1_full", O_CREAT,  0644, 0);
	empty_TA1 = sem_open("/TA1_empty", O_CREAT,  0644, QUEUE_SIZE);

	full_TA2 = sem_open("/TA2_full", O_CREAT,  0644, 0);
	empty_TA2 = sem_open("/TA2_empty", O_CREAT,  0644, QUEUE_SIZE);

	full_TA3 = sem_open("/TA3_full", O_CREAT,  0644, 0);
	empty_TA3 = sem_open("/TA3_empty", O_CREAT,  0644, QUEUE_SIZE);

	full_TA4 = sem_open("/TA4_full", O_CREAT,  0644, 0);
	empty_TA4 = sem_open("/TA4_empty", O_CREAT,  0644, QUEUE_SIZE);
}

int main(){

	pid_t pid_TA1,pid_TA2,pid_TA3,pid_TA4;
	
	int pipe_TA1[2];
	int pipe_TA2[2];
	int pipe_TA3[2];
	int pipe_TA4[2];

	pipe(pipe_TA1);
	pipe(pipe_TA2);
	pipe(pipe_TA3);
	pipe(pipe_TA4);

	unlink_semaphores();

	initialize_semaphores();

	pid_TA1 = fork();
	if(pid_TA1==0){
		char data[BUFF_SIZE];
		while(1){
			sem_trywait(full_TA1);
			close(pipe_TA1[1]);
			read(pipe_TA1[0],data,BUFF_SIZE);
			printf("TA1:%s\n",data);
			sem_post(empty_TA1);
		}		
	}else{
		pid_TA2=fork();
		if(pid_TA2==0){
			char data[BUFF_SIZE];
			while(1){
				sem_trywait(full_TA2);
				close(pipe_TA2[1]);
				read(pipe_TA2[0],data,BUFF_SIZE);
				printf("TA2:%s\n",data);
				sem_post(empty_TA2);
			}
		}else{
			pid_TA3=fork();
			if(pid_TA3==0){
				char data[BUFF_SIZE];
				while(1){
					sem_trywait(full_TA3);
					close(pipe_TA3[1]);
					read(pipe_TA3[0],data,BUFF_SIZE);
					printf("TA3:%s\n",data);
					sem_post(empty_TA3);
				}
			}else{
				pid_TA4=fork();
				if(pid_TA4==0){
					char data[BUFF_SIZE];
					while(1){
						sem_trywait(full_TA4);
						close(pipe_TA4[1]);
						read(pipe_TA4[0],data,BUFF_SIZE);
						printf("TA4:%s\n",data);
						sem_post(empty_TA4);
					}
				}else{
					int mssg=0;
					for(int i=1;i<=TOTAL_ELEMENTS;){
						char data[BUFF_SIZE];

						mssg=sem_trywait(empty_TA1);
						if(mssg!=-1){
							printf("Produced 1 :%d\n",i);
							sprintf(data,"%d",i);
							i++;
							close(pipe_TA1[0]);
							write(pipe_TA1[1],data,BUFF_SIZE);
							sem_post(full_TA1);
						}else{
							printf("Queue 1 Full\n");
						}
						if(i>TOTAL_ELEMENTS)
							break;

						mssg=sem_trywait(empty_TA2);
						if(mssg!=-1){
							printf("Produced 2 :%d\n",i);
							sprintf(data,"%d",i);
							i++;
							close(pipe_TA2[0]);
							write(pipe_TA2[1],data,BUFF_SIZE);
							sem_post(full_TA2);
						}else{
							printf("Queue 2 Full\n");
						}

						if(i>TOTAL_ELEMENTS)
							break;

						mssg=sem_trywait(empty_TA3);
						if(mssg!=-1){
							printf("Produced 3 :%d\n",i);
							sprintf(data,"%d",i);
							i++;
							close(pipe_TA3[0]);
							write(pipe_TA3[1],data,BUFF_SIZE);
							sem_post(full_TA3);
						}else{
							printf("Queue 3 Full\n");
						}
						if(i>TOTAL_ELEMENTS)
							break;

						mssg=sem_trywait(empty_TA4);
						if(mssg!=-1){
							printf("Produced 4 :%d\n",i);
							sprintf(data,"%d",i);
							i++;
							close(pipe_TA4[0]);
							write(pipe_TA4[1],data,BUFF_SIZE);
							sem_post(full_TA4);
						}else{
							printf("Queue 4 Full\n");
						}
						if(i>TOTAL_ELEMENTS)
							break;

					}
					
				}
			}
		}
	}	

	unlink_semaphores();

	return 0;
}	
