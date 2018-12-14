#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

void *recvsocket(void *arg)
{
	int st = *(int *)arg;
	char s[1024];

	while(1)
	{
		memset(s, 0, sizeof(s));
		int rc = recv(st, s, sizeof(s), 0);
		if (rc <= 0)
			break;

		if(strcmp(s,"/filestart")==0)
		{
			memset(s, 0, sizeof(s));
			while(1)
			{
				if(recv(st, s, sizeof(s), 0)>0)
				break;
			}
			printf("1\n");
			FILE *ftp;
			ftp=fopen(s,"w");
			while(1)
			{
				memset(s, 0, sizeof(s));
				if( recv(st, s, sizeof(s), 0)>0)
				{
					if(strcmp(s,"end\n")==0)
						break;
					else
						fprintf(ftp,"%s",s);
				}
			}
			fclose(ftp);
			printf("over\n");
			continue;
		}
		printf("server:%s", s);

	}
	return NULL;
}
void *sendsocket(void *arg)
{
	int st = *(int *)arg;
	char s[1024];
	while(1)
	{
		memset(s, 0, sizeof(s));
		read(STDIN_FILENO, s, sizeof(s));
		send(st, s, strlen(s), 0);
	}
	return NULL;
}

int main()
{

	int port = 1111;
	int st = socket(AF_INET, SOCK_STREAM, 0); 

	struct sockaddr_in addr; 
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET; 
	addr.sin_port = htons(port); 
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

	if (connect(st, (struct sockaddr *) &addr, sizeof(addr)) == -1)
	{
		printf("connect failed %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	pthread_t thrd1, thrd2;
	pthread_create(&thrd1, NULL, recvsocket, &st);
	pthread_create(&thrd2, NULL, sendsocket, &st);
	pthread_join(thrd1, NULL);
	close(st);
	return EXIT_SUCCESS;

}

