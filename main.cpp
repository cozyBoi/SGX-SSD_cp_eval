#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <pthread.h>

typedef struct packet{
    int flag;
    int pid;
    int fid;
} _packet;

typedef struct packet_out{
    int flag;
    char dir[100];
} _packet_out;

sem_t mysem0, mysem1;
int shmid_pid;
int shmid_out;
_packet*shmaddr_f_to_p;
_packet_out*shmaddr_out;

#define DIRRECTORY "/Users/ppp123/Desktop/eval1"
#define dstDIR "/Users/ppp123/Desktop/eval"
#define BUF_SIZE 4096

char buf[BUF_SIZE];

int main(void)
{
    struct dirent *de;  // Pointer for directory entry
 
    // opendir() returns a pointer of DIR type.
    DIR *dr = opendir(DIRRECTORY);
 
    if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory" );
        return 0;
    }
    
    shmid_pid = shmget((key_t)0x1237, sizeof(_packet), IPC_CREAT | 0666);
    shmaddr_f_to_p = (_packet*)shmat(shmid_pid, NULL, 0);

    shmid_out = shmget((key_t)0x1236, sizeof(_packet_out), IPC_CREAT | 0666);
    shmaddr_out = (_packet_out*)shmat(shmid_out, NULL, 0);
    
    char srcPath[1000];
    char dstPath[1000];
    //sem_init(&mysem0, 0, 1);
    while ((de = readdir(dr)) != NULL)
    {
        if (de->d_type == DT_REG){
            //printf("%s\n", de->d_name);
            int len = strlen(DIRRECTORY);
            memcpy(srcPath, DIRRECTORY, len);
            memcpy(&srcPath[len], "/", 1);
            memcpy(&srcPath[len+1], de->d_name, strlen(de->d_name));
            memcpy(&srcPath[len+1 + strlen(de->d_name)], "\0", 1);
            printf("src path : %s\n", srcPath);
            
            len = strlen(dstDIR);
            memcpy(dstPath, dstDIR, len);
            memcpy(&dstPath[len], "/", 1);
            memcpy(&dstPath[len+1], de->d_name, strlen(de->d_name));
            memcpy(&dstPath[len+1 + strlen(de->d_name)], "\0", 1);
            printf("dst path : %s\n", dstPath);
            /*
            sem_wait(&mysem0);
            strcpy(shmaddr_out->dir, dstPath);
            shmaddr_out->flag = 1;
            
            shmaddr_f_to_p->flag = 0;
            while(!shmaddr_f_to_p->flag);
            int pid = shmaddr_f_to_p->pid;
            pid = 0x11223344;
            int fid = shmaddr_f_to_p->fid;
            sem_post(&mysem0);*/
            

            int srcFd = open(srcPath, O_RDWR|O_CREAT);
            int dstFd = open(dstPath, O_RDWR|O_CREAT);
            //int dstFd = syscall(OPEN_KEY, file_name, O_RDWR|O_CREAT, 0, pid ,fid);
            long long int file_size =  lseek(srcFd, 0, SEEK_END) + 1;
            lseek(srcFd, 0, SEEK_SET);
            lseek(dstFd, 0, SEEK_SET);
            
            for(int i=0; i < file_size/BUF_SIZE; i++){
                read(srcFd, buf, BUF_SIZE);
                write(dstFd, buf, BUF_SIZE);
            }
            if(file_size % BUF_SIZE > 0){
                read(srcFd, buf, file_size % BUF_SIZE);
                write(dstFd, buf, file_size % BUF_SIZE);
            }
            close(srcFd);
            close(dstFd);
        }
    }
 
    closedir(dr);
    return 0;
}
