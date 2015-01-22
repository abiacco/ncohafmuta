#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void) {
int port=23;
int sock;
int ret=-1;
char host[64];
struct sockaddr_in raddr;
int size=sizeof(struct sockaddr_in);
unsigned long f;
static int lsr_size;
int sock_opt_len;

strcpy(host,"127.0.0.1");

/* Zero out memory for address */
memset((char *)&raddr, 0, size);

ret=fcntl(sock,F_GETFD,0);
printf("Sock1 state is: %d\n",ret);

                if((f = inet_addr(host)) == -1L) {
			printf("bad host convert\n");
			exit(0);
                        }
                (void)bcopy((char *)&f,(char *)&raddr.sin_addr,sizeof(f));

        raddr.sin_port = htons((unsigned short)port);
        raddr.sin_family = AF_INET;

        if ((sock = socket(AF_INET,SOCK_STREAM,0)) == -1) {
		printf("bad socket creation\n");
		exit(0);
	}

 if (getsockopt(sock,
		 SOL_SOCKET,	
		 SO_RCVBUF,
		 (char *)&lsr_size,
		 &sock_opt_len) < 0) {
		printf("bad getsockopt\n");
		exit(0);
}

printf("size: %d\n",lsr_size);

ret=fcntl(sock,F_GETFD,0);
printf("Sock2 state is: %d\n",ret);

if (connect(sock,(struct sockaddr *)&raddr,sizeof(raddr)) == -1) {
	printf("bad connect\n");
	exit(0);
}

ret=fcntl(sock,F_GETFD,0);
printf("Sock3 state is: %d\n",ret);

close(sock);

ret=fcntl(sock,F_GETFD,0);
printf("Sock4 state is: %d\n",ret);

}
