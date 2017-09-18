#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <map>
#include <string>
#include <d3dx9.h>


class CTexture
{
public:
	LPDIRECT3DTEXTURE9 pTexture;
	unsigned uRefCount;
	CTexture(LPDIRECT3DTEXTURE9 pTex)
	{
		uRefCount=1;
		pTexture=pTex;
	}
	~CTexture()
	{
		pTexture->Release();
	}
};

class CTextureCatalog
{
private:
	LPDIRECT3DDEVICE9	m_pD3DDevice;
	std::map<std::string,CTexture*> m_mapTextures;
	char name[128];
public:
	CTextureCatalog(LPDIRECT3DDEVICE9 pD3DDev);
	~CTextureCatalog();
	LPDIRECT3DTEXTURE9 GetTexture(char* pName);
	void ReleaseTexture(char* pName);
};

extern CTextureCatalog* g_pTextureCatalog;


#endif // _TEXTURE_H_