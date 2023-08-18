/* run using: ./load_gen localhost <server port> <number of concurrent users>
   <think time (in s)> <test duration (in s)> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>

#include <pthread.h>
#include <sys/time.h>

int time_up;
FILE *log_file;

// user info struct
struct user_info
{
  // user id
  int id;

  // socket info
  int portno;
  char *hostname;
  float think_time;

  // user metrics
  int total_count;
  float total_rtt;
};

// error handling function
void error(char *msg)
{
  perror(msg);
  exit(0);
}

// time diff in seconds
float time_diff(struct timeval *t2, struct timeval *t1)
{
  return (t2->tv_sec - t1->tv_sec) + (t2->tv_usec - t1->tv_usec) / 1e6;
}

// user thread function
void *user_function(void *arg)
{
  /* get user info */
  struct user_info *info = (struct user_info *)arg;
  struct timeval start, end;

  while (1)
  {
    /* start timer */
    gettimeofday(&start, NULL);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct hostent *host = gethostbyname(info->hostname);
    if (host == NULL)
    {
      error("cant resolve hostname");
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(info->portno);
    bcopy((char *)(*host).h_addr, (char *)&serv_addr.sin_addr.s_addr, host->h_length);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
      error("connect failed");
    char buffer[1024];
    char *buf = "GET /index.html HTTP/1.0\n";
    bzero(buffer, 1024);
    strcpy(buffer, buf);
    if (write(sockfd, buffer, strlen(buf)) < 0)
    {
      error("wrtie failed");
    }
    bzero(buffer, 1024);
    if (read(sockfd, buffer, 1023) < 0)
    {
      printf("\n read failed\n");
    }
    // printf("%s",buffer);
    /* TODO: create socket */

    /* TODO: set server attrs */

    /* TODO: connect to server */

    /* TODO: send message to server */

    /* TODO: read reply from server */

    /* TODO: close socket */

    /* end timer */
    close(sockfd);
    gettimeofday(&end, NULL);
    /* if time up, break */
    if (time_up)
      break;

    info->total_count++;
    info->total_rtt = info->total_rtt + time_diff(&end, &start);
    usleep(info->think_time*1e6);
    /* TODO: update user metrics */

    /* TODO: sleep for think time */
  }

  /* exit thread */
  fprintf(log_file, "User #%d finished\n", info->id);
  fflush(log_file);
  pthread_exit(NULL);
}




int main(int argc, char *argv[])
{
  int user_count, portno, test_duration;
  float think_time;
  char *hostname;

  if (argc != 6)
  {
    fprintf(stderr,
            "Usage: %s <hostname> <server port> <number of concurrent users> "
            "<think time (in s)> <test duration (in s)>\n",
            argv[0]);
    exit(0);
  }

  hostname = argv[1];
  portno = atoi(argv[2]);
  user_count = atoi(argv[3]);
  think_time = atof(argv[4]);
  test_duration = atoi(argv[5]);

  printf("Hostname: %s\n", hostname);
  printf("Port: %d\n", portno);
  printf("User Count: %d\n", user_count);
  printf("Think Time: %f s\n", think_time);
  printf("Test Duration: %d s\n", test_duration);

  /* open log file */
  log_file = fopen("load_gen.log", "w");
  FILE *plot_file=fopen("plot_file.txt","a+");

  pthread_t threads[user_count];
  struct user_info info[user_count]; // int s[5] ;
  struct timeval start, end;

  /* start timer */
  gettimeofday(&start, NULL);
  time_up = 0;
  pthread_t td;
  for (int i = 0; i < user_count; ++i)
  {
    /* TODO: initialize user info */
    info[i].portno = portno;
    info[i].hostname = hostname;
    info[i].id = i;
    info[i].think_time = think_time;
    info[i].total_rtt=0;
    info[i].total_count=0;
    int t_id = pthread_create(&threads[i], NULL, &user_function, (void *)&info[i]);
    /* TODO: create user thread */
    if (t_id != 0)
    {
      error("theread creation failed");
    }
    fprintf(log_file, "Created thread %d\n", i);
  }
  sleep(test_duration);
  /* TODO: wait for test duration */

  fprintf(log_file, "Woke up\n");

  /* end timer */
  time_up = 1;
  gettimeofday(&end, NULL);
  for (int i = 0; i < user_count; i++)
  {
    pthread_join(threads[i], NULL);
  }
  long int total_requests = 0;
  float response_time = 0.0;
  for (int i = 0; i < user_count; i++)
  {
    total_requests += info[i].total_count;
    response_time += info[i].total_rtt;
  }

  float r_time = response_time / (float)total_requests;
  fprintf(plot_file,"User Count: %d\t", user_count);
  fprintf(plot_file,"Think Time: %f s\t", think_time);
  fprintf(plot_file,"Test Duration: %d s\t", test_duration);
  fprintf(plot_file,"total requests-> %ld\t", total_requests);
  fprintf(plot_file,"total round trip time: %f\t", response_time);
  printf("total round trip time: %f\t", response_time);

  fprintf(plot_file,"avg response time: %f\t", r_time);
  int t_put = (int)((float)total_requests/test_duration);
  fprintf(plot_file, "throughput: %d\n", t_put);
  /* TODO: wait for all threads to finish */

  /* TODO: print results */

  /* close log file */
  fclose(log_file);

  return 0;
}
