#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>

#include "texture.h"
#include "skinpatch.h"

class CSklSeg;
class CMotion;
struct Vertex
{
	D3DXVECTOR3 posit;
	D3DXVECTOR2 tex;
	//D3DCOLOR    color;

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZ | D3DFVF_TEX2//D3DFVF_DIFFUSE
	};
};
class CCharacter
{
public:
	CSklSeg*		m_pRootSeg;		// skeleton root node
	CSklSeg*		ord[18];		/* all skeleton segments ordered (as in SKL): 
										"Hips","Chest","Neck","Head",
										"LeftCollar","LeftUpArm","LeftLowArm","LeftHand",
										"RightCollar","RightUpArm","RightLowArm","RightHand",
										"LeftUpLeg","LeftLowLeg","LeftFoot",
										"RightUpLeg","RightLowLeg","RightFoot"
									*/
	int				m_nVertices;	// vertices
	D3DXVECTOR3*	m_pVertex;
	D3DXVECTOR3*	m_pTransformedVertex;
	char			name[128];
	int				m_nPatches;		// skin patches
	CSkinPatch*		m_pPatch; //tex coords?
	CMotion*		m_pMotion;

//public:
	CCharacter();
	~CCharacter();
	inline int GetNumPatches() { return m_nPatches; }
	inline bool IsPatchTransparent(int cPatch) { return m_pPatch[cPatch].m_bAlpha; }
	inline int GetNumOfTrianglesInPatch(int n) 
	{
		if(n<m_nPatches) return m_pPatch[n].m_nTriangles;
		return 0;
	}
	inline void GetTriangle(int cPatch, int cTriangle, D3DXVECTOR3* vertex, D3DXVECTOR2* tex)
	{
		for(int i=0; i<3; i++)
		{
			vertex[i] = m_pTransformedVertex[m_pPatch[cPatch].m_pTriangle[cTriangle].posidx[i]];
			tex[i] = m_pPatch[cPatch].m_pPoint[m_pPatch[cPatch].m_pTriangle[cTriangle].skinidx[i]].s;
		}
	}
	inline LPDIRECT3DTEXTURE9 GetTexture(int cPatch)
	{
		return m_pPatch[cPatch].m_pTexture;
	}
	bool SetMotion(CMotion* pMotion, float fTime);
	bool Load(char* pSKLfilename, char* pCRPfilename);
	char* getFileName(int cPatch) {
		return m_pPatch[cPatch].filename;
	}
	void to_OBJ();
	void bones();
};