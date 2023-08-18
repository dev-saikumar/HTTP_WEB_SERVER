#include <bits/stdc++.h>
#include <sys/socket.h> //for socket
#include <pthread.h>
#include <string.h>     // bzero()
#include <netinet/in.h> // for sockaddr_in struct
#include <unistd.h>     // for read and write system call
#include "./http_server_functions.cpp"
using namespace std;

pthread_mutex_t m;
pthread_cond_t cv;
queue<int> fd_q;

int check_for_new_connection()
{
    int temp;
    pthread_mutex_lock(&m);
    while (fd_q.empty())
    {
        pthread_cond_wait(&cv, &m);
    }
    temp = fd_q.front();

    fd_q.pop();

    pthread_mutex_unlock(&m);
    return temp;
}

// thread function to send messeges
void *thread_function(void *args)
{
    while (1)
    {
        int csock = check_for_new_connection(); // check for new connection and waits or
        char buffer[2048];
        bzero(&buffer, 2048);
        int n = read(csock, buffer, 2047);
        if (n < 0)
        { // read failed
            perror("read failed:");
            close(csock);
            continue;
        }
        // read from client is successfull
        cout << buffer << endl;
        HTTP_Response *res = handle_request(buffer);
        bzero(&buffer, 256);
        // sending response to client
        string msg = res->get_string();

        // strcpy(buffer,msg.c_str());
        int w_l = write(csock, msg.c_str(), msg.size());
        if (w_l < 0)
        {
            perror("server response failed:");
            delete (res);
            close(csock);
            continue;
        }

        // response successfully sent

        delete (res);
        close(csock);
    }
    return NULL;
}

void add_connection_to_q(int sock)
{
    pthread_mutex_lock(&m);
    fd_q.push(sock);
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&m);
}

int main(int args, char **argv)
{

    int psock, csock, portno;
    socklen_t cln_len;
    struct sockaddr_in srv_addr, cln_addr;
    if (args != 2)
    {
        cout << "usage:" << argv[0] << " port_NO" << endl;
        exit(1);
    }
    psock = socket(AF_INET, SOCK_STREAM, 0); // written  sock fd or -1 on error
    if (psock < 0)
    {
        cout << "SOmething went wrong" << endl;
        exit(1);
    }
    bzero(&srv_addr, sizeof(srv_addr));
    portno = atoi(argv[1]);
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(portno); // changes the host or system order of bytes to network order bytes like bigendian to little endian
    if (bind(psock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
    { // -1 on error
        perror("bind failed:");
        exit(1);
    }
    cout << "binded successfully" << endl;
    if (pthread_cond_init(&cv, NULL) != 0)
    {
        cout << endl
             << "sys err: lock initialisation failed";
        return 0;
    }
    // creating pool of threads
    if (pthread_mutex_init(&m, NULL) != 0)
    {
        cout << endl
             << "sys err: lock initialisation failed";
        return 0;
    }
    pthread_t newthread_id;
    for (int i = 0; i < 10; i++)
    {
        int p_thrd = pthread_create(&newthread_id, NULL, thread_function, NULL);
        if (p_thrd != 0)
        {
            perror("thread creation failed:");
            exit(1);
        }
    }

    listen(psock, 100); // 0 on success -1 on failure
    while (1)
    {
        cln_len = sizeof(cln_addr);
        csock = accept(psock, (struct sockaddr *)&cln_addr, &cln_len); // -1 on error
        add_connection_to_q(csock);
    }
    close(psock);
    return 0;
}