#include "character.h"
#include "sklseg.h"
#include "motion.h"
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>



bool FindPatternInFile(FILE* f, BYTE* pPattern, int nPatternLength)
{
	int nMatch=0;
	BYTE b;
	while(  (nMatch<nPatternLength) && (!feof(f)) )
	{
		fread(&b,sizeof(b),1,f);
		if(b==pPattern[nMatch])
			nMatch++;
		else
			nMatch=0;
	}
	if(feof(f)) return false;
	return true;
}

CSklSeg* FindSeg(char* name, CSklSeg* segs[18])
{
	static int nLastFound=0;
	int check = (nLastFound+1)%18;
	for(int i=0; i<18; i++)
	{
		if(segs[check]->IsName(name)) 
		{
			nLastFound = check;
			return segs[check];
		}
		check = (check+1)%18;
	}
	return NULL;
}

// order of segments in SKL
static char* orderedSegName[18] = 
{	
	"Hips","Chest","Neck","Head",
	"LeftCollar","LeftUpArm","LeftLowArm","LeftHand",
	"RightCollar","RightUpArm","RightLowArm","RightHand",
	"LeftUpLeg","LeftLowLeg","LeftFoot",
	"RightUpLeg","RightLowLeg","RightFoot"
};

// order of segments is different in CRP need this to rearrange
// to SKL order
static int crpord[18] =
{
	0, 1, 12, 15, 16, 17, 13, 14, 2, 4, 8, 9, 10, 11, 5, 6, 7, 3
};

CCharacter::CCharacter()
{
	m_pRootSeg	= NULL;
	m_nVertices	= 0;
	m_pVertex	= NULL;
	m_nPatches	= 0;
	m_pPatch	= NULL;
	m_pTransformedVertex = NULL;
}

CCharacter::~CCharacter()
{
	if(m_pVertex)	delete[] m_pVertex;
	if(m_pRootSeg)	delete m_pRootSeg;
	if(m_pTransformedVertex) delete[] m_pTransformedVertex;
	if(m_pPatch)	delete[] m_pPatch;
	
}

