#include "common.h"
#include "parson.h"

#define CERTFILE "client.pem"

typedef struct mail{
  char mail_id[20]; // mail 각각 고유값
  char sub[200]; // 제목
  char from_name[100]; // 보낸이 이름
  char from_email[100]; // 보낸이 이메일
  long day;
  int is_trash; // 지운 메일
  int is_read; // 읽은 메일
}mail_info;

typedef struct user_info{
  char user_email[100];
  char user_name[100];
  char user_pass[30];
}user_info;

typedef struct multiargv{
  char email[100];
  char spam_email[100];
}multiargv;

user_info * user;
mail_info * mail;
int * mail_cnt;
int * old_mail_cnt;
int * new_mail;
pthread_mutex_t mutex_mail = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_spam = PTHREAD_MUTEX_INITIALIZER;

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

void send_mail(){
  int pchild;
  char to_email[30];
  char to_name[20];
  char sub[100];
  char * body = (char *)malloc(5000);
  strcpy(body,"");
  printf("받는 사람 이메일 주소 : ");
  scanf("%s",to_email);
  printf("받는 사람 이름 : ");
  scanf("%s",to_name);
  printf("메일 제목 : ");
  scanf("%s",sub);
  printf("메일 내용 (내용 입력을 마친 후 EOM(End Of Mail)을 입력해주세요.):\n");
  char * phrase = (char *)malloc(1000);
  strcpy(phrase,"");
  while(getchar()!='\n');
  fgets(phrase,malloc_usable_size(phrase),stdin);
  while((strcmp(phrase,"EOM\n"))){
	  strcat(body,phrase);
    strcpy(phrase,"");
    fgets(phrase,malloc_usable_size(phrase),stdin);
  }
  strcat(body,phrase);
  free(phrase);
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
  free(body);
} //done
int get_idcnt(){
  // 메일의 총 갯수 리턴
  JSON_Value *rootValue;
  JSON_Object *rootObject;
  pthread_mutex_lock(&mutex_mail);
  char file_name[70] = "list_mail-";
  strcat(file_name,user->user_email);
  strcat(file_name,".json");
  rootValue = json_parse_file(file_name);
  rootObject = json_value_get_object(rootValue);
  pthread_mutex_unlock(&mutex_mail);
  return json_object_get_number(rootObject,"Count");
}
void *update_mail(){
  // mail_cnt만큼 mail 변수에 동적할당
  // 파이썬 크롤러 프로그램을 통해 json파일에 저장 받은 메세지들을 메일 구조체타입 배열에 담음.
  JSON_Value *rootValue;
  JSON_Object *rootObject;

  char file_name[70] = "list_mail-";
  strncat(file_name,user->user_email,strlen(user->user_email));
  strncat(file_name,".json",strlen(".json"));
  int pchild;
  if((pchild = fork()) == 0){
    char * argv[] = {"./update_mail",user->user_email, user->user_pass,NULL};
    execvp("./update_mail",argv);
  }
  else if(pchild > 0){
    int result = 0;
    wait(NULL);
  }
  else{
    printf("Update error\n");
    return (void*)-1;
  }

  *old_mail_cnt = *mail_cnt;
  *mail_cnt = get_idcnt();
  rootValue = json_parse_file(file_name);
  rootObject = json_value_get_object(rootValue);
  char * buf = (char *)malloc(4);
  JSON_Object * mail_tmp;
  long time = json_object_get_number(rootObject,"update");
  pthread_mutex_lock(&mutex_mail);
  pthread_mutex_lock(&mutex_spam);
  if(malloc_usable_size(mail) == 0){
    mail = (mail_info*)malloc(sizeof(mail_info)*(*mail_cnt));
    for(int i=0;i<*mail_cnt;i++){
      sprintf(buf,"%d",i+1);
      mail_tmp = json_object_get_object(rootObject,buf);
      strncpy((&mail[i])->mail_id, json_object_get_string(mail_tmp,"mail_id"),20);
      strncpy((&mail[i])->sub, json_object_get_string(mail_tmp,"sub"),100);
      strncpy((&mail[i])->from_name, json_object_get_string(mail_tmp,"from_name"),50);
      strncpy((&mail[i])->from_email, json_object_get_string(mail_tmp,"from_email"),50);
      mail[i].day = json_object_get_number(mail_tmp,"day");
      mail[i].is_trash = json_object_get_number(mail_tmp,"is_trash");
      mail[i].is_read = json_object_get_number(mail_tmp,"is_read");
      if(time<mail[i].day){
        (*new_mail)++;
      }
    }
  }
  else if(*old_mail_cnt != *mail_cnt){
    mail = realloc(mail,(*mail_cnt)*sizeof(mail_info));
    for(int i=*old_mail_cnt;i<*mail_cnt;i++){
      sprintf(buf,"%d",i);
      mail_tmp = json_object_get_object(rootObject,buf);
      strncpy((&mail[i])->mail_id, json_object_get_string(mail_tmp,"mail_id"),50);
      strncpy((&mail[i])->sub, json_object_get_string(mail_tmp,"sub"),100);
      strncpy((&mail[i])->from_name, json_object_get_string(mail_tmp,"from_name"),50);
      strncpy((&mail[i])->from_email, json_object_get_string(mail_tmp,"from_email"),50);
      mail[i].day = json_object_get_number(mail_tmp,"day");
      mail[i].is_trash = json_object_get_number(mail_tmp,"is_trash");
      mail[i].is_read = json_object_get_number(mail_tmp,"is_read");
      if(time<mail[i].day){
        (*new_mail)++;
      }
    }
  }
  pthread_mutex_unlock(&mutex_mail);
  pthread_mutex_unlock(&mutex_spam);
  json_value_free(rootValue);
}
void get_mail_body(){
    //mail 내용 가져오기.
}
void list_mail(){
  // 메일 보관함에 있는 메일들을 모두 출력. (spam 이메일로 지정된 메일은 제외)
  JSON_Value *rootValue;
  JSON_Object *rootObject;

  rootValue = json_parse_file("spam_email.json");
  rootObject = json_value_get_object(rootValue);
  JSON_Array * spam = json_object_get_array(rootObject,user->user_email);
  int json_cnt = json_array_get_count(spam);
  if(!(*mail_cnt))
    printf("Mail doesn't exist.\n");
  printf("There are %d in storage.(Including in trash)\n",*mail_cnt);
  for(int i=0;i<*mail_cnt;i++){
    if(!mail[i].is_trash){
      int j = 0;
      if(spam != NULL){
        for(j=0;j<json_cnt;j++){
          if(!strcmp((&mail[i])->from_email,json_array_get_string(spam,j))){
            break;
          }
        }
      }
      if(spam == NULL || j == json_cnt){
        if(mail[i].is_read == 1){
          int year = mail[i].day/10000000000;
          int month = mail[i].day%10000000000/100000000;
          int day = mail[i].day%10000000000%100000000/1000000;
          int time = mail[i].day%10000000000%100000000%1000000/10000;
          int minute = mail[i].day%10000000000%100000000%1000000%10000/100;
          int second = mail[i].day%10000000000%100000000%1000000%10000%100;
          printf("%d-----%s-----%s<%s>-----%d-%d-%d-%d-%d-%d\n",i+1,(&mail[i])->sub,(&mail[i])->from_name,(&mail[i])->from_email,year,month,day,time,minute,second);
        }
        else{
          int year = mail[i].day/10000000000;
          int month = mail[i].day%10000000000/100000000;
          int day = mail[i].day%10000000000%100000000/1000000;
          int time = mail[i].day%10000000000%100000000%1000000/10000;
          int minute = mail[i].day%10000000000%100000000%1000000%10000/100;
          int second = mail[i].day%10000000000%100000000%1000000%10000%100;
          printf("%d\t%s\t%s<%s>\t%d-%d-%d-%d-%d-%d\n",i+1,(&mail[i])->sub,(&mail[i])->from_name,(&mail[i])->from_email,year,month,day,time,minute,second);
        }
      }
    }
  }
  json_value_free(rootValue);
}
void open_mail(int index){
  if(index < 1 || index > *mail_cnt){
    printf("Incorrect number.\n");
    return;
  }
  mail[index].is_read = 1;
  int year = mail[index].day/10000000000;
  int month = mail[index].day%10000000000/100000000;
  int day = mail[index].day%10000000000%100000000/1000000;
  int time = mail[index].day%10000000000%100000000%1000000/10000;
  int minute = mail[index].day%10000000000%100000000%1000000%10000/100;
  int second = mail[index].day%10000000000%100000000%1000000%10000%100;
  printf("%s\n%s<%s>\n%d-%d-%d-%d-%d-%d\n",(&mail[index])->sub,(&mail[index])->from_name,(&mail[index])->from_email,year,month,day,time,minute,second);
  get_mail_body();
}
void *spam_email(void * data){
  pthread_mutex_lock(&mutex_spam);
  char * email = (char *)malloc(100);
  char * spam_email = (char *)malloc(100);
  strcpy(email,((char*)((multiargv*)data)->email));
  strcpy(spam_email,(char*)(((multiargv*)data)->spam_email));
  JSON_Value *rootValue;
  JSON_Object *rootObject;

  rootValue = json_parse_file("spam_email.json");
  rootObject = json_value_get_object(rootValue);

  int count = json_object_get_number(rootObject,"count");
  JSON_Array ** emails = (JSON_Array **)malloc(sizeof(JSON_Array*)*(count+1));
  JSON_Array * tmp = json_object_get_array(rootObject, email);

  for(int i=0;i<=count;i++){
    const char * tmp_email = (const char *)malloc(1);
    char * buf = (char *)malloc(4);
    sprintf(buf,"%d",i);
    tmp_email = json_object_get_string(rootObject,buf);
    emails[i] = json_object_get_array(rootObject,tmp_email);
  }
  if(tmp == NULL){
    count++;
    emails = (JSON_Array **)realloc(emails,sizeof(JSON_Array*)*(count+1));
    char * buf = (char *)malloc(4);
    sprintf(buf,"%d",count);
    json_object_set_string(rootObject,buf,email);
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
} //done
void *mail_to_trash(void * data){
  pthread_mutex_lock(&mutex_mail);
  int index = *(int*)data;
  if(index < 1 || index > *mail_cnt){
    return (void *)-1;
  }
  JSON_Value *rootValue;
  JSON_Object *rootObject;

  char file_name[70] = "list_mail-";
  strncat(file_name,user->user_email,strlen(user->user_email));
  strncat(file_name,".json",strlen(".json"));
  rootValue = json_parse_file(file_name);
  rootObject = json_value_get_object(rootValue);
  char * buf = (char *)malloc(4);
  sprintf(buf,"%d",index);
  JSON_Object *subObject = json_object_get_object(rootObject,buf);
  json_object_set_number(subObject,"is_trash",1);
  mail[index-1].is_trash = 1;
  json_serialize_to_file_pretty(rootValue,file_name);
  json_value_free(rootValue);
  pthread_mutex_unlock(&mutex_mail);
}
void helper(){
  printf("--------------------------------------------------------\n");
  printf("This is Simple Gmail Program.\n");
  printf("We support mail open, mail send, mail remove etc...\n\n");

  printf("update\t\t\t : Update new mail from storage.\n");
  printf("list\t\t\t : Print the mail list\n");
  printf("open (list number)\t\t : Open the mail to see in detail\n");
  printf("remove (mail_id)\t : Remove the mail from list\n");
  printf("send\t\t\t : Send mail\n");
  printf("spam (email address)\t : Set the spam email\n");
  printf("exit\t\t\t : Program exit\n");
  printf("\nps : In the case that mail is related with other thing except simple string.\n     I'm not sure that the mail is opened well...\n");
  printf("--------------------------------------------------------\n");
} //done
void login(){
  user = (user_info *)malloc(sizeof(user_info));
  printf("***************** Simple Gmail Program *****************\n");
  char c;
  char * pass;
  do{
    printf("------------------------ Login -------------------------\n");
    printf("Name : ");
    scanf("%s",user->user_name);
    printf("\n");
    printf("Gmail Id : ");
    scanf("%s",user->user_email);
    printf("\n");
    printf("Gmail Password : ");
    int i = 0;
    pass = (char *)malloc(30);
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
    int pchild;
    if((pchild = fork()) == 0){
      char * argv[] = {"./login",user->user_email,user->user_pass,NULL};
      execvp("./login",argv);
    }
    else if(pchild > 0){
      int result = 0;
      wait(NULL);
    }
    else{
      printf("Login error\n");
      return;
    }
    FILE * fp = fopen("login.txt","r");
    c = fgetc(fp);
    fclose(fp);
    system("rm login.txt");
    printf("\n");
    if(c=='0'){
      printf("Login fail. Check again.\n");
      printf("********************************************************\n");
      free(pass);
    }
  }while(c=='0');
  printf("Login success.\n");
  printf("********************************************************\n");
} //done

int main(void)
{
  login();
  char command[120];
  char * str;
  pthread_t p_thread[3];
  int thr_id;
  int status;
  int * flag = (int *)calloc(3,sizeof(int));
  mail_cnt = (int *)calloc(1,sizeof(int));
  old_mail_cnt = (int *)calloc(1,sizeof(int));
  new_mail = (int *)calloc(1,sizeof(int));
  if(flag[0]){
    pthread_join(p_thread[0], (void **)&status);
    flag[0] = 0;
  }
  if(flag[1]){
    pthread_join(p_thread[1], (void **)&status);
    flag[1] = 0;
  }
  if(flag[2]){
    pthread_join(p_thread[2], (void **)&status);
    flag[2] = 0;
  }
  thr_id = pthread_create(&p_thread[0], NULL, update_mail, NULL);
  flag[0] = 1;
  if(flag[0]){
    pthread_join(p_thread[0], (void **)&status);
    flag[0] = 0;
  }
  printf("All mails are loaded.\n");
  *new_mail = 0;
  printf("If you need help, write command : help\n\n");

  while(1){
    if(flag[2]){
      pthread_join(p_thread[2], (void **)&status);
      flag[2] = 0;
    }
    printf("command : ");
    fgets(command,sizeof(command),stdin);
    command[strlen(command)-1] = '\0';
    str = strtok(command, " ");
    if(!strcmp(str,"update")){
      if(*new_mail){
        printf("New %d mails are updated.\n",*new_mail);
        *new_mail = 0;
      }
      else{
        printf("New mail doesn't exist.\n");
      }
      if(flag[0]){
        pthread_join(p_thread[0], (void **)&status);
        flag[0] = 0;
      }
      if(flag[1]){
        pthread_join(p_thread[1], (void **)&status);
        flag[1] = 0;
      }
      if(flag[2]){
        pthread_join(p_thread[2], (void **)&status);
        flag[2] = 0;
      }
      thr_id = pthread_create(&p_thread[0], NULL, update_mail, NULL);
      flag[0] = 1;
      if(thr_id<0){
        perror("thread create error.");
        continue;
      }
    }
    else if(!strcmp(str,"list")){
      if(flag[0]){
        pthread_join(p_thread[0], (void **)&status);
        flag[0] = 0;
      }
      if(flag[1]){
        pthread_join(p_thread[1], (void **)&status);
        flag[1] = 0;
      }
      if(flag[2]){
        pthread_join(p_thread[2], (void **)&status);
        flag[2] = 0;
      }
      list_mail();
    }
    else if(!strcmp(str,"open")){
      if(flag[0]){
        pthread_join(p_thread[0], (void **)&status);
        flag[0] = 0;
      }
      if(flag[2]){
        pthread_join(p_thread[2], (void **)&status);
        flag[2] = 0;
      }
      str = strtok(NULL, " ");
      if(str == NULL){
        printf("Check Usage by command : help\n");
        continue;
      }
      open_mail(atoi(str));
    }
    else if(!strcmp(str,"remove")){
      str = strtok(NULL, " ");
      if(str == NULL){
        printf("Check Usage by command : help\n");
        continue;
      }
      int index = atoi(str);
      if(index < 1 || index > *mail_cnt){
        printf("Incorrect number.\n");
        continue;
      }
      if(flag[0]){
        pthread_join(p_thread[0], (void **)&status);
        flag[0] = 0;
      }
      if(flag[2]){
        pthread_join(p_thread[2], (void **)&status);
        flag[2] = 0;
      }
      thr_id = pthread_create(&p_thread[2], NULL,mail_to_trash,(void *)&index);
      flag[2] = 1;
      if(thr_id<0){
        perror("thread create error.");
        continue;
      }
      printf("Mail is deleted.\n");
    }
    else if(!strcmp(str,"send")){
      if(flag[0]){
        pthread_join(p_thread[0], (void **)&status);
        flag[0] = 0;
      }
      if(flag[1]){
        pthread_join(p_thread[1], (void **)&status);
        flag[1] = 0;
      }
      send_mail();
    }
    else if(!strcmp(str,"spam")){
      str = strtok(NULL, " ");
      if(str == NULL){
        printf("Check Usage by command : help\n");
        continue;
      }
      multiargv * multi = (multiargv *)malloc(sizeof(multiargv));
      strcpy(multi->email, user->user_email);
      strcpy(multi->spam_email, str);
      if(flag[1]){
        pthread_join(p_thread[1], (void **)&status);
        flag[1] = 0;
      }
      thr_id = pthread_create(&p_thread[1], NULL,spam_email,(void *)multi);
      flag[1] = 1;
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
