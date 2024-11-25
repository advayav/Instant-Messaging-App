#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

//Handling messages from the server
volatile sig_atomic_t usr_msg_num_srv = 0;

//Counter increments when the user has a new message from the server
void user_handle_new_msg_from_srv()
{
  ++usr_msg_num_srv;
}

void user_handler(int userid, int writefd, int readfd[3][2])
{
  //Nanosleep
  struct timespec req, rem; // * advanced sleep which will not be interfered by signals

  req.tv_sec = 3;  // The time to sleep in seconds
  req.tv_nsec = 0; // Additional time to sleep in nanoseconds
  while(nanosleep(&req, &rem) == -1){
    if(errno == EINTR){
      req = rem;
    }
  }

  //Setting up the signal for messages from the server to a user
  struct sigaction sa_usr;
  sigaddset(&sa_usr.sa_mask, SIGRTMIN + userid);
  sigprocmask(SIG_UNBLOCK, &sa_usr.sa_mask, NULL);

  sa_usr.sa_flags = SA_RESTART;
  sa_usr.sa_handler = user_handle_new_msg_from_srv;
  sigaction(SIGRTMIN + userid, &sa_usr, NULL);

  char buffer[128];

  //To send the messages from the user
  //Checks userid so that all users are not using the same signal to send the message
  if (userid == 0)
  {
    printf("user_1_sent_msg_to_user_1: All men are mortal \n");
    sprintf(buffer, "user_1_sent_msg_to_user_1: All men are mortal");
    write(writefd, buffer, 128);
    kill(getppid(), SIGRTMIN);
    sleep(1);

    printf("user_1_sent_msg_to_user_2: Socrates is a man \n");
    sprintf(buffer, "user_1_sent_msg_to_user_2: Socrates is a man");
    write(writefd, buffer, 128);
    kill(getppid(), SIGRTMIN);
    sleep(1);

    printf("user_1_sent_msg_to_user_3: Therefore, Socrates is mortal \n");
    sprintf(buffer, "user_1_sent_msg_to_user_3: Therefore, Socrates is mortal");
    write(writefd, buffer, 128);
    kill(getppid(), SIGRTMIN);
    sleep(1);
    
    close(writefd);
    kill(getppid(), SIGRTMIN);
    //End of user 1, signal is sent that the write pipe has been closed
  }

  if (userid == 1)
  {
    printf("user_2_sent_msg_to_user_1: We know what we are \n");
    sprintf(buffer, "user_2_sent_msg_to_user_1: We know what we are");
    write(writefd, buffer, 128);
    kill(getppid(), SIGRTMIN+1);
    sleep(1);

    printf("user_2_sent_msg_to_user_3: But know not what we may be \n");
    sprintf(buffer, "user_2_sent_msg_to_user_3: But know not what we may be");
    write(writefd, buffer, 128);
    kill(getppid(), SIGRTMIN+1);
    sleep(1);
    
    close(writefd);
    kill(getppid(), SIGRTMIN+1);
    //End of user 2, signal is sent that the write pipe has been closed
  }

  if (userid == 2)
  {
    printf("user_3_sent_msg_to_user_1: We wish you a prosperous year of the dragon \n");
    sprintf(buffer, "user_3_sent_msg_to_user_1: We wish you a prosperous year of the dragon");
    write(writefd, buffer, 128);
    kill(getppid(), SIGRTMIN+2);
    sleep(1);

    close(writefd);
    kill(getppid(), SIGRTMIN+2);
    //End of user 3, signal is sent that the write pipe has been closed
  }

  //Counter to check when the read function returns a 0 value
  int cuser = 0;

  //Reading messages from the server
  //Loop runs till the read returns a 0 value
  while(cuser == 0)
  {
    
    if(usr_msg_num_srv > 0)
    {
      
      int readval = read(readfd[userid][0], buffer, 128); //Stores the value of read function
      
      if (readval == 0)
      {
        cuser = 1;
        close(readfd[userid][0]);
      }
      else
      {
        //Parsing the string to get the message
	      char delim[] = "_";
        char *x = strtok(buffer, delim);
        int flagx = 0;
        int flagy = 0;
        char numx, numy; //x and y values for the message where x = receiver, y = sender

        //Extracting the x and y values
        while(x != NULL)
        {
          if (isdigit(*x) && flagy == 0) 
          {
            numy = *x;
            flagy = 1;
            x = strtok(NULL, delim);
            continue;
          }

          if (isdigit(*x) && flagy == 1 && flagx == 0)
          {
            numx = *x;
            flagx = 1;
            x = strtok(NULL, delim);
            continue;
          }

          x = strtok(NULL, delim);

        }

        //Extracting the actual message from the buffer
        char* start = buffer;
        while (start != NULL)
        {
          if (*start == ':') 
          {
            break;
          }
          start++;
        }


        //Printing the message that the user received from the server
        if (start == NULL) 
        {
          perror("Rest of message not received");
        } 
        else 
        {
          printf("user_%c_received_msg_from_user_%c: %s \n", numx, numy, start);
        }
        
        --usr_msg_num_srv;
      }

    }

  }
}

