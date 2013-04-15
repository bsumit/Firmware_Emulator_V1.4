/*  
   Modem Emulator Basic verison

 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <math.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/signal.h>

#include <malloc.h>

#include "Iridium_Emulator_Basic.h"
#include "Emulator_Properties.h" 
#include "Config_File_read.h" 
#include "send_data.h"

const char PROPERTIES_FILE[] = "./iwi.conf";  


volatile int STOP=FALSE; 
       
void signal_handler_IO (int status);   /* definition of signal handler */
int wait_flag=TRUE;                    /* TRUE while no signal received */


const char *input_commands[NUMBER_OF_COMMANDS] = { "AT\n","AT+CGMM\n","AT+CGMR\n",
                                                   "AT+CLCK=\n","AT+CPIN=\n","AT+CPIN?\n","AT+CSQ\n","AT-MSSTM\n","AT+SBDD0\n","AT+SBDIXA\n","AT+SBDMTA=1\n","AT+SBDRB\n","AT+SBDREG\n","AT+SBDREG?\n","AT+CGSN\n",
                                                   "AT+SBDWB=\n","AT+SBDIX\n","AT*F\n" 
                                                 };
const char *output_responses[NUMBER_OF_COMMANDS] = {"OK\r\n", "AT+CGMM\r\nIRIDIUM 9600 Family Transceiver\r\nOK\r\n",
                                      "AT+CGMR\r\nCall Processor Version: TD10003\r\nDSP Version: 0x002b\r\nASIC Version:32.1\r\nNVM Version:TAL0007\r\nBOOT Version:Unknown\r\n",
                                    "OK\r\n","+CPIN:SIM PIN2\r\n","OK\r\n","+CSQ:4\r\nOK\r\n","+SBDREG:2,32\r\nOK\r\n","AT+SBDD0\r\n0\r\nOK\r\n","Not implemented\r\n",
                                      "+SBDMTA:1\r\nOK\r\n","Not implemented\r\n","+SBDREG:2,32\r\nOK\r\n","+SBDREG:2\r\nOK\r\n","OK\r\n",
                                    "OK\r\n","OK\r\n","OK\r\n"};


const char *error_commands[1] =  {"Invalid Command Entered : ERROR\r\n"};

int init_flag = FALSE;

//char emu_buf[32];
int res;	
int fd;        

BYTE* tx_data; 
int payload_length = 0;


int Iridium_wakeup(void);
void Iridium_comm(Properties* pProps,char serial_buf[]);

BYTE* packetize_data(Properties* pProps,BYTE* DataPkt,int payload_length);



int main(int argc, char *argv[])
{
 char emu_buf[32];

 Properties* pProperties;

 pProperties = (Properties*) malloc(sizeof(Properties));
 
 pProperties = Properties_load(pProperties, PROPERTIES_FILE); /* Load Properties from  Configuation file */
 

 /* open the device to be non-blocking (read will return immediatly) */ 
 fd = open_port(pProperties->em_SatellitePort, B19200, 1, 0);//"/dev/ttyS0"

 Get_Host_Addrs(pProperties->em_Domain_Server,PORT_NUMBER);
 
 printf("Ready to receive commands from COM port\n");
 fflush(stdout);

 memset(emu_buf, 0, sizeof(emu_buf));

 while(STOP==FALSE)  {
      sleep(1);   //To capture the entered command, small delay is introduced
                  //after receiving SIGIO, wait_flag = FALSE, input is available and can be read 
      if(wait_flag==FALSE) { 
        res = read(fd,emu_buf,25);//max size of the buffer = 25
        emu_buf[res]=0;
        printf("%s", emu_buf);   
        if (res==1) STOP=TRUE; // stop loop if only a CR was input //    
        wait_flag = TRUE;      // wait for new input //
                                                
        //write(fd,"\033[2J",5); 
        usleep(50);//Wait to print the  the data to terminal
    
        Iridium_comm(pProperties,emu_buf);                 
      }
 } 
  return 1;
}
/***************************************************************************
* signal handler. sets wait_flag to FALSE, to indicate above loop that     *
* characters have been received.                                           *
***************************************************************************/

void signal_handler_IO (int status)
{
//printf("received SIGIO signal.\n"); //uncomment it for debugging purpose
 wait_flag = FALSE;
}


