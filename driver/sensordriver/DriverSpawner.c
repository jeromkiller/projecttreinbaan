/*
this program will probably give recieve a pipe when its created from the sync server
(but i'm not going to worry about that yet)

this program should do the following:
create two pipes for comunitaction with the driver,
fork off a child process,
make that child execve into the driver, and let it take its pipes with it
(that way the driver can do its thing, and send information back to the event handler)

then either this program wil execve into the event handler program, or it will just start running that code directly

*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include "linkedlist.h"

void sensor_event_start(/*int sync_pipe,*/ int driver_pipe);

char *read_driver(int driver_pipe);


int main(int argc, char *argv[], char *env[])
{
    /* variables */
    int driverPipe[2];
    pid_t pid;
    int error;

    char minibuf[100];

    /* open up a pipe */
    error = pipe(driverPipe);
    if (error == -1)
    {
        printf("error on pipe creation\n");
        exit(1);
    }

    pid = fork();
    if (pid == -1)
    {
        printf("error on fork\n");
        exit(1);
    }

    if (pid == 0)
    { /*child process*/
        /* closes output side of the pipe */
        close(driverPipe[0]);
        /* redirect stdout to pipe */
        error = dup2(driverPipe[1], 1);
        if (error == -1)
        {
            printf("dup failed...\n");
        }
        /* replace child process with the driver */
        error = execve("a.out", argv, env);
        
        if(error == -1){
          printf("execve returned -1\n");
        }
        /* if this code gets to run, that means the execve failed */
        printf("execve failed :(\n");
        exit(2);
    }
    else
    { /* parent process */
        /* closes input side of the pipe */
        close(driverPipe[1]);

        /* set the recieving end to nonblocking */
        error = fcntl(driverPipe[0], F_SETFL, O_NONBLOCK);
        if(error == -1)
        {
          printf("failed to set read end to nonblock");
        }

        sensor_event_start(/*sync_pipe,*/ driverPipe[0]);
    }
    exit(0);
}

void sensor_event_start(/*int sync_pipe,*/ int driver_pipe)
{
    char inBuff[sizeof(int16_t)];
    int16_t val = 0;
    int error;

    while (1)
    {
        size_t number = 0;
        /* the event handler checks to see if the driver has sent it a message */
        read_driver(driver_pipe);
  
        /* then it prepares a sensitivity list to send to the sync server */
        /* then it sends the sensitivities */
    }
}

char *read_driver(int driver_pipe)
{
    struct sensorupdate TMPelement;
    int16_t val;
    int nread;

    nread = read(driver_pipe, &TMPelement, sizeof(struct sensorupdate));
/*    printf("number of bytes read: %d\n", nread);*/
    switch(nread){
    case -1:
        if(errno == EAGAIN){
          /*if this piece of code gets hit, then there is no data on the pipe and no action has to be taken */
        }else{
          perror("error on read from pipe");
          exit(3);
        }
        break;
    case 0:
      printf("eof reached\n");
      break;
    default:
      printf("recieved from pipe:\n");
 /*     printf("Sensor Number: %hd\n", val);
      read(driver_pipe, &val, sizeof(int16_t));
      printf("sensor state: %hd\n", val);
  */
      printf("Sensor Number: %hd\n Sensor State: %hd\n", TMPelement.sensor, TMPelement.state);
    }
}

/*char *recieve_action(char *action, struct exec_data *data)
{

}
*/

/*void send_sensitivity(int *CURRENT_SENS, int *NEW_SENS, struct exec_data *data){

}
*/