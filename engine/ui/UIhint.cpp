#include "UIhint.h"
#include "TouchScreen.h"
#include "TheApp.h"

extern TheApp theApp;

int UIhint::hintSubjN=-1;

int UIhint::init() {

    UIhint* pUI = new UIhint("hint", NULL, NULL);
    hintSubjN = pUI->nInSubjs;

    setCoords(pUI, 0, 0, 1, 1, "top left");
    //set tex DJ
    std::vector<DrawJob*>* pDrawJobs = pUI->pDrawJobs;
    pUI->djStartN = djNtex;
    memcpy((void*)&pUI->mt0, (void*)&pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));
    pUI->mt0.uTex0 = -1;
    return pUI->nInSubjs;
}

int UIhint::clear() {
    std::vector<UISubj*>* pSubjs = &theApp.UISubjs;

    UISubj* pUI = pSubjs->at(hintSubjN);
    pUI->mt0.uTex0 = -1;
    return 1;
}
int UIhint::renderHint(UIhint* pUI) {
    if (TouchScreen::cursorStatus < 0) {
        //off-screen
        return 0;
    }

    Material* pMt = &pUI->mt0;
    if (pMt->uTex0 < 0)
        return 1;
    //update position
    pUI->ownCoords.pos[0] = TouchScreen::cursorPos[0]+ buttonsH * 0.5;
    pUI->ownCoords.pos[1] = TouchScreen::cursorPos[1] - buttonsH * 1.0;

    if(pUI->ownCoords.pos[0] +pUI->scale[0] >screenSize[0])
        pUI->ownCoords.pos[0] = TouchScreen::cursorPos[0] - pUI->scale[0] - buttonsH * 0.5;
    if (pUI->ownCoords.pos[1] - pUI->scale[1] < 0)
        pUI->ownCoords.pos[1] = TouchScreen::cursorPos[1];// +pUI->scale[1] + buttonsH * 0.5;
    pUI->transformMatrixIsReady = false;
    renderStandard(pUI);
    return 1;
}
