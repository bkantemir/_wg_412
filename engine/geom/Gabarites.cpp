#include "Gabarites.h"
#include "utils.h"
#include "TheApp.h"

extern TheApp theApp;

void Gabarites::adjustMidRad(Gabarites* pGB) {
	for (int i = 0; i < 3; i++) {
		pGB->bbRad[i] = (pGB->bbMax[i] - pGB->bbMin[i])/2;
		pGB->bbMid[i] = (pGB->bbMax[i] + pGB->bbMin[i])/2;
	}
	pGB->boxRad = v3lengthFromTo(pGB->bbMin, pGB->bbMax) / 2;
}
void Gabarites::adjustMinMaxByPoint(Gabarites* pGB, float* newPoint) {
    for (int i = 0; i < 3; i++) {
        if (pGB->bbMin[i] > newPoint[i])
            pGB->bbMin[i] = newPoint[i];
        if (pGB->bbMax[i] < newPoint[i])
            pGB->bbMax[i] = newPoint[i];
    }
}
void Gabarites::adjustMinMaxByBBox(Gabarites* pGBdst, Gabarites* pGBsrc) {
    for (int i = 0; i < 3; i++) {
        if (pGBdst->bbMin[i] > pGBsrc->bbMin[i])
            pGBdst->bbMin[i] = pGBsrc->bbMin[i];
        if (pGBdst->bbMax[i] < pGBsrc->bbMax[i])
            pGBdst->bbMax[i] = pGBsrc->bbMax[i];
    }
}

void Gabarites::toLog(std::string title, Gabarites* pGB) {
	mylog("%s:\n",title.c_str());
	mylog("bbMin: %s\n", v3toStr(pGB->bbMin));
	mylog("bbMax: %s\n", v3toStr(pGB->bbMax));
	mylog("bbMid: %s\n", v3toStr(pGB->bbMid));
	mylog("bbRad: %s\n", v3toStr(pGB->bbRad));
}

void Gabarites::fillBBox(Gabarites* pGBout, Gabarites* pGBin, mat4x4 mMVP, float* targetRads, float nearClip, float farClip) {
    
    pGBout->clear();
    float vIn[4];
    int pointsInRange = 0;
    for (int z = -1; z <= 1; z += 2) {
        vIn[2] = pGBin->bbMid[2] + pGBin->bbRad[2] * z;
        for (int y = -1; y <= 1; y += 2) {
            vIn[1] = pGBin->bbMid[1] + pGBin->bbRad[1] * y;
            for (int x = -1; x <= 1; x += 2) {
                vIn[0] = pGBin->bbMid[0] + pGBin->bbRad[0] * x;
                float vOut[4];
                pointsInRange += mat4x4_mul_vec4screen(vOut, mMVP, vIn, targetRads, nearClip, farClip);
                adjustMinMaxByPoint(pGBout,vOut);
            }
        }
    }
    adjustMidRad(pGBout);

    if (targetRads == NULL)
        pGBout->isInViewRange = 0; //not a GL range
    else { //dealing with GL range, 
        /*
        if (pointsInRange == 0)
            pGBout->isInViewRange = -1; //off-screen
        else 
        */
        if (pointsInRange == 8)
            pGBout->isInViewRange = 1; //fully on screen
        else
            pGBout->isInViewRange = 0; //partially
    }
}
/*
int Gabarites::fillGabarites(Gabarites* pG, mat4x4 absModelMatrix, Gabarites* pG00, mat4x4 mViewProjection, float* targetRads, float nearClip, float farClip) {
    //build MVP matrix for given subject
    mat4x4 mMVP;
    mat4x4_mul(mMVP, mViewProjection, absModelMatrix);

    fillBBox(pG, pG00, mMVP, targetRads, nearClip, farClip);
    if (pG->isInViewRange < 0)
        return 0;
    if(pG->chordType<0)
        return 0; //don't need chord
    //build 3 axisses
    float vIn[4];
    for (int axN = 0; axN < 3; axN++) {
        LineXY* pLn = &pG->axis[axN];
        v3copy(vIn, pG00->bbMid);
        //p0
        vIn[axN] = pG00->bbMid[axN] - pG00->bbRad[axN];
        mat4x4_mul_vec4screen(pLn->p0, mMVP, vIn, targetRads, nearClip, farClip);
        //p1
        vIn[axN] = pG00->bbMid[axN] + pG00->bbRad[axN];
        mat4x4_mul_vec4screen(pLn->p1, mMVP, vIn, targetRads, nearClip, farClip);
        LineXY::calculateLineXY(pLn);
    }
     //CHORDE: select biggest axis
     int biggestAxisN = 0;
     float biggestSize = 0;
     for (int axN = 0; axN < 3; axN++) {
         float l = pG->axis[axN].length;
         if (biggestSize >= l)
             continue;
         biggestSize = l;
         biggestAxisN = axN;
     }
     LineXY* pLong = &pG->axis[biggestAxisN];
     float dirLong[2];
     float midLong[2];
     for (int i = 0; i < 2; i++) {
         dirLong[i] = pLong->p1[i] - pLong->p0[i];
         midLong[i] = (pLong->p1[i] + pLong->p0[i])/2;
     }
     v2norm(dirLong);
     //perpendicular
     float dirPerp[2];
     dirPerp[0] = dirLong[1];
     dirPerp[1] = -dirLong[0];
     float chordL = 0; //full length
     float chordD = 0; //diameter
     for (int axN = 0; axN < 3; axN++) {
         LineXY* pAx=&pG->axis[axN];
         if (axN == biggestAxisN) {
             chordL += pAx->length;
             continue;
         }
         if (pAx->length == 0)
             continue;
         float dirAxis[2];
         for (int i = 0; i < 2; i++)
             dirAxis[i] = pAx->p1[i] - pAx->p0[i];
         v2norm(dirAxis);
         chordL = chordL + abs(v2dotProductNorm(dirLong, dirAxis)) * pAx->length;
         chordD = chordD + abs(v2dotProductNorm(dirPerp, dirAxis)) * pAx->length;
     }
     pG->chordR = chordD/2;
     chordL -= chordD;
     
     LineXY* pChord = &pG->chord;
     if (chordL < 2.0f) //dot
         pChord->initLineXY(pChord, midLong, midLong);
     else {
         v2setLength(dirLong, chordL / 2);
         for (int i = 0; i < 2; i++) {
             pChord->p0[i] = midLong[i] - dirLong[i];
             pChord->p1[i] = midLong[i] + dirLong[i];
         }
         pChord->calculateLineXY(pChord);
     }
     
     return 1;
}
*/
bool Gabarites::boxesIntersect(Gabarites* pBox1, Gabarites* pBox2) {
    for (int i = 0; i < 3; i++) {
        if (pBox1->bbMin[i] > pBox2->bbMax[i])
            return false;
        if (pBox1->bbMax[i] < pBox2->bbMin[i])
            return false;
    }
    return true;
}
bool Gabarites::pointInBox(float* p, Gabarites* pBox, float margin) {
    for (int i = 0; i < 3; i++) {
        if (p[i] > pBox->bbMax[i]+ margin)
            return false;
        if (p[i] < pBox->bbMin[i]- margin)
            return false;
    }
    return true;
}
