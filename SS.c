#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
extern interrno;

#define GROUP 9

// ### SERVIDOR TCP ###

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     
     int i, a;
     int flagSUCESS = 0;
    
     int PORT = 58000 + GROUP;

	 /*Verifica se foi dada uma porta ao qual o servidor se ligar*/
     for(a=0; a<argc; a++) { if(!strcmp(argv[a], "-p")) { PORT = atoi(argv[a+1]); flagSUCESS = 1; } }
    
     if(!flagSUCESS) { printf("<< DEFAULT PORT ID SET (%d) >>\n", PORT); }
     printf("<< PORT: %d >>\n\n", PORT);
     
     
    int fd, newfd, addrlen, n, nw, ret;
    struct sockaddr_in addr;
    char *ptr, buffer[1000000], mensagem[1000000], filename[150];
    pid_t pid;
    
    if((fd=socket(AF_INET,SOCK_STREAM,0)) == -1) exit(1);//error
    
    memset((void*)&addr,(int)'\0',sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(PORT);
    
    if(bind(fd,(struct sockaddr*)&addr,sizeof(addr))==-1) exit(1); //error
    if(listen(fd,2) == -1) exit(1); //error
    
    while(1)
    {
      addrlen=sizeof(addr);
	  
      /*Ciclo de espera por uma nova ligação*/
      do newfd=accept(fd,(struct sockaddr*)&addr,&addrlen);
      
      //Wait for a connection
      while(newfd==-1&&errno==EINTR);
      
      if(newfd==-1)exit(1); //error
      
      if((pid=fork())==-1)exit(1); //error
      
      else if(pid == 0) //child process
      {
	  
	close(fd);
	
	
	while((n=read(newfd,buffer,128))!=0)
	{
	  if(n == -1) exit(1); //error
	  if (sscanf(buffer, "REQ %s\n", filename) == 1)
	  {
	  
	  int byteslidos = 0;
	  long sizeoffile = 0;
	  int x = 0;
	  
	  /*Leitura do ficheiro "filename"*/
	  FILE * ficheiro; 
	  ficheiro = fopen(filename, "rb");
	  if (ficheiro==NULL) { fputs ("<< FILE ERROR >>", stderr); exit (1); } 
	  
	  /*Determina o tamanho do ficheiro*/
	  fseek(ficheiro, 0, SEEK_END);
	  sizeoffile = ftell(ficheiro);
	  rewind(ficheiro);
	  sprintf(mensagem,"REP ok %ld ", sizeoffile);
	  ptr = &mensagem[0];
	  
	  int nleft = strlen(mensagem);
	  int nwritten=0;
	  
	  /*Leitura da "mensagem"*/
	  while(nleft>0)
	  {
	    nwritten=write(newfd,ptr,nleft);
	    
	    if(nwritten<=0) exit(1);//error
	    
	    nleft-=nwritten;
	    ptr+=nwritten;	    
	  }
	
	  /*Ciclo de leitura do ficheiro*/
	  while(byteslidos < sizeoffile)
	  {
	    // Copia bytes para mensagem
	    // memset(mensagem, 0, sizeof(mensagem));
	    // memset(buffer, 0, sizeof(buffer));
	    
	    x = fread(buffer, 1, 1024, ficheiro); /*Tamanho de cada packet = x*/
	    byteslidos += x;
	    
	    ptr = &buffer[0];
	    
	    nleft = 1024;
	    nwritten = 0;
	    
		/*Leitura parte do ficheiro contido no buffer*/
	    while(nleft>0)
	    {
	      nwritten=write(newfd,ptr,nleft);
	      
	      if(nwritten<=0) exit(1);//error
	      
	      nleft-=nwritten;
	      ptr+=nwritten;	    
	      
	    }
	  }
	  
	  fclose(ficheiro);
	  
	   /*Concatena "\n"*/
	  strcpy(mensagem, "\n");
	  
	  /*Envia para o socket*/
	  write(newfd, mensagem, strlen(mensagem));
	  
	  }
	  
	  else
	  {
	    strcpy(mensagem, "REP nok\n");
	    if(n == -1) exit(1); //error
	    if((nw = write(newfd,mensagem, strlen(mensagem)) ) <= 0) exit(1); //error
	  }
	}
      
      /*Limpeza do buffer*/
      memset(buffer, 0, sizeof(buffer));
	  /*Fecho da ligação*/
      close(newfd);
      exit(0);
      
    } // (pid == 0)
      
    //parent process
    do ret = close(newfd); 
    while(ret==-1&&errno==EINTR);
    if(ret==-1)	exit(1); //error
    
  } // while(1)
    return 0;
}