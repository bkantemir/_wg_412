#pragma once
#include "Vertex01.h"
#include "Triangle01.h"
#include "MyPolygonRib.h"
#include <vector>

class MyPolygon
{
public:
	std::vector<Vertex01*> vertices;
	std::vector<MyPolygonRib*> ribs;
	std::vector<Triangle01*> triangles;
	float normal[3] = {0,0,0};
	int ribsN = 0;
	//bounding box
	float bbMin[3] = { 0,0,0 };
	float bbMax[3] = { 0,0,0 };
public:
	virtual ~MyPolygon() { clearAll(this); };
	static void clearAll(MyPolygon* pPL);
	static int addVertex(MyPolygon* pPL, Vertex01* pV);
	static int addVertex(MyPolygon* pPL, float x, float y, float z);
	static int finalizePolygon(MyPolygon* pPL);
	static int finalizeLinePolygon(MyPolygon* pPL);
	static int setTriangle(MyPolygon* pPL, Triangle01* pT, std::vector<Vertex01*>* pVxSrc);
	static int setRectangle(MyPolygon* pPL, float w, float h, float x=0,float y=0);
	static int xyIntersection(MyPolygon* pDst, MyPolygon* pFrame, MyPolygon* pSrc, bool takeDataFromSrc);
	static int xy2pointsLineIntersection(MyPolygon* pDst, MyPolygon* pFrame, MyPolygon* pSrc);
	static int xyAddPointProjection(MyPolygon* pDst, Vertex01* pV, MyPolygon* pSrc);
	static int addLinesIntersection(MyPolygon* pDst, MyPolygon* pFrame, int ribNframe, MyPolygon* pSrc, int ribNsrc);

	static bool dotFitsRib(float* p0, MyPolygonRib* pPR);
	static bool dotFitsPoly(float* p0, MyPolygon* pPL);
	static bool correctSide(float* p0, MyPolygonRib* pPR);
	static int addVert(MyPolygon* pDst, float* p0, Vertex01* pV0, Vertex01* pV1);
	static int addVert(MyPolygon * pDst, float* p0, MyPolygon * pSrc);
	static int buildTriangles(MyPolygon* pPL);
	static int xyPointFromMesh(Vertex01* pV, std::vector<Vertex01*>* pVs, std::vector<Triangle01*>* pTs, bool takeAll);
	static int xyPointFromPolygon(Vertex01* pV, MyPolygon* pSrc, bool takeAll);
	static int xyPointBetween(Vertex01* pV, Vertex01* pV0, Vertex01* pV1, bool takeAll);
	static int polygon2mesh(std::vector<Vertex01*>* pVxDst, std::vector<Triangle01*>* pTrDst, MyPolygon* pPoly,
		std::vector<Vertex01*>* pVxMesh, std::vector<Triangle01*>* pTrMesh, bool takeDataFromMesh);
	static int mesh2mesh(std::vector<Vertex01*>* pVxDst, std::vector<Triangle01*>* pTrDst,
		std::vector<Vertex01*>* pVx0, std::vector<Triangle01*>* pTr0,
		std::vector<Vertex01*>* pVxMesh, std::vector<Triangle01*>* pTrMesh);
};

