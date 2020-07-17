
#include "common.h"
#define MAX_CLIENTS_NUM 80


pthread_mutex_t m_lock = PTHREAD_MUTEX_INITIALIZER;

/**the structure handles clients*/
struct client {
    /** Client remote address */
    struct sockaddr_in addr;
    /**clients name*/
    char c_name[40];
    /** Connection file descriptor */
    int connfd;
    /** Client unique identifier */
    int cuid;
};

/** the variable that keeps a count of clients.
It's much more comfortable in our case to use atomic variable instead of
mutex lock even in spite of the fact that we lose some speed */
static _Atomic unsigned int client_count = 0;

struct client *clients [MAX_CLIENTS_NUM];
/**
 * add a client
 *@cli: slient structure that is commented
 */
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

/**
 * remove client
 *@c_uid: clients unique id that is kept in clients structure
 */
static int  client_remove(int c_uid)
{
    pthread_mutex_lock(&m_lock);
    for (int i = 0; i < MAX_CLIENTS_NUM; i++) {
        if(clients[i]) {
            if (clients[i]->cuid == c_uid) {
                clients[i] = NULL;
                pthread_mutex_unlock(&m_lock);
                return 0;
            }
        }
    }
    pthread_mutex_unlock(&m_lock);
    return 1;
}


/**
 *send a message
 *@str: a text to sender
 *@c_uid: unique id of a client that gets a message
 */
int message_direct(char * str, int c_uid )
{
    pthread_mutex_lock(&m_lock);
    for (int i = 0; i < MAX_CLIENTS_NUM ; ++i) {
        if (clients[i]) {
            if (clients[i]->cuid == c_uid) {
                if (write(clients[i]->connfd, str, strlen(str)) < 0) {
                    perror("Write to descriptor failed");
                    pthread_mutex_unlock(&m_lock);
                    return 1;
                } else {
                    pthread_mutex_unlock(&m_lock);
                    return 0;
                }
            }
        }
    }
    return 1;
}

/**
 *the function send a message to everybod exept the sender
 *@str: a text to sender
 *@c_uid: unique id of a sender
 */
int message_all(char * str, int c_uid)
{
    pthread_mutex_lock(&m_lock);
    for (int i = 0; i <MAX_CLIENTS_NUM ; ++i){
        if (clients[i]) {
            if(clients[i]->cuid != c_uid){
            if (write(clients[i]->connfd, str, strlen(str)) < 0) {
                pthread_mutex_unlock(&m_lock);
                return 1;
            }
            }
        }
    }
    pthread_mutex_unlock(&m_lock);
    return 0;
}

/**
 *the function send a message to the sender
 *@str: a text to sender
 *@c_uid: unique id of the client
 */
int message_self(const char * str, int connfd){
    if (write(connfd, str, strlen(str)) < 0) {
        perror("Write to descriptor failed");
        return 1;
    }
    return 0;
}

/** the function outputs the names of all clients */
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


/**
 *get the client's personal id
 *@name: name of a client whoo's uid you want to get
 */
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

char * inettos(void * addr)
{
     /* 16 is a length of ipv4 represented in string*/
    char * addr_str = malloc(16);
    inet_ntop(AF_INET, addr, addr_str, MAX);
    return addr_str;
}

void help(int connfd){
    message_self("/quit           :  exit_chatroom \n", connfd);
    message_self("/s_msg|who|text :  send a message\n", connfd);
    message_self("/s_msg_all|text :  send a message to all users\n", connfd);
    message_self("/list_users     :  to see who is currently online\n", connfd);
    message_self("/help           :  help\n",connfd);
    message_self("/in_msg         :  to see who  currently online\n", connfd);
    message_self("/nick           :  change nickname \n", connfd);

}
/**
 *the function handles client and respongs to clients requests in the different thread
 *@arg: structure of a client to handle
 */

void *handle_client(void *arg)
{
    char buff_in[MAX];
    char out[MAX];
    char * command;
    int read_byte;

    client_count++;
    struct client *cli = (struct client *)arg;

    log("<< accept referenced by %d; address : %s", cli->cuid, inettos(&cli->addr));

    sprintf(out, "<< %s has joined\r\n", cli->c_name);
    message_all(out, cli->cuid);

    message_self("users online : \n", cli->connfd);
    message_self(list_users (), cli->connfd);

    bzero(out, sizeof(out));

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
                message_self("enter the name of the user \n",cli->connfd);
                continue;
            }
            if ( (text = strtok(NULL, "|\n")) ) {
                    sprintf(out," %s : %s\n",cli->c_name, text);
                    log("text %s",out);
                    message_direct(out,c_uid);
                    bzero(out, sizeof(out));
            } else
                    message_self("enter the text tou want to send\n",
                        cli->connfd);
        } else if (!strcmp("/list_users", command )) {
            log("here");
            message_self("users online : \n", cli->connfd);
            log("users : \n %s", list_users ());
            message_self(list_users (), cli->connfd);
        } else if (!strcmp("/quit", command )) {
            break;
        } else if (!strcmp("/s_msg_all", command )) {
            char * text;

            if ( (text = strtok(NULL, "|\n")) ) {
                    sprintf(out,"%s to everyone : %s\n", cli->c_name, text);
                    log("text %s",out);
                    message_all(out, cli->cuid);
                    bzero(out, sizeof(out));
            } else
                    message_self("enter the text tou want to send\n",
                        cli->connfd);

        } else if (!strcmp("/help", command )) {
            help(cli->connfd);
        } else if (!strcmp("/nick",command)) {
            char * nick;

            if ( (nick = strtok(NULL, "|\n")) ) {
                    sprintf(out,"%s changed nicnkame to %s\n",cli->c_name,nick);
                    message_all(out, cli->cuid);
                    bzero(out, sizeof(out));
                    bzero(cli->c_name,sizeof(cli->c_name));
                    sprintf(cli->c_name, "%s", nick);

            } else
                    message_self("enter the nick you want to use\n",
                        cli->connfd);
        }

        bzero(&buff_in, sizeof(buff_in));
    }
    /* Close connection */
    sprintf(out, "<< %s has left\r\n", cli->c_name);
    message_all(out, cli->cuid);
    close(cli->connfd);

    /* Delete client */
    client_remove(cli->cuid);
    log("quit referenced by %d", cli->cuid);
    free(cli);
    client_count--;
    /* detech a thread */
    pthread_detach(pthread_self());
    return NULL;
}

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


        /* Client settings */
        struct client *cli = (struct client *)malloc(sizeof(struct client));
        cli->addr = cli_addr;
        cli->connfd = connfd;
        cli->cuid = client_count + 10;
        sprintf(cli->c_name, "%d", cli->cuid); //ch

        /* Add client and make new thread */
        client_add(cli);
        pthread_create(&tid, NULL, &handle_client, (void*)cli);

        /* Reduce CPU usage */
        sleep(1);
    }

		return 0;
}
