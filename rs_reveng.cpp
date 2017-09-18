//-----------------------------------------------------------------------------
//           Name: dx9_point_sprites.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to use hardware accelerated 
//                 point sprites with Direct3D.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "resource.h"
#include <stdio.h>

#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include "character.h"
#include "motion.h"


//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND                    g_hWnd          = NULL;
LPDIRECT3D9             g_pD3D          = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice    = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer = NULL;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

//-----------------------------------------------------------------------------
// PROTOYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void		init(void);
void		shutDown(void);
void		render(void);
void		renderPatches(void);
void		NextAnim();
float		getRandomMinMax(float fMin, float fMax);
void		to_OBJ();
D3DXVECTOR3	getRandomVector(void);

CCharacter			chr;
CMotion*			g_pMotion = NULL;
CTextureCatalog*	g_pTextureCatalog;
float				g_fTime	  = 0.0f;


// Helper function to stuff a FLOAT into a DWORD argument
inline DWORD FtoDW( FLOAT f ) { return *((DWORD*)&f); }

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR     lpCmdLine,
					int       nCmdShow )
{
	WNDCLASSEX winClass; 
	MSG        uMsg;

    memset(&uMsg,0,sizeof(uMsg));
    
	winClass.lpszClassName = "RS_REVENG_WINDOWS_CLASS";
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstance;
	winClass.hIcon	       = NULL;
	winClass.hIconSm	   = NULL;
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;

	if( !RegisterClassEx(&winClass) )
		return E_FAIL;

	g_hWnd = CreateWindowEx( NULL, "RS_REVENG_WINDOWS_CLASS", 
                             "RS Viewer",
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					         0, 0, 640, 480, NULL, NULL, hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();
    
	g_pTextureCatalog = new CTextureCatalog(g_pd3dDevice);
	
	chr.Load("test.skl","test.crp");	


	NextAnim();
	
	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{ 
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
        else
		    render();
	}

	shutDown();

    UnregisterClass( "MY_WINDOWS_CLASS", winClass.hInstance );

	return uMsg.wParam;
}

//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   hWnd, 
							 UINT   msg, 
							 WPARAM wParam, 
							 LPARAM lParam )
{
	static POINT ptLastMousePosit;
	static POINT ptCurrentMousePosit;
	static bool bLMousing;
	static bool bRMousing;
	
	
    switch( msg )
	{
        case WM_KEYDOWN:
		{
			switch( wParam )
			{
				case VK_ESCAPE:
					g_fSpinX = g_fSpinY = 0.0f;
					g_fTime = 0.0f;
					break;
				case VK_SPACE:
					g_fTime+=1.0f;
					if(g_fTime>(float)(g_pMotion->GetFrames()-1)) g_fTime=0.0f;
					chr.SetMotion(g_pMotion,g_fTime);
					break;
				case VK_RETURN:
					NextAnim();
					break;

			}
		}
        break;

		case WM_LBUTTONDOWN:
		{
			ptLastMousePosit.x = ptCurrentMousePosit.x = LOWORD (lParam);
            ptLastMousePosit.y = ptCurrentMousePosit.y = HIWORD (lParam);
			bLMousing = true;
		}
		break;

		case WM_LBUTTONUP:
		{
			bLMousing = false;
		}
		break;

		case WM_RBUTTONDOWN:
		{
			ptLastMousePosit.x = ptCurrentMousePosit.x = LOWORD (lParam);
            ptLastMousePosit.y = ptCurrentMousePosit.y = HIWORD (lParam);
			bRMousing = true;
		}
		break;

		case WM_RBUTTONUP:
		{
			bRMousing = false;
		}
		break;

		case WM_MOUSEMOVE:
		{
			ptCurrentMousePosit.x = LOWORD (lParam);
			ptCurrentMousePosit.y = HIWORD (lParam);

			if( bLMousing )
			{
				g_fSpinX -= (ptCurrentMousePosit.x - ptLastMousePosit.x);
				if ( fabs(g_fSpinX) >= 360.0f ) g_fSpinX = 0.0f; 
			}

			if( bRMousing )
			{
				g_fSpinY -= (ptCurrentMousePosit.y - ptLastMousePosit.y);
				if( g_fSpinY >= 89.0f  ) g_fSpinY =  89.0f;
				if( g_fSpinY <= -89.0f ) g_fSpinY = -89.0f;
			}
			
			ptLastMousePosit.x = ptCurrentMousePosit.x;
            ptLastMousePosit.y = ptCurrentMousePosit.y;
		}
		break;

		case WM_CLOSE:
		{
			PostQuitMessage(0);	
		}
		
        case WM_DESTROY:
		{
            PostQuitMessage(0);
		}
        break;

		default:
		{
			return DefWindowProc( hWnd, msg, wParam, lParam );
		}
		break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Name: InitDirect3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
void init( void )
{
	g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

    D3DDISPLAYMODE d3ddm;

    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );

    d3dpp.Windowed               = TRUE;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat       = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd,
                          D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice );
	
	g_pd3dDevice->CreateVertexBuffer( 128*2048 * sizeof(Vertex), 
                                      D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_POINTS, 
									  Vertex::FVF_Flags, D3DPOOL_DEFAULT, 
									  &g_pVertexBuffer, NULL );

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING,  FALSE );

}


//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
void shutDown( void )
{
	if( g_pVertexBuffer != NULL )        
        g_pVertexBuffer->Release();

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();

	if( g_pMotion ) 
		delete g_pMotion;

}



//-----------------------------------------------------------------------------
// Name: getRandomMinMax()
// Desc: Gets a random number between min/max boundaries
//-----------------------------------------------------------------------------
float getRandomMinMax( float fMin, float fMax )
{
    float fRandNum = (float)rand () / RAND_MAX;
    return fMin + (fMax - fMin) * fRandNum;
}

//-----------------------------------------------------------------------------
// Name: getRandomVector()
// Desc: Generates a random vector where X,Y, and Z components are between
//       -1.0 and 1.0
//-----------------------------------------------------------------------------
D3DXVECTOR3 getRandomVector( void )
{
	D3DXVECTOR3 vVector;

    // Pick a random Z between -1.0f and 1.0f.
    vVector.z = getRandomMinMax( -1.0f, 1.0f );
    
    // Get radius of this circle
    float radius = (float)sqrt(1 - vVector.z * vVector.z);
    
    // Pick a random point on a circle.
    float t = getRandomMinMax( -D3DX_PI, D3DX_PI );

    // Compute matching X and Y for our Z.
    vVector.x = (float)cosf(t) * radius;
    vVector.y = (float)sinf(t) * radius;

	return vVector;
}

int nTrianglesInPatch[128];
int nSolids[128];
int nTransparents[128];

