#include "common.h"

#define MAXBUF 1024
#define CERTFILE "client.pem"

typedef union{
    struct{
        unsigned char c1,c2,c3;
    };
    struct{
        unsigned int e1:6,e2:6,e3:6,e4:6;
    };
} BF;

static const char MimeBase64[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

void base64e(char *src, char *result, int length){
    int i, j = 0;
    BF temp;

    for(i = 0 ; i < length ; i = i+3, j = j+4){
        temp.c3 = src[i];
        if((i+1) > length) temp.c2 = 0;
        else temp.c2 = src[i+1];
        if((i+2) > length) temp.c1 = 0;
        else temp.c1 = src[i+2];

        result[j]   = MimeBase64[temp.e4];
        result[j+1] = MimeBase64[temp.e3];
        result[j+2] = MimeBase64[temp.e2];
        result[j+3] = MimeBase64[temp.e1];

        if((i+2) > length) result[j+2] = '=';
        if((i+3) > length) result[j+3] = '=';
    }
}

SSL_CTX *setup_client_ctx(void){
	SSL_CTX *ctx;

	ctx = SSL_CTX_new(SSLv23_method( ));
  if(SSL_CTX_use_PrivateKey_file(ctx, CERTFILE, SSL_FILETYPE_PEM)!=1)
	 int_error("Error loading private key from file");

  return ctx;
}

int main(int argc, char *argv[])
{
	char from_email[100];
	strcpy(from_email, argv[1]);
  char to_email[100];
  strcpy(to_email, argv[2]);
  char from_name[100];
	strcpy(from_name, argv[3]);
  char to_name[100];
  strcpy(to_name, argv[4]);
	char from_pass[30];
	strcpy(from_pass, argv[5]);
  char sub[200];
  strcpy(sub, argv[6]);
  char * body = (char *)malloc(5000);
	char * head = (char *)malloc(5500);
  strcpy(body, argv[7]);

  BIO *conn;
  SSL *ssl;
  SSL_CTX *ctx;
  struct hostent * gmail_ip;
  char *gmail_ip_str = (char *)malloc(20);
  gmail_ip = gethostbyname("gmail-smtp-msa.l.google.com.");
  gmail_ip_str = inet_ntoa( *(struct in_addr*)gmail_ip->h_addr_list[0]);
  char buffer[MAXBUF];

	strcat(head,"from: ");
	strcat(head,from_name);
	strcat(head,"<");
	strcat(head,from_email);
	strcat(head,">\r\nto: ");
	strcat(head,to_name);
	strcat(head,"<");
	strcat(head,to_email);
	strcat(head,">\r\nsubject: ");
	strcat(head,sub);
	strcat(head,"\r\n");


	init_OpenSSL();
  seed_prng();

  ctx = setup_client_ctx();
  gmail_ip_str = strcat(gmail_ip_str,":465");
  conn = BIO_new_connect(gmail_ip_str);

  if(!conn)
    int_error("Error creating connection BIO");
  if(BIO_do_connect(conn) <= 0)
    int_error("Error connection to remote machine");
  if(!(ssl = SSL_new(ctx)))
    int_error("Error creating an SSL context");
  SSL_set_bio(ssl, conn, conn);
  if(SSL_connect(ssl)<=0)
    int_error("Error connecting SSL object");

  bzero(buffer,sizeof(buffer));
  SSL_read(ssl, buffer, sizeof(buffer));

	char * helo = "helo hi\r\n";
	SSL_write(ssl,helo,strlen(helo));

  bzero(buffer,sizeof(buffer));
  SSL_read(ssl, buffer, sizeof(buffer));

	char * login = "auth login\r\n";
  SSL_write(ssl,login,strlen(login));

  bzero(buffer,sizeof(buffer));
  SSL_read(ssl, buffer, sizeof(buffer));

	char * base64encryptedemail = (char *)malloc(100);
	base64e(from_email,base64encryptedemail,strlen(from_email));
	strcat(base64encryptedemail,"\r\n");
	SSL_write(ssl,base64encryptedemail,strlen(base64encryptedemail));

	bzero(buffer,sizeof(buffer));
  SSL_read(ssl, buffer, sizeof(buffer));

  char * base64encryptedpass = (char *)malloc(100);
  base64e(from_pass,base64encryptedpass,strlen(from_pass));
  strcat(base64encryptedpass,"\r\n");
	SSL_write(ssl,base64encryptedpass,strlen(base64encryptedpass));

  bzero(buffer,sizeof(buffer));
  SSL_read(ssl, buffer, sizeof(buffer));
  strcat(from_email,">\r\n");
	char from[100] = "mail from: <";
	strcat(from,from_email);
	SSL_write(ssl,from,strlen(from));

  bzero(buffer,sizeof(buffer));
  SSL_read(ssl, buffer, sizeof(buffer));

  strcat(to_email,">\r\n");
	char rcpt[100] = "rcpt to: <";
  strcat(rcpt,to_email);
  SSL_write(ssl,rcpt,strlen(rcpt));

  bzero(buffer,sizeof(buffer));
  SSL_read(ssl, buffer, sizeof(buffer));

	char * data ="data\r\n";
  SSL_write(ssl,data,strlen(data));

  bzero(buffer,sizeof(buffer));
  SSL_read(ssl, buffer, sizeof(buffer));

	strcat(head,body);
	head[strlen(head)-5] = '\r';
	head[strlen(head)-4] = '\n';
	head[strlen(head)-3] = '.';
	head[strlen(head)-2] = '\r';
	head[strlen(head)-1] = '\n';
	SSL_write(ssl,head,strlen(head));

  bzero(buffer,sizeof(buffer));
  SSL_read(ssl, buffer, sizeof(buffer));

  char * quit = "quit\n";
  SSL_write(ssl,quit,strlen(quit));

  bzero(buffer,sizeof(buffer));
  SSL_read(ssl, buffer, sizeof(buffer));

  SSL_free(ssl);
  SSL_CTX_free(ctx);
	return 0;
}
