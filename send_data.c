#include<stdio.h>
#include<sys/socket.h> 
#include <sys/types.h>

#include<netdb.h> //hostent
#include<arpa/inet.h>//inet_addr

#include<string.h>


#include "Iridium_Emulator_Basic.h"
#include "send_data.h" 
#include "Config_File_read.h" /*configuration file*/

int socket_desc;//socket handle
struct sockaddr_in socketInfo;    

char ip_address[100];
struct hostent *hip;
struct in_addr **addr_list;
   
int Get_Host_Addrs(const char* szDomainName);



int Get_Host_Addrs(const char* szDomainName)
{
    int i;         
   
    if ((hip = gethostbyname(szDomainName)) == (struct hostent *)NULL)
    {
        //gethostbyname failed
        printf("gethostbyname failed");
        return 1;
    }
    //Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only
    addr_list = (struct in_addr **) hip->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++)
    {
        //Return the first one;
        strcpy(ip_address , inet_ntoa(*addr_list[i]) );
    }     
  
    return 0;
}


int Modem_Send_Data_to_Portal(BYTE* data_receive,int data_size)
{
   
    int rx_bytes = 0;
    int iBytesSent = 0;

    //printf("%d\n",data_size); 
    //fflush(stdout);

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1){
       puts("Could not create socket");
    }

    socketInfo.sin_addr.s_addr = inet_addr(ip_address);
    socketInfo.sin_family = AF_INET;
    socketInfo.sin_port = htons(PORT_NUMBER);//7780	 

    //connect to server
    if(connect(socket_desc,(struct sockaddr *)&socketInfo , sizeof(socketInfo)) < 0){
       puts("connect error");
       fflush(stdout);	
       return 1;
    }
    else{
        printf("Connected to SERVER\n");//'%s' with IP Address '%s' on port:%d\n",szDomainName,ip_address,PortNumber);
        fflush(stdout);
        //send data to server
        if((iBytesSent = send(socket_desc, data_receive, data_size, 0)) < 0 ){
            printf("Send failed with error");//cerr << "Send failed with error " << errno << endl;
            close(socket_desc);
            return 1;
       }
       else{
           memset(data_receive,0,data_size);
           close(socket_desc);
           return 0;
      }
  }  
}

