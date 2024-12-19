#include "MeltingSign.h"
#include "Texture.h"

int MeltingSign::addMeltingSign(std::string srcFile, float x,float y) {

	MeltingSign* pUI = new MeltingSign("MeltingSign", NULL, NULL);

	std::vector<DrawJob*>* pDrawJobs = pUI->pDrawJobs;

	setCoords(pUI, x- buttonsH/2, y- buttonsH*1.5, buttonsH, buttonsH, "top left");

	pUI->djStartN = djNround;
	memcpy((void*)&pUI->mt0, (void*)&pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));

	int uTex0 = Texture::loadTexture(srcFile);

	pUI->mt0.uTex0 = uTex0;

	pUI->mt0.uAlphaFactor = 1;


	return pUI->nInSubjs;
}
int MeltingSign::renderMeltingSign(MeltingSign* pUI) {
	//melting
	pUI->mt0.uAlphaFactor -= 0.02;
	if (pUI->mt0.uAlphaFactor <= 0) {
		pUI->pSubjs->at(pUI->nInSubjs) = NULL;
		delete pUI;
		return 0;
	}
	//lift up
	pUI->ownCoords.pos[1] -= 2.0;
	pUI->transformMatrixIsReady = false;
	UISubj::renderStandard(pUI);
	return 1;
}