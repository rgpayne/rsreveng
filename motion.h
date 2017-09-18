#ifndef _MOTION_H_
#define _MOTION_H_

#include <d3dx9.h>
#include <stdio.h>


struct MotionFrame
{
	D3DXVECTOR3		m_vOffset;
	D3DXQUATERNION	m_pRotQuats[18];
};
	
class CMotion
{
	friend class CCharacter;
private:
	int				m_nFrames;
	MotionFrame*	m_pFrame;
	char			name[128];
	float			m_pTail[6]; // not sure yet...
public:
	CMotion()
	{
		m_nFrames=0;
		m_pFrame=NULL;
	}
	~CMotion()
	{
		if(m_pFrame) delete[] m_pFrame;
	}
	inline int GetFrames() { return m_nFrames; }
	bool Load(char* pFileName);
};


#endif // _MOTION_H_