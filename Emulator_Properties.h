
#ifndef EMULATOR_PROPERTIES_H
#define EMULATOR_PROPERTIES_H

#define PROPERTIES_TOTAL 3

#define IMEI_SIZE 16 //+1
#define PORT_SIZE 32 
#define DOMAIN_SERVER_SIZE 32

#define success 1
#define failure 0



typedef struct {     		
     char em_SatellitePort[PORT_SIZE];
     char em_IMEI[IMEI_SIZE];
     char em_Domain_Server[DOMAIN_SERVER_SIZE];
 } Properties;
	
extern Properties* Properties_load(Properties* pThis, const char* filename);
extern Properties* Properties_set(Properties* pThis, char* key, char* value);

//extern char* packetize_data(void);
	

#endif