void renderPatches()
{
	g_pd3dDevice->BeginScene();
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE  );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,	D3DBLEND_SRCALPHA );
	g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,	D3DBLEND_INVSRCALPHA );
	
	Vertex *pPointVertices;

	// set rendering order (to render transparents in the end)
	int solidx=0;
	int traidx=0;
	int i,j,k;
	for( j = 0; j<chr.GetNumPatches(); j++ )
	{
		if(chr.IsPatchTransparent(j))
			nTransparents[traidx++]=j;
		else
			nSolids[solidx++]=j;
	}
	
	g_pVertexBuffer->Lock( 0, 2048 * sizeof(Vertex),
		                   (void**)&pPointVertices, D3DLOCK_DISCARD );

	float minY = 1000.0f;
	float maxY = -1000.0f;
	int nPatch=0;


	//std::stringstream tex, vert, vertIndices;
	int ct = 1;


	for( k = 0; k<solidx; k++ )
	{
		j=nSolids[k];
		if(chr.GetTexture(j)==NULL) 
		{
			nTrianglesInPatch[nPatch] = 0;
			continue;
		}
		nTrianglesInPatch[nPatch] = chr.GetNumOfTrianglesInPatch(j);
		//vertIndices << "usemtl " << chr.getFileName(j) << std::endl; //chr.getFileName(j) << std::endl;
		for( i = 0; i < nTrianglesInPatch[nPatch]; i++)
		{
			D3DXVECTOR3 v[3];
			D3DXVECTOR2	t[3];
			chr.GetTriangle(j,i,v,t);
			//minY = min( minY , min( v[0].y, min (v[1].y, v[2].y) ) );
			//maxY = max( maxY , max( v[0].y, max (v[1].y, v[2].y) ) );

			pPointVertices->posit	= v[0];
			//vert << "v " << v[0].x << " " << v[0].y << " " << v[0].z << std::endl;
			pPointVertices->tex		= t[0];
			//tex << "vt " << t[0].x << " " << std::abs(t[0].y) << std::endl;
			pPointVertices++;

			pPointVertices->posit	= v[1];
			//vert << "v " << v[1].x << " " << v[1].y << " " << v[1].z << std::endl;
			pPointVertices->tex		= t[1];
			//tex << "vt " << t[1].x << " " << std::abs(t[1].y) << std::endl;
			pPointVertices++;

			pPointVertices->posit	= v[2];
			//vert << "v " << v[2].x << " " << v[2].y << " " << v[2].z << std::endl;
			pPointVertices->tex		= t[2];
			//tex << "vt " << t[2].x << " " << std::abs(t[2].y) << std::endl;
			pPointVertices++;
			/*
			vertIndices << "f "
				<< ct << "/" << ct << " "
				<< ct + 1 << "/" << ct + 1 << " "
				<< ct + 2 << "/" << ct + 2 << std::endl;
				ct+=3;*/


		}
		nPatch++;
	}

	for( k = 0; k<traidx; k++ )
	{
		j=nTransparents[k];
		if(chr.GetTexture(j)==NULL) 
		{
			nTrianglesInPatch[nPatch++] = 0;
			continue;
		}
		nTrianglesInPatch[nPatch] = chr.GetNumOfTrianglesInPatch(j);
		//vertIndices << "usemtl " << chr.getFileName(j) << std::endl; //chr.getFileName(j) << std::endl;
		for( i = 0; i < nTrianglesInPatch[nPatch]; i++)
		{
			D3DXVECTOR3 v[3];
			D3DXVECTOR2	t[3];
			chr.GetTriangle(j,i,v,t);

			pPointVertices->posit = v[0];
			//vert << "v " << v[0].x << " " << v[0].y << " " << v[0].z << std::endl;
			pPointVertices->tex = t[0];
			//tex << "vt " << t[0].x << " " << std::abs(t[0].y) << std::endl;
			pPointVertices++;

			pPointVertices->posit = v[1];
			//vert << "v " << v[1].x << " " << v[1].y << " " << v[1].z << std::endl;
			pPointVertices->tex = t[1];
			//tex << "vt " << t[1].x << " " << std::abs(t[1].y) << std::endl;
			pPointVertices++;

			pPointVertices->posit = v[2];
			//vert << "v " << v[2].x << " " << v[2].y << " " << v[2].z << std::endl;
			pPointVertices->tex = t[2];
			//tex << "vt " << t[2].x << " " << std::abs(t[2].y) << std::endl;
			pPointVertices++;

			/*vertIndices << "f " 
				<< ct << "/" << ct << " " 
				<< ct + 1 << "/" << ct+1 << " " 
				<< ct + 2 << "/" << ct+2 << std::endl;
			ct += 3;*/
		}
		nPatch++;
    }
	/*
	//building a .obj
	std::ofstream outFile("testdude.obj", std::ios::trunc);
	outFile << "mtllib testmtl.mtl" << std::endl
		<< vert.str() << std::endl << std::endl //v
		<< tex.str() << std::endl << std::endl  //vt
		<< vertIndices.str() << std::endl;	    //f
	outFile.close();
	
	//make the .mtl. important part is the texture path
	std::ofstream mtl("testmtl.mtl", std::ios::trunc);
	std::string i_n, i_n1;
	for (int i = 0; i < solidx+traidx; i++) {
		i_n = chr.getFileName(i);
		i_n1 = chr.getFileName(i + 1);
		if (i_n.std::string::compare(i_n1) == 0) continue; //exclude duplicates
		mtl << "newmtl " << chr.getFileName(i) << std::endl
			//<< "Ns 96.078431 0.000000 0.0000000" << std::endl
			//<< "Ka .500000 0.000000 0.000000" << std::endl
			//<< "Kd 0.500000 0.500000 0.500000" << std::endl
			//<< "Ks 0.500000 0.500000 0.500000" << std::endl
			//<< "Ni 1.00000" << std::endl
			//<< "d 1.000000" << std::endl
			//<< "illum 2" << std::endl
			<< "map_Kd " << i_n << ".bmp" << std::endl
			<< "map_Disp " << i_n << ".bmp" << std::endl
			<< "map_Ks " << i_n << ".bmp" << std::endl << std::endl;
	}
	

	mtl.close();
	*/
	g_pVertexBuffer->Unlock();

	g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, 0, sizeof(Vertex) );

	

    g_pd3dDevice->SetFVF( Vertex::FVF_Flags );

	int nVertex = 0;
	nPatch=0;
	for( k = 0; k<solidx; k++ )
	{
		j=nSolids[k];
		int triangles = nTrianglesInPatch[nPatch++];
		if(triangles==0) continue;
		g_pd3dDevice->SetTexture( 0, chr.GetTexture(j) );
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, nVertex, triangles );
		nVertex+=3*triangles;
	}
	for( k = 0; k<traidx; k++ )
	{
		j=nTransparents[k];
		int triangles = nTrianglesInPatch[nPatch++];
		if(triangles==0) continue;
		g_pd3dDevice->SetTexture( 0, chr.GetTexture(j) );
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, nVertex, triangles );
		nVertex+=3*triangles;
	}

	g_pd3dDevice->EndScene();
}


