/*
* ajtc.c- Sigmastar
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
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
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
int d=0;
int generate_aj_packet(char **buf,int dataLen, int packetid )
{
    char *pkt=malloc(dataLen+8);
    int ckm=0;
    int i=0;
    if(pkt==NULL)
    {
        printf("malloc aj_packet failed!!\n");
        return -1;
    }
    pkt[0]=(char)0x51;
    pkt[1]=(char)0x58;
    pkt[2]=(char)0x91;
    pkt[3]=(char)0x58;
    pkt[4]=(char)(dataLen&0xFF);
    pkt[5]=(char)((dataLen>>8)&0xFF);
    pkt[6]=(char)((dataLen>>16)&0xFF);
    pkt[7]=(char)((dataLen>>24)&0xFF);
    d=0;
//      System.out.println("dataLen="+dataLen+", "+String.format("0x%02X,0x%02X,0x%02X,0x%02X",pkt[4],pkt[5],pkt[6],pkt[7]));
    pkt[8]=(char)(packetid&0xFF);
    pkt[9]=(char)((packetid>>8)&0xFF);
    pkt[10]=(char)((packetid>>16)&0xFF);
    pkt[11]=(char)((packetid>>24)&0xFF);

    pkt[12]=(char)('A');
    pkt[13]=(char)('J');
    pkt[14]=(char)('S');
    pkt[15]=(char)('X');

    pkt[16]=(char)('X');
    pkt[17]=(char)('S');
    pkt[18]=(char)('J');
    pkt[19]=(char)('A');

    pkt[20]=(char)(0);
    pkt[21]=(char)(0);
    pkt[22]=(char)(0);
    pkt[23]=(char)(0);


        for(i=16;i<dataLen-4;i++){
//          pkt[i+8]=(byte)(d1+((d++)&0x0000000F));//0x5A;//(byte)ran.nextInt(255);
            pkt[i+8]=(char)rand()%255+1;
        }
//      d1+=0x10;
//      d1&=0x000000F0;
        ckm=checksum((unsigned char *)(pkt+8),dataLen-4);
        pkt[dataLen+4]=(char)(ckm&0xFF);
        pkt[dataLen+5]=(char)((ckm>>8)&0xFF);;
        pkt[dataLen+6]=(char)((ckm>>16)&0xFF);;
        pkt[dataLen+7]=(char)((ckm>>24)&0xFF);;
        //System.out.println("dataLen="+dataLen+", "+String.format("0x%02X,0x%02X,0x%02X,0x%02X, 0x%08X, 0x%02X,0x%02X,0x%02X,0x%02X",pkt[4],pkt[5],pkt[6],pkt[7],ckm,pkt[dataLen+4],pkt[dataLen+5],pkt[dataLen+6],pkt[dataLen+7]));
        //System.out.println("pktid="+packetid+", checksum=0x"+Integer.toHexString(ckm).toUpperCase());
        *buf=pkt;
        return (dataLen+8);
}

int pktcount;
int main(int argc , char **argv)
{

    int sock;
    const char* default_ip="10.10.10.107";
    int port=3333;
    int count=100000;
    int loop=0;
    int option=0;
    int packet_size_fixed=0;
    char server_ip[128];
    struct sockaddr_in server;
    srand( (unsigned)time(NULL) );




    printf("[AJ Test Client(packet sender)]\n");

    memset(server_ip,0,sizeof(server_ip));
    memcpy(server_ip,default_ip,strlen(default_ip));


   while ((option = getopt(argc, argv,"p:s:c:f")) != -1) {
      switch (option) {
           case 's' : //server ip
           {
               memset(server_ip,0,sizeof(server_ip));
               memcpy(server_ip,optarg,strlen(optarg));
           }
               break;
           case 'p' : port = atoi(optarg); //port
               break;
           case 'c' : count = atoi(optarg); // test packet count
                          break;

           case 'f' : packet_size_fixed=1;
                         break;


           default: printf("invalide args\n");
               exit(EXIT_FAILURE);
      }
   }

   if(0==count)loop=1;

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }

    bzero(&server,sizeof(server));
    server.sin_addr.s_addr = inet_addr(server_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons( port );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    printf("Connected\n");

    //keep communicating with server
    while((count>0) || loop)
    {
        char *abuf[1]={0};
        int packet_size=1024;
        int alen=0;
        if(!packet_size_fixed)
        {
            packet_size= 64+(int)(((double)(rand()) / (RAND_MAX + 1.0))*128*1024);
        }

        if((alen=generate_aj_packet(abuf, packet_size, pktcount))<=0)
        {
            printf("generate_aj_packet error!!\n");
            goto DONE;
        }
        send(sock,*abuf,alen,0);
        free(*abuf);
        if(!loop)count--;
        pktcount++;
        if((pktcount%1000)==0)printf("%d ajtest packet sent...\n",pktcount);

    }
DONE:
    close(sock);
    return 0;
}
