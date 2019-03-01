#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 

#define QUEUE_SIZE 50
#define TOTAL_ELEMENTS 10000
#define BUFF_SIZE 5

//DECLARATION OF SEMAPHORES
sem_t *full_TA1,*empty_TA1;
sem_t *full_TA2,*empty_TA2;
sem_t *full_TA3,*empty_TA3;
sem_t *full_TA4,*empty_TA4;
sem_t *mutex;

//TO UNLINK THE SEMAPHORES BEFORE AND AFTER USE
void unlink_semaphores(){
	sem_unlink("/TA1_full");
	sem_unlink("/TA1_empty");
	sem_unlink("/TA2_full");
	sem_unlink("/TA2_empty");
	sem_unlink("/TA3_full");
	sem_unlink("/TA3_empty");
	sem_unlink("/TA4_full");
	sem_unlink("/TA4_empty");
	sem_unlink("/mutex");
}

//TO INITIALIZE THE SEMAPHORES WITH APPROPRIATE VALUES
void initialize_semaphores(){

	//FULL ND EMPTY SEMAPHORES FOR EACH OF THE TA's
	full_TA1 = sem_open("/TA1_full", O_CREAT,  0644, 0);
	empty_TA1 = sem_open("/TA1_empty", O_CREAT,  0644, QUEUE_SIZE);

	full_TA2 = sem_open("/TA2_full", O_CREAT,  0644, 0);
	empty_TA2 = sem_open("/TA2_empty", O_CREAT,  0644, QUEUE_SIZE);

	full_TA3 = sem_open("/TA3_full", O_CREAT,  0644, 0);
	empty_TA3 = sem_open("/TA3_empty", O_CREAT,  0644, QUEUE_SIZE);

	full_TA4 = sem_open("/TA4_full", O_CREAT,  0644, 0);
	empty_TA4 = sem_open("/TA4_empty", O_CREAT,  0644, QUEUE_SIZE);

	//MUTEX SEMAPHORE FOR SYNCHRONIZATION OF THE SHARED VARIABLE SUM
	mutex = sem_open("/mutex", O_CREAT,  0644, 1);
}

