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
#define _GNU_SOURCE

#define LISTENQ 5
#define MAXLINE 512
#define MAXMEM 10
#define NAMELEN 20

int listenfd,connfd[MAXMEM];
char name[MAXMEM][NAMELEN];
void quit();
void rcv_snd(int n);

int main()
{
	pthread_t thread;
	struct sockaddr_in servaddr,cliaddr;
	socklen_t len;
	time_t ticks;
	char buff[MAXLINE];
	//Socket
	printf("Socket...\n");
	listenfd=socket(AF_INET,SOCK_STREAM,0);
	if(listenfd<0)
	{
		printf("Socket created failed.\n");
		return -1;
	}
	//Bind
	printf("Bind...\n");
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(1111);
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
	{
		printf("Bind failed.\n");
		return -1;
	}
	//listen
	printf("listening...\n");
	listen(listenfd,LISTENQ);

	pthread_create(&thread,NULL,(void*)(&quit),NULL);
	int i=0;
	for(i=0;i<MAXMEM;i++)
	{
		connfd[i]=-1;
	}
	while(1)
	{
		len=sizeof(cliaddr);
		for(i=0;i<MAXMEM;i++)
		{
			if(connfd[i]==-1)
			{
				break;
			}
		}
		connfd[i]=accept(listenfd,(struct sockaddr*)&cliaddr,&len);
		ticks=time(NULL);
		sprintf(buff,"%.24s \r \n",ctime(&ticks));

		pthread_create(malloc(sizeof(pthread_t)),NULL,(void*)(&rcv_snd),(void*)i);

	}
	return 0;
}
void quit()
{
	char msg[10];
	while(1)
	{
		scanf("%s",msg);
		if(strcmp("quit",msg)==0)
		{
			printf("Byebye...\n");
			close(listenfd);
			exit(0);
		}
	}
}
void rcv_snd(int n)
{
	char* ask="What's your name?\n";
	char buff[MAXLINE];
	char buff1[MAXLINE];
	char buff2[MAXLINE];
	char buff3[MAXLINE];
	int i=0;
	time_t ticks;
	char *ptr;
	char *str;
	char pmn[NAMELEN]={'\0'};
	int pmc;
	int retval;
	char yn;

	write(connfd[n],ask,strlen(ask));
	int len;
	len=read(connfd[n],name[n],NAMELEN);
	if(len>0)
	{
		name[n][len]=0;
	}

	strcpy(buff,name[n]);
	strcat(buff,"\tjoin in\n");
	for(i=0;i<MAXMEM;i++)
	{
		if(connfd[i]!=-1)
		{
			write(connfd[i],buff,strlen(buff));
		}
	}
	//message
	while(1)
	{
		if((len=read(connfd[n],buff1,MAXLINE))>0)
		{
			buff1[len]=0;
			for(i=0;i<MAXMEM;i++)
			{
				if(strcmp(name[i],buff1)==0)
				{
					while(1)
					{
						if((len=read(connfd[n],buff3,MAXLINE))>0)
							buff3[len]=0;
						if(strcmp("byebye\n",buff3)==0)
						{
							strcpy(buff1,"\n");
							break;
						}
						ticks=time(NULL);
						sprintf(buff2,"%.24s\r\n",ctime(&ticks));

						strcpy(buff,name[n]);
						strcat(buff,"\t");
						strcat(buff,buff2);
						strcat(buff,buff3);

						write(connfd[i],buff,strlen(buff));
					}
				}
			}

			if(strcmp("bye\n",buff1)==0)
			{
				close(connfd[n]);
				connfd[n]=-1;
				pthread_exit(&retval);
			}
			if(strcmp("/all\n",buff1)==0)
			{
				strcpy(buff,"NOW on:\n");
				for(i=0;i<MAXMEM;i++)
				{
					if(connfd[i]!=-1)
						strcat(buff,name[i]);
				}
				write(connfd[n],buff,strlen(buff));
				continue;
			}
			ptr=buff1;
			str=buff1;
			if(strstr(ptr,"sendto ")==str)
			{
				
				pmc=0;
				ptr=ptr+7;
				while(*ptr!='|')
				{
					pmn[pmc]=*ptr;
					pmc++;
					ptr++;
				}
				ptr++;
				pmn[pmc]='\n';
				memset(buff2,0,sizeof(buff1));
				pmc=0;
				str=ptr;
				while(*str!='\n')
				{
					printf("%c",*str);
					buff2[pmc]=*str;
					pmc++;
					str++;
				}
				for(i=0;i<MAXMEM;i++)
				{
					
					if(strcmp(name[i],pmn)==0)
					{
						write(connfd[n],"Don't do anything\n",strlen("Don't do anything\n"));
						memset(buff,0,sizeof(buff));
						strcpy(buff,"Do you want to get file from ");
						strcat(buff,name[n]);
						strcat(buff," y/n \n");
						write(connfd[i],buff,strlen(buff));
						len=read(connfd[i],buff,MAXLINE);
						buff[len]=0;
						
						if(strcmp(buff,"y\n")==0)
						{
							
							memset(buff,0,sizeof(buff));
							FILE *ftp;
							
							ftp=fopen(buff2,"r");
							
							if(ftp!=NULL)
							{
								
								write(connfd[i],"/filestart",strlen("/filestart"));
								sleep(1);
								write(connfd[i],buff2,strlen(buff2));
								sleep(1);
								while(fgets(buff1,sizeof(buff1),ftp)!=NULL)
								{
									printf("%s",buff1);
									write(connfd[i],buff1,strlen(buff1));
									memset(buff1,0,sizeof(buff1));
									sleep(0.01);
								}
								write(connfd[i],"end\n",strlen("end\n"));
								write(connfd[n],"end\n",strlen("end\n"));
							}
							fclose(ftp);
						}
						else write(connfd[n],"Don't want your file\n end\n",strlen("Don't want your file\n end\n"));
						break;
					}
				}
				continue;
			}

			if(strcmp("\n",buff1)!=0)
			{
				ticks=time(NULL);
				sprintf(buff2,"%.24s\r\n",ctime(&ticks));

				strcpy(buff,name[n]);
				strcat(buff,"\t");
				strcat(buff,buff2);
				strcat(buff,buff1);
				//to all client
				for(i=0;i<MAXMEM;i++)
				{
					if(i==n)continue;
					if(connfd[i]!=-1)
					{
						write(connfd[i],buff,strlen(buff));
					}
				}
			}
		}

	}
}
