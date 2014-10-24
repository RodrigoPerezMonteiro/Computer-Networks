#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define GROUP 9
 
// ### SERVIDOR UDP ###

struct UDPserver 
{
  char name[100];
  char ipAddress[100];
  char port[100];
};


int main(int argc, char* argv[]) {
 
    int i, a;
    int flagSUCESS = 0;
    
    int PORT = 58000 + GROUP;
 
    int fd, addrlen;
    struct hostent *hostptr;
    struct sockaddr_in serveraddr, clientaddr;
    char buffer[1000000];
    char mensagem[1000000];
	
	/*Verifica se foi dada uma porta ao qual o servidor se ligar*/
    for(a=0; a<argc; a++) { if(!strcmp(argv[a], "-p")) { PORT = atoi(argv[a+1]); flagSUCESS = 1; } }
    
    if(!flagSUCESS) { printf("<< DEFAULT PORT ID SET (%d) >>\n", PORT); }
    
    printf("<< PORT: %d >>\n\n", PORT);

    char *header;
    int itemsDim;
    struct UDPserver *serverInfo;

    char itemsDim_s[10];
    FILE * ficheiro; 
    char * linha = NULL; 
    size_t size = 0; 
    ssize_t read; 
    
	/*Abertura do ficheiro "file.txt apenas para leitura*/
    ficheiro = fopen("file.txt", "r"); 

	if (ficheiro == NULL) { exit(EXIT_FAILURE); }
	
	else
	{	
	  int j=0;
	  /*Lê todas as linhas do ficheiro acabado de abrir*/
	  while ((read = getline(&linha, &size, ficheiro)) != -1) 
	  {
		/*Guarda a primeira linha dessa ficheiro e aloca espaço - Cabeçalho*/
		if(j==0)
		{ 
		  header= (char*) malloc((strlen(linha)+1)*sizeof(char)); 
		  sscanf(linha, "%s\0", header);
		}

		else 
		if(j==1)
		{
		  /*Guarda o número de items que o ficheiro possui*/
		  itemsDim = atoi(linha);
		  serverInfo = malloc(itemsDim*sizeof(struct UDPserver)); 
		}

		else /*Nas restantes linhas do ficheiro, guarda a informação contida, como o "name", "ipAddress" e "port"*/
		if(j>=2){ sscanf(linha,"%s %s %s\n", serverInfo[j-2].name, serverInfo[j-2].ipAddress, serverInfo[j-2].port); }
		
		j++;
	  }
	} 
    
    char valido[5];
    char numerostr[5];
    int numero;
    
  while(1)
  { 
    memset((void*)&valido,(int)'\0',sizeof(valido));
    memset((void*)&numerostr,(int)'\0',sizeof(numerostr));  
    memset((void*)&buffer, (int)'\0',sizeof(buffer));
    memset((void*)&mensagem, (int)'\0',sizeof(mensagem));
    
	/*Inicio de uma concatenação, utilizando a informação anteriormente recolhida*/
    strcpy(mensagem, "AWT ");
    sprintf(itemsDim_s, "%d", itemsDim);
    strcat(mensagem, header);
    strcat(mensagem, " ");
    strcat(mensagem, itemsDim_s);
    strcat(mensagem, " ");
    int b = 0;
	  
	/*Ciclo For com o objectivo de acrescentar o nome de cada item da lista*/
    for(b=0; b<itemsDim; b++) 
    {
     strcat(mensagem, serverInfo[b].name);
     if(b!=itemsDim-1) { strcat(mensagem, " "); }
     else { strcat(mensagem, "\n"); }
    }
    
	/*Criação de um socket para efectuar a ligação com o cliente*/
    fd=socket(AF_INET,SOCK_DGRAM,0);
    memset((void*)&serveraddr,(int)'\0',sizeof(serveraddr));
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
    serveraddr.sin_port=htons((u_short)PORT);

	/*Vincula o socket criado ao address*/
    bind(fd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
 
	addrlen=sizeof(clientaddr);
	/*Recepção da mensagem do buffer enviada pelo cliente*/
	recvfrom(fd,buffer,sizeof(buffer),0,(struct sockaddr*)&clientaddr,&addrlen);
	
	/*Verifica se a resposta se encontra no formato correcto*/
	if(!strcmp(buffer, "RQT\n")) 
	{
	  printf("<< RQT REQUEST >>\n");
	  sendto(fd, mensagem, strlen(mensagem), 0,(struct sockaddr*)&clientaddr,addrlen); 
	}

	else
	{
	  memset((void*)&mensagem,(int)'\0',sizeof(mensagem));
	  printf("<< RQT ERROR >>\n");
	}
	
	/*Fecho do socket*/
    close(fd);

    /////////////////////////////////////////////////////////////////////////
    
    memset((void*)&buffer, (int)'\0',sizeof(buffer));
    memset((void*)&mensagem, (int)'\0',sizeof(mensagem));
	
    /*Criação de um novo socket para efectuar a ligação com o cliente*/
    fd=socket(AF_INET,SOCK_DGRAM,0);
    memset((void*)&serveraddr,(int)'\0',sizeof(serveraddr));
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
    serveraddr.sin_port=htons((u_short)PORT);
 
	/*Vincula o socket criado ao address*/
    bind(fd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
 
    addrlen=sizeof(clientaddr);
	/*Recepção da mensagem do buffer enviada pelo cliente*/
    recvfrom(fd,buffer,sizeof(buffer),0,(struct sockaddr*)&clientaddr,&addrlen);

	/*Leitura e atribuição das variaveis valido e numerostr*/
    sscanf(buffer, "%s %s\n", valido, numerostr);
    strcat(valido, "\n");
    numero = atoi(numerostr);
    
	/*Verifica se o comando se é mesmo o RQC*/
    if(!strcmp(valido, "RQC\n"))
    {
		printf("<< RQC REQUEST >>\n");
		printf("RQC %s %s\n\n", serverInfo[numero-1].ipAddress, serverInfo[numero-1].port);
		/*Concatenação da resposta AWC de acordo com o número pedido pelo cliente*/
		strcpy(mensagem, "AWC ");
		strcat(mensagem, serverInfo[numero-1].name);
		strcat(mensagem, " ");
		strcat(mensagem, serverInfo[numero-1].ipAddress);
		strcat(mensagem, " ");
		strcat(mensagem, serverInfo[numero-1].port);
		strcat(mensagem, " ");
		sendto(fd, mensagem, strlen(mensagem), 0,(struct sockaddr*)&clientaddr,addrlen);	
    }	
	
    else
    {
      memset((void*)&mensagem, (int)'\0',sizeof(mensagem));
      printf("<< RQC ERROR >>>\n");
    }
    
	/*Fecho do socket/ligação*/
    close(fd);
  }
    return 0;
}
