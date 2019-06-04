#include "common.h"
#include "parson.h"

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

typedef struct multiargv{
  char email[50];
  char spam_email[50];
}multiargv;

user_info * user;
mail_info * mail;
int * mail_cnt;
pthread_mutex_t mutex_mail = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_spam = PTHREAD_MUTEX_INITIALIZER;

char * itoa(int val, char * buf, int radix){
  char * p = buf;
  if(!val){
    *p++ = '0';
    *p = '\0';
    return buf;
  }
  while(val){
    if(radix<=10)
      *p++ = (val % radix) + '0';
    else{
      int t = val % radix;
      if(t <= 9)
        *p++ = t+ '0';
      else
        *p++ = t - 10 + 'a';
    }
    val /= radix;
  }
  *p = '\0';
  return buf;
}
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
    printf("Sending mail is finished.\n");
  }
  else{
    printf("send error\n");
  }
}

int get_idcnt(){
  // 메일의 총 갯수 리턴
  return 0;
}
void *receive_mail(){
  // get_idcnt를 통해 얻은 수만큼 mail 변수에 동적할당
  // 크롤러를 통해 받은 메세지들을 메일 구조체에 담음.
  // spam_email.txt 확인 후 존재하는 이메일은 구조체에 담지 않을 것.
  pthread_mutex_lock(&mutex_mail);
  pthread_mutex_lock(&mutex_spam);
  mail = (mail_info*)malloc(sizeof(mail_info)*get_idcnt());
  int i = 0;
  // mail_id?;sub?;from?;from_email?;is_trash?;is_read
  pthread_mutex_unlock(&mutex_mail);
  pthread_mutex_unlock(&mutex_spam);
}
void get_mail_body(){
    //mail 내용 가져오기.
    //python으로 로그인 체크.
}

void list_mail(){
  // 메일 보관함에 있는 메일들을 모두 출력.
  JSON_Value *rootValue;
  JSON_Object *rootObject;

  rootValue = json_parse_file("spam_email.json");
  rootObject = json_value_get_object(rootValue);

  for(int i=0;i<*mail_cnt;i++){
    if(!mail[i].is_trash){
      int j = 0;
      int json_cnt = 0;
      JSON_Array * spam = json_object_get_array(rootObject,user->user_email);
      if(spam != NULL){
        json_cnt = json_array_get_count(spam);
        for(j=0;j<json_cnt;j++){
          if(!strcmp((&mail[i])->from_email,json_array_get_string(spam,j))){
            break;
          }
        }
        free(spam);
      }
      if(spam == NULL || j == json_cnt)
        printf("%s\t%s\t%s<%s>\t%d",(&mail[i])->mail_id,(&mail[i])->sub,(&mail[i])->from_name,(&mail[i])->from_email,mail[i].day);
    }
  }
}

void alert_unread_mail(){
 // 처음 프로그램 작동시 새롭게 추가된 메세지가 있으면 알려줌.
}

void open_mail(void * data){
  char * id = (char *)malloc(50);
  id = (char *)data;
  int index = mail_index_search(id);
  if(index < 0)
    printf("ID is not presented.\n");
  printf("%s\n%s<%s>\n%d\n",(&mail[index])->sub,(&mail[index])->from_name,(&mail[index])->from_email,mail[index].day);
  get_mail_body();
}

