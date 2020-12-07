#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAX_NAME    30
#define    MAX_MSG    1024
#define SOCKETPORT    9000

typedef struct _client{   //클라이언트 구조체 
    int disc;
    int clientnum;
    int num;
    char name[MAX_NAME];
}Client;

typedef struct _room{   //채팅방 구조체 
    int num;
    int client_list[5];
    int room_count;
}Room; 

Client list[5];
Room rlist[5];

int sd;
int num;
int room_count = 0;
int clientnum = 0;
int rusernum = 0;
pthread_mutex_t usermutex, roommutex;

void SSockExit(int signo);
int SSockets(char *ip, int port);
void *Notice(void *client);
void CSockExit(Client client);
void FileSend(Client client, char *rbuf);
void *ReceiveSend(void *client);
void *StartTalk(void *client);
void *thread_main(void *room);

void *thread_main(void *rm)      //각각의 채팅방이 클라이언트 연결과 관련된 쓰레드 
{                        
    pthread_mutex_lock(&roommutex);   //채팅방의 뮤텍스 락 
    Client client;   //클라이언트 구조체 선언 
    Room room;      //채팅방 구조체 선언 

    int sockid;   //접속하는 소켓의 id 
    int port = SOCKETPORT + room_count;   //채팅방의 포트번호, 방의 개수가 증가하면 포트가 1씩 증가 
    room.room_count = room_count;

   //IPv4 주소영역의 양방향 TCP/IP 기반의 통신 
    sockid = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in ssockaddress = {0};
    ssockaddress.sin_family = AF_INET;   //IPv4 주소체계 
    ssockaddress.sin_port = htons(port);   //포트번호 
    ssockaddress.sin_addr.s_addr = inet_addr("127.0.0.1");   //IP주소 

   //소켓에 IP주소와 포트번호를 지정 
    if(bind(sockid, (struct sockaddr *)&ssockaddress, sizeof(ssockaddress)))
    {
       //bind 실패시 에러문구 출력 및 해당 소켓 close 
        perror("bind error");
        close(sockid);
    }
    
    //소켓을 통해 클라이언트의 접속요청을 기다림 
    if(listen(sockid, 5) == -1)
    {
       //listen 실패시 에러문구 출력 및 해당 소켓 close 
        perror("listen error");
        close(sockid);
    }

    printf("port %d\n",port);

    room_count++; 

    pthread_mutex_unlock(&roommutex);   //채팅방 뮤텍스 언락 

    struct sockaddr_in csockaddress = {0};
    int clen = sizeof(csockaddress);
    pthread_t ptr;
    
    //클라이언트의 접속 요청을 받아드리고 클라이언트와 통신하는 전용소켓 생성 
    while((client.disc = accept(sockid, (struct sockaddr *)&csockaddress, &clen)))
    {
        if(client.disc == -1)
        {
           //accept 실패시 에러문구 출력 
            perror("accept error");
            return 0;
        }

        pthread_mutex_lock(&usermutex);   //유저 뮤텍스 락 
        
        client.clientnum = clientnum;

        client.num = sockid;   //소켓 아이디를 클라이언트가 가짐 
        room.num = sockid;   //소켓 아이디를 방 구조체가 가짐 
        room.client_list[room.room_count] == client.disc;
        
        rlist[room.room_count] = room;   
        pthread_create(&ptr, NULL, StartTalk, &client);   //StartTalk 함수 쓰레드 생성 
        pthread_detach(ptr);

        clientnum++;   //accept가 된것이므로 클라이언트 수 증가 
        room.room_count++;
        
        pthread_mutex_unlock(&usermutex);   //유저 뮤텍스 언락 
    }
    
    return 0;
}

