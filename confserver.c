/*--------------------------------------------------------------------*/
/* conference server */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <time.h> 
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

extern char * recvtext(int sd);
extern int sendtext(int sd, char *msg);

extern int startserver();
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
int fd_isset(int fd, fd_set *fsp) {
	return FD_ISSET(fd, fsp);
}
/* main routine */
int main(int argc, char *argv[]) {
	int servsock; /* server socket descriptor */

	fd_set livesdset, livesdset2; /* set of live client sockets */
	int livesdmax,i; /* largest live client socket descriptor */

	/* check usage */
	if (argc != 1) {
		fprintf(stderr, "usage : %s\n", argv[0]);
		exit(1);
	}

	/* get ready to receive requests */
	servsock = startserver();
	if (servsock == -1) {
		perror("Error on starting server: ");
		exit(1);
	}

	
	FD_ZERO(&livesdset);
	livesdmax=servsock;
	FD_SET(servsock, &livesdset);
	/* receive requests and process them */
	while (1) {
		int frsock; /* loop variable */

		
		livesdset2=livesdset;
		
		if(select(livesdmax+1,&livesdset2,NULL,NULL,NULL)<0)
			continue;
		/* look for messages from live clients */
		for (frsock = 3; frsock <= livesdmax; frsock++) 
		{
			/* skip the listen socket */
			/* this case is covered separately */
			if (frsock == servsock)
				continue;

			if (fd_isset(frsock,&livesdset2)) 
			{
				char * clienthost; /* host name of the client */
				ushort clientport; /* port number of the client */
				struct sockaddr_in ca;
				struct hostent *cap;
				char *msg;
				int size=sizeof(ca);
				
				if(getpeername(frsock,(struct sockaddr *) &ca, &size)==-1)
				{
					printf ("getpeername error\n");
				}
				clientport=ntohs(ca.sin_port);

				if((cap=gethostbyaddr(&(ca.sin_addr),sizeof(struct in_addr), AF_INET))==NULL)
				{
					printf ("gethostbyaddr error\n");
				}
				clienthost=cap->h_name;

				/* read the message */
				msg = recvtext(frsock);
				if (!msg) 
				{
					/* disconnect from client */
					printf("admin: disconnect from '%s(%hu)'\n", clienthost,
							clientport);

					
					FD_CLR(frsock, &livesdset);
					/* close the socket */
					if (frsock == livesdmax) 
						livesdmax--;
					close(frsock);
					//	break;
				} 
				else 
				{
					
					for(i=3;i<=livesdmax;i++)
					{
						if(i==servsock || i==frsock)
							continue;
						else
						{
							sendtext(i, msg);
						}
						
					}


					/* display the message */
					printf("%s(%hu): %s", clienthost, clientport, msg);

					/* free the message */
					free(msg);
				}
			}
		}

		/* look for connect requests */
		if (FD_ISSET(servsock, &livesdset2) /* FILL HERE: connect request from a new client? */ ) {
			struct hostent * host;
			struct sockaddr_in client; 
			int csd, len=sizeof(struct sockaddr);

			/*
			   FILL HERE:
			   accept a new connection request
			 */
			csd=accept(servsock,(struct sockaddr *)&client,&len);
			/* if accept is fine? */
			if (csd != -1) {
				char * clienthost; /* host name of the client */
				ushort clientport; /* port number of the client */

				
				if ((host = gethostbyaddr(&(client.sin_addr),sizeof(struct in_addr),AF_INET)) == NULL)
				{
					printf ("gethostbyaddr error");
					return -1;
				}
				clientport = ntohs (client.sin_port);
				clienthost=host->h_name;

				printf("admin: connect from '%s' at '%hu'\n", clienthost,
						clientport);

				
				FD_SET(csd, &livesdset);
				if (csd > livesdmax)
					livesdmax=csd;

			} else {
				perror("accept");
				exit(0);
			}
		}
	}
	return 0;
}
/*--------------------------------------------------------------------*/

