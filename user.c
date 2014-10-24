#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

#define GROUP 9

// ### CLIENTE ###

int fd;
struct hostent *hostptr;
struct sockaddr_in serveraddr, clientaddr;
int addrlen, n;

int main(int argc, char *argv[]){

	// # UDP #

	/*Declaração e atribuição a variáveis a enviar ao servidor*/
	int ret = 0;
	char number[1024];
	char buffer[1000000];
	char msg[1000000];
	
	strcpy(msg, "RQT\n");
	strcpy(buffer, "\n");
	strcpy(number, "RQC\n");
	
	char *SERVER = "localhost";
	int PORT = 58000 + GROUP;
	
	int a;
	
	
	for(a=0; a<argc; a++) 
	{ 
	  /*Verifica se foi dado um servidor ao qual o cliente se ligar*/
	  if(!strcmp(argv[a], "-n"))
	  {
	    SERVER = (char*) malloc(strlen(argv[a+1]) * sizeof(char));
	    strcpy(SERVER, argv[a+1]);
	  } 
	  /*Verifica se foi dada uma porta à qual o cliente se ligar*/
	  else if(!strcmp(argv[a], "-p")) { PORT = atoi(argv[a+1]); } 
	}
	
	printf("<< PORT: %d >>\n", PORT);
	printf("<< SERVER NAME: %s >>\n\n", SERVER);

	/*Criação de um socket para efectar ligação ao servidor - UDP*/
	fd = socket(AF_INET,SOCK_DGRAM,0);
	hostptr = gethostbyname(SERVER);
	memset((void*)&serveraddr,(int)'\0', sizeof(serveraddr));
	
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_addr.s_addr=((struct in_addr *) (hostptr->h_addr_list[0]))->s_addr;
	serveraddr.sin_port=htons((u_short)PORT);

	addrlen=sizeof(serveraddr);

	/*Envio de uma mensagem como pedido da Lista de Conteúdos ao servidor*/
	sendto(fd, msg, strlen(msg), 0, (struct sockaddr*) &serveraddr, addrlen);

	addrlen = sizeof(serveraddr);
	/*Recepção da resposta do servidor ao pedido da Lista de Conteúdos*/
	ret = recvfrom(fd,buffer,sizeof(buffer),0,(struct sockaddr*)&serveraddr, &addrlen);
	
	buffer[ret] = '\0';
	char *aux;
	aux = strtok (buffer," ");
	int nr = 1;
	
	/*Ciclo While com o objectivo de imprimir apenas o conteúdo relevante da resposta "ret"*/
	while (aux != NULL)
	{
		if(nr == 2)
		{
			printf ("%s:\n", aux);
		}
		
		else if(nr > 3)
		{
			printf ("%d - %s\n",(nr-3), aux);	
		}
		
		nr++;
		aux = strtok (NULL, " ");
	}
	
	/*Limpeza do buffer*/
	memset((void*)&buffer,(int)'\0',sizeof(buffer));
	
	printf("> ");
	
	/*Concatenação da mensagem a enviar ao servidor - com o número do item da lista que o cliente escolher*/
	scanf("%s", number);
	
	/*Se o pedido for RQC 0, o programa encerra*/
	if (strcmp("0", number) == 0) exit(0);
	
	strcpy(buffer, "RQC ");
	strcat(buffer, number);
	strcat(buffer ,"\n");

	/*Envio de um pedido de informacao sobre um item ao servidor*/
	sendto(fd, buffer, strlen(buffer), 0, (struct sockaddr*) &serveraddr, addrlen);
	
	/*Limpeza do buffer*/
	memset((void*)&buffer,(int)'\0',sizeof(buffer));

	addrlen = sizeof(serveraddr);
	/*Recepção da resposta do servidor ao pedido de informacao sobre um item*/
	ret = recvfrom(fd,buffer,sizeof(buffer),0,(struct sockaddr*)&serveraddr, &addrlen);
	
	buffer[ret] = '\0';
	
	// # TCP #
	
	/*Declaração e atribuição a variáveis a enviar ao servidor*/
	int nbytes, nleft, nwritten, nread, clientlen, newfd;
	char *ptr;
	char name[50];
	char server[50];
	int porta;
	nbytes = 7;
  
	/*Recolha das variáveis: name, server e porta, de um certo item e sua escrita no buffer*/
	sscanf(buffer,"AWC %s %s %d", name, server, &porta);
	
	printf("%s %s %d\n", name, server, porta);
	
		
	/*Criação de um socket para efectar ligação ao servidor - TCP*/
	fd = socket(AF_INET,SOCK_STREAM,0);
	hostptr = gethostbyname(server);
  
	memset((void*)&serveraddr,(int)'\0', sizeof(serveraddr));
  
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = ((struct in_addr *)(hostptr->h_addr_list[0]))->s_addr;
	serveraddr.sin_port = htons((u_short)porta);
	/*Estabelece ligação do socket ao servidor*/
	connect(fd,(struct sockaddr*)&serveraddr, sizeof(serveraddr));
	
	/*Concatenação de um novo pedido de acordo com o "name" guardado previamente*/
	memset(msg, 0, sizeof(msg));
	strcpy(msg,"REQ ");
	strcat(msg, name);
	strcat(msg,"\n");
	//printf("%s", msg);
	
	/*Envio de pedido de transferencia para o servidor*/
	write(fd, msg, strlen(msg));
  
	/*Recepcão da resposta do servidor com identificacão e conteúdo*/
	memset(buffer, 0, sizeof(buffer));
	read(fd, buffer, 7);
	
	char tamanho[50] = "";
  
	/*Split da resposta dada pelo servidor*/
	if(!strcmp(buffer, "REP ok ")) {
	  memset(buffer, 0, sizeof(buffer));
	  
	  while(strcmp(buffer, " ")) {
	    read(fd, buffer, 1);
	    strcat(tamanho, buffer);
	  }
	  printf("REP ok %s\n", tamanho);
	}
	else { printf("REP nok %s\n", tamanho); return -1; }
	
	int tamanhoi = atoi(tamanho);
	memset(buffer, 0, sizeof(buffer));
  
	nleft = tamanhoi;
	nread = 0;
	ptr = buffer; 
	
	while (nleft > 0) {
	  nread = read(fd,ptr,nleft);
	  if(nread == 0) break;
	  nleft -= nread;
	  ptr += nread;
	}
  
	nread = tamanhoi - nleft;	
	close(fd);	
	
	/*Escrita do conteúdo do buffer para um ficheiro com o nome do item*/
	FILE *file;
	file = fopen(name, "w");
	fwrite(buffer, 1, nread, file);
	fclose(file);
	
  return 0;
}
