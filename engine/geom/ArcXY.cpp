#include "ArcXY.h"
#include "utils.h"
#include "TheApp.h"

extern TheApp theApp;
extern float PI;
extern float degrees2radians;

void ArcXY::initArcXY(ArcXY* pA, float centerX, float centerY, float rad, float a00, float a01, int arcDir0) {
	pA->centerPos[0] = centerX;
	pA->centerPos[1] = centerY;
	pA->a0 = angleDgNorm360(a00);
	pA->a1 = angleDgNorm360(a01);
	pA->radius = rad;
	if (arcDir0 == 0) //unknown
		arcDir0 = signOf(angleDgFromTo(pA->a0, pA->a1));
	pA->arcDir = arcDir0;
	pA->fullCircle = (abs(pA->a1 - pA->a0) == 360);

	pA->angleDgCCWstart= pA->a0;
	pA->angleDgCCWrange = angleDgFromTo(pA->a0, pA->a1);
	if (pA->arcDir < 0) {
		pA->angleDgCCWstart = pA->a1;
		pA->angleDgCCWrange = -pA->angleDgCCWrange;
	}
	if (pA->angleDgCCWrange < 0)
		pA->angleDgCCWrange += 360;

	//mylog("arc %d->%d dir=%d angleDgCCWstart=%d angleDgCCWrange=%d\n", (int)pA->a0, (int)pA->a1, pA->arcDir, (int)pA->angleDgCCWstart,(int)pA->angleDgCCWrange);
	
	//2 points positions
	pA->p0[0] = pA->centerPos[0] + sin(pA->a0 * degrees2radians) * pA->radius;
	pA->p0[1] = pA->centerPos[1] + cos(pA->a0 * degrees2radians) * pA->radius;
	pA->p1[0] = pA->centerPos[0] + sin(pA->a1 * degrees2radians) * pA->radius;
	pA->p1[1] = pA->centerPos[1] + cos(pA->a1 * degrees2radians) * pA->radius;
	
}
int ArcXY::circleCrossLinePick1(float* vOut2d, ArcXY* pA, LineXY* pL, int* solutionPreference) {
	//solutionPreference: 0,1-pick greater Y, -1,0 - pick lower X
	float vOuts[2][2];
	int optsN = circleCrossLine(vOuts, pA, pL);
	if (optsN == 0)
		return 0;
	if (optsN == 1) {
		v2copy(vOut2d, vOuts[0]);
		return 1;
	}
	//if here - have 2 solutions
	int prferredSolutionN = 1;
	for(int dimN=0; dimN <2; dimN++){ //dimension N
		if (solutionPreference[dimN] == 0)
			continue;
		float sign = solutionPreference[dimN];
		if (vOuts[0][dimN] * sign < vOuts[1][dimN] * sign)
			prferredSolutionN = 2;
		break;
	}
	if (prferredSolutionN == 1)
		v2copy(vOut2d, vOuts[0]);
	else
		v2copy(vOut2d, vOuts[1]);

	return 2;
}
int ArcXY::circleCrossLine(float (*vOuts)[2], ArcXY* pA, LineXY* pL) {
	/*
	m - line aSlope
	n - line bIntercept
	p - circle center x
	q - circle center y
	circle equation:
	(x-p)^2+(y-q)^2=r^2
	unpack:
	xx-xp2+pp + yy-yq2+qq - rr = 0
	substityte y by line equation (xm+n)=0
	xx-xp2+pp + (xm+n)^2-(xm+n)q2+qq - rr = 0
	unpack:
	xx-xp2+pp + xxmm+2xmn+nn-xmq2-nq2+qq - rr = 0
	combine x:
	xx + xxmm-xp2+xmn2-xmq2+pp+nn-nq2+qq - rr = 0
	xx(1+mm) + x2(-p+mn-mq) + pp+nn-nq2+qq-rr = 0
	Quadratic Equation: ax^2+bx+c=0
	a = 1+mm
	b = (-p+mn-mq)*2
	c = pp+nn-nq2+qq-rr
	check: (m2+1)x^2+2(mc?mq?p)x+(q^2?r^2+p^2?2cq+c^2)=0.
	discriminant D:
	D = (bb-4ac)
	d=sqrt(D)
	solution
	x=(-b +/- d)/2a
	*/
	float m = pL->a_slope;
	float n = pL->b_intercept;
	float p = pA->centerPos[0];
	float q = pA->centerPos[1];
	float r = pA->radius;

	if (pL->isVertical) {
		//dxLLine2CircleCenter
		float dx = abs(pL->x_vertical - p);
		if (dx > pA->radius)
			return 0; //no intersections
		else if (dx == pA->radius) {
			//1 intersection
			vOuts[0][0] = pL->x_vertical;
			vOuts[0][1] = q;
			return 1;
		}
		//if here - 2 intersectios
		vOuts[0][0] = pL->x_vertical;
		vOuts[1][0] = pL->x_vertical;
		//square equision
		float x = pL->x_vertical;
		float b = -q * 2;
		float c = x * x - x * p * 2 + p * p + q * q - r * r;
		float D = b * b - c * 4; // discriminant
		float d = sqrt(D);
		//x=(-b +/- d)/2a
		vOuts[0][1] = (-b + d) / 2;
		vOuts[1][1] = (-b - d) / 2;
		return 2;
	}
	else { //line not vertical
		float a = 1.0 + m * m;
		float b = (-p + m * n - m * q) * 2;
		float c = p * p + n * n - n * q * 2 + q * q - r * r;
		float D = b * b - a * c * 4;
		if (D < 0)
			return 0; //no solutions
		else if (D == 1) { //1 solution
			vOuts[0][0] = -b / (a * 2);
			vOuts[0][1] = vOuts[0][0] * m + n;
			return 1;
		}
		//if here - 2 solutions
		float d = sqrt(D);
		//solution: x = (-b + / -d) / 2a
		vOuts[0][0] = (-b + d) / (a * 2);
		vOuts[1][0] = (-b - d) / (a * 2);
		vOuts[0][1] = vOuts[0][0] * m + n;
		vOuts[1][1] = vOuts[1][0] * m + n;
		return 2;
	}
}
bool ArcXY::pointWithinArc(ArcXY* pA,float* p2d) {
	float angle2point = v2dirDgFromToDownCCW(pA->centerPos, p2d);
	return angleWithinArc(pA, angle2point,false);
}
bool ArcXY::angleWithinArc(ArcXY* pA, float angleDg, bool printDrbug) {
	float a = angleDgNorm360(angleDg - pA->angleDgCCWstart);
	bool fits = true;
	if (a > pA->angleDgCCWrange)
		fits = false;

	//debug
	if(printDrbug)
		mylog("%d ArcXY::angleWithinArc: CCWstart=%d range=%d, angleDg=%d a=%d fits=%d\n", theApp.frameN, (int)pA->angleDgCCWstart, (int)pA->angleDgCCWrange, 
			(int)angleDg,(int)a, fits);

	return fits;
}
int ArcXY::arcCrossLine(float(*points)[2], ArcXY* pA, LineXY* pL,bool printDegug) {
	int optsN = circleCrossLine(points, pA, pL);

	//debug
	if (printDegug) {
		mylog("%d ArcXY::arcCrossLine: circleCrossLine arcCenter %dx%d, pos %dx%d to %dx%d\n",
			theApp.frameN, (int)pA->centerPos[0], (int)pA->centerPos[1],
			(int)pA->p0[0], (int)pA->p0[1], (int)pA->p1[0], (int)pA->p1[1]);
		mylog("%d ArcXY::arcCrossLine: a %d to %d CCW=%d, CCWstart=%d range=%d, optsN=%d\n",
			theApp.frameN, (int)pA->a0, (int)pA->a1,pA->arcDir,
			(int)pA->angleDgCCWstart, (int)pA->angleDgCCWrange,
			optsN);
	}

	if (optsN == 0)
		return 0;
	for (int optN = optsN - 1; optN >= 0; optN--) {
		float* p2d = points[optN];
		float angle2point = v2dirDgFromToDownCCW(pA->centerPos, p2d);


		//debug
		if (printDegug)
			mylog("%d ArcXY::arcCrossLine: optN=%d %dx%d, angle2point=%d fits=%d\n",
				theApp.frameN, optN, (int)p2d[0], (int)p2d[1], (int)angle2point, angleWithinArc(pA, angle2point,true));

		if (angleWithinArc(pA, angle2point,false))
			continue;
		//if here - angle is outside of arc
		if (optsN - 1 > optN)
			v2copy(points[optN], points[optN + 1]);
		optsN--;
		if (optsN == 0)
			return 0;
	}
	return optsN;
}
float ArcXY::dist_arc2point(ArcXY* pA, float* p2d) {
	if (v2match(pA->centerPos, p2d))
		return pA->radius;
	//build line from point to circle center
	LineXY* pL = new LineXY(pA->centerPos, p2d);
	float points[2][2];
	int optsN = arcCrossLine(points, pA, pL,false);
	if (optsN == 0) {
		//pick from p0/1
		float d0 = v2lengthFromTo(p2d, pA->p0);
		float d1 = v2lengthFromTo(p2d, pA->p1);
		if (d0 < d1) {
			v2copy(pA->closestPoint, pA->p0);
			return d0;
		}
		else {
			v2copy(pA->closestPoint, pA->p1);
			return d1;
		}
	}
	if (optsN == 1) {
		v2copy(pA->closestPoint, points[0]);
		return v2lengthFromTo(p2d, pA->closestPoint);
	}
	//if here - 2 crossings
	float d0 = v2lengthFromTo(p2d, points[0]);
	float d1 = v2lengthFromTo(p2d, points[1]);
	if (d0 < d1) {
		v2copy(pA->closestPoint, points[0]);
		return d0;
	}
	else {
		v2copy(pA->closestPoint, points[1]);
		return d1;
	}
}
int ArcXY::arcCrossLineSegment(float(*points)[2], ArcXY* pA, LineXY* pL) {
	int optsN = arcCrossLine(points, pA, pL,true);

	//debug
	mylog("%d ArcXY::arcCrossLineSegment: arc2Center optsN %d\n",
		theApp.frameN, optsN);
	
	if (optsN == 0)
		return 0;
	for (int optN = optsN - 1; optN >= 0; optN--) {
		float* p2d = points[optN];

		//debug
		mylog("%d ArcXY::arcCrossLineSegment: optN=%d %dx%d, segment %dx%d to %dx%d, fits=%d\n",
			theApp.frameN, optN, (int)p2d[0], (int)p2d[1], 
			(int)pL->p0[0], (int)pL->p0[1], (int)pL->p1[0], (int)pL->p1[1],
			pL->isPointIn(p2d, pL));

		if (pL->isPointIn(p2d, pL))
			continue;
		//if here - point is outside of line segment
		if (optsN - 1 > optN)
			v2copy(points[optN], points[optN + 1]);
		optsN--;
		if (optsN == 0)
			return 0;
	}
	return optsN;
}
int ArcXY::arcCrossArc(float(*points)[2], ArcXY* pA, ArcXY* pA2) {
	int optsN = circleCrossCircle(points, pA, pA2);

	//debug
	mylog("%d ArcXY::arcCrossArc: circleCrossCircle optsN=%d\n", theApp.frameN, optsN);

	if (optsN == 0)
		return 0;
	for (int optN = optsN - 1; optN >= 0; optN--) {
		float* p2d = points[optN];

		{//debug
			mylog("%d ArcXY::arcCrossArc: optN=%d %dx%d\n", theApp.frameN, optN,
				(int)p2d[0], (int)p2d[1]);
		}

		if (pointWithinArc(pA, p2d))
			if (pointWithinArc(pA2, p2d))
				continue; //belongs to both arcs

		//debug
		mylog("%d ArcXY::arcCrossArc: does't fit\n", theApp.frameN);

		//if here - point is outside of arcs
		if (optsN - 1 > optN)
			v2copy(points[optN], points[optN + 1]);
		optsN--;
		if (optsN == 0)
			return 0;
	}
	//debug
	mylog("%d ArcXY::arcCrossArc: approved optsN=%d\n", theApp.frameN, optsN);

	return optsN;
}
int ArcXY::circleCrossCircle(float(*points)[2], ArcXY* pA, ArcXY* pA2) {
	float center2center[2];
	for (int i = 0; i < 2; i++)
		center2center[i] = pA2->centerPos[i] - pA->centerPos[i];
	float distBetween = v2length(center2center);

	if (distBetween > pA->radius + pA2->radius)
		return 0; //too far
	if (distBetween + pA->radius < pA2->radius)
		return 0; //pA inside of pA2
	if (distBetween + pA2->radius < pA->radius)
		return 0; //pA2 inside of pA
	if (pA->radius + pA2->radius - distBetween < 0.01) {
		//one solution
		v2setLength(center2center, pA->radius);
		float* p2d = points[0];
		for (int i = 0; i < 2; i++)
			p2d[i] = pA->centerPos[i] + center2center[i];
		return 1;
	}
	//if here - 2 solutions
	//1-st circle: x^2+y^2=R^2; => y^2 = R^2 - x^2
	//2-nd circle: (x-d)^2 + y^2 = x^2 - 2dx +d^2 + y^2 = R2^2
	//replace y^2 by line 1
	//x^2 - 2dx +d^2 + R^2 - x^2 = R2^2
	// - 2dx +d^2 + R^2 = R2^2
	// 2dx = d^2 + R^2 - R2^2
	float x;
	if(pA->radius == pA2->radius)
		x = distBetween / 2;
	else
		x = (distBetween * distBetween + pA->radius * pA->radius - pA2->radius * pA2->radius) / (distBetween * 2);
	LineXY ln0;
	LineXY::initLineXY(&ln0, pA->centerPos, pA2->centerPos);
	v2setLength(center2center, x);
	float p00[2];
	for (int i = 0; i < 2; i++)
		p00[i] = pA->centerPos[i] + center2center[i];
	LineXY ln1;
	LineXY::buildPerpendicular(&ln1, &ln0, p00);
	circleCrossLine(points, pA, &ln1);
	return 2;
}

float ArcXY::arcPercent(ArcXY* pA, float* p2d) {
	float angle2point = v2dirDgFromToDownCCW(pA->centerPos, p2d);
	float angleFromStart = angleDgNorm360(angle2point - pA->angleDgCCWstart);
	float percent = angleFromStart / pA->angleDgCCWrange;
	if (pA->arcDir < 0) //CW
		percent = 1.0 - percent;

	mylog("%d ArcXY::arcPercent: arc %d to %d CCW=%d, CCWstart=%d CCWrang=%d, angle2point=%.2f angleFromStart=%.2f percent=%.2f\n", 
		theApp.frameN, (int)pA->a0, (int)pA->a1,pA->arcDir, (int)pA->angleDgCCWstart, (int)pA->angleDgCCWrange, 
		angle2point, angleFromStart, percent);

	return percent;
}