void *StartTalk(void *client)   //채팅방 접속하여 초반 
{
    Client cs = *(Client *)client;   //클라이언트 구조체 선언 
    char nick[MAX_NAME]="";   //사용자 이름을 저장할 변수 
    pthread_t ptr[2];

    send(cs.disc, "채팅을 시작합니다.\n이름을 입력하세요.\n", 100, 0);   //문구를 클라이언트에 전송 
    recv(cs.disc, nick, sizeof(nick), 0);   //클라이언트가 입력한 이름을 변수에 저장 
    nick[strlen(nick)-1] = '\0';   //쓰레기 값을 없애기 위해 문자열 마지막에 '\0' 강제 삽입 
    printf("%s 접속\n",nick);   //서버 화면에 접속한 이름 출력 
    
    strcpy(cs.name, nick);   //사용자가 입력한 이름을 구조체에 저장 
    list[cs.clientnum] = cs;   //구조체 정보를 리스트에 저장 
    pthread_create(&ptr[0], NULL, ReceiveSend, &cs);   //메시지 전달 쓰레드 생성 
    pthread_create(&ptr[1], NULL, Notice, &cs);   //공지 쓰레드 생성 
    pthread_join(ptr[0], NULL);
    pthread_join(ptr[1], NULL);
}

void *ReceiveSend(void *client)   //메시지를 전달해주는 역할 
{
    Client cs = *(Client *)client;   //클라이언트 구조체 선언 
    int i;   //반복문을 위한 변수 
    char rbuf[MAX_MSG];   //receive 메시지를 저장할 배열 
    char sbuf[MAX_MSG];   //send 메시지를 저장할 배열 

    while(recv(cs.disc, rbuf, sizeof(rbuf), 0) >0)   //메시지를 receive 
    {
       //파일 전송을 요청하는 메시지인지를 판단 
        if(!strncmp(rbuf,"/f",2))
        {
           //파일 전송 함수 실행 
            FileSend(cs, rbuf);
            continue;
        }
        else   //일반 채팅 
        {
            sprintf(sbuf, "%s : %s", cs.name, rbuf);   //send 메시지 배열에 보낸사람과 받은 메시지를 저장 
            for(i=0;i<clientnum;i++)   //접속한 클라이언트 수만큼 반복 
            {
                if(list[i].disc == cs.disc)
                    continue;
                if(list[i].num == cs.num)
                    send(list[i].disc, sbuf, sizeof(sbuf), 0);   //전달할 메시지 send 
            }
        }

        memset(rbuf, 0, sizeof(rbuf));   //입력 배열 초기화 
        memset(sbuf, 0, sizeof(sbuf));   //입력 배열 초기화 
    }    

    CSockExit(cs);   //클라이언트 소켓 종료 
}

void FileSend(Client client, char *rbuf)   //파일 전송 함수 
{
    int fd;   //파일 open을 저장할 변수 
    char *fio;   //파일명을 저장할 변수 
    char sbuf[MAX_MSG];   //send에 이용할 배열 
    fio = strtok(rbuf, " ");   //공백을 기준으로 받은 문자열을 자름( /f ) 
    fio = strtok(NULL, " ");   //공백을 기준으로 다음 토큰을 받음( 파일명 ) 
    fio[strlen(fio)-1] = '\0';   //문자열의 마지막에 '\0'를 강제 삽입 
    fd = open(fio, O_RDONLY, 0644);   //fio에 저장된 파일명을 open 

    while((read(fd, sbuf, sizeof(sbuf))) > 0)   //파일이 끝날때까지 반복 
    {
        if(send(client.disc, sbuf, sizeof(sbuf), 0) == -1)   //MAX_MSG만큼 전송 
        {
           //실패시 에러 출력 
            perror("send");
            exit(1);
        }
    }
}

void CSockExit(Client client)   //클라이언트 종료 함수 
{
    int i, j;   //반복문에 이용하는 변수 
    char sbuf[MAX_MSG];   //퇴장 메시지를 저장할 배열 
    
    //퇴장한 클라이언트의 이름으로 퇴장 메시지를 sbuf에 저장 
    sprintf(sbuf, "%s님이 퇴장하였습니다.\n", client.name);

    for(i=0;i<clientnum;i++)   //클라이언트의 갯수만큼 반복 
    {
        if(list[i].disc == client.disc)
        {
            for(j=0;j<clientnum;j++)
            {
                if(list[j].disc == client.disc)
                    continue;
                if(list[j].num == client.num)
                    send(list[j].disc, sbuf, sizeof(sbuf), 0); //클라이언트에 메시지 전송 
            }
            break;
        }
    }

    pthread_mutex_lock(&usermutex);
    for(j=i;j<clientnum-1;j++)
    {
        list[j] = list[j+1];
    }
    clientnum --;
    pthread_mutex_unlock(&usermutex);
}