bool CCharacter::Load(char* pSKLfilename, char* pCRPfilename)
{
	// first we read the skeleton file
	//
	FILE* f = NULL;
	fopen_s(&f,pSKLfilename,"rb");
	if(!f) 
	{
		MessageBox(NULL,"Could not open SKL file.","SKL Error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);	
		return false;
	}
	char c;
	char name[128];
	DWORD magic;
	fread(&magic,sizeof(magic),1,f);		// read magic dw
	if(magic!=0x3fc00000) 
	{ 
		MessageBox(NULL,"Invalid SKL file.","SKL Error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
		fclose(f); 
		return false; 
	}
	int num;
	fread(&num,sizeof(num),1,f);	// read number of bones
	CSklSeg* segs[18];
	if(num!=18) 
	{
		MessageBox(NULL,"SKL does not define 18 segments.","SKL Error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
		fclose(f);
		return false;
	}
	fread(&c,sizeof(c),1,f);		// read dud 00
	while(num>0)
	{
		int numzeros=0;
		fread(&c,sizeof(c),1,f);
		if(c==0x01){
			while(numzeros<2)
			{
				fread(&c,sizeof(c),1,f);
				if(c>0x00) numzeros=0; else numzeros++;
			}
		}
		else
			fseek(f,-1,SEEK_CUR);
		D3DXVECTOR3 vOffset;
		// likely the relative position to parent bone (?)
		fread(vOffset,sizeof(float),3,f);
		// seek ahead over
		// 16 floats (4x4 matrix ?) of 1.00-s and 0.00-s (same for all bones)
		// + 3 floats likely the absolute position of the bone (?)
		// + 8 floats of 1.00-s and 0.00-s (same for all bones)
		fseek(f,4*(16+3+8),SEEK_CUR);
		int namelength;
		fread(&namelength,sizeof(namelength),1,f);	// read name length
		fread(name,sizeof(char),namelength,f);
		segs[num-1] = new CSklSeg(name,vOffset);
		// seek ahead over
		// 1 int ordinal
		// 1 char dud 01
		// 11 floats ... ??? purpose unknown 
		fseek(f,4+1+11*4,SEEK_CUR);
		num--;
	}
	fclose(f);

	// then construct the skeleton
	//
	for(int j=0; j<18; j++)
	{
		ord[j] = FindSeg(orderedSegName[j],segs);
		if(ord[j]==NULL)
		{
			MessageBox(NULL,orderedSegName[j],"SKL Segment Not Found",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
			for(int k=0; k<18; k++)
				delete segs[k];
			return false;
		}
	}
	// right leg and foot
	ord[16]->SetNumOfChildren(1);
	ord[16]->AddChild(ord[17]);
	ord[15]->SetNumOfChildren(1);
	ord[15]->AddChild(ord[16]);
	// left leg and foot
	ord[13]->SetNumOfChildren(1);
	ord[13]->AddChild(ord[14]);
	ord[12]->SetNumOfChildren(1);
	ord[12]->AddChild(ord[13]);
	// right collar and arm and hand
	ord[10]->SetNumOfChildren(1);
	ord[10]->AddChild(ord[11]);
	ord[9]->SetNumOfChildren(1);
	ord[9]->AddChild(ord[10]);
	ord[8]->SetNumOfChildren(1);
	ord[8]->AddChild(ord[9]);
	// left collar and arm and hand
	ord[6]->SetNumOfChildren(1);
	ord[6]->AddChild(ord[7]);
	ord[5]->SetNumOfChildren(1);
	ord[5]->AddChild(ord[6]);
	ord[4]->SetNumOfChildren(1);
	ord[4]->AddChild(ord[5]);
	// head and neck and chest and hips
	ord[2]->SetNumOfChildren(1);
	ord[2]->AddChild(ord[3]);
	ord[1]->SetNumOfChildren(3);
	ord[1]->AddChild(ord[2]);
	ord[1]->AddChild(ord[4]);
	ord[1]->AddChild(ord[8]);
	ord[0]->SetNumOfChildren(3);
	ord[0]->AddChild(ord[1]);
	ord[0]->AddChild(ord[12]);
	ord[0]->AddChild(ord[15]);
	// set root to hip
	m_pRootSeg = ord[0];

	// then we parse the CRP file
	//
	BYTE splitC[4] = {0xC,0x0,0x0,0x0};
	BYTE split0101[2] = {0x1,0x1};
	f=NULL;
	fopen_s(&f,pCRPfilename,"rb");
	if(!f)
	{
		MessageBox(NULL,"Could not open CRP file.","CRP Error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);	
		delete m_pRootSeg;
		return false;
	}
	D3DXVECTOR3 v;
	fread(&magic,sizeof(magic),1,f);		// magic number
	if(magic!=0x3fe66666)
	{
		MessageBox(NULL,"Invalid CRP file.","CRP Error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);	
		delete m_pRootSeg;
		fclose(f);
		return false;
	}
	m_nVertices = 0;
	for(int cBlock=0; cBlock<18; cBlock++)
	{
		int nPointsInBlock=0;
		int cPoints;
		fread(&nPointsInBlock,sizeof(nPointsInBlock),1,f);
		if(nPointsInBlock==0) continue;
		ord[crpord[cBlock]]->SetNumOfVertices(nPointsInBlock);
		int maxidx = 0;
		for(cPoints=0; cPoints<nPointsInBlock; cPoints++) 
		{	
			int index;
			fread(&index,sizeof(index),1,f);
			maxidx = max(maxidx,index);
			ord[crpord[cBlock]]->m_pVertexIdx[cPoints] = (WORD)index;
		}
		maxidx++; // array size must be biggest index +1
		if(maxidx>m_nVertices)
		{
			if(m_pVertex)
			{
				D3DXVECTOR3* bigger = new D3DXVECTOR3[maxidx];
				for(int j=0; j<m_nVertices; j++) bigger[j]=m_pVertex[j];
				delete[] m_pVertex;
				m_pVertex = bigger;
			}
			else
			{
				m_pVertex = new D3DXVECTOR3[maxidx];
			}
			m_nVertices = maxidx;
			
		}
		for(cPoints=0; cPoints<nPointsInBlock; cPoints++) 
		{
			fread(m_pVertex[ord[crpord[cBlock]]->m_pVertexIdx[cPoints]],sizeof(float),3,f);
		}
	}

	int namelength;
	fread(&namelength,sizeof(namelength),1,f);	// read name length
	fread(name,sizeof(char),namelength,f);
	FindPatternInFile(f,split0101,2);
	fseek(f,8,SEEK_CUR);

	int numpoints;
	fread(&numpoints,sizeof(numpoints),1,f);	// read number of points
	//seek over transformed vertices
	fseek(f,4*8*numpoints,SEEK_CUR);

	m_pTransformedVertex = new D3DXVECTOR3[m_nVertices];

	fread(&m_nPatches,sizeof(int),1,f);	// number of skin patches
	m_pPatch = new CSkinPatch[m_nPatches];

	char line[1024];

	for(int cPatch=0; cPatch<m_nPatches; cPatch++)
	{
		FindPatternInFile(f,splitC,4);
		int i, n;
		fread(&n,sizeof(n),1,f);
		fread(line,sizeof(char),n,f);
		for(i=n-1;i>=0;i--)
			if(line[i]=='\\') break;
		i++;
		int pos=0;
		while(line[i]!='.')
			m_pPatch[cPatch].GetFileName()[pos++]=line[i++];
		m_pPatch[cPatch].GetFileName()[pos]='\0';
		
		// next byte seems to be 0x04 for transparents
		BYTE cAlpha;
		fread(&cAlpha,sizeof(BYTE),1,f);
		if(cAlpha==0x04) m_pPatch[cPatch].m_bAlpha = true;

		m_pPatch[cPatch].InitTexture();

		FindPatternInFile(f,split0101,2);
		int numskinpoints;
		fread(&numskinpoints,sizeof(numskinpoints),1,f);
		m_pPatch[cPatch].SetNumPoints(numskinpoints);
		for(i=0;i<numskinpoints;i++)
		{
			fread(m_pPatch[cPatch].m_pPoint[i].dud,sizeof(float),4,f);
		}
		fread(&n,sizeof(n),1,f); // read dud 01 00 00 00
		for(i=0;i<numskinpoints;i++)
		{
			fread(&(m_pPatch[cPatch].m_pPoint[i].s),sizeof(float),2,f);
		}
		// testing is the above truly repeated?
		int nd;
		fread(&nd,sizeof(nd),1,f);
		if(nd!=numskinpoints)
			MessageBox(NULL,"hmm","oops",MB_OK|MB_SYSTEMMODAL);
		for(i=0;i<numskinpoints;i++)
		{
			float fl;
			for(int j=0;j<4;j++)
			{
				fread(&fl,sizeof(float),1,f);
				if(fl!=m_pPatch[cPatch].m_pPoint[i].dud[j])
					MessageBox(NULL,"hmm","oops",MB_OK|MB_SYSTEMMODAL);
			}
		}
		fread(&nd,sizeof(nd),1,f); // read dud 01 00 00 00
		for(i=0;i<numskinpoints;i++)
		{
			float fl;
			fread(&fl,sizeof(float),1,f);
			if(fl!=m_pPatch[cPatch].m_pPoint[i].s.x)
				MessageBox(NULL,"hmm","oops",MB_OK|MB_SYSTEMMODAL);
			fread(&fl,sizeof(float),1,f);
			if(fl!=m_pPatch[cPatch].m_pPoint[i].s.y)
				MessageBox(NULL,"hmm","oops",MB_OK|MB_SYSTEMMODAL);
		}
		// reading normals(?)
		for(i=0;i<numskinpoints;i++)
		{
			fread(m_pPatch[cPatch].m_pPoint[i].normal,sizeof(float),3,f);
		}
		int numtriangles;
		fread(&numtriangles,sizeof(numtriangles),1,f);
		m_pPatch[cPatch].SetNumTriangles(numtriangles);
		// reading ??? 4*float/triangle
		for(i=0;i<4*numtriangles;i++)
		{
			float fl;
			fread(&fl,sizeof(float),1,f);
		}
		// read triangle corners (pos index)
		for(i=0;i<numtriangles;i++)
		{
			WORD idx;
			fread(&idx,sizeof(WORD),1,f);
			m_pPatch[cPatch].m_pTriangle[i].posidx[0]=(int)idx;
			fread(&idx,sizeof(WORD),1,f);
			m_pPatch[cPatch].m_pTriangle[i].posidx[1]=(int)idx;
			fread(&idx,sizeof(WORD),1,f);
			m_pPatch[cPatch].m_pTriangle[i].posidx[2]=(int)idx;
		}
		// read triangle corners (texcoord index)
		for(i=0;i<numtriangles;i++)
		{
			WORD idx;
			fread(&idx,sizeof(WORD),1,f);
			m_pPatch[cPatch].m_pTriangle[i].skinidx[0]=(int)idx;
			fread(&idx,sizeof(WORD),1,f);
			m_pPatch[cPatch].m_pTriangle[i].skinidx[1]=(int)idx;
			fread(&idx,sizeof(WORD),1,f);
			m_pPatch[cPatch].m_pTriangle[i].skinidx[2]=(int)idx;

		}
	}		
	fclose(f);
	D3DXMATRIX m;
	D3DXMatrixIdentity(&m);
	D3DXVECTOR3 pos(0,0,0);

	

	m_pRootSeg->TransformRec(m_pVertex,m_pTransformedVertex,m,pos);
	to_OBJ();
	bones();
	

	return true;
}

bool CCharacter::SetMotion(CMotion* pMotion, float fTime)
{
	if(!pMotion) return false; 
	if(pMotion->m_nFrames==0) return false;
	int nFrame = (int) fTime;
	if(nFrame < 0) nFrame = 0;
	if(nFrame >= pMotion->m_nFrames-1) nFrame = pMotion->m_nFrames-1;
	for(int i=0; i<18; i++)
	{
		D3DXMatrixRotationQuaternion( 
			&(ord[i]->m_mRotation), 
			&(pMotion->m_pFrame[nFrame].m_pRotQuats[i])
		);
	}
	D3DXMATRIX m;
//	D3DXMatrixIdentity(&m);
	D3DXMatrixRotationYawPitchRoll(&m, rot, 0, 0);

	m_pRootSeg->TransformRec(
		m_pVertex, m_pTransformedVertex,
		m, pMotion->m_pFrame[nFrame].m_vOffset
	);
	return true;
}




void CCharacter::to_OBJ() {
	int nTrianglesInPatch[128];
	int nSolids[128];
	int nTransparents[128];

	Vertex *pPointVertices;

	// set rendering order (to render transparents in the end)
	int solidx = 0;
	int traidx = 0;
	int i, j, k;
	for (j = 0; j<GetNumPatches(); j++)
	{
		if (IsPatchTransparent(j))
			nTransparents[traidx++] = j;
		else
			nSolids[solidx++] = j;
	}

	float minY = 1000.0f;
	float maxY = -1000.0f;
	int nPatch = 0;


	std::stringstream tex, vert, vertIndices;
	int ct = 1;


	for (k = 0; k<solidx; k++)
	{
		j = nSolids[k];
		if (GetTexture(j) == NULL)
		{
			nTrianglesInPatch[nPatch] = 0;
			continue;
		}
		nTrianglesInPatch[nPatch] = GetNumOfTrianglesInPatch(j);
		vertIndices << "usemtl " << getFileName(j) << std::endl; //chr.getFileName(j) << std::endl;
		for (i = 0; i < nTrianglesInPatch[nPatch]; i++)
		{
			D3DXVECTOR3 v[3];
			D3DXVECTOR2	t[3];
			GetTriangle(j, i, v, t);

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
		if (GetTexture(j) == NULL)
		{
			nTrianglesInPatch[nPatch++] = 0;
			continue;
		}
		nTrianglesInPatch[nPatch] = GetNumOfTrianglesInPatch(j);
		vertIndices << "usemtl " << getFileName(j) << std::endl; //chr.getFileName(j) << std::endl;
		for (i = 0; i < nTrianglesInPatch[nPatch]; i++)
		{
			D3DXVECTOR3 v[3];
			D3DXVECTOR2	t[3];
			GetTriangle(j, i, v, t);

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
		i_n = getFileName(i);
		i_n1 = getFileName(i + 1);
		if (i_n.std::string::compare(i_n1) == 0) continue; //exclude duplicates
		mtl << "newmtl " << getFileName(i) << std::endl
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

void CCharacter::bones() {
	std::ofstream file("bones.obj", std::ios::trunc);
	std::stringstream v, f;
	int ct = 1;
	for (int i = 0; i < ord[0]->m_nVertices; i++) {
		v << "v " << m_pVertex[ord[0]->m_pVertexIdx[i]].x << " "
			  << m_pVertex[ord[0]->m_pVertexIdx[i]].y << " "
			  << m_pVertex[ord[0]->m_pVertexIdx[i]].z << std::endl;

		f << "f " << ct << "/" << ct << " "
			<< ct + 1 << "/" << ct + 1 << " "
			<< ct + 2 << "/" << ct + 2 << std::endl;

		ct ++;
	}
	file << v.str();
	file << f.str();
	file.close();
}
void CCharacter::rotate(CMotion* pMotion, float fTime, float yaw) {
	rot = rot + yaw;
	D3DXMATRIX m;
	D3DXMatrixIdentity(&m);
	D3DXMatrixRotationYawPitchRoll(&m, rot, 0, 0);
	int nFrame = (int)fTime;
	m_pRootSeg->TransformRec(
		m_pVertex, m_pTransformedVertex,
		m, pMotion->m_pFrame[nFrame].m_vOffset
	);
}

