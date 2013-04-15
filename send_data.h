
#ifndef CONN_SERVER_H
#define CONN_SERVER_H

#include "Iridium_Emulator_Basic.h"
//#include "Emulator_Properties.h" /* Properties */
#include "Config_File_read.h" /*configuration file*/



#define PORT_NUMBER 7780
#define header_length 37 //size fixed for present


extern int Modem_Send_Data_to_Portal(BYTE* data_receive,int data_size);


#endif