void *Notice(void *client)   //클라이언트에 서버의 메시지를 일괄전달하는 함수 
{
    int i;   //반복문에 사용되는 변수 
    Client cs = *(Client *)client;   //클라이언트 구조체 선언 
    char sbuf[MAX_MSG];   //클라이언트에게 보낼 메시지를 저장할 변수 
    char inbuf[MAX_MSG];   //서버의 입력을 저장할 변수 
    
    //클라이언트의 이름으로 접속 메시지를 sbuf에 저장 
    sprintf(sbuf, "%s님이 접속 하였습니다.\n", cs.name);
    
    for(i=0;i<clientnum;i++)   //클라이언트의 수만큼 반복 
    {
        if(list[i].num == cs.num)
            send(list[i].disc, sbuf, strlen(sbuf), 0);   //접속메시지를 전송 
    }

    while(1)
    {
        memset(sbuf, 0, sizeof(sbuf));   //sbuf 초기화 
        memset(inbuf, 0, sizeof(inbuf));   //inbuf 초기화 
        fgets(inbuf, sizeof(inbuf), stdin);   //서버가 입력한 문자열을 inbuf에 저장 
        sprintf(sbuf, "[SERVER] : %s\n", inbuf);   //sbuf에 서버가 보내는 메시지를 저장 
        for(i=0;i<clientnum;i++)   //클라이언트 수만큼 반복 
            send(list[i].disc, sbuf, strlen(sbuf), 0);   //서버의 메시지를 전송 
    }
}

int SSockets(char *ip, int port)   //필요한 환경을 세팅하는 함수 
{
    int sockid;   //소켓 식별자를 저장할 변수 
    sockid = socket(AF_INET, SOCK_STREAM, 0);   //소켓 생성 - IPv4 주소영역의 양방향 TCP/IP 기반의 통신 
    
    struct sockaddr_in ssockaddress = {0};   //sockaddr_in 구조체 선언 
    ssockaddress.sin_family = AF_INET;   //IPv4
    ssockaddress.sin_port = htons(port);   //포트번호 
    ssockaddress.sin_addr.s_addr = inet_addr(ip);   //IP

   //소켓에 IP주소와 포트번호를 지정 
    if(bind(sockid, (struct sockaddr *)&ssockaddress, sizeof(ssockaddress)))
    {
       //bind 실패시 에러메시지 출력 및 소켓 close 
        perror("bind error");
        close(sockid);
        return -1;
    }
    
    //클라이언트 접속 연결을 기다림 
    if(listen(sockid, 5) == -1)
    {
       //listen 실패시 에러메시지 출력 및 소켓 close 
        perror("listen error");
        close(sockid);
        return -1;
    }
    return sockid;
}

void SSockExit(int signo)   //서버를 종료하는 함수 
{
    printf("서버 종료\n");
    close(num);
    close(sd);
    exit(0);
}


int main()
{
    int room, i;

    signal(SIGINT, SSockExit);

    pthread_mutex_init(&usermutex, NULL);
    pthread_mutex_init(&roommutex, NULL);

    printf("생성할 채팅방 수\n");
    scanf("%d",&room);
    getchar();

    pthread_t ptr[256];

    for(i=0;i<room;i++)   //입력받은 채팅방 수만큼 반복 
    {
        pthread_create(&ptr[i], NULL, thread_main, &room);   //thread_main 함수 쓰레드 생성 
        pthread_detach(ptr[i]);
    }

    while(1)
        pause();

    return 0;
}
