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
#include <stdbool.h>
#include <arpa/inet.h> 

#define BUFSIZE 1033

#define USAGE                                                                 \
"usage:\n"                                                                    \
"  echoserver [options]\n"                                                    \
"options:\n"                                                                  \
"  -p                  Port (Default: 12041)\n"                                \
"  -m                  Maximum pending connections (default: 1)\n"            \
"  -h                  Show this help message\n"                              \

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
  {"port",          required_argument,      NULL,           'p'},
  {"maxnpending",   required_argument,      NULL,           'm'},
  {"help",          no_argument,            NULL,           'h'},
  {NULL,            0,                      NULL,             0}
};


int main(int argc, char **argv) {
  int option_char;
  int portno = 12041; /* port to listen on */
  int maxnpending = 1;
  
  // Parse and set command line arguments
  while ((option_char = getopt_long(argc, argv, "p:m:hx", gLongOptions, NULL)) != -1) {
   switch (option_char) {
      case 'p': // listen-port
        portno = atoi(optarg);
        break;                                        
      default:
        fprintf(stderr, "%s ", USAGE);
        exit(1);
      case 'm': // server
        maxnpending = atoi(optarg);
        break; 
      case 'h': // help
        fprintf(stdout, "%s ", USAGE);
        exit(0);
        break;
    }
  }

    setbuf(stdout, NULL); // disable buffering

    if ((portno < 1025) || (portno > 65535)) {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }
    if (maxnpending < 1) {
        fprintf(stderr, "%s @ %d: invalid pending count (%d)\n", __FILE__, __LINE__, maxnpending);
        exit(1);
    }

  /******************** Create the server socket ***************************/
  int echoServer_socket = socket(AF_INET, SOCK_STREAM, 0);
  // error checking
  if (echoServer_socket == -1)
  {
    fprintf(stderr, "ERROR creating echoServer socket\n");
    exit(1);
  }

  /************************ Setup A Socket Host ***************************/
  int socket_reuse = 1;
  setsockopt(echoServer_socket, SOL_SOCKET, SO_REUSEADDR, 
             &socket_reuse, sizeof(socket_reuse));

  /******************* Specify Socket Address *****************************/
  struct sockaddr_in server_address;
  memset(&server_address, 0, sizeof(server_address));// Set address to Zeros
  server_address.sin_family      = AF_INET;          // Internet Protocol (AF_INET)
  server_address.sin_port        = htons(portno);    // Address port (16 bits)
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);// IP Address (32 bits)
  
  /******************* Bind Socket to IP and Port *************************/
  int bind_error = 0;
  bind_error = bind(echoServer_socket,
                   (struct sockaddr *) &server_address,
                    sizeof(server_address));
  // error checking
  if (bind_error < 0)
  {
    fprintf(stderr, "ERROR binding to incoming connections\n");
    exit(1);
  }

  int listen_error = 0;
  listen(echoServer_socket, maxnpending);
  // error checking
  if (listen_error < 0)
  {
    fprintf(stderr, "ERROR opening a socket for listening\n");
    exit(1);
  }

  for (;;)  // Run Indefinitely 
  {
  /************* Open New Socket To Transmit Data Per Connection **************/
    int echoClient_socket;
    echoClient_socket = accept(echoServer_socket, NULL, NULL);
  
    /*** Recieve Client Request and Echo the Message Back to the Client *********/
    char echo_buff[16];
    recv(echoClient_socket, &echo_buff, sizeof(echo_buff), 0);
    echo_buff[sizeof(echo_buff)-1] = '\0';  // null terminate the buffer.
    // print messeage recieved from the client.
    fprintf(stderr, "%s", echo_buff);
    send(echoClient_socket, &echo_buff, sizeof(echo_buff), 0);
    /*************************** Close the Socket ****************************/
    close(echoClient_socket);
  }
  
  return 0;

}
