#ifndef _SKLSEG_H_
#define _SKLSEG_H_

#include <d3dx9.h>

/*
skeleton segments for building hierarchic skeleton

note: destructor destroys all chidren recursively, 
so only call the destructor of the root to clean up
*/

class CSklSeg
{
	friend class CCharacter;
public:
	char			m_pName[16];
	CSklSeg*		m_pParent;
	int				m_nChildren;
	CSklSeg**		m_pChild;
	D3DXVECTOR3		m_vOffset;
	D3DXMATRIX		m_mRotation;
	int				m_nVertices;
	WORD*			m_pVertexIdx;
	// working variables;
	D3DXVECTOR3		pos;
	D3DXMATRIX		rot;
//public:
	CSklSeg(char* pName, D3DXVECTOR3 &vOffset);
	~CSklSeg();
	void SetNumOfVertices(int nVertices);
	void SetNumOfChildren(int nChildren);
	bool AddChild(CSklSeg* pChild);
	bool IsName(char* pName);
	void TransformRec(D3DXVECTOR3* pSource, D3DXVECTOR3* pTarget, D3DXMATRIX &accrot, D3DXVECTOR3 &accpos);
	void bones();
};


#endif // _SKLSEG_H_