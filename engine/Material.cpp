#include "Material.h"
#include "Shader.h"
#include "platform.h"

//std::vector<Material*> Material::materialsList;

Material::~Material() {
}
int Material::getShaderNumber(Material* pMT, std::string shaderTypeString) {
    const char* shaderType = shaderTypeString.c_str();
    if (strcmp(shaderType, "flat") == 0)
        pMT->takesShadow = 0;

    int shadersN = Shader::shaders.size();
    for (int i = 0; i < shadersN; i++) {
        Shader* pSH = Shader::shaders.at(i);

        if (strcmp(shaderType, pSH->shaderType32) != 0)
            continue;
        if (pMT->uColor.isZero() != (pSH->l_uColor < 0))
            continue;
        if ((pMT->uTex0 < 0) != (pSH->l_uTex0 < 0))
            continue;
        if ((pMT->uTex1mask < 0) != (pSH->l_uTex1mask < 0))
            continue;
        if ((pMT->uTex2nm < 0) != (pSH->l_uTex2nm < 0))
            continue;
        if ((strcmp(shaderType, "depthmap") == 0 || pMT->takesShadow < 1) != (pSH->l_uTex4dm < 0))
            continue;
        if ((pMT->bTexMod > 0) != (pSH->l_uTexMods >= 0))
            continue;
        if ((strcmp(shaderType, "flat") == 0 || strcmp(shaderType, "z-buffer") == 0) != (pSH->l_aNormal < 0))
            continue;
        return i;
    }
    mylog("ERROR in Material::pickShaderNumber:\n");
    mylog("Can't find '%s' shader for uColor=%08x uTex0=%d uTex1mask=%d uTex2nm=%d takesShadow=%d bTexMod=%d\n",
        shaderType, pMT->uColor.getUint32(), pMT->uTex0, pMT->uTex1mask, pMT->uTex2nm, pMT->takesShadow, pMT->bTexMod);
    return -1;
}

void Material::clear(Material* pMT) {
    Material mtZero;
    memcpy((void*)pMT, (void*)&mtZero, sizeof(Material));
}
int Material::assignShader(Material* pMT, std::string shaderType) {
    setShaderType(pMT, shaderType);
    pMT->shaderN=getShaderNumber(pMT, shaderType);
    return pMT->shaderN;
}

std::string Material::primitive2string(Material* pMT) {
    switch (pMT->primitiveType) {
    case GL_TRIANGLES:
        return "GL_TRIANGLES";
    case GL_POINTS:
        return "GL_POINTS";
    case GL_LINES:
        return "GL_LINES";
    case GL_LINE_STRIP:
        return "GL_LINE_STRIP";
    case GL_LINE_LOOP:
        return "GL_LINE_LOOP";
    case GL_TRIANGLE_STRIP:
        return "GL_TRIANGLE_STRIP";
    case GL_TRIANGLE_FAN:
        return "GL_TRIANGLE_FAN";
    }
    return "UNKNOWN";
}
bool Material::isLine(int primitiveType) {
    if (primitiveType == GL_TRIANGLES) return false;
    if (primitiveType == GL_TRIANGLE_STRIP) return false;
    if (primitiveType == GL_TRIANGLE_FAN) return false;
    return true;
}
int Material::mt2log(Material* pMt) {
    if (strlen(pMt->materialName32) > 0)
        mylog(" %s", pMt->materialName32);
    if (strlen(pMt->shaderType32) > 0)
        mylog(" %s", pMt->shaderType32);
    mylog(" %s", pMt->primitive2string(pMt).c_str());
    if(pMt->takesShadow>0)
        mylog(" takesShadow");
    if (pMt->uColor.isSet())
        mylog(" uColor=%06x", pMt->uColor.getUint32());
    if (pMt->uTex0 >= 0)
        mylog(" uTex0=%d", pMt->uTex0);
    if (pMt->uTex1mask >= 0)
        mylog(" uTex1mask=%d", pMt->uTex1mask);
    if (pMt->uTex2nm >= 0)
        mylog(" uTex2nm=%d", pMt->uTex2nm);
    mylog("\n");
    return 1;
}

