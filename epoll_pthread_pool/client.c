#include "common.h"

int main()
{
	struct sockaddr_in serv_addr;
	int cfd;
	char buf[BUFSIZ];
    memset(buf,0,BUFSIZ);
	cfd=Socket(AF_INET,SOCK_STREAM,0);
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(SERV_PORT);
	inet_pton(AF_INET,IP,&serv_addr.sin_addr.s_addr);
	Connect(cfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	int len;
	while(1)
	{
        printf("input your info:\n");
        memset(buf,0,BUFSIZ);
        Fgets(buf,BUFSIZ,stdin);
		len=0;
		for(;buf[len]!='\n';++len){}
		buf[len]=0;
		printf("wirting....\n");
        Write(cfd,buf,len);
        memset(buf,0,BUFSIZ);
		printf("reading....\n");
        Read(cfd,buf,sizeof(buf));
		printf("rev:%s\n",buf);
	}
	return 0;	
}