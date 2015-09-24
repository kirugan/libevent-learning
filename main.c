#include <stdio.h>
#include <event.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>

#define DEFAULT_PORT 7777
#define MAX_CLIENTS 100

struct holder{
	int sfd;
	struct event ev;
};

struct holder ** heap;
int holder_count = 0;

struct event** client_events;
int client_events_cnt = 0;
/* CALLBACKS */
/* well, in memcached there is only one event_handler (for main server socket and client connection) 
   it might be useful for one enter point, but for learning purposes I created 2 event handlers (one for main socket, and another one for client socket)
*/
void event_handler_client(int fd, short which, void *arg){
	printf("CLIENT handler %s\n", (char *) arg);
}

void event_handler(int fd, short which, void *arg){
	int sfd;

	struct sockaddr addr;
	socklen_t addrlen = sizeof(addr);

	if((sfd = accept(fd, &addr, &addrlen)) < 0){
		perror("accept");
	} else {
		struct event* ev = (struct event*) malloc(sizeof(struct event));
		client_events[holder_count++] = ev;

		printf("BEFORE \n");
		event_set(ev, sfd, EV_READ | EV_PERSIST, event_handler_client, (void *) "placeholder");
		printf("AFTER\n");
		event_add(ev, 0);
	}
}
/* /CALLBACKS */

int main(int argc, char** argv){
	int port;
	
	if(argc < 2){
		port = DEFAULT_PORT;
	} else {
		port = atoi(argv[1]);
	}

	client_events = malloc(MAX_CLIENTS * sizeof(struct event));

	int sfd;
	/* SERVER SOCKET */
	if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket");
		exit(1);
	}

	struct in_addr iaddr;
	iaddr.s_addr = htonl(INADDR_ANY);
	
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = iaddr;

	if(bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1){
		perror("bind");
		exit(1);
	}

	if(listen(sfd, 1024) < 0){
		perror("listen");
		exit(1);
	}
	/* /SERVER SOCKET */

	struct event ev;
	event_init();
	// if you omit EV_PERSIST, event_handler will be called just once, and event_loop finish execution
	// q: so what happens with sfd socket? there is no clients that want to connect
	// but it still call event_handler very often 
	event_set(&ev, sfd, EV_READ | EV_PERSIST, event_handler, (void *) "placeholder");
	event_add(&ev, 0);


	event_loop(0);
	printf("EXIT");	
}
