
#ifndef CONFIGFILE_H
#define CONFIGFILE_H


#define BYTE unsigned char
#define UINT unsigned int
#define BOOL int 


#define ONE 1
#define TWO 2
#define THREE 3

typedef struct {
	unsigned char* m_pData;
	int m_nLength;
	int m_nAllocatedLength;
	int m_nPosition;
} MemFile;


typedef enum { begin, current, end } SeekType;

extern void CfgFile_InitEx(MemFile* pThis, int length);
extern MemFile* CfgFile_Load(const char* szFilename, BOOL bReadOnly, BOOL bForceChksumPass); 	 // Read Only,add any other arguments if require in future


#endif