int open_port(const char* szDevice, int baudRate, int nMinCharacters, int nTimeCharacters)
{
	struct termios newtio; 
        struct sigaction saio = {0};       
	
	fd = open(szDevice, O_RDWR | O_NOCTTY | O_NONBLOCK); //O_NDELAY
     
	if (fd < 0) {
		printf("failed to open %s\n", szDevice);
		return fd;
	}
        else {
               printf("COM port '%s' opened successfully\n",szDevice); 
        }
        
       
        saio.sa_handler = signal_handler_IO;

//	saio.sa_mask = 0;
//	saio.sa_flags = 0;
	saio.sa_restorer = NULL;
	sigaction(SIGIO,&saio,NULL);
	  
	/* allow the process to receive SIGIO */
	fcntl(fd, F_SETOWN, getpid());
	/* Make the file descriptor asynchronous (the manual page says only 
	   O_APPEND and O_NONBLOCK, will work with F_SETFL...) */
	fcntl(fd, F_SETFL, FASYNC);
 

        tcgetattr(fd, &newtio); //get port newtio

        newtio.c_cflag |= (CLOCAL | CREAD); //enable receiver and set local modechar* packetize_data(void)


        newtio.c_cflag &= ~(PARENB | PARODD);
        newtio.c_cflag &= ~CSTOPB;
        newtio.c_cflag &= ~CSIZE;
        newtio.c_cflag |= CS8;
        newtio.c_cflag &= ~CRTSCTS; //disable hardware flow control

        newtio.c_iflag |= IGNPAR | ICRNL;//added recently 
        newtio.c_oflag |= 0; //Output newtio
        newtio.c_lflag |= ICANON;//added recently
        newtio.c_lflag &= ~(ECHO | ECHOE); //set raw input

        newtio.c_cflag |= IXOFF; //disable software flow control
	
        newtio.c_cc[VMIN] = nMinCharacters;// Minium characters to read(why it is zero??)
	newtio.c_cc[VTIME] = nTimeCharacters;//DATA SAVED\r\n
     
        cfsetispeed(&newtio, B19200); //set input baud rate
        cfsetospeed(&newtio, B19200); //set output baud rate

        tcsetattr(fd, TCSANOW, &newtio); //set new port newtio 
	tcflush(fd, TCIOFLUSH);

	return fd;
}


int select_string(char s[]) 
{
 int m=0;
 for(m=0; m < NUMBER_OF_COMMANDS; m++) {
   if(m==15 && strncmp(s,input_commands[15],9) == 0) { //SDBWB       		 
	 return m;        
     }
   else if(strcmp(s,input_commands[m]) == 0){
          return m;
       }
 }
 return -1; 
}
 
void Iridium_comm(Properties* pProps,char serial_buf[]){

   static BOOL Notes_Status = 0;
   BOOL tx_status = 1;
  // BYTE* tx_data; 
   BYTE* pData;
   int pCount,buf_cnt = 0;
   
   char temp_buffer[2];

   /* switch case implementation */
   
    switch(select_string(serial_buf)) {
  
          case 0  : write(fd,output_responses[0],strlen(output_responses[0]));  
                    break;
          case 1  : write(fd,output_responses[1],strlen(output_responses[1]));  
                    break;
          case 2  : write(fd,output_responses[2],strlen(output_responses[2]));  
                    break;
          case 3  : write(fd,output_responses[3],strlen(output_responses[3]));  
                    break;
          case 4  : write(fd,output_responses[4],strlen(output_responses[4]));  
                    break;
          case 5  : write(fd,output_responses[5],strlen(output_responses[5]));  
                    break;
          case 6  : write(fd,output_responses[6],strlen(output_responses[6]));  
                    break;
          case 7  : write(fd,output_responses[7],strlen(output_responses[7]));  
                    break;
          case 8  : write(fd,output_responses[8],strlen(output_responses[8]));  
                    break;
          case 9  : write(fd,output_responses[9],strlen(output_responses[9]));  
                    break;
          case 10 : write(fd,output_responses[10],strlen(output_responses[10]));  
                    break;
          case 11 : write(fd,output_responses[11],strlen(output_responses[11]));  
                    break;
          case 12 : write(fd,output_responses[12],strlen(output_responses[12]));  
                    break;
          case 13 : write(fd,output_responses[13],strlen(output_responses[13]));  
                    break;            
          case 14 : write(fd,"AT+CGSN\r\n",strlen("AT+CGSN\r\n"));   //CGSN
                    write(fd,pProps->em_IMEI,strlen(pProps->em_IMEI));
                    write(fd,"\r\n",strlen("\r\n"));
                    write(fd,output_responses[14],strlen(output_responses[14])); 
                    break;
          case 15 : for(pCount=9;serial_buf[pCount]!='\n';pCount++,buf_cnt++){//SBDWB
			 temp_buffer[buf_cnt] = (serial_buf[pCount]); //9 --> position where the data length starts
		      }
		      payload_length = (atoi(temp_buffer)); 
		      printf("%d\n",payload_length);
		      fflush(stdout);
                      if(0!= payload_length) {      
  		         tx_data = packetize_data(pProps,pData,payload_length);                        
		         write(fd,"DATA SAVED\r\n",strlen("DATA SAVED\r\n")); //added      
                         write(fd,output_responses[15],strlen(output_responses[15]));//expected response  
		         Notes_Status = 1;                      
                      }
                      else {
                          write(fd,"ENTER PAYLOAD DATA\r\n",strlen("ENTER PAYLOAD DATA\r\n")); 
                          write(fd,error_commands[0],strlen(error_commands[0])); 
                          printf("%s",error_commands[0]);  
                      }                             
                      break;	    
          case 16 : if(1 == Notes_Status){ //SBDIX
		       tx_status = Modem_Send_Data_to_Portal(tx_data,header_length + payload_length);  //presently payload data is hardcoded because we dont know how much memory is needed
		       memset(tx_data,0,header_length + payload_length);
		       if(1 == tx_status)
		       { 
			 printf("NOTES SENDING FAILED");fflush(stdout);
			 write(fd,"NOTES SENDING FAILED\r\n",strlen("NOTES SENDING FAILED\r\n"));                  
		       }
		       else 
		       {
			 printf("NOTES SENT\n");fflush(stdout);
			 write(fd,"NOTE SENT\r\n",strlen("NOTE SENT\r\n"));                  
			 write(fd,output_responses[16],strlen(output_responses[16]));  
		       } 
		     }	 
		     else{
		      write(fd,"NOTES NOT AVAILABLE\r\n",strlen("NOTES NOT AVAILABLE\r\n"));       
		    }				   
                    break;
          case 17 : if(1==Notes_Status){
		       memset((BYTE*)tx_data,0,sizeof(pData));
		       write(fd,"NOTES FLUSHED SUCCESFULLY\r\n",strlen("NOTES FLUSHED SUCCESFULLY\r\n"));
                       write(fd,output_responses[17],strlen(output_responses[17]));  
		       Notes_Status = 0;       
		     }
		     else {
		      write(fd,"NOTES NOT AVAILABLE\r\n",strlen("NOTES NOT AVAILABLE\r\n"));   
		     }
		    break;
          
         default : write(fd,error_commands[0],strlen(error_commands[0]));  
                   printf("%s",error_commands[0]);  
                   break;   
 
    }
 }
   

BYTE* packetize_data(Properties* pProps,BYTE* DataPkt,int payload_len)
{
 time_t now_in_secs;
 time(&now_in_secs);

 BYTE header[header_length];
 BYTE* Payload_data;

 Payload_data = (BYTE *)malloc(payload_len);
 DataPkt = (BYTE*)malloc(header_length + payload_len); 


 if(pProps != NULL) {
	
      	memset(header, 0, sizeof(header));

	header[0] = (BYTE) 0x01;  //protocol revison number
	
	header[1] = (BYTE) (((payload_len + header_length) & 0xFF00) >> 8);  //Message Length
	header[2] = (BYTE)((payload_len + header_length) & 0xff);  //Message Length
	
	header[3] = (BYTE) 1;  //IEI
	
	header[4] = (BYTE) ((28 & 0xFF00) >> 8);  //Header Length
	header[5] = (BYTE) (28 & 0xff);  //Header Length
	
	header[6] = (BYTE) ((1234 & 0xff000000) >> 24);  //cdr   
	header[7] = (BYTE) ((1234 & 0x00ff0000) >> 16);  //cdr
	header[8] = (BYTE) ((1234 & 0x0000ff00) >> 8);  //cdr
	header[9] = (BYTE) ((1234 & 0x000000ff));  //cdr
	
	//IMEI goes here 10-24       
	header[10] = pProps->em_IMEI[0];
	header[11] = pProps->em_IMEI[1];
	header[12] = pProps->em_IMEI[2];
	header[13] = pProps->em_IMEI[3];
	header[14] = pProps->em_IMEI[4];
	header[15] = pProps->em_IMEI[5];
	header[16] = pProps->em_IMEI[6];
	header[17] = pProps->em_IMEI[7];
	header[18] = pProps->em_IMEI[8];
	header[19] = pProps->em_IMEI[9];
	header[20] = pProps->em_IMEI[10];
	header[21] = pProps->em_IMEI[11];
	header[22] = pProps->em_IMEI[12];
	header[23] = pProps->em_IMEI[13];
	header[24] = pProps->em_IMEI[14];

	header[25] = (BYTE) 1;  //status
	
	header[26] = (BYTE) ((2 & 0xFF00) >> 8);  //momsn
	header[27] = (BYTE) (2 & 0xff);  //momsn
	header[28] = (BYTE) ((3 & 0xFF00) >> 8);  //mtmsn
	header[29] = (BYTE) (3 & 0xff);  //mtmsn
	
	header[30] = (BYTE) ((9999 & 0xff000000)>>24);  //session time   
	header[31] = (BYTE) ((9999 & 0x00ff0000)>>16);  //session time
	header[32] = (BYTE) ((9999 & 0x0000ff00)>>8);  //session time
	header[33] = (BYTE) ((9999 & 0x000000ff));  //session time
	header[34] = (BYTE) 1;  //payload iei
        header[35] = (BYTE) ((payload_len & 0xFF00) >> 8);  //Message Length
	header[36] = (BYTE) (payload_len & 0xff);  //Message Length
	
	 memcpy(&DataPkt[0], header, sizeof(header));
			
	 Payload_data[0] = 0x2E;
	 Payload_data[1] = 0x06;
	 Payload_data[2] = 0x03;
	 Payload_data[3] = (now_in_secs & 0xFF000000) >> 24;
	 Payload_data[4] = (now_in_secs & 0x00FF0000) >> 16;
	 Payload_data[5] = (now_in_secs & 0x0000FF00) >> 8;
	 Payload_data[6] = (now_in_secs & 0x000000FF);
	 Payload_data[7] = 0xF2;
	 Payload_data[8] = 0x5F;
	 Payload_data[9] = 0x46;
	 Payload_data[10] = 0x1A;
	 Payload_data[11] = 0x0C; //48
	 Payload_data[12] = 0xF1;
	 Payload_data[13] = 0xB0;
	 Payload_data[14] = 0x5C;
	 Payload_data[15] = 0x63;
	 Payload_data[16] = 0x36;
	 Payload_data[17] = 0x00;
	 Payload_data[18] = 0x46;
	 Payload_data[19] = 0x00;
	 Payload_data[20] = 0x46;
	 Payload_data[21] = 0xA1;
	 Payload_data[22] = 0x89;
	 Payload_data[23] = 0x00;
	 Payload_data[24] = 0x00;
	 Payload_data[25] = 0x00;
	 Payload_data[26] = 0x00;
	 Payload_data[27] = 0x01;
	 Payload_data[28] = 0x36;
	 Payload_data[29] = 0x00;
	 Payload_data[30] = 0x01;
	 Payload_data[31] = 0x27;
	 Payload_data[32] = 0xD4;
	 Payload_data[33] = 0x40;
	 Payload_data[34] = 0x08;
	 Payload_data[35] = 0x03;
	 Payload_data[36] = 0x93;
	 Payload_data[37] = 0x60;
	 Payload_data[38] = 0x0F;
	 Payload_data[39] = 0x00;
	 Payload_data[40] = 0x80;
	 Payload_data[41] = 0x59;
	 Payload_data[42] = 0x00;
	 Payload_data[43] = 0x00;
	 Payload_data[44] = 0x00;
	 Payload_data[45] = 0x01;

         memcpy(&DataPkt[header_length], Payload_data, payload_len);
         
         //free(pProps);
                  
      	 return (DataPkt);

 }
}


























 