//Handling messages from the user
//Each user has a counter that increments with a new message

//User 1
volatile sig_atomic_t msg_num_usr0 = 0;

void handle_new_msg_user0()
{
  ++msg_num_usr0;
}

//User 2
volatile sig_atomic_t msg_num_usr1 = 0;

void handle_new_msg_user1()
{
  ++msg_num_usr1;
}

//User 3
volatile sig_atomic_t msg_num_usr2 = 0;

void handle_new_msg_user2()
{
  ++msg_num_usr2;
}


void server_handler(int readfd[3][2], int writefd[3][2], pid_t pids[3])
{
  sleep(2);
  
  char buffer[128];

  int cuser0 = 0;//counter to check when the write pipe user 0 has been closed
  int cuser1 = 0;//counter to check when the write pipe user 1 has been closed
  int cuser2 = 0;//counter to check when the write pipe user 2 has been closed

  //Loop to handle the message from user
  while (cuser0 < 1 && cuser1 < 1 && cuser2 < 1)
  {
    sleep(2);

    //Parsing the string to know where the user needs to send the message
    char delim[] = "_";
    char *x = strtok(buffer, delim);
    int flagx = 0;
    int flagy = 0;
    char numx, numy; //x = receiver value, y = sender value

    //Extracting the x and y values
    while(x != NULL){
      if (isdigit(*x) && flagy == 0)
      {
        numy = *x;
        flagy = 1;
        x = strtok(NULL, delim);
        continue;
      }

      if (isdigit(*x) && flagy == 1 && flagx == 0) 
      {
        numx = *x;
        flagx = 1;
        break;
      }

      x = strtok(NULL, delim);
    }

    //Reading messages for user 0
    if (msg_num_usr0 > 0)
    {
      if(read(readfd[0][0], buffer, 128) == 0)
      {
        cuser0++;

        close(writefd[0][1]);
        kill(pids[0], SIGRTMIN);
      }
      else
      {
        if (numx == '1')
        {
          //Message from user 1 to user 1
          write(writefd[0][1], buffer, 128);
          
          kill(pids[0], SIGRTMIN);
        }

        if (numx == '2')
        {
          //Message from user 1 to user 2
          write(writefd[1][1], buffer, 128);
          kill(pids[1], SIGRTMIN+1);
        }

        if(numx == '3')
        {
          //Message from user 1 to user 3
          write(writefd[2][1], buffer, 128);
          kill(pids[2], SIGRTMIN+2);
        }

      }

      --msg_num_usr0; //Decrementing because the message was processed
    }

    //Reading messages for user 1
    if(msg_num_usr1 > 0)
    {
      if(read(readfd[1][0], buffer, 128) == 0)
      {
        cuser1++;

        close(writefd[1][1]);
        kill(pids[1], SIGRTMIN+1);
      }
      else
      {
        if (numx == '1')
        {
          //Message from user 1 to user 0
          write(writefd[1][1], buffer, 128);
          kill(pids[0], SIGRTMIN);
        }

        if (numx == '3')
        {
          //Message from user 1 to user 2
          write(writefd[2][1], buffer, 128);
          kill(pids[2], SIGRTMIN+2);
        }

      }

      --msg_num_usr1; //Decremeting cause message has been processed
    }

    //Reading messages for user 2
    if(msg_num_usr2 > 0)
    {
      if(read(readfd[2][0], buffer, 128) == 0)
      {
        cuser2++;

        close(writefd[2][1]);
        kill(pids[2], SIGRTMIN+2);
      }
      else
      {
        //Message from user 2 to user 0
        write(writefd[2][1], buffer, 128);
        kill(pids[0], SIGRTMIN);
      }

      --msg_num_usr2;
    }

  }
  sleep(3);
  close(readfd[0][0]); //closing read pipe for user 0 in the user to server array
  close(readfd[1][0]); //closing read pipe for user 1 in the user to server array
  close(readfd[2][0]); //closing read pipe for user 2 in the user to server array


  sleep(1);

  close(writefd[0][1]); //closing write pipe for user 0 in the server to user array
  kill(pids[0], SIGRTMIN); //signal sent to user 0 that the pipe was closed
  sleep(3);

  close(writefd[1][1]); //closing write pipe for user 1 in the server to user array
  kill(pids[1], SIGRTMIN+1); //signal sent to user 0 that the pipe was closed
  sleep(3);

  close(writefd[2][1]); //closing write pipe for user 2 in the server to user array
  kill(pids[2], SIGRTMIN+2); //signal sent to user 0 that the pipe was closed
  sleep(3);

}


