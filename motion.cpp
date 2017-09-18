#include "motion.h"
#include <stdio.h>

bool CMotion::Load(char* pFileName)
{
	FILE* f=NULL;
	fopen_s(&f,pFileName,"rb");
	if(!f) 
	{
		MessageBox(NULL,"Could not open DAM file.","DAM Error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);	
		return false;
	}
	DWORD dw;
	fread(&dw,sizeof(dw),1,f);		// read magic dw
	if(dw!=0x3faeb852)
	{
		MessageBox(NULL,"Invalid DAM file.","DAM Error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);	
		fclose(f);
		return false;
	}
	fread(&dw,sizeof(dw),1,f);		// not sure yet...
	fread(&dw,sizeof(dw),1,f);		// number of blocks
	m_nFrames = dw;
	m_pFrame = new MotionFrame[m_nFrames];
	fread(&dw,sizeof(dw),1,f);		// not sure yet...
	//now the frames
	for(int k=0; k<m_nFrames; k++)
	{
		fread(&dw,sizeof(dw),1,f);		// num of quats in frame
		if(dw!=0x12)
		{
			MessageBox(NULL,"Frame in DAM file does not define 18 joints.","DAM Error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);	
			fclose(f);
			return false;
		}
		fread(m_pFrame[k].m_vOffset,sizeof(float),3,f);
		for(int j=0; j<0x12; j++)
		{
			fread(m_pFrame[k].m_pRotQuats[j],sizeof(float),4,f);
		}
	}
	fread(&dw,sizeof(dw),1,f);		// name length
	fread(name,sizeof(char),dw,f);	// name
	fread(m_pTail,sizeof(float),6,f);
	fclose(f);
	return true;
}