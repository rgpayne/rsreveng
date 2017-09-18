#include "texture.h"


DWORD* LoadRSB(char* pFileName, int &width, int& height)
{
	FILE* f;
	fopen_s(&f,pFileName,"rb");
	if(!f) return NULL;
	int rbits,gbits,bbits,abits;
	fseek(f,4,SEEK_CUR);
	fread(&width, sizeof(int),1,f);
	fread(&height,sizeof(int),1,f);
	fread(&rbits, sizeof(int),1,f);
	fread(&gbits, sizeof(int),1,f);
	fread(&bbits, sizeof(int),1,f);
	fread(&abits, sizeof(int),1,f);
	DWORD* pBuffer = new DWORD[width*height];
	WORD w;
	WORD rmask, gmask, bmask, amask;
	int i;
	bmask=0;
	for(i=0; i<bbits; i++)
		bmask += 1<<(i);
	gmask=0;
	for(i=0; i<gbits; i++)
		gmask += 1<<(i+bbits);
	rmask=0;
	for(i=0; i<rbits; i++)
		rmask += 1<<(i+bbits+gbits);
	amask=0;
	for(i=0; i<abits; i++)
		amask += 1<<(i+bbits+gbits+rbits);
	DWORD* p = pBuffer;
	for(int y=0; y<height; y++)
	{
		for(int x=0; x<width; x++)
		{
			fread(&w,sizeof(WORD),1,f);
			if(abits==0)
			{
				*pBuffer =	0xff000000 + 
							(((w&rmask)>>(gbits+bbits))<<(16+8-rbits)) +
							(((w&gmask)>>(bbits))<<(8+8-gbits)) +
							(((w&bmask))<<(8-bbits)) ;
			}
			else
			{
				*pBuffer =	(((w&amask)>>(gbits+bbits+rbits))<<(24+8-abits)) + 
							(((w&rmask)>>(gbits+bbits))<<(16+8-rbits)) +
							(((w&gmask)>>(bbits))<<(8+8-gbits)) +
							(((w&bmask))<<(8-bbits)) ;
			}
			pBuffer++;
		}
	}
	fclose(f);
	return p;
}



CTextureCatalog::CTextureCatalog(LPDIRECT3DDEVICE9 pD3DDev)
{
	m_pD3DDevice=pD3DDev;
}

CTextureCatalog::~CTextureCatalog()
{
	std::map<std::string,CTexture*>::iterator it;
	while( !m_mapTextures.empty() )
	{
		it = m_mapTextures.begin();
		delete (it->second);
		m_mapTextures.erase(it);
	}
}

LPDIRECT3DTEXTURE9 CTextureCatalog::GetTexture(char* pName)
{
	std::string name = pName;
	std::map<std::string,CTexture*>::iterator it = m_mapTextures.find(name);
	if( it != m_mapTextures.end() )
	{
		it->second->uRefCount++;
		return it->second->pTexture;
	}
	else
	{
		LPDIRECT3DTEXTURE9 pTex=NULL;
		char pFileName[128];
		//attempt to load .bmp first
		sprintf_s(pFileName,128,"%s.bmp",pName);
		D3DXCreateTextureFromFile( m_pD3DDevice, pFileName, &pTex );
		if(!pTex)
		{
			//try to load .rsb
			sprintf_s(pFileName,128,"%s.rsb",pName);
			int w, h;
			DWORD* pBuffer = LoadRSB(pFileName,w,h);
			if(!pBuffer) return NULL;
			if(FAILED(D3DXCreateTexture(m_pD3DDevice,w,h,1,0,D3DFMT_A8R8G8B8, D3DPOOL_MANAGED ,&pTex)))
			{
				delete[] pBuffer;
				return NULL;
			}
			D3DLOCKED_RECT LockedBox;
			if( FAILED( pTex->LockRect( 0, &LockedBox, 0, 0 ) ) )
			{
				delete[] pBuffer;
				return NULL;
			}
			DWORD* p = pBuffer;
			for (int j=0; j<h; j++) 
			{
				memcpy(LockedBox.pBits,p,sizeof(DWORD)*w);
				LockedBox.pBits = (BYTE*)LockedBox.pBits + LockedBox.Pitch;
				p += w;
			}
			pTex->UnlockRect( 0 );
			delete[] pBuffer;
		}
		m_mapTextures[name] = new CTexture(pTex);
		return pTex;
	}
}

void CTextureCatalog::ReleaseTexture(char* pName)
{
	std::string name = pName;
	std::map<std::string,CTexture*>::iterator it = m_mapTextures.find(name);
	if( it != m_mapTextures.end() )
	{
		it->second->uRefCount--;
		if(it->second->uRefCount==0)
		{
			delete (it->second);
			m_mapTextures.erase(it);
		}
	}
}