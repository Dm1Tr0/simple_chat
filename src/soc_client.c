// dev version
#include "common.h"


off_t to_write;
off_t to_read;

/** the structure is used to pass socket and file ids into the thread*/
struct file_socet {
    /** socet id */
    int socid;
    /** file id */
    int fileid;
};

/**
 *the function always keeps reading socet and write in into the file
 *@f_soc:  strusture that consists of fileid and socketid
 */
void *get_the_messeges(void *f_soc)
{
    int file = ((struct file_socet*)f_soc)->fileid;
    int soc = ((struct file_socet*)f_soc)->socid;
    char buff[MAX];
    int r_bytes;

    while(1) {
        r_bytes = read(soc, buff, sizeof(buff)); //ch it has to be metexed
        if (r_bytes > 0) {
            if(flock(file, LOCK_EX))log("flock_miss");
            /*setting the file pointer offset*/
            lseek(file, to_write, SEEK_SET);
            write(file, buff, sizeof(buff));
            /*saving the offset*/
            to_write = lseek(file, to_write + r_bytes, SEEK_SET);
            if(flock(file, LOCK_UN))log("flock_miss");
            bzero(buff, sizeof(buff));
            //ch cleen the buffer
        }
    }
}


/**
 *output the contet of the file
 *@f: descriptor of a file we are going to use
 */
int out_in_messeges (int f)
{
    char buff[MAX];
    int n;

    lseek(f,to_read,SEEK_SET);
    while(1) {
        flock(f, LOCK_EX);
        n = read(f, buff, sizeof(buff));
        flock(f, LOCK_UN);
        if (n > 0) {
            printf("%s", buff);
        }
        else if (n < 0)
            exit_ha("error while reading socket")
        else
            break;
        }
        if(to_write-to_read == 0)
        {
            printf("no new messages\n");
        }
    to_read = lseek(f,to_write,SEEK_SET);

    return 0;
}
/**
 *the function handles the interactions beween server and client
 *@servid: id of a socket that is used to comunicate vith a server
 */
int interactions (int servid)
{
    char in_buf[MAX];
    int f;
    int n;
    char * command;
    pthread_t read_thread_id;

    struct file_socet * f_s = (struct file_socet *)malloc(sizeof(struct file_socet));

    if((f = open(FILEPATH, O_RDWR |
         O_CREAT | O_TRUNC | O_NONBLOCK, S_IRWXU | S_IRWXG)) < 1)
        exit_ha("error while opening the file");


    f_s->fileid = f;
    f_s->socid = servid;

    if(pthread_create(&read_thread_id, NULL, get_the_messeges,(void*)f_s)) {
        exit_ha("error whilwe opening file");
    }
        printf("enter :"COL_DlO"/help\\n(enter key)"COL_R" and "COL_DlO
            "/in_msg\\n(enter key)"COL_R" to get help msg from server \n");

    while(1) {
        printf("input : ");
        while(1) {
            n = 0;
            while((in_buf[n++] = getchar()) != '\n');
            if(n < MAX)
                break;
        }
/* getting a command from a buffer */
        command = strtok(in_buf,">\n");
        if(strcmp("/in_msg", command) == 0) {
                out_in_messeges (f);
        }
        else if(strcmp("/quit", command) == 0) {
            write(servid, command, strlen(command));
            break;
        } else if(strcmp("/help", command) == 0 ||
            strcmp("/list", command) == 0
        ) {
            write(servid, command, strlen(command));
        }
        else {
            write(servid, in_buf, strlen(in_buf));
        }
        bzero(in_buf, sizeof(in_buf));
    }
    return 0;
}


int main(int argc,char **argv)
{
	int 				socdf;
	struct sockaddr_in  servaddr;

	if (argc != 2)
		exit_ha("invalid ammount of arguments : %d; needed 1 ", argc - 1);

	if ((socdf = socket(AF_INET, SOCK_STREAM,0)) < 0)
		exit_ha("error while creating socket ");

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERVER_PORT);

	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
		exit_ha("inet_pton error for %s ", argv[1]);

	log("connecting to server");

	if (connect(socdf, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
		exit_ha("connection failed");

	log("starting chat with server");

	if ((interactions(socdf)) == 0) {
		log("exit ");
		close(socdf);
	}

	return 0;
}
