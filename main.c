#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

static void server();
static void client(int port);

int main(int argc, char *argv[]) {
    // Checks if Server or client is running the program
    // Client needs to specify "client" and port number when running program
    if (argc > 1 && !strcmp(argv[1], "client")) {
        // Checks if atleast 2 arguments where given
        if (argc != 3) {
            fprintf(stderr, "not enough args!");
            return -1;
        }

        // If client ran the program then call the client function
        // and pass the port number into the function
        int port;
        sscanf(argv[2], "%d", &port);

        client(port);
    } 
    // If server calls the server function
    else {
        server();
    }

    return 0;
}


static void server() {
    // create socket
    const int fd = socket(PF_INET, SOCK_STREAM, 0);
    int optval = 1;

    // Makes it so clients can reuse the same address to connect with the server
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt failed");
        close(fd);
        return;
    }

    // bind server address to open port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;

    //bind will return -1 if an error occurs
    if (bind(fd, (struct sockaddr*) &addr, sizeof(addr))) {
        perror("bind error");
        return;
    }

    // Continues looking for client, keeps socket open for clients to connect to 
    while(1){
        // read port
        socklen_t addr_len = sizeof(addr);
        getsockname(fd, (struct sockaddr*) &addr, &addr_len);
        printf("server is on port %d\n", (int) ntohs(addr.sin_port));

        // Makes sure socket is ready to accept a new client 
        if (listen(fd, 1)) {
            perror("listen error");
            return;
        }

        // accept incoming connection
        struct sockaddr_storage caddr;
        socklen_t caddr_len = sizeof(caddr);
        const int cfd = accept(fd, (struct sockaddr*) &caddr, &caddr_len);
        printf("Client Connected\n");

        // read from client with recv!
        char buf[1024];
        
        // Recieves messages from client unless recv encounters an error 
        // then it closes client socket (cfd)
        while(recv(cfd, buf, sizeof(buf), 0) > 0){
            // Echos back what the client sent to the server
            send(fd, buf, strlen(buf)+1, 0);
        }
        // If client disconnects print it to the server and close client socket (cfd)
        printf("Client Disconnected\n");
        close(cfd);
    }
    // Close server socket
    close(fd);
}

static void client(int port) {
    // Open Client side socket
    const int fd = socket(PF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_port = htons((short) port);

    // connect to local machine at specified port
    char addrstr[NI_MAXHOST + NI_MAXSERV + 1];
    snprintf(addrstr, sizeof(addrstr), "127.0.0.1:%d", port);

    // parse into address
    inet_pton(AF_INET, addrstr, &addr.sin_addr);

    // connect to server
    if (connect(fd, (struct sockaddr*) &addr, sizeof(addr))) {
        perror("connect error:");
        return;
    }

    // accept incoming connection
    struct sockaddr_storage caddr;
    socklen_t caddr_len = sizeof(caddr);
    const int cfd = accept(fd, (struct sockaddr*) &caddr, &caddr_len);

    // say hey with send!
    char msg[256];
    do{
        //Tasks user to input a message
        printf("What is your message: ");
        fgets(msg, sizeof(msg), stdin);

        //Sends message to server
        send(fd, msg, strlen(msg) + 1, 0);

        //Recieve message back from server and prints it to the screen
        recv(cfd, msg, sizeof(msg), 0);
        printf("Echo: %s\n", msg);

    }while(1); // Keeps client serer from closing

    close(fd);

}

















