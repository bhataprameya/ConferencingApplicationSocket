/*--------------------------------------------------------------------*/
/* functions to connect clients and server */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXNAMELEN 256
/*--------------------------------------------------------------------*/

/*----------------------------------------------------------------*/

int startserver ()
{
	int sd;			/* socket descriptor */
	struct sockaddr_in addr,addr2;
	char *servhost=malloc(sizeof("remote**.cs.binghamton.edu"));	/* full name of this host */
	ushort servport;		/* port assigned to this server */
	struct hostent *host;
	
	if ((sd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf ("socket error");
		return -1;
	}
	

	addr.sin_family=AF_INET;
	addr.sin_port=htons(0);
	addr.sin_addr.s_addr=htons(INADDR_ANY);
	if ((bind(sd,(struct sockaddr *)&addr, sizeof(addr)))==-1)
	{
		printf("Bind error");return -1;
	}

	/* we are ready to receive connections */
	listen (sd, 5);
	
	 if (gethostname(servhost,sizeof(servhost)+1) == -1)
        {
                printf ("gethostname error");
                return -1;
        }
	 if ((host = gethostbyname(servhost)) == NULL)
        {
                printf ("gethostbyname error");
                return -1;
        }



	
	int size = sizeof (struct sockaddr_in);
	 if (getsockname(sd, (struct sockaddr *) &addr2, &size) == -1)
        {
                printf ("getsockname error\n");
        }
	servport=ntohs(addr2.sin_port);
	
	/* ready to accept requests */
	printf ("admin: started server on '%s' at '%hu'\n", host->h_name, servport);
	free (servhost);
	return (sd);
}

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

int hooktoserver (char *servhost, ushort servport)
{
	int sd;			/* socket descriptor */
	ushort clientport;		/* port assigned to this client */
	struct sockaddr_in sa;
	struct hostent *host;
	int size = sizeof (struct sockaddr);
	
	if ((sd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf ("socket error");
		exit (1);
	}

	
	if ((host = gethostbyname(servhost)) == NULL)
	{
		printf ("gethostbyname error");
		exit (1);
	}
	sa.sin_family = AF_INET;
	sa.sin_port = htons (servport);
	memcpy (&sa.sin_addr.s_addr, host->h_addr, host->h_length);

	if (connect (sd, (struct sockaddr *) &sa, sizeof (sa)) == -1)
	{
		printf ("connect error");
		exit (1);
	}

	
	if (getsockname(sd, (struct sockaddr *) &sa, &size) == -1)
	{
		printf ("getsockname error\n");
	}
	clientport = ntohs (sa.sin_port);
	/* succesful. return socket descriptor */
	printf ("admin: connected to server on '%s' at '%hu' thru '%hu'\n",
			servhost, servport, clientport);
	printf (">");
	fflush (stdout);
	return (sd);
}

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
int readn (int sd, char *buf, int n)
{
	int toberead;
	char *ptr;

	toberead = n;
	ptr = buf;
	while (toberead > 0)
	{
		int byteread;

		byteread = read (sd, ptr, toberead);
		if (byteread <= 0)
		{
			if (byteread == -1)
				perror ("read");
			return (0);
		}

		toberead -= byteread;
		ptr += byteread;
	}
	return (1);
}

char *recvtext (int sd)
{
	char *msg;
	long len;

	/* read the message length */
	if (!readn (sd, (char *) &len, sizeof (len)))
	{
		return (NULL);
	}
	len = ntohl (len);

	/* allocate space for message text */
	msg = NULL;
	if (len > 0)
	{
		msg = (char *) malloc (len);
		if (!msg)
		{
			fprintf (stderr, "error : unable to malloc\n");
			return (NULL);
		}

		/* read the message text */
		if (!readn (sd, msg, len))
		{
			free (msg);
			return (NULL);
		}
	}

	/* done reading */
	return (msg);
}

int sendtext (int sd, char *msg)
{
	long len;

	/* write lent */
	len = (msg ? strlen (msg) + 1 : 0);
	len = htonl (len);
	write (sd, (char *) &len, sizeof (len));

	/* write message text */
	len = ntohl (len);
	if (len > 0)
		write (sd, msg, len);
	return (1);
}

/*----------------------------------------------------------------*/