//-----------------------------------------------------------------------------
// Name: render()
// Desc: render the scene
//-----------------------------------------------------------------------------
void render( void )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.2f,0.3f,0.0f), 1.0f, 0 );

    
	// corrds in [cm] 
	// model
	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

	// view
	// a bit funky on this one: 
	// sets standard orbit (radius 300cm) 
	// then adds 170cm eyeheight to make it look better
	// and looks at belly height (120cm)
	D3DXMATRIX m;
	D3DXMatrixRotationYawPitchRoll( &m, 
		                            D3DXToRadian(-g_fSpinX), 
		                            D3DXToRadian(-g_fSpinY), 
		                            0.0f );
	D3DXVECTOR3 _eye( 0.0f, 0.0f, 300.0f ), eye;
	D3DXVec3TransformCoord( &eye, &_eye, &m );
	eye.y += 170.0f;
	D3DXVECTOR3 at(	 0.0f, 120.0f,   0.0f ); 	
	D3DXMATRIX matView;
    D3DXMatrixLookAtLH( &matView, &eye, 
		                          &at, 
		                          &D3DXVECTOR3(0.0f, 1.0f, 0.0f ) );
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );
    
	//projection
	D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 60.0f ), 1.0f, 1.0f, 600.0f );
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );


	g_pd3dDevice->SetTexture( 0, NULL );
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	
	renderPatches();

    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

char* LoadNextAnim()
{
	static HANDLE hHandle = INVALID_HANDLE_VALUE;
	static WIN32_FIND_DATA findFileData;
	if(hHandle==INVALID_HANDLE_VALUE)
	{
		hHandle = FindFirstFile("motion/*.dam",&findFileData);
		if(hHandle!=INVALID_HANDLE_VALUE)
		{
			return findFileData.cFileName;
		}
		else
		{
			return NULL;
		}
	} 
	else
	{
		if(FindNextFile(hHandle,&findFileData))
		{
			return findFileData.cFileName;
		}
		else
		{
			hHandle = INVALID_HANDLE_VALUE;
			return NULL;
		}
	}
}

void NextAnim()
{
	char pAnimFileName[256];
	char* pFileName;
	pFileName = LoadNextAnim();
	SetWindowText(g_hWnd,pFileName);
	if(pFileName)
	{
		sprintf_s(pAnimFileName,256,"motion\\%s",pFileName);
		if(g_pMotion) 
		{
			delete g_pMotion;
			g_pMotion = NULL;
		}
		g_pMotion = new CMotion();
		g_pMotion->Load(pAnimFileName);
		g_fTime = 0.0f;
		chr.SetMotion(g_pMotion,g_fTime);
	}
}