void *spam_email(void * data){
  pthread_mutex_lock(&mutex_spam);
  char * email = (char *)malloc(50);
  char * spam_email = (char *)malloc(50);
  email = ((multiargv*)data)->email;
  spam_email = ((multiargv*)data)->spam_email;
  JSON_Value *rootValue;
  JSON_Object *rootObject;

  rootValue = json_parse_file("spam_email.json");
  rootObject = json_value_get_object(rootValue);

  int count = json_object_get_number(rootObject,"count");
  JSON_Array ** emails = (JSON_Array **)malloc(sizeof(JSON_Array*)*(count+1));
  JSON_Array * tmp = json_object_get_array(rootObject, email);

  char * buf = (char *)malloc(4);
  for(int i=0;i<=count;i++){
    const char * tmp_email = (const char *)malloc(1);
    tmp_email = json_object_get_string(rootObject,itoa(i,buf,10));
    emails[i] = json_object_get_array(rootObject,tmp_email);
  }
  if(tmp == NULL){
    count++;
    emails = (JSON_Array **)realloc(emails,sizeof(JSON_Array*)*(count+1));
    json_object_set_string(rootObject,itoa(count,buf,10),email);
    json_object_set_value(rootObject, email, json_value_init_array());
    emails[count] = json_object_get_array(rootObject, email);
    json_array_append_string(emails[count],spam_email);
    json_object_set_number(rootObject,"count",count);
    json_serialize_to_file_pretty(rootValue,"spam_email.json");
    json_value_free(rootValue);
  }
  else{
    json_array_append_string(tmp,spam_email);

    json_serialize_to_file_pretty(rootValue,"spam_email.json");
    json_value_free(rootValue);
  }
  pthread_mutex_unlock(&mutex_spam);
}

void *mail_to_trash(void * data){
  pthread_mutex_lock(&mutex_mail);
  char * id = (char *)malloc(50);
  id = (char *)data;
  int index = mail_index_search(id);
  if(index < 0){
    printf("ID is not presented.\n");
  }
  mail[index].is_trash = 1;
  printf("Mail is deleted.\n");
  pthread_mutex_unlock(&mutex_mail);
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
  //login.py
  int pchild;
  // if((pchild = fork()) == 0){
  //   char * argv[] = {"./login",user->user_email,to_email,tuser->user_pass,NULL};
  //   execvp("./login",argv);
  // }
  // else if(pchild > 0){
  //   int result = 0;
  //   wait(NULL);
  // }
  // else{
  //   printf("Login error\n");
  //   return;
  // }
  printf("\n");
  printf("********************************************************\n");
}

int main(void)
{
  login();
  char command[30];
  char * str;
  pthread_t p_thread[7];
  int thr_id;
  int status;
  printf("If you need help, write command : help\n\n");
  int * flag = (int *)calloc(6,sizeof(int));
  while(1){
    printf("command : ");
    fgets(command,sizeof(command),stdin);
    command[strlen(command)-1] = '\0';
    str = strtok(command, " ");
    if(!strcmp(str,"update")){
      *mail_cnt = get_idcnt();
      if(flag[0])
        pthread_join(p_thread[0], (void **)&status);
      thr_id = pthread_create(&p_thread[0], NULL, receive_mail, NULL);
      flag[0] = 1;
      if(thr_id<0){
        perror("thread create error.");
        continue;
      }
    }
    else if(!strcmp(str,"list")){
      list_mail();
    }
    else if(!strcmp(str,"open")){
      str = strtok(NULL, " ");
      open_mail(str);
    }
    else if(!strcmp(str,"remove")){
      str = strtok(NULL, " ");
      if(flag[1])
        pthread_join(p_thread[1], (void **)&status);
      thr_id = pthread_create(&p_thread[3], NULL,mail_to_trash,(void *)str);
      flag[1] = 1;
      if(thr_id<0){
        perror("thread create error.");
        continue;
      }
    }
    else if(!strcmp(str,"send")){
      send_mail();
    }
    else if(!strcmp(str,"spam")){
      str = strtok(NULL, " ");
      multiargv * multi = (multiargv *)malloc(sizeof(multiargv));
      strcpy(multi->email, user->user_email);
      strcpy(multi->spam_email, str);
      if(flag[2])
        pthread_join(p_thread[2], (void **)&status);
      thr_id = pthread_create(&p_thread[2], NULL,spam_email,(void *)multi);
      flag[2] = 1;
      if(thr_id<0){
        perror("thread create error.");
        continue;
      }
    }
    else if(!strcmp(str,"help")){
      helper();
    }
    else if(!strcmp(str,"exit")){
      if(flag[0])
        pthread_join(p_thread[0], (void **)&status);
      if(flag[1])
        pthread_join(p_thread[1], (void **)&status);
      if(flag[2])
        pthread_join(p_thread[2], (void **)&status);
      printf("~~Bye bye~~\n");
      exit(0);
    }
    else{
      printf("Please write command again...\n");
      continue;
    }
  }
  return 0;
}