int main()
{
  //Creating the arrays that will be made into pipes
  int server_to_user[3][2];
  int user_to_server[3][2];
  pid_t pids[3];

  //signal handling for user 1
  struct sigaction sa_usr0;
  sigaddset(&sa_usr0.sa_mask, SIGRTMIN);
  sigprocmask(SIG_UNBLOCK, &sa_usr0.sa_mask, NULL);

  sa_usr0.sa_flags = SA_RESTART;
  sa_usr0.sa_handler = handle_new_msg_user0;
  sigaction(SIGRTMIN, &sa_usr0, NULL);

  //signal handling for user 2
  struct sigaction sa_usr1;
  sigaddset(&sa_usr1.sa_mask, SIGRTMIN+1);
  sigprocmask(SIG_UNBLOCK, &sa_usr1.sa_mask, NULL);

  sa_usr1.sa_flags = SA_RESTART;
  sa_usr1.sa_handler = handle_new_msg_user1;
  sigaction(SIGRTMIN+1, &sa_usr1, NULL);

  //signal handling for user 3
  struct sigaction sa_usr2;
  sigaddset(&sa_usr2.sa_mask, SIGRTMIN+2);
  sigprocmask(SIG_UNBLOCK, &sa_usr2.sa_mask, NULL);

  sa_usr2.sa_flags = SA_RESTART;
  sa_usr2.sa_handler = handle_new_msg_user2;
  sigaction(SIGRTMIN+2, &sa_usr2, NULL);


  for(int i = 0; i < 3; i++)
  {
    //Making the pipes
    pipe(server_to_user[i]);
    pipe(user_to_server[i]);

    //Creating the children
    pid_t pid = fork();
    pids[i] = pid; //Storing the process id for the child


    if(pid == 0){
      close(server_to_user[i][1]); //Closing the write end
      close(user_to_server[i][0]); //Closing the read end

      //Closing the connections between the children
      //This is so that the messages are not leaking between the users
      for(int k = 0; k <= i; k++)
      {
        if(i != k)
        {
          if(k > i)
          {
            if(close(server_to_user[k][0]) == -1)
            {
              perror("Close failure");
            }

            if(close(user_to_server[k][1]) == -1)
            {
              perror("Close failure");
            }

            if(close(user_to_server[k][0]) == -1)
            {
              perror("Close failure");
            }

            if(close(server_to_user[k][1]) == -1)
            {
              perror("Close failure");
            }

          }

        }
      }

      user_handler(i, user_to_server[i][1], server_to_user);
      //User handler ends for the user

      wait(NULL);

      exit(0); //End child process
    }

    if(pid < 0)
    {
      //Error handling for incorrect forking
      perror("Fork failure");
      exit(1); //End parent process
    }

    //Closing all connections at the end once messages have been sent
    close(server_to_user[i][0]);
    close(user_to_server[i][1]);
    pids[i] = pid;
  }

  server_handler(user_to_server, server_to_user, pids);

  //Closing all pipes once both server handler and user handler are
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      close(server_to_user[i][j]);
      close(user_to_server[i][j]);
    }
  }
  //Server handler ends
  return 0;
  //End of main function
}