void to_OBJ() {

	Vertex *pPointVertices;

	// set rendering order (to render transparents in the end)
	int solidx = 0;
	int traidx = 0;
	int i, j, k;
	for (j = 0; j<chr.GetNumPatches(); j++)
	{
		if (chr.IsPatchTransparent(j))
			nTransparents[traidx++] = j;
		else
			nSolids[solidx++] = j;
	}

	g_pVertexBuffer->Lock(0, 2048 * sizeof(Vertex),
		(void**)&pPointVertices, D3DLOCK_DISCARD);

	float minY = 1000.0f;
	float maxY = -1000.0f;
	int nPatch = 0;


	std::stringstream tex, vert, vertIndices;
	int ct = 1;


	for (k = 0; k<solidx; k++)
	{
		j = nSolids[k];
		if (chr.GetTexture(j) == NULL)
		{
			nTrianglesInPatch[nPatch] = 0;
			continue;
		}
		nTrianglesInPatch[nPatch] = chr.GetNumOfTrianglesInPatch(j);
		vertIndices << "usemtl " << chr.getFileName(j) << std::endl; //chr.getFileName(j) << std::endl;
		for (i = 0; i < nTrianglesInPatch[nPatch]; i++)
		{
			D3DXVECTOR3 v[3];
			D3DXVECTOR2	t[3];
			chr.GetTriangle(j, i, v, t);

			vert << "v " << v[0].x << " " << v[0].y << " " << v[0].z << std::endl;
			tex << "vt " << t[0].x << " " << std::abs(t[0].y) << std::endl;

			vert << "v " << v[1].x << " " << v[1].y << " " << v[1].z << std::endl;
			tex << "vt " << t[1].x << " " << std::abs(t[1].y) << std::endl;

			vert << "v " << v[2].x << " " << v[2].y << " " << v[2].z << std::endl;
			tex << "vt " << t[2].x << " " << std::abs(t[2].y) << std::endl;

			vertIndices << "f "
				<< ct << "/" << ct << " "
				<< ct + 1 << "/" << ct + 1 << " "
				<< ct + 2 << "/" << ct + 2 << std::endl;
			ct += 3;


		}
		nPatch++;
	}

	for (k = 0; k<traidx; k++)
	{
		j = nTransparents[k];
		if (chr.GetTexture(j) == NULL)
		{
			nTrianglesInPatch[nPatch++] = 0;
			continue;
		}
		nTrianglesInPatch[nPatch] = chr.GetNumOfTrianglesInPatch(j);
		vertIndices << "usemtl " << chr.getFileName(j) << std::endl; //chr.getFileName(j) << std::endl;
		for (i = 0; i < nTrianglesInPatch[nPatch]; i++)
		{
			D3DXVECTOR3 v[3];
			D3DXVECTOR2	t[3];
			chr.GetTriangle(j, i, v, t);

			vert << "v " << v[0].x << " " << v[0].y << " " << v[0].z << std::endl;
			tex << "vt " << t[0].x << " " << std::abs(t[0].y) << std::endl;

			vert << "v " << v[1].x << " " << v[1].y << " " << v[1].z << std::endl;
			tex << "vt " << t[1].x << " " << std::abs(t[1].y) << std::endl;

			vert << "v " << v[2].x << " " << v[2].y << " " << v[2].z << std::endl;
			tex << "vt " << t[2].x << " " << std::abs(t[2].y) << std::endl;

			vertIndices << "f "
				<< ct << "/" << ct << " "
				<< ct + 1 << "/" << ct + 1 << " "
				<< ct + 2 << "/" << ct + 2 << std::endl;
			ct += 3;
		}
		nPatch++;
	}

	//building a .obj
	std::ofstream outFile("testdude.obj", std::ios::trunc);
	outFile << "mtllib testmtl.mtl" << std::endl
		<< vert.str() << std::endl << std::endl //v
		<< tex.str() << std::endl << std::endl  //vt
		<< vertIndices.str() << std::endl;	    //f
	outFile.close();

	//make the .mtl. important part is the texture path
	std::ofstream mtl("testmtl.mtl", std::ios::trunc);
	std::string i_n, i_n1;
	for (int i = 0; i < solidx + traidx; i++) {
		i_n = chr.getFileName(i);
		i_n1 = chr.getFileName(i + 1);
		if (i_n.std::string::compare(i_n1) == 0) continue; //exclude duplicates
		mtl << "newmtl " << chr.getFileName(i) << std::endl
			//<< "Ns 96.078431 0.000000 0.0000000" << std::endl
			//<< "Ka .500000 0.000000 0.000000" << std::endl
			//<< "Kd 0.500000 0.500000 0.500000" << std::endl
			//<< "Ks 0.500000 0.500000 0.500000" << std::endl
			//<< "Ni 1.00000" << std::endl
			//<< "d 1.000000" << std::endl
			//<< "illum 2" << std::endl
			<< "map_Kd " << i_n << ".bmp" << std::endl
			<< "map_Disp " << i_n << ".bmp" << std::endl
			<< "map_Ks " << i_n << ".bmp" << std::endl << std::endl;
	}


	mtl.close();


}