/*
* ajts.c- Sigmastar
*
* Copyright (c) [2019~2020] SigmaStar Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h> // for usleep
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#define __ERR printf
volatile sig_atomic_t flag = 0;
int outfd;
char* fName=NULL;
void finish_function(int sig){ // can be called asynchronously
    flag = 1; // set flag
    if(NULL!=fName)
    {
        free(fName);
    }
    close(outfd);
    exit(-3);
}

//using namespace std;
int recv_with_header(int fd, char **buf)
{
    unsigned char flag[4];
    int ret=recv(fd,flag,4,0);
    int len;
    int offset=0;
    if(ret<=0)
    {

        if(errno==0)
        {
            return 0;
        }
        __ERR("recv flag error, err=%s, %d\n",strerror(errno),errno);
        return -3;
    }

    if(flag[0]!=0x51 || flag[1]!=0x58 || flag[2]!=0x91 || flag[3]!=0x58)
    {
        __ERR("recv bad flag: 0x%02x 0x%02x 0x%02x 0x%02x\n",flag[0],flag[1],flag[2],flag[3]);
        return -4;
    }


    ret=recv(fd,flag,4,0);
    if(ret<=0)
    {
        __ERR("recv len error, err=%s\n",strerror(errno));
        return -5;
    }

    len=(flag[3]<<24) + (flag[2]<<16)+(flag[1]<<8) + (flag[0]);
    if(len<=0)
    {
        __ERR("recv bad len: %d, 0x%02x 0x%02x 0x%02x 0x%02x\n",len, flag[0],flag[1],flag[2],flag[3]);
        return -6;
    }
//    printf("  len=0x%08X\n",len);

    if(len>=(15*1024*1024))
    {
        __ERR("recv bad len: %d\n",len);
        return -7;
    }


    *buf=(char *)malloc(len + 2);
    if(*buf==NULL)
    {
        __ERR("new buf failed.\n");
        return -8;
    }
//    printf("  malloc success, 0x%08X 0x%08X!!\n",*buf, tbuf);
    memset( *buf, 0, len+2);
//    printf("  memset success!!\n");

    while(len)
    {
        ret=recv(fd,  *buf + offset,len,0);
//        printf("  ret=%d\n",ret);
        if(ret<=0)
        {
            __ERR("recv data error, err=%s\n",strerror(errno));
            return -9;
        }
        else
        {
            offset+=ret;
            len-=ret;
        }
    }
//    printf("  recv: %d bytes success\n",offset);
    return offset;
}

unsigned int checksum( unsigned char * buffer, long length )
{

     long index;
     unsigned int checksum;

     for( index = 0L, checksum = 0; index < length; index++)
     {
         checksum += (int)buffer[index];
         checksum &= 0x0FFFFFFF;
//         if(index<10)printf("%d %d\n",buffer[index],checksum);

     }
     return checksum;

}

int main(int argc, char** argv)
{
    int option = 0;
    int port =3333;

    char *tbuf[1]={0};
    int sockfd;
    struct sockaddr_in dest;

    printf("[AJ Test Server]\n");


    //The two options l and b expect numbers as argument
    while ((option = getopt(argc, argv,"p:f:")) != -1) {
       switch (option) {
            case 'f' :
            {
                int len=strlen(optarg)+1;
                fName=(char *)malloc(strlen(optarg)+1);
                if(NULL==fName)
                {
                    printf("failed to allocate memory, exit -1!!\n");
                    exit(-1);
                }
                memset(fName,0,len);
                memcpy(fName,optarg,(len-1));

            }
                break;
            case 'p' : port = atoi(optarg);
                break;

            default: printf("invalide args\n");
                exit(EXIT_FAILURE);
       }
    }
#if 0
    outfd = open(fName,O_RDWR|O_CREAT|O_TRUNC,0777);

    if(outfd<0)
    {
        printf("open file:%s error!!\n",fName);
        perror("error:");
        return -1;
    }
#endif
    if(NULL!=fName)
    {
        printf("Server ready!! fName: %s, socket_port: %d\n",fName,port);
    }else
    {
        printf("Server ready!! socket_port: %d\n",port);
    }
    sleep(1);

    sockfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&dest,sizeof(dest));
    dest.sin_family=AF_INET;
    dest.sin_port=htons(port);
    dest.sin_addr.s_addr=INADDR_ANY;
    signal(SIGINT, finish_function);
    bind(sockfd,(struct sockaddr*)&dest,sizeof(dest));
    listen(sockfd,20);


    while(1)
    {
        int clientfd;
        struct sockaddr_in client_addr;
        int addrlen=sizeof(client_addr);
        int pktCount=0;

        clientfd=accept(sockfd,(struct sockaddr*)&client_addr,(socklen_t*)&addrlen);
        if (clientfd < 0)
        {
            perror("accept failed");
            return -1;
        }
        printf("client connection accepted\n");
        while(1)
        {
            int ret=0;

            ret=recv_with_header(clientfd,tbuf);

            if(ret>0)
            {
#if 0
                write(outfd,*tbuf,ret);
#endif
                unsigned char *buf=(unsigned char*)tbuf[0];//(unsigned char *)(*tbuf);
                int pktid=0;
                int pktcs=0;//((buf[ret-1])<<24) + (buf[ret-2]<<16)+(buf[ret-3]<<8) + (buf[ret-4]);
                int cs=0;
//                if(0==pktCount%100)
//                {
//                    printf("pktCount: %08d\n",pktCount);
//                }
                pktcs=((buf[ret-1])<<24) + (buf[ret-2]<<16)+(buf[ret-3]<<8) + (buf[ret-4]);
                cs=checksum(buf,ret-4);
                pktid=(buf[3]<<24) + (buf[2]<<16)+(buf[1]<<8) + (buf[0]);
                if(pktcs!=cs || pktid!=pktCount)
                {
                    int j=0;
                    unsigned int lc=0;
                    printf("  ERROR!! pktCount[%d, %d]: cs=0x%08X, pktcs=0x%08X\n\n",pktCount,pktid,cs,pktcs);
                    for(j=0;j<ret;j++)
                    {
                        if(0==(j%16)){printf("0x%08X: ",lc++);}
                        printf("%02X ",buf[j]);
                        if(15==(j%16))printf("\n");
                    }
                    printf("\n");
                    while(1);
                }
//                else
//                {
//                    printf("pktCount: %08d, 0x%08X\n",pktCount,cs);
//                }
                free(buf);
                tbuf[0]=NULL;

                pktCount++;
            }
            else if(ret<-2)
            {
                printf("ERROR!!\n");
                break;
            }
            else if(ret==0)
            {

                break;
            }
        }
        printf("client connection closed\n\n");
        close(clientfd);
            //Receive a message from client
//        while( (read_size = recv(clientfd , client_message , 2000 , 0)) > 0 )
//        {
//            //Send the message back to client
//            write(client_sock , client_message , strlen(client_message));
//        }

    }

    if(NULL!=fName)
    {
        free(fName);
    }
    close(outfd);
    return 0;
}
