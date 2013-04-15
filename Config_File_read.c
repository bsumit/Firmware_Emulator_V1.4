#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <termios.h>
#include <sys/stat.h>


#include "Config_File_read.h"
#include "Iridium_Emulator_Basic.h"


void CfgFile_InitEx(MemFile* pThis, int length);
MemFile* CfgFile_Load(const char* szFilename, BOOL bReadOnly, BOOL bForceChksumPass); 	 // Read Only,add any other arguments if require in future

struct stat statbuf;

void CfgFile_InitEx(MemFile* pThis, int length)
{
 
  pThis->m_nLength = 0;
  pThis->m_nAllocatedLength = length;
  pThis->m_nPosition = 0;
  pThis->m_pData = (unsigned char*) malloc(length);
  memset(pThis->m_pData, 0, length);

}

MemFile* CfgFile_Load(const char* szFilename,BOOL bReadOnly, BOOL bForceChksumPass) 
{
   
  int fd = -1;
  int nFileSize = 0;
  int bytes_read = 0;
  MemFile* pMemFile = NULL;

  pMemFile = (MemFile*)malloc(sizeof(MemFile)); //removed +2
  
   if (stat(szFilename, &statbuf)==0){
    nFileSize = statbuf.st_size;
   }

   CfgFile_InitEx(pMemFile, nFileSize); 
   if(pMemFile != NULL)
   {
      fd = open(szFilename,O_RDONLY);
      if(fd>=0){
               bytes_read = read(fd, &pMemFile->m_pData[0], nFileSize);
               pMemFile->m_nLength = bytes_read;
               close(fd);             
               return pMemFile;
     } else {
	 printf("unable to open file");
         return NULL;
     }
  }	

}



