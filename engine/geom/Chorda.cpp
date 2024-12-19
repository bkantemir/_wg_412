
#include "Chorda.h"

int Chorda::buildChorda(Chorda* pCh, Gabarites* pG00, mat4x4 mMVP, 
	float* targetRads, float nearClip, float farClip) {

    if (pCh->chordType < 0)
        return 0; //don't need chord

    pCh->chordaQuality = 1;
    
    float offScreenLimit = 3;
    float pM[4];//mid point
    int pMonScreen = mat4x4_mul_vec4screen(pM, mMVP, pG00->bbMid, targetRads, nearClip, farClip, offScreenLimit);
    
    if (pM[0] != pM[0]) { //NaN
        pCh->chordaQuality = -1;
        return 1;
    }

    //build 3 axisses
    float vIn[4];
    for (int axN = 0; axN < 3; axN++) {
        LineXY* pLn = &pCh->axis[axN];
        v3copy(vIn, pG00->bbMid);
        //p0
        vIn[axN] = pG00->bbMid[axN] - pG00->bbRad[axN];
        int p0onScreen=mat4x4_mul_vec4screen(pLn->p0, mMVP, vIn, targetRads, nearClip, farClip, offScreenLimit);

        //debug
        if (pLn->p0[0] != pLn->p0[0])
            int a = 0;

        //p1
        vIn[axN] = pG00->bbMid[axN] + pG00->bbRad[axN];
        int p1onScreen = mat4x4_mul_vec4screen(pLn->p1, mMVP, vIn, targetRads, nearClip, farClip, offScreenLimit);

        //debug
        if (pLn->p1[0] != pLn->p1[0])
            int a = 0;


        if (p0onScreen + p1onScreen < 2) {
            //something off screen
            if (p0onScreen == 0 && p1onScreen == 0) {
                //entire line out of screen
                pCh->chordaQuality = -1;
                return 1;
            }
            else if (p0onScreen == 0 && p1onScreen == 1) {
                //p0 off screen
                if(pMonScreen>0)
                    v3copy(pLn->p0, pM);
                else
                    v3copy(pLn->p0, pLn->p1);
                pCh->chordaQuality = 0;
            }
            else {//if (p0onScreen == 1 && p1onScreen == 0) {
                //p1 off screen
                if (pMonScreen > 0)
                    v3copy(pLn->p1, pM);
                else
                    v3copy(pLn->p1, pLn->p0);
                pCh->chordaQuality = 0;
            }
        }
        LineXY::calculateLineXY(pLn);
    }
    //CHORDE: select biggest axis
    int biggestAxisN = 0;
    float biggestSize = 0;
    for (int axN = 0; axN < 3; axN++) {
        float l = pCh->axis[axN].length;
        if (biggestSize >= l)
            continue;
        biggestSize = l;
        biggestAxisN = axN;
    }
    LineXY* pLong = &pCh->axis[biggestAxisN];
    float dirLong[2];
    float midLong[2];
    for (int i = 0; i < 2; i++) {
        dirLong[i] = pLong->p1[i] - pLong->p0[i];
        midLong[i] = (pLong->p1[i] + pLong->p0[i]) / 2;
    }
    v2norm(dirLong);
    //perpendicular
    float dirPerp[2];
    dirPerp[0] = dirLong[1];
    dirPerp[1] = -dirLong[0];
    float chordL = 0; //full length
    float chordD = 0; //diameter
    for (int axN = 0; axN < 3; axN++) {
        LineXY* pAx = &pCh->axis[axN];
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
    pCh->chordR = chordD / 2;
    chordL -= chordD;

    LineXY* pChord = &pCh->chord;
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
