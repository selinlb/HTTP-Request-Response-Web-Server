/** 
* Selin Leblebicioğlu 170418047
* Sistem Programlama Final Projesi
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 1024

/**
* çalışmayan bütün sistem fonksiyonlarının hatalarını yazdırdığımız fonksiyon.
*/
void error(char *msg){
  perror(msg);
  exit(1);
}

/**
* web tarayıcısında hatamızı html formatında gösterdiğimiz fonksiyon.
*/
void cerror(FILE *stream,char *filename,char *errno,char *errname,char *errmsg){
  fprintf(stream,"HTTP/1.1 %s %s\n",errno,errname);
  fprintf(stream,"Content-type: text/html\n");
  fprintf(stream,"\n");
  fprintf(stream,"<html><title>Error</title>");
  fprintf(stream,"%s: %s\n",errno,errname);
  fprintf(stream,"<p>%s: %s\n</p>",errmsg,filename);
}

/**
* main fonksiyon. burada bütün bağlanma, dinleme, istek alma ve cevaplama işlemleri 
* gerçekleşiyor.
* @param socket_pr server tarafı socket değişkeni.
* @param socket_ch client tarafı socket değişkeni.
* @param portno kullanıcıdan alınan port numarası.
* @param clientlen clientaddr struct uzunluğu. accept fonksiyonunda kullanılıyor.
* @param serveraddr ve clientaddr, sockaddr_in struct değişkenleri.
* @param buf istek mesajı tutulan değişken.
* @param method isteğin metodu. GET kullanıldı
* @param uri isteğin yolu
* @param version HTTP/1.1 versiyonu
* @param filename isteğin atıldığı dosya, filetype istek uzantısının değişkeni
*/


int main(int argc,char **argv){
  int socket_pr;
  int socket_ch;           
  int portno;           
  int clientlen;              
  struct sockaddr_in serveraddr; 
  struct sockaddr_in clientaddr; 

  FILE *stream;          
  char buf[BUFSIZE];     
  char method[BUFSIZE];  
  char uri[BUFSIZE];     
  char version[BUFSIZE]; 
  char filename[BUFSIZE];
  char filetype[BUFSIZE];
  char *p;              
  struct stat sbuf;      
  int fd;                

  /*USAGE*/
  if(argc != 2){
    fprintf(stderr,"usage: %s <portnum>\n",argv[0]);
    exit(1);
  }
  portno=atoi(argv[1]); 
  
  /*socket bağlantısı*/
  socket_pr=socket(AF_INET,SOCK_STREAM,0);
  if(socket_pr<0) error("ERROR socket");

  bzero((char *)&serveraddr,sizeof(serveraddr));

  /*binding işlemi*/
  serveraddr.sin_family=AF_INET;
  serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
  serveraddr.sin_port=htons((unsigned long)portno);
  if(bind(socket_pr,(struct sockaddr *)&serveraddr,sizeof(serveraddr))<0) error("ERROR bind");

  /*socket dinleniyor*/
  if(listen(socket_pr,1)<0) error("ERROR listen");

  /*burada while döngüsü içinde işlemler gerçekleştiriyoruz*/
  clientlen=sizeof(clientaddr);
  while(1){
    
    /*bağlantının kabul edilmesi*/
    socket_ch=accept(socket_pr,(struct sockaddr *)&clientaddr,&clientlen);
    if(socket_ch<0) error("ERROR accept");
    
    /*burada client socket içeriğini dosyaya açıyoruz*/
    if((stream=fdopen(socket_ch,"r+"))==NULL) error("ERROR fdopen");

    /*isteğin metodunu, uzantıyı ve http versiyonunu alıyoruz*/
    fgets(buf,BUFSIZE,stream);
    printf("%s",buf);
    sscanf(buf,"%s %s %s\n",method,uri,version);

    /*sadece get komutuyla çalışacağımızdan komutu kontrol ediyoruz.*/
    if(strcasecmp(method,"GET")){
      cerror(stream,method,"501","Not Implemented","Sadece GET metoduyla istek yapiniz.");
      fclose(stream);
      close(socket_ch);
      continue;
    }

    /*isteği okuyoruz.*/
    fgets(buf,BUFSIZE,stream);
    printf("%s",buf);
    while(strcmp(buf,"\r\n")){
      fgets(buf,BUFSIZE,stream);
      printf("%s",buf);
    }

    /*burada uzantıyı tek tek ayırıp dosya tipini buluyoruz.*/
      strcpy(filename,".");
      strcat(filename,uri);
      if(uri[strlen(uri)-1]=='/') strcat(filename,"index.html");

    /*dosyanın varlığını kontrol ediyoruz*/
    if(stat(filename,&sbuf)<0) {
      cerror(stream,filename,"404","Not found","Boyle bir dosya mevcut degil.");
      fclose(stream);
      close(socket_ch);
      continue;
    }

    /*dosya tiplerimiz filetype içine kopyalanıyor*/
      if(strstr(filename,".html")) strcpy(filetype,"text/html");
      else if(strstr(filename,".css")) strcpy(filetype,"text/css");
      else if(strstr(filename,".jpg")) strcpy(filetype,"image/jpg");
      else if(strstr(filename,".png")) strcpy(filetype,"image/png");
      else strcpy(filetype,"text/plain");

      /*başarılı olması durumunda yazdırılan cevap*/
      fprintf(stream,"HTTP/1.1 200 OK\n");
      fprintf(stream,"Server: localhost\n");
      fprintf(stream,"Content-length: %d\n",(int)sbuf.st_size);
      fprintf(stream,"Content-type: %s\n",filetype);
      fprintf(stream,"\r\n"); 

      fd=open(filename,O_RDONLY);
      p=mmap(0,sbuf.st_size,PROT_READ,MAP_PRIVATE,fd,0);
      fwrite(p,1,sbuf.st_size,stream);
      munmap(p,sbuf.st_size);

    fclose(stream);
    close(socket_ch);

  }
}
