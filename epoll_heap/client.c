#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netinet/in.h>
#define PORT 6666
#define IP "127.0.0.1"
#define SIZE 10
int main()
{
	struct sockaddr_in serv_addr;
	int cfd;
	char buf[BUFSIZ]="aaaabbbb\n";
	cfd=socket(AF_INET,SOCK_STREAM,0);
	int i;

	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(PORT);
	inet_pton(AF_INET,IP,&serv_addr.sin_addr.s_addr);
	connect(cfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	//memset(buf,0,sizeof(buf));

	while(1)
	{
		//fgets(buf,sizeof(buf),stdin);
		write(cfd,buf,SIZE);
		ssize_t size=read(cfd,buf,sizeof(buf));
		printf("rev:%s",buf);
		for(i=0;i<8;++i)
		{
			buf[i]+=2;
		}
		sleep(2);
	}
	return 0;	
}
