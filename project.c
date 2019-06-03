#include "common.h"

#define CERTFILE "client.pem"

typedef struct mail{
  char mail_id[2]; // mail 각각 고유값
  char sub[100]; // 제목
  char from_name[20]; // 보낸이 이름
  char from_email[30]; // 보낸이 이메일
  int day;
  int is_trash; // 지운 메일
  int is_read; // 읽은 메일
}mail_info;

typedef struct user_info{
  char user_email[30];
  char user_name[20];
  char user_pass[30];
}user_info;

user_info * user;
mail_info * mail;
int * mail_cnt;

int getch(void) {
  int ch;
  struct termios old, new;
  tcgetattr(0, &old);
  new = old;
  new.c_lflag &= ~(ICANON|ECHO);
  new.c_cc[VMIN] = 1;
  new.c_cc[VTIME] = 0;
  tcsetattr(0, TCSAFLUSH, &new);
  ch = getchar();
  tcsetattr(0, TCSAFLUSH, &old);
  return ch;
}

int mail_index_search(char id[]){
  for(int i=0;i<*mail_cnt;i++){
    if(!strcmp((&mail[i])->mail_id,id))
      return i;
  }
  return -1;
}
void send_mail(){
  int pchild;
  char to_email[30];
  char to_name[20];
  char sub[100];
  char * body = (char *)malloc(5000);
  printf("받는 사람 이메일 주소 : ");
  scanf("%s",to_email);
  printf("받는 사람 이름 : ");
  scanf("%s",to_name);
  printf("메일 제목 : ");
  scanf("%s",sub);

  printf("메일 내용 (내용 입력을 마친 후 EOM(End Of Mail)을 입력해주세요.): ");
  char * phrase = (char *)malloc(5000);
  fgets(phrase,sizeof(phrase),stdin);
  while((strcmp(phrase,"EOM\n"))){
	  strcat(body,phrase);
  	fgets(phrase,sizeof(phrase),stdin);
  }
  strcat(body,phrase);

  if((pchild = fork()) == 0){
    char * argv[] = {"./sendmail",user->user_email,to_email,user->user_name,to_name,user->user_pass,sub,body,NULL};
    execvp("./sendmail",argv);
  }
  else if(pchild > 0){
    int result = 0;
    printf("Mail is being sended~\n");
    wait(NULL);
    printf("Sending mail is set.\n");
  }
  else{
    printf("send error\n");
    return;
  }
}

int get_idcnt(){
  // 메일의 총 갯수 리턴
  return 0;
}
int receive_mail(){
  // get_idcnt를 통해 얻은 수만큼 mail 변수에 동적할당
  // 크롤러를 통해 받은 메세지들을 메일 구조체에 담음.
  // spam_email.txt 확인 후 존재하는 이메일은 구조체에 담지 않을 것.
  mail = (mail_info*)malloc(sizeof(mail_info)*get_idcnt());
  int i = 0;
  // mail_id?;sub?;from?;from_email?;is_trash?;is_read
}

char * get_mail_body(){
    //mail 내용 가져오기.
}

void list_mail(){
  // 메일 보관함에 있는 메일들을 모두 출력.
  for(int i=0;i<*mail_cnt;i++){
    if(!mail[i].is_trash){
      printf("%s\t%s\t%s<%s>\t%d",(&mail[i])->mail_id,(&mail[i])->sub,(&mail[i])->from_name,(&mail[i])->from_email,mail[i].day);
    }
  }
}

int alert_unread_mail(){
 // 처음 프로그램 작동시 새롭게 추가된 메세지가 있으면 알려줌.
}

int open_mail(char id[]){
  int index = index = mail_index_search(id);
  if(index < 0)
    return -1;
  printf("%s\n%s<%s>\n%d\n%s",(&mail[index])->sub,(&mail[index])->from_name,(&mail[index])->from_email,mail[index].day,get_mail_body());
}

void spam_email(char spam_email[]){
  FILE * sf = fopen("spam_email.txt","rw");
  char email[30] = "\t";
  char * str = (char*)malloc(10000);
  strcat(email,spam_email);
  strcat(email,"\n");
  int flag = 0;
  while(fgets(str,sizeof(str),sf)!=NULL){
    if(strcmp(str,user->user_email)){
      fputs(email,sf);
      break;
    }
  }
  fclose(sf);
}

int mail_to_trash(char id[]){
  int index = mail_index_search(id);
  if(index < 0){
    return -1;
  }
  mail[index].is_trash = 1;
  return 1;
}

void helper(){
  printf("--------------------------------------------------------\n");
  printf("This is Simple Gmail Program.\n");
  printf("We support mail open, mail send, mail remove etc...\n\n");

  printf("update\t\t\t : Update new mail from storage.\n");
  printf("list\t\t\t : Print the mail list\n");
  printf("open (mail_id)\t\t : Open the mail to see in detail\n");
  printf("remove (mail_id)\t : Remove the mail from list\n");
  printf("send\t\t\t : Send mail\n");
  printf("spam (email address)\t : Set the spam email\n");
  printf("exit\t\t\t : Program exit\n");
printf("--------------------------------------------------------\n");
}

void login(){
  user = (user_info *)malloc(sizeof(user_info));
  printf("***************** Simple Gmail Program *****************\n");
  printf("------------------------ Login -------------------------\n");
  printf("Name : ");
  scanf("%s",user->user_name);
  printf("\n");
  printf("Gmail Id : ");
  scanf("%s",user->user_email);
  printf("\n");
  printf("Gmail Password : ");
  int i=0;
  char pass[30];
  while(getchar()!='\n');
  while(1){
      pass[i] = getch();
      if(pass[i] == 10){
        break;
      }
      i++;
  }
  pass[i] = '\0';
  strcpy(user->user_pass,pass);
  printf("\n");
  printf("********************************************************\n");
}

int main(void)
{
  login();
  FILE * fp = fopen("mail_storage.txt","w");
  FILE * sp = fopen("spam_email.txt","w"); // 이후에 txt파일 읽어서 같은 email 이 존재 하지 않으면 spam_email.txt에 email 레이블 쓰기.
  char deilm[2] = "?;";
  char command[30];
  char * str;
  printf("If you need help, write command : help\n\n");


  while(1){
    printf("command : ");
    fgets(command,sizeof(command),stdin);
    command[strlen(command)-1] = '\0';
    str = strtok(command, " ");
    if(!strcmp(str,"update")){
      receive_mail();
      *mail_cnt = get_idcnt();
    }
    else if(!strcmp(str,"list")){
      list_mail();
    }
    else if(!strcmp(str,"open")){
      str = strtok(NULL, " ");
      if(open_mail(str) < 0)
        printf("There is no such mail ID\n");
    }
    else if(!strcmp(str,"remove")){
      str = strtok(NULL, " ");
      if(mail_to_trash(str)<0)
        printf("There is no such mail ID\n");
    }
    else if(!strcmp(str,"send")){
      send_mail();
    }
    else if(!strcmp(str,"spam")){
      str = strtok(NULL, " ");
      spam_email(str);
    }
    else if(!strcmp(str,"help")){
      helper();
    }
    else if(!strcmp(str,"exit")){
      printf("~~Bye bye~~\n");
      exit(0);
    }
    else{
      printf("Please write command again...\n");
      continue;
    }
  }
  fclose(fp);
  return 0;
}
