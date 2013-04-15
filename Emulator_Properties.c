#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "Emulator_Properties.h"
#include "Iridium_Emulator_Basic.h"
#include "Config_File_read.h" /*configuration file*/


const char PROPERTY_SATELLITE_PORT[] = "satellite_port ";
const char PROPERTY_SAT_MODEM_ID[] = "sat_imei ";
const char PROPERTY_DOMAIN_SERVER[] = "domain_server ";

Properties* Properties_load(Properties* pThis, const char* filename);
Properties* Properties_set(Properties* pThis, char* key, char* value);

Properties* Properties_load(Properties* pThis, const char* filename)
{

   char szLine[192];
   int linePos = 0;
   char c;
   int i;
   char* szKey;
   char* szValue;

   MemFile* cCfgFile;  
  
   cCfgFile = CfgFile_Load(filename,FALSE,FALSE);

   Properties* loc_Properties;

   if (cCfgFile != NULL) {		
		linePos = 0;
		for (i = 0; i < cCfgFile->m_nLength; i++) {
			c = cCfgFile->m_pData[i];
                      
			if (c != '\r') {
				if (c == '\n') {
					if (linePos > 0) {						
						szLine[linePos] = '\0';
						szValue = strchr(szLine, '=');
                                               
						if (szValue != NULL) {
							*szValue = '\0';
					 		szValue++;  
							while (*szValue == ' ') { 							       
		                                               szValue++;
							}                                                       
						}
						loc_Properties = Properties_set(pThis, szLine, szValue);                                            
				               	linePos = 0;
						}
				} else {
					if (linePos < sizeof(szLine)) {
						szLine[linePos] = c;
						linePos++;                                               
					}
				}//IF END    
			}
		}
  }
  return loc_Properties;
}



Properties* Properties_set(Properties* pThis, char* key, char* value)
{
  char i;
  static loop_cnt =  0;

  loop_cnt++; 
  switch (loop_cnt)
  {
       case ONE: 
                  if(strcmp(key,PROPERTY_SATELLITE_PORT) == 0){ 
		       memset(pThis->em_SatellitePort, 0, PORT_SIZE);
		       for (i = 0; value[i]!= '\0';i++) {
				pThis->em_SatellitePort[i] = value[i];				
		 	}
		    }
                    break;
      case TWO:                  
		 if(strcmp(key,PROPERTY_SAT_MODEM_ID) == 0) { 
		       memset(pThis->em_IMEI, 0, IMEI_SIZE);
                       for (i = 0; value[i]!= '\0'; i++) {
				pThis->em_IMEI[i] = value[i];
		 	}				     
		 }
                 break; 

      case THREE:                  
		  if(strcmp(key,PROPERTY_DOMAIN_SERVER) == 0) { 
		       memset(pThis->em_Domain_Server, 0,DOMAIN_SERVER_SIZE);
		       for (i = 0; value[i]!= '\0';i++) {
				pThis->em_Domain_Server[i] = value[i];
		 	}
		  }
                  break; 
  
      
    }
  
  return pThis; 
  
 }


