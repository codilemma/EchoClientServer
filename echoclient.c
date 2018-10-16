#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>
#include <arpa/inet.h> 

/* Be prepared accept a response of this length */
#define BUFSIZE         1033

#define USAGE                                                                       \
    "usage:\n"                                                                      \
    "  echoclient [options]\n"                                                      \
    "options:\n"                                                                    \
    "  -s                  Server (Default: localhost)\n"                           \
    "  -p                  Port (Default: 12041)\n"                                  \
    "  -m                  Message to send to server (Default: \"hello world.\")\n" \
    "  -h                  Show this help message\n"


/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
    {"server", required_argument, NULL, 's'},
    {"port", required_argument, NULL, 'p'},
    {"message", required_argument, NULL, 'm'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}};

/* main =================================================================== */
int main(int argc, char **argv)
{
    int option_char = 0;
    char *hostname = "localhost";
    unsigned short portno = 12041;
    char *message = "Hello World!!";

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "s:p:m:hx", gLongOptions, NULL)) != -1)
    {
        switch (option_char)
        {
        case 's': // server
            hostname = optarg;
            break;
        case 'p': // listen-port
            portno = atoi(optarg);
            break;
        default:
            fprintf(stderr, "%s", USAGE);
            exit(1);
        case 'm': // message
            message = optarg;
            break;
        case 'h': // help
            fprintf(stdout, "%s", USAGE);
            exit(0);
            break;
        }
    }

    setbuf(stdout, NULL); // disable buffering

    if ((portno < 1025) || (portno > 65535))
    {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }

    if (NULL == message)
    {
        fprintf(stderr, "%s @ %d: invalid message\n", __FILE__, __LINE__);
        exit(1);
    }

    if (NULL == hostname)
    {
        fprintf(stderr, "%s @ %d: invalid host name\n", __FILE__, __LINE__);
        exit(1);
    }

    /******************** Collect Information About the Host Machine *********/
    // Source: https://github.com/zx1986/xSinppet/blob/master/unix-socket-practice/client.c 
    struct hostent* pHost_info;
    unsigned long host_address;

    // Get IP address from host name
    pHost_info = gethostbyname(hostname);
    // Copy host address into a long
    host_address = *(unsigned long *)(pHost_info->h_addr_list[0]);

    /******************** Create the client socket ***************************/
    int echoClient_socket = socket(AF_INET, SOCK_STREAM, 0);
    // error checking
    if (echoClient_socket == -1)
    {
      fprintf(stderr, "ERROR creating echoClient socket\n");
      exit(1);
    }

    /******************* Specify Socket Address *****************************/
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));// Set address to Zeros
    server_address.sin_family      = AF_INET;      // Internet Protocol (AF_INET)
    server_address.sin_port        = htons(portno);// Address port (16 bits)
    server_address.sin_addr.s_addr = host_address;   // Internet address (32 bits)

  /***************** Initiate a Connection to the socket ********************/
  int connection_status = connect(echoClient_socket, 
                                 (struct sockaddr *) &server_address, 
                                 sizeof(server_address));
  // error checking
  if (connection_status == -1)
  {
    fprintf(stderr, "ERROR connecting to the echoClient socket\n");
    exit(1);
  }

  /******************* Send Client Request to the Server ********************/
  // allocate 16 bytes for message to be sent from client.
  char send_buff[16];
  // copy the command line message into the buffer
  strncpy(send_buff, message, sizeof(send_buff)-1);
  send_buff[sizeof(send_buff)-1] = '\0';  // null terminate the buffer.
  // send the message
  send(echoClient_socket, &send_buff, sizeof(send_buff), 0);

  /*********************** Recieve Server Response **************************/
  // allocate 16 bytes for message recieved from server.
  char recv_buff[16];
  recv(echoClient_socket, &recv_buff, sizeof(recv_buff), 0);
  // print messeage recieved from the server.
  fprintf(stderr, "%s", recv_buff);
  /*************************** Close the Socket ****************************/
  close(echoClient_socket);

  return 0;
}
