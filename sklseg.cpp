#include "sklseg.h"
#include <iostream>
#include <sstream>
#include <fstream>


CSklSeg::CSklSeg(char* pName, D3DXVECTOR3 &vOffset)
{
	strncpy_s(m_pName,16,pName,max(strlen(pName),16));
	m_vOffset		= vOffset;
	m_pParent		= 0;
	m_nChildren		= 0;
	m_pChild		= NULL;
	m_nVertices		= 0;
	m_pVertexIdx	= NULL;
	D3DXMatrixIdentity(&m_mRotation);
}

CSklSeg::~CSklSeg()
{
	if(m_pChild) 
	{
		for(int i=0; i<m_nChildren; i++)
			delete m_pChild[i];
		delete[] m_pChild;
	}
	if(m_pVertexIdx) 
		delete[] m_pVertexIdx;
}

void CSklSeg::SetNumOfVertices(int nVertices)
{
	m_nVertices = nVertices;
	m_pVertexIdx = new WORD[m_nVertices];
}

void CSklSeg::SetNumOfChildren(int nChildren)
{
	m_nChildren = nChildren;
	m_pChild	= new CSklSeg*[m_nChildren];
	for(int i=0; i<m_nChildren; i++)
		m_pChild[i]=NULL;
}

bool CSklSeg::AddChild(CSklSeg* pChild)
{
	for(int i=0; i<m_nChildren; i++)
	{
		if(m_pChild[i]==NULL)
		{
			m_pChild[i]=pChild;
			pChild->m_pParent=this;
			return true;
		}
	}
	return false;
}

bool CSklSeg::IsName(char* pName)
{
	if(strstr(m_pName,pName)) return true;
	return false;
}

void CSklSeg::TransformRec(D3DXVECTOR3* pSource, D3DXVECTOR3* pTarget, D3DXMATRIX &accrot, D3DXVECTOR3 &accpos)
{
	D3DXVECTOR3 vTraOffset;
	D3DXVec3TransformCoord(&vTraOffset,&m_vOffset,&accrot);
	pos = accpos + vTraOffset;
	D3DXMatrixMultiply(&rot,&m_mRotation,&accrot);
	for(int i=0; i<m_nVertices; i++)
	{
		D3DXVec3TransformCoord(	&(pTarget[m_pVertexIdx[i]]),
			&(pSource[m_pVertexIdx[i]]),
			&rot );
		pTarget[m_pVertexIdx[i]]+=pos;	
	}
	for(int i=0; i<m_nChildren; i++)
		m_pChild[i]->TransformRec(pSource,pTarget,rot,pos);
}

void CSklSeg::bones() {
	std::ofstream file("bones.txt", std::ios::trunc);
	std::stringstream bones;

	for (int i = 0; i < m_nVertices; i++) {
		//bones << m_
	}

}