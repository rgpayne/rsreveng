#ifndef _SKINPATCH_H_
#define _SKINPATCH_H_

#include <d3dx9.h>
#include "texture.h"

struct SSkinPoint
{
	D3DXVECTOR2 s;			// texture coordinates
	float dud[4];			// ???
	D3DXVECTOR3 normal;		// probably
};

struct SSkinTriangle
{
	int posidx[3];
	int skinidx[3];
};

struct CSkinPatch
{
	friend class CCharacter;
private:
	char				filename[128];
	int					m_nPoints;
	int					m_nTriangles;
	SSkinPoint*			m_pPoint;
	SSkinTriangle*		m_pTriangle;
	LPDIRECT3DTEXTURE9	m_pTexture;
	bool				m_bAlpha;
public:
	CSkinPatch() 
	{
		m_nPoints		= 0;
		m_pPoint		= NULL;
		m_nTriangles	= 0;
		m_pTriangle		= NULL;
		m_pTexture		= NULL;
		m_bAlpha		= false;
	}
	~CSkinPatch()
	{
		if(m_pPoint)	delete[] m_pPoint;
		if(m_pTriangle) delete[] m_pTriangle;
		if(m_pTexture)	g_pTextureCatalog->ReleaseTexture(filename);
	}
	bool IsAlpha() { return m_bAlpha; }
	char* GetFileName() { return filename; }
	void InitTexture()
	{
		m_pTexture = g_pTextureCatalog->GetTexture(filename);
	}
	void SetNumPoints(int num)
	{ 
		m_nPoints = num;
		m_pPoint = new SSkinPoint[num];
	}
	void SetNumTriangles(int num)
	{ 
		m_nTriangles = num;
		m_pTriangle = new SSkinTriangle[num];
	}
};


#endif // _SKINPATCH_H_
