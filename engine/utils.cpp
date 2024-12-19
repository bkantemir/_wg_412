#include "utils.h"
#include "platform.h"
#include <chrono>
#include <stdlib.h>     /* srand, rand */
#include <sys/stat.h> //if fileExists
#include <time.h> //for srand()

extern std::string filesRoot;
float PI = 3.141592f;
float degrees2radians = PI / 180.0f;
float radians2degrees = 180.0f / PI;

int checkGLerrors(std::string ref) {
    //can be used after any GL call
    int res = glGetError();
    if (res == 0)
        return 0;
    std::string errCode;
    switch (res) {
        //case GL_NO_ERROR: errCode = "GL_NO_ERROR"; break;
    case GL_INVALID_ENUM: errCode = "GL_INVALID_ENUM"; break;
    case GL_INVALID_VALUE: errCode = "GL_INVALID_VALUE"; break;
    case GL_INVALID_OPERATION: errCode = "GL_INVALID_OPERATION"; break;
    case GL_INVALID_FRAMEBUFFER_OPERATION: errCode = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
    case GL_OUT_OF_MEMORY: errCode = "GL_OUT_OF_MEMORY"; break;
    default: errCode = "??"; break;
    }
    mylog("GL ERROR %d-%s in %s\n", res, errCode.c_str(), ref.c_str());
    return -1;
}


int mat4x4_mul_vec4plus(vec4 vOut, mat4x4 M, vec4 vIn, float v3, bool adjustByW) {
    vec4 v2;
    if (vOut == vIn) {
        memcpy(&v2, vIn, sizeof(vec4));
        vIn = v2;
    }
    vIn[3] = v3;
    mat4x4_mul_vec4(vOut, M, vIn);
    if (adjustByW) {
        float d = vOut[3];
        if ( d != 1)
            for (int i = 0; i < 3; i++)
                vOut[i] /= d;
    }
    /*
    for (int i = 0; i < 3; i++)
        if (vOut[i] != vOut[i]) {
            //mylog("ERROR in utils:mat4x4_mul_vec4plus d=%.2f\n", vOut[3]);
            return -1;
        }
        */
    if (vOut[0] != vOut[0])
        return -1; //NaN
    return 1;
}
int mat4x4_mul_vec4screen(vec4 vOut, mat4x4 M, vec4 vIn, float* targetRads, float nearClip, float farClip, float offScreenLimit) {
    mat4x4_mul_vec4plus(vOut, M, vIn, 1, true);
    //GL to screen coords
    int onScreen = 1;//on screen
    if (targetRads != NULL) {
        for (int i = 0; i < 3; i++)
            if (abs(vOut[i]) > 1) {
                float d = abs(vOut[i]) - 1;
                if(d>= offScreenLimit)
                    onScreen = 0;//off-screen
            }
        vOut[0] = vOut[0] * targetRads[0] + targetRads[0];
        vOut[1] = -vOut[1] * targetRads[1] + targetRads[1];
    }
    return onScreen;
}
void v3set(float* vOut, float x, float y, float z) {
    vOut[0] = x;
    vOut[1] = y;
    vOut[2] = z;
}
void v2set(float* vOut, float x, float y) {
    vOut[0] = x;
    vOut[1] = y;
}
void v4copy(float* vOut, float* vIn) {
    for (int i = 0; i < 4; i++)
        vOut[i] = vIn[i];
}
void v3copy(float* vOut, float* vIn) {
    for (int i = 0; i < 3; i++)
        vOut[i] = vIn[i];
}
void v2copy(float* vOut, float* vIn) {
    for (int i = 0; i < 2; i++)
        vOut[i] = vIn[i];
}
void m16copy(float* vOut, float* vIn) {
    for (int i = 0; i < 16; i++)
        vOut[i] = vIn[i];
}
void m16copy(mat4x4 vOut, mat4x4 vIn) {
    m16copy((float*)vOut, (float*) vIn);
}
float v3yawRd(float* vIn) {
    return atan2f(vIn[0], vIn[2]);
}
float v3pitchRd(float* vIn) {
    return -atan2f(vIn[1], sqrtf(vIn[0] * vIn[0] + vIn[2] * vIn[2]));
}
float v3pitchDg(float* vIn) {
    return v3pitchRd(vIn) * radians2degrees;
}
float v3yawDg(float* vIn) {
    return v3yawRd(vIn) * radians2degrees;
}

uint64_t getSystemMillis() {
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime).count();
}
uint64_t getSystemNanos() {
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime).count();
}
int randomCallN = 0;
int getRandom() {
    if (randomCallN % 1000 == 0)
        srand((unsigned int)getSystemNanos()); //re-initialize random seed:
    randomCallN++;
    return rand();
}
int getRandom(int fromN, int toN) {
    int randomN = getRandom();
    int range = toN - fromN + 1;
    return (fromN + randomN % range);
}
float getRandom(float fromN, float toN) {
    int randomN = getRandom();
    float range = toN - fromN;
    return (fromN + (float)randomN / (float)RAND_MAX * range);
}
std::vector<std::string>* splitString(std::string inString, std::string delimiter) {
    std::vector<std::string>* pOutStrings = new std::vector<std::string>;
    int delimiterSize = delimiter.size();
    while (1) {
        int delimiterPosition = inString.find(delimiter);
        if (delimiterPosition == std::string::npos) {
            //last element
            pOutStrings->push_back(trimString(inString));
            break;
        }
        std::string outString = inString.substr(0, delimiterPosition);
        pOutStrings->push_back(trimString(outString));
        int startAt = delimiterPosition + delimiterSize;
        inString = inString.substr(startAt, inString.size() - startAt);
    }
    return pOutStrings;
}
std::string trimString(std::string inString) {
    //Remove leading and trailing spaces
    int startsAt = inString.find_first_not_of(" ");
    if (startsAt == std::string::npos)
        return "";
    int endsAt = inString.find_last_not_of(" ") + 1;
    return inString.substr(startsAt, endsAt - startsAt);
}
bool fileExists(const char* filePath) {
    struct stat info;
    if (stat(filePath, &info) == 0)
        return true;
    else
        return false;
}
std::string getFullPath(std::string filePath) {
    if (filePath.find(filesRoot) == 0)
        return filePath;
    else
        return (filesRoot + filePath);
}
std::string getInAppPath(std::string filePath) {
    std::string inAppPath(filePath);
    if (inAppPath.find(filesRoot) == 0) {
        int startsAt = filesRoot.size();
        inAppPath = inAppPath.substr(startsAt, inAppPath.size() - startsAt);
    }
    if (inAppPath.find(".") != std::string::npos) {
        //cut off file name
        int endsAt = inAppPath.find_last_of("/");
        inAppPath = inAppPath.substr(0, endsAt + 1);
    }
    return inAppPath;
}
int makeDirs(std::string filePath) {
    filePath = getFullPath(filePath);
    std::string inAppPath = getInAppPath(filePath);
    std::vector<std::string>* pPathNodes = splitString(inAppPath, "/");
    int pathSize = pPathNodes->size();
    filePath.assign(filesRoot);
    for (int i = 0; i < pathSize; i++) {
        filePath.append("/" + pPathNodes->at(i));
        if (fileExists(filePath.c_str())) {
            continue;
        }
        //create dir
        myMkDir(filePath.c_str());
        mylog("Folder %d: %s created.\n", i, filePath.c_str());
    }
    delete pPathNodes;
    return 1;
}
void v3inverse(float inV[]) {
    return v3inverse(inV, inV);
}
void v3inverse(float outV[], float inV[]) {
    for (int i = 0; i < 3; i++)
        outV[i] = -inV[i];
    return;
}

float v4dotProduct(float* a0, float* b0) {
    float a[4];
    float b[4];
    vec4_norm(a, a0);
    vec4_norm(b, b0);
    return v4dotProductNorm(a, b);
}
float v4dotProductNorm(float* a, float* b) {
    //assuming that vectors are normalized
    float dp = 0.0f;
    for (int i = 0; i < 4; i++)
        dp += a[i] * b[i];
    return dp;
}
float v3dotProduct(float* a0, float* b0) {
    float a[3];
    float b[3];
    vec3_norm(a, a0);
    vec3_norm(b, b0);
    return v3dotProductNorm(a, b);
}
float v2dotProduct(float* a0, float* b0) {
    float a[2];
    float b[2];
    vec2_norm(a, a0);
    vec2_norm(b, b0);
    return v2dotProductNorm(a, b);
}
float v3dotProductNorm(float* a, float* b) {
    //assuming that vectors are normalized
    float dp = 0.0f;
    for (int i = 0; i < 3; i++)
        dp += a[i] * b[i];
    return dp;
}
float v2dotProductNorm(float* a, float* b) {
    //assuming that vectors are normalized
    float dp = 0.0f;
    for (int i = 0; i < 2; i++)
        dp += a[i] * b[i];
    return dp;
}
float v3length(float* v) {
    float r = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    if (r == 0)
        return 0;
    return sqrtf(r);
}
float v2length(float* v) {
    float r = v[0] * v[0] + v[1] * v[1];
    if (r == 0)
        return 0;
    return sqrtf(r);
}
float v3lengthXZ(float v[]) {
    float r = v[0] * v[0] + v[2] * v[2];
    if (r == 0)
        return 0;
    return sqrtf(r);
}
float v3lengthXY(float v[]) {
    float r = v[0] * v[0] + v[1] * v[1];
    if (r == 0)
        return 0;
    return sqrtf(r);
}
bool v3equals(float v[], float x) {
    for (int i = 0; i < 3; i++)
        if (v[i] != x)
            return false;
    return true;
}
bool v2equals(int v[], int x) {
    for (int i = 0; i < 2; i++)
        if (v[i] != x)
            return false;
    return true;
}
bool vNmatch(int n, float v0[], float v1[]) {
    for (int i = 0; i < n; i++)
        if (v0[i] != v1[i])
            return false;
    return true;
}
bool v4match(float v0[], float v1[]) { return vNmatch(4, v0, v1); };
bool v3match(float v0[], float v1[]) { return vNmatch(3, v0, v1); };
bool v2match(float v0[], float v1[]) { return vNmatch(2, v0, v1); };

void v3fromTo(float* v, float* v0, float* v1) {
    for (int i = 0; i < 3; i++)
        v[i] = v1[i] - v0[i];
}
void v2fromTo(float* v, float* v0, float* v1) {
    for (int i = 0; i < 2; i++)
        v[i] = v1[i] - v0[i];
}
float v3lengthFromTo(float* v0, float* v1) {
    float v[3];
    v3fromTo(v, v0, v1);
    float r = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    if (r == 0)
        return 0;
    return sqrtf(r);
}
float v3lengthFromToXY(float* v0, float* v1) {
    float v[3];
    v3fromTo(v, v0, v1);
    float r = v[0] * v[0] + v[1] * v[1];
    if (r == 0)
        return 0;
    return sqrtf(r);
}
float v2lengthFromTo(float* v0, float* v1) {
    float v[2];
    v2fromTo(v, v0, v1);
    float r = v[0] * v[0] + v[1] * v[1];
    if (r == 0)
        return 0;
    return sqrtf(r);
}
float v3lengthFromToXZ(float* v0, float* v1) {
    float v[3];
    v3fromTo(v, v0, v1);
    float r = v[0] * v[0] + v[2] * v[2];
    if (r == 0)
        return 0;
    return sqrtf(r);
}
void v3dirFromTo(float* v, float* v0, float* v1) {
    v3fromTo(v, v0, v1);
    vec3_norm(v, v);
}
void mylog_v3(std::string vTitle, float* v) {
    mylog("%s %.2f x %.2f x %.2f\n", vTitle.c_str(), v[0], v[1], v[2]);
}
void mylog_m16(std::string vTitle, float* v) {
    mylog("%s\n", vTitle.c_str());
    mylog("%.2f x %.2f x %.2f x %.2f\n", v[0], v[1], v[2], v[3]);
    mylog("%.2f x %.2f x %.2f x %.2f\n", v[4], v[5], v[6], v[7]);
    mylog("%.2f x %.2f x %.2f x %.2f\n", v[8], v[9], v[10], v[11]);
    mylog("%.2f x %.2f x %.2f x %.2f\n", v[12], v[13], v[14], v[15]);
}
void mylog_mat4x4(std::string vTitle, mat4x4 m) {
    mylog_m16(vTitle, (float*)m);
}
void v3setAll(float* vOut, float x) {
    v3set(vOut, x, x, x);
}
void v2setAll(float* vOut, float x) {
    v2set(vOut, x, x);
}
void v3norm(float* v) {
    if (!v3equals(v, 0))
        vec3_norm(v, v);
}
void v2norm(float* v) {
    if (!v3equals(v, 0))
        vec2_norm(v, v);
}
float v3max(float* v, float x) {
    float l = v3length(v);
    if (l <= x)
        return l;
    float k = x / l;
    for (int i = 0; i < 3; i++)
        v[i] *= k;
    return x;
}
float v2max(float* v, float x) {
    float l = v2length(v);
    if (l <= x)
        return l;
    float k = x / l;
    for (int i = 0; i < 2; i++)
        v[i] *= k;
    return x;
}
float angleDgNorm360(float a) {
    //returns angle in 0:360 range
    while (a < 0)
        a += 360;
    while (a >= 360)
        a -= 360;
    return a;
}
float angleDgNorm180(float a) {
    //returns angle in -180:+180 range
    a = angleDgNorm360(a);
    if (a > 180)
        a -= 360;
    return a;
}
float angleDgFromTo(float a0, float a2) {
    float a = angleDgNorm180(a2 - a0);
    return a;
}
void v3normXY(float* vOut, float* vIn) {
    float s = 0;
    for (int i = 0; i < 2; i++)
        s = s + vIn[i] * vIn[i];
    if (s == 0) {
        for (int i = 0; i < 2; i++)
            vOut[i] = 0;
        return;
    }
    if (s == 1) {
        for (int i = 0; i < 2; i++)
            vOut[i] = vIn[i];
        return;
    }
    s = sqrtf(s);
    for (int i = 0; i < 2; i++)
        vOut[i] = vIn[i] / s;
}
void v3setLength(float* v3, float ln) {
    v3norm(v3);
    if (ln != 1) {
        for (int i = 0; i < 3; i++)
            v3[i] *= ln;
    }
    return;
}
void v2setLength(float* v2, float ln) {
    v2norm(v2);
    if (ln != 1) {
        for (int i = 0; i < 2; i++)
            v2[i] *= ln;
    }
    return;
}
char* v3toStr(float* v) {
    char outStr[256];
    sprintf_s(outStr, 256, "%7.2f x%7.2f x%7.2f", v[0], v[1], v[2]);
    return outStr;
}
char* v3toStrXZ(float* v) {
    char outStr[256];
    sprintf_s(outStr, 256, "%7.2f x%7.2f", v[0], v[2]);
    return outStr;
}
float v3pitchDgFromTo(float* v0, float* v1) {
    float v[3];
    for (int i = 0; i < 3; i++)
        v[i] = v1[i] - v0[i];
    return v3pitchDg(v);
}
float v3yawDgFromTo(float* v0, float* v1) {
    float v[3];
    for (int i = 0; i < 3; i++)
        v[i] = v1[i] - v0[i];
    return v3yawDg(v);
}
int signOf(float n) {
    if (n < 0)
        return -1; 
    return 1; 
}

float v2dirRdAsInSchool(float* v) { //as in school
    return -atan2f(v[1], v[0]);
}
float v2dirDgAsInSchool(float* v) { //as in school
    return v2dirRdAsInSchool(v) * radians2degrees;
}
float v2dirDgFromToAsInSchool(float* v0, float* v1) { //as in school
    float v[2];
    for (int i = 0; i < 2; i++)
        v[i] = v1[i] - v0[i];
    return v2dirDgAsInSchool(v);
}

float v2dirDgDownCCW(float* v) { //as in school
    float aAsInSchool = v2dirRdAsInSchool(v) * radians2degrees;
    return angleDgNorm180(aAsInSchool+90);
}
float v2dirDgFromToDownCCW(float* v0, float* v1) { //as in school
    float v[2];
    for (int i = 0; i < 2; i++)
        v[i] = v1[i] - v0[i];
    return v2dirDgDownCCW(v);
}
float circumference(float r) {
    return r * PI * 2;
}
bool isInRange(float x, float x0, float x1) {
    float mn = fmin(x0, x1);
    float mx = fmax(x0, x1);
    if (x < mn)
        return false;
    if (x > mx)
        return false;
    return true;
}
float minimax(float x, float mn, float mx) {
    if (mn > mx)
        std::swap(mn, mx);
    if (x < mn)
        return mn;
    if (x > mx)
        return mx;
    return x;
}
float round4(float x) {
    x = round(x * 10000) / 10000;
    return x;
}