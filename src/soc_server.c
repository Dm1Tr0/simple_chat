/*
    the project is under development
*/
#include "common.h"
#define MAX_CLIENTS_NUM 80


pthread_mutex_t m_lock = PTHREAD_MUTEX_INITIALIZER;

/*
    this is a draft of a mapping type
    that will be used in order to handle
    mistakes

   static enum errs {
   errcodes
   ...
}
    static char[] = {
    1 : error while adding a new client func client_add
    2 : error while removing a client client_remove
    3 : error while sending a message to a client
    4 : error while sending a message to all clients
    5 : error while writing to myself
}
*/
/* Client structure */
struct client {
    /* Client remote address */
    struct sockaddr_in addr;
    /*clients name*/
    char c_name[40];
    /* Connection file descriptor */
    int connfd;
    /* Client unique identifier */
    int cuid;
};

/* the variable that keeps a count of clients.
It's much more comfortable to use atomic variable instead of
mutex lock even in spite of the fact that we lose some speed */
static _Atomic unsigned int client_count = 0;

struct client *clients [MAX_CLIENTS_NUM];

static int client_add(struct client * cli)
{
    pthread_mutex_lock(&m_lock);
    for (int i = 0; i < MAX_CLIENTS_NUM; i++) {
        if (clients[i] ==NULL) {
            clients[i] = cli;
            pthread_mutex_unlock(&m_lock);
            return 0;
        }

    }
    pthread_mutex_unlock(&m_lock);
    return 1;
}


static int  client_remove(int c_uid)
{
    pthread_mutex_lock(&m_lock);
    for (int i = 0; i < MAX_CLIENTS_NUM; i++) {
        if (clients[i]->cuid == c_uid) {
            clients[i] = NULL;
            pthread_mutex_unlock(&m_lock);
            return 0;
        }
    }
    pthread_mutex_unlock(&m_lock);
    return 2;
}

/* send a message */
int message_direct(char * str, int c_uid )
{
    pthread_mutex_lock(&m_lock);
    for (int i = 0; i < MAX_CLIENTS_NUM ; ++i) {
        if (clients[i]) {
            if (clients[i]->cuid == c_uid) {
                if (write(clients[i]->connfd, str, strlen(str)) < 0) {
                    perror("Write to descriptor failed");
                    pthread_mutex_unlock(&m_lock);
                    return 3;
                } else {
                    pthread_mutex_unlock(&m_lock);
                    return 0;
                }
            }
        }
    }
    return 6;
}
/* the function send a message to everybod */
int message_all(char * str)
{
    pthread_mutex_lock(&m_lock);
    for (int i = 0; i <MAX_CLIENTS_NUM ; ++i){
        if (clients[i]) {
            if (write(clients[i]->connfd, str, strlen(str)) < 0) {
                pthread_mutex_unlock(&m_lock);
                return 4;
            }
        }
    }
    pthread_mutex_unlock(&m_lock);
    return 0;
}
/* the function send a message to the client */
int message_self(const char * str, int connfd){
    if (write(connfd, str, strlen(str)) < 0) {
        perror("Write to descriptor failed");
        return 5;
    }
    return 0;
}

/* the function outputs the names of all clients */
char * list_users ()
{
    char * nics = malloc(1);
    pthread_mutex_lock(&m_lock);
    int size = 0;

    for (int i = 0; i <MAX_CLIENTS_NUM ; ++i){
        if (clients[i] != NULL) {
            nics = (char *)realloc(nics,size =
                 size +
                 strlen(clients[i]->c_name) +
                 sizeof("\n") + 1);

            strcat(nics, clients[i]->c_name);
            strcat(nics, "\n");
        }
    }
    pthread_mutex_unlock(&m_lock);
    return nics;
}

/* get the client's personal id */
int get_c_uid(char * name)
{
    pthread_mutex_lock(&m_lock);
    for (int i = 0; i < MAX_CLIENTS_NUM ; ++i) {
        if (clients[i]) {
            if (!strcmp(clients[i]->c_name ,name)) {
                    pthread_mutex_unlock(&m_lock);
                    return clients[i]->cuid;
            }
        }
    }
    pthread_mutex_unlock(&m_lock);
    return 0;
}

void help(int connfd){
    message_self("/quit           :  exit_chatroom \n", connfd);//ch dont forget to put
    message_self("/s_msg|who|text :  send a message\n", connfd);
    message_self("/s_msg_all|text :  send a message to all users\n", connfd);
    message_self("/list_users     :  to see who is currently online\n", connfd);
    message_self("/help           :  help\n",connfd);
    message_self("/in_msg         :  to see who  currently online\n", connfd);
    message_self("  nicknames are currently unavailable \n", connfd);
    message_self("  but they will be implemented soon\n", connfd);
}

void *handle_client(void *arg)
{
    char buff_in[MAX];
    char out[MAX];
    char * command;
    int read_byte;

    client_count++;
    struct client *cli = (struct client *)arg;

    log("<< accept ");
    log(" referenced by %d", cli->cuid);

    sprintf(out, "<< %s has joined\r\n", cli->c_name);
    message_all(out);

    message_self("users online : \n", cli->connfd);
    message_self(list_users (), cli->connfd);

    while ((read_byte = read(cli->connfd, buff_in, sizeof(buff_in))) > 0) {
        log("%s : %d",buff_in, read_byte);
/* getting a command from the input buffer */
        command = strtok(buff_in,"|\n");
        log("%s : %d",buff_in, read_byte);
        if (command[0] != '/') {
            help(cli->connfd);
            continue;
        } else if (!strcmp("/s_msg", command)) {
            char * name;
            char * text;
            int c_uid;
            if ( (name = strtok(NULL, "|\n")) ) {
                    log("name : %s ", name);
                if(!(c_uid = get_c_uid(name)))
                    message_self("there is no such user currently in the chat \n",
                        cli->connfd);
            } else {
                message_self("enter the name of the user",cli->connfd);
                continue;
            }
            if ( (text = strtok(NULL, "|\n")) ) {
                    strcat(text,"\n");
                    log("text %s",text);
                    message_direct(text,c_uid);
            } else
                    message_self("enter the text tou want to send ",
                        cli->connfd);
        } else if (!strcmp("/list_users", command )) {
            log("here");
            message_self("users online : \n", cli->connfd);
            log("users : \n %s", list_users ());
            message_self(list_users (), cli->connfd);
        } else if (!strcmp("/quit", command )) {
            break;
        } else if (!strcmp("/s_msg_all", command )) {
            char * text = strtok(NULL, "|\n");
            strcat(text,"\n");
            log("text %s",text);
            message_all(text);

        } else if (!strcmp("/help", command )) {
            help(cli->connfd);
        }

        bzero(&buff_in, sizeof(buff_in));
    }
    /* Close connection */
    sprintf(out, "<< %s has left\r\n", cli->c_name);
    message_all(out);
    close(cli->connfd);

    /* Delete client from queue */
    client_remove(cli->cuid);
    printf("<< quit ");
    printf(" referenced by %d\n", cli->cuid);
    free(cli);
    client_count--;
    /* detech a thread */
    pthread_detach(pthread_self());
    return NULL;
}

//ch void pirnt_clients_ip() do I need it

int main()
{
	int soclistenfd, connfd;
 	struct sockaddr_in serv_addr, cli_addr;
    pthread_t tid;

	if ((soclistenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		exit_ha("error while creating socket");
	}

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // long
	serv_addr.sin_port        = htons(SERVER_PORT); // short

	if ((bind(soclistenfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) < 0) {
		exit_ha("error while binding");
	}

	if (listen(soclistenfd, 10) < 0) {
		exit_ha("error while listening");
	}

	log("getting to the work");

    while (1) {
        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(soclistenfd, (struct sockaddr*)&cli_addr, &clilen);
        /* Check if max clients is reached */
        if ((client_count + 1) == MAX_CLIENTS_NUM) {
            printf("<< max clients reached\n");
            printf("<< reject ");
            printf("\n");
            close(connfd);
            continue;
        }

        //ch inet_ntop(AF_INET, &addr, addres_str, MAX); in main you should add it

        /* Client settings */
        struct client *cli = (struct client *)malloc(sizeof(struct client));
        cli->addr = cli_addr;
        cli->connfd = connfd;
        cli->cuid = client_count + 10;
        sprintf(cli->c_name, "%d", cli->cuid); //ch

        /* Add client to the queue and fork thread */
        client_add(cli);
        pthread_create(&tid, NULL, &handle_client, (void*)cli);

        /* Reduce CPU usage */
        sleep(1);
    }

		return 0;
}