int main(){

	//INITIALIZATIONS OF THE PIPE AND PROCESS VARIABLES
	pid_t pid_TA1,pid_TA2,pid_TA3,pid_TA4;
	
	int pipe_TA1[2];
	int pipe_TA2[2];
	int pipe_TA3[2];
	int pipe_TA4[2];

	pipe(pipe_TA1);
	pipe(pipe_TA2);
	pipe(pipe_TA3);
	pipe(pipe_TA4);

	
	// SETTING UP OF THE SHARED MEMORY
	key_t key = ftok("shared_file",65); 
	int shmid = shmget(key,1024,0666|IPC_CREAT); 
	//ADDING SUM TO THE SHARED MEMORY
	int *sum = (int*) shmat(shmid,(void*)0,0);
	*sum=0;

	//UNLINK AND INITIALIZE SEMAPHORES
	unlink_semaphores();
	initialize_semaphores();

	//CREATION AND MANAGING THE PROCESSES
	pid_TA1 = fork();

	if(pid_TA1==0){
		// ################################## TA 1 ###################################
		char data[BUFF_SIZE];
		for(int i=0;i<TOTAL_ELEMENTS/4;i++){

			// SYNCHRONIZE WITH WAIT AND FULL SEMAPHORES
			sem_wait(full_TA1);
			close(pipe_TA1[1]);
			read(pipe_TA1[0],data,BUFF_SIZE);

			// SYNCHRONIZING WITH MUTEX FOR SHARED VARIABLE 
			sem_wait(mutex);
			*sum=*sum+atoi(data);
			sem_post(mutex);

			//printf("Sum:%d\n",*sum);
			printf("<Completed,1,%s>\n",data);
			sem_post(empty_TA1);
		}		
	}else{

		pid_TA2=fork();
		if(pid_TA2==0){
			// ################################## TA 2 ###################################
			char data[BUFF_SIZE];

			// EXECUTE ONLY 1/4th JOBS 
			for(int i=0;i<TOTAL_ELEMENTS/4;i++){

				// SYNCHRONIZE WITH WAIT AND FULL SEMAPHORES
				sem_wait(full_TA2);
				close(pipe_TA2[1]);
				read(pipe_TA2[0],data,BUFF_SIZE);

				// SYNCHRONIZING WITH MUTEX FOR SHARED VARIABLE 
				sem_wait(mutex);
				*sum=*sum+atoi(data);
				sem_post(mutex);

				//printf("Sum:%d\n",*sum);
				printf("<Completed,2,%s>\n",data);
				sem_post(empty_TA2);

			}
		}else{
			pid_TA3=fork();
			if(pid_TA3==0){
				// ################################## TA 3 ###################################
				char data[BUFF_SIZE];
				for(int i=0;i<TOTAL_ELEMENTS/4;i++){

					// SYNCHRONIZE WITH WAIT AND FULL SEMAPHORES
					sem_wait(full_TA3);
					close(pipe_TA3[1]);
					read(pipe_TA3[0],data,BUFF_SIZE);

					// SYNCHRONIZING WITH MUTEX FOR SHARED VARIABLE 
					sem_wait(mutex);
					*sum=*sum+atoi(data);
					sem_post(mutex);

					//printf("Sum:%d\n",*sum);
					printf("<Completed,3,%s>\n",data);
					sem_post(empty_TA3);
				}
			}else{							

				pid_TA4=fork();
				if(pid_TA4==0){
					// ################################## TA 4 ###################################
					char data[BUFF_SIZE];
					for(int i=0;i<TOTAL_ELEMENTS/4;i++){

						// SYNCHRONIZE WITH WAIT AND FULL SEMAPHORES
						sem_wait(full_TA4);
						close(pipe_TA4[1]);
						read(pipe_TA4[0],data,BUFF_SIZE);

						// SYNCHRONIZING WITH MUTEX FOR SHARED VARIABLE 
						sem_wait(mutex);
						*sum=*sum+atoi(data);
						sem_post(mutex);

						//printf("Sum:%d\n",*sum);
						printf("<Completed,4,%s>\n",data);
						sem_post(empty_TA4);
					}
				}else{

					// ################################## COURSE INSTRUCTOR ###################################	
					int flag_full_TA1=0;
					int flag_full_TA2=0;
					int flag_full_TA3=0;
					int flag_full_TA4=0;
					int flag_job_TA1=0;
					int flag_job_TA2=0;
					int flag_job_TA3=0;
					int flag_job_TA4=0;
					int mssg=0;
					int job_count1=0,job_count2=0,job_count3=0,job_count4=0;
					

					// LOOP UNTIL ALL THE JOBS ARE PRODUCED
					for(int i=1;i<=TOTAL_ELEMENTS;){
						printf("************* %d",flag_full_TA1);
						char data[BUFF_SIZE];
						
					// ######################################### MAKE JOBS FOR TA 1 ###########################################	
						// MAKE JOBS FOR TA 1 IF QUEUE FULL THEN SKIP
						mssg=sem_trywait(empty_TA1);
						if(mssg!=-1){
							flag_full_TA1=0;
							// CHECK FOR LIMIT OF JOBS TA 1 CAN HAVE (i.e. 1/4th of the Total Jobs)
							if(job_count1<TOTAL_ELEMENTS/4){								
								printf("<Assigned,1,%d>\n",i);
								sprintf(data,"%d",i);
								i++;
								job_count1++;
								close(pipe_TA1[0]);
								write(pipe_TA1[1],data,BUFF_SIZE);
							}else{
								if(flag_job_TA1==0){
									flag_job_TA1=1;
									printf("Done Producing Job for TA1\n");
								}
							}
							sem_post(full_TA1);
						}else{
							if(flag_full_TA1==0){
								printf("Queue 1 Full\n");
								flag_full_TA1=1;
							}
						}

						//BREAK IF ALL THE JOBS ARE PRODUCED
						if(i>TOTAL_ELEMENTS)
							break;

					// ######################################### MAKE JOBS FOR TA 2 ###########################################
						// MAKE JOBS FOR TA 2 IF QUEUE FULL THEN SKIP
						mssg=sem_trywait(empty_TA2);
						if(mssg!=-1){
							flag_full_TA2=0;
							// CHECK FOR LIMIT OF JOBS TA 2 CAN HAVE (i.e. 1/4th of the Total Jobs)
							if(job_count2<TOTAL_ELEMENTS/4){
								printf("<Assigned,2,%d>\n",i);
								sprintf(data,"%d",i);
								i++;
								job_count2++;
								close(pipe_TA2[0]);
								write(pipe_TA2[1],data,BUFF_SIZE);
							}else{
								if(flag_job_TA2==0){
									flag_job_TA2=1;
									printf("Done Producing Job for TA2\n");
								}
							}
							sem_post(full_TA2);
						}else{
							if(flag_full_TA2==0){
								printf("Queue 2 Full\n");
								flag_full_TA2=1;
							}
						}

						//BREAK IF ALL THE JOBS ARE PRODUCED
						if(i>TOTAL_ELEMENTS)
							break;

					// ######################################### MAKE JOBS FOR TA 3 ###########################################
						// MAKE JOBS FOR TA 3 IF QUEUE FULL THEN SKIP
						mssg=sem_trywait(empty_TA3);
						if(mssg!=-1){
							flag_full_TA3=0;
							// CHECK FOR LIMIT OF JOBS TA 3 CAN HAVE (i.e. 1/4th of the Total Jobs)
							if(job_count3<TOTAL_ELEMENTS/4){
								printf("<Assigned,3,%d>\n",i);
								sprintf(data,"%d",i);
								i++;
								job_count3++;
								close(pipe_TA3[0]);
								write(pipe_TA3[1],data,BUFF_SIZE);
							}else{
								if(flag_job_TA3==0){
									flag_job_TA3=1;
									printf("Done Producing Job for TA3\n");
								}

							}
							sem_post(full_TA3);
						}else{
							if(flag_full_TA3==0){
								printf("Queue 3 Full\n");
								flag_full_TA3=1;
							}
						}

						//BREAK IF ALL THE JOBS ARE PRODUCED
						if(i>TOTAL_ELEMENTS)
							break;

					// ######################################### MAKE JOBS FOR TA 4 ###########################################
						// MAKE JOBS FOR TA 4 IF QUEUE FULL THEN SKIP
						mssg=sem_trywait(empty_TA4);
						if(mssg!=-1){
							flag_full_TA3=0;
							// CHECK FOR LIMIT OF JOBS TA 4 CAN HAVE (i.e. 1/4th of the Total Jobs)
							if(job_count4<TOTAL_ELEMENTS/4){
								printf("<Assigned,4,%d>\n",i);
								sprintf(data,"%d",i);
								i++;
								job_count4++;
								close(pipe_TA4[0]);
								write(pipe_TA4[1],data,BUFF_SIZE);
							}else{
								if(flag_job_TA4==0){
									flag_job_TA4=1;
									printf("Done Producing Job for TA4\n");
								}

							}
							sem_post(full_TA4);
						}else{
							if(flag_full_TA4==0){
								printf("Queue 4 Full\n");
								flag_full_TA4=1;
							}
						}

						//BREAK IF ALL THE JOBS ARE PRODUCED
						if(i>TOTAL_ELEMENTS)
							break;

					}

					//WAIT FOR ALL THE TA PROCESSES TO COMPLETE 
					int status;
					waitpid(pid_TA1,&status,0);
					waitpid(pid_TA2,&status,0);
					waitpid(pid_TA3,&status,0);
					waitpid(pid_TA4,&status,0);
					printf("\nJob count 1:%d\n",job_count1);
					printf("Job count 2:%d\n",job_count2);
					printf("Job count 3:%d\n",job_count3);
					printf("Job count 4:%d\n",job_count4);
					
				}
			}
		}
	}	
	
	//PRINT THE FINAL SUM AND UNLINK ALL THE SEMAPHORES
	printf("Sum:%d\n",*sum);
	unlink_semaphores();

	return 0;
}	
