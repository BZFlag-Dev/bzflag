#include "VBO_Drawing.h"

/* initialize the singleton */
template <>
VBO_Drawing *Singleton<VBO_Drawing>::_instance = (VBO_Drawing*)0;

VBO_Drawing::VBO_Drawing()
{
    simmetricRectVBOIndex = -1;
    asimmetricRectVBOIndex = -1;
    simmetricTexturedRectVBOIndex = -1;
    asimmetricTexturedRectVBOIndex = -1;
    shotLineVBOIndex = -1;
    verticalTexturedRectVBOIndex = -1;
    outlineCircle20VBOIndex = -1;
    sphere12VBOIndex = -1;
    sphere32VBOIndex = -1;
    cylinder10VBOIndex = -1;
    cylinder16VBOIndex = -1;
    cylinder24VBOIndex = -1;
    cylinder32VBOIndex = -1;
    disk8VBOIndex = -1;
    vboManager.registerClient(this);
}

VBO_Drawing::~VBO_Drawing()
{
    vboV.vboFree(simmetricRectVBOIndex);
    vboV.vboFree(asimmetricRectVBOIndex);
    vboVT.vboFree(simmetricTexturedRectVBOIndex);
    vboVT.vboFree(asimmetricTexturedRectVBOIndex);
    vboV.vboFree(shotLineVBOIndex);
    vboVT.vboFree(verticalTexturedRectVBOIndex);
    vboV.vboFree(outlineCircle20VBOIndex);
    vboVN.vboFree(sphere12VBOIndex);
    vboVN.vboFree(sphere32VBOIndex);
    vboVN.vboFree(cylinder8VBOIndex);
    vboVN.vboFree(cylinder10VBOIndex);
    vboVN.vboFree(cylinder16VBOIndex);
    vboVN.vboFree(cylinder24VBOIndex);
    vboVN.vboFree(cylinder32VBOIndex);
    vboV.vboFree(disk8VBOIndex);
    vboManager.unregisterClient(this);
}

void VBO_Drawing::initVBO()
{
    glm::vec3 vertex[20];
    glm::vec2 textur[4];

    vertex[0] = glm::vec3(-1.0f, -1.0f, 0.0f);
    vertex[1] = glm::vec3(+1.0f, -1.0f, 0.0f);
    vertex[2] = glm::vec3(-1.0f, +1.0f, 0.0f);
    vertex[3] = glm::vec3(+1.0f, +1.0f, 0.0f);
    simmetricRectVBOIndex = vboV.vboAlloc(4);
    vboV.vertexData(simmetricRectVBOIndex, 4, vertex);

    vertex[0] = glm::vec3(0.0f, 0.0f, 0.0f);
    vertex[1] = glm::vec3(1.0f, 0.0f, 0.0f);
    vertex[2] = glm::vec3(0.0f, 1.0f, 0.0f);
    vertex[3] = glm::vec3(1.0f, 1.0f, 0.0f);
    asimmetricRectVBOIndex = vboV.vboAlloc(4);
    vboV.vertexData(asimmetricRectVBOIndex, 4, vertex);

    textur[0] = glm::vec2(0.0f, 0.0f);
    textur[1] = glm::vec2(1.0f, 0.0f);
    textur[2] = glm::vec2(0.0f, 1.0f);
    textur[3] = glm::vec2(1.0f, 1.0f);

    vertex[0] = glm::vec3(-1.0f, -1.0f, 0.0f);
    vertex[1] = glm::vec3( 1.0f, -1.0f, 0.0f);
    vertex[2] = glm::vec3(-1.0f,  1.0f, 0.0f);
    vertex[3] = glm::vec3( 1.0f,  1.0f, 0.0f);

    simmetricTexturedRectVBOIndex = vboVT.vboAlloc(4);
    vboVT.textureData(simmetricTexturedRectVBOIndex, 4, textur);
    vboVT.vertexData(simmetricTexturedRectVBOIndex, 4, vertex);

    vertex[0] = glm::vec3(0.0f, 0.0f, 0.0f);
    vertex[1] = glm::vec3(1.0f, 0.0f, 0.0f);
    vertex[2] = glm::vec3(0.0f, 1.0f, 0.0f);
    vertex[3] = glm::vec3(1.0f, 1.0f, 0.0f);

    asimmetricTexturedRectVBOIndex = vboVT.vboAlloc(4);
    vboVT.textureData(asimmetricTexturedRectVBOIndex, 4, textur);
    vboVT.vertexData(asimmetricTexturedRectVBOIndex, 4, vertex);

    vertex[0] = glm::vec3( 0.0f,  0.0f, 0.0f);
    vertex[1] = glm::vec3( 1.0f,  1.0f, 0.0f);
    vertex[2] = glm::vec3( 0.0f,  0.0f, 0.0f);
    vertex[3] = glm::vec3(-1.0f, -1.0f, 0.0f);
    shotLineVBOIndex = vboV.vboAlloc(4);
    vboV.vertexData(shotLineVBOIndex, 4, vertex);

    textur[0] = glm::vec2(0.0f, 1.0f);
    textur[1] = glm::vec2(0.0f, 0.0f);
    textur[2] = glm::vec2(1.0f, 1.0f);
    textur[3] = glm::vec2(1.0f, 0.0f);

    vertex[0] = glm::vec3(0.0f, 0.0f, +1.0f);
    vertex[1] = glm::vec3(0.0f, 1.0f, +1.0f);
    vertex[2] = glm::vec3(0.0f, 0.0f, -1.0f);
    vertex[3] = glm::vec3(0.0f, 1.0f, -1.0f);

    verticalTexturedRectVBOIndex = vboVT.vboAlloc(4);
    vboVT.textureData(verticalTexturedRectVBOIndex, 4, textur);
    vboVT.vertexData(verticalTexturedRectVBOIndex, 4, vertex);

    // draw circle of current radius
    static const int sides = 20;
    for (int i = 0; i < sides; i++)
    {
        const float angle = (float)(2.0 * M_PI * double(i) / double(sides));
        vertex[i] = glm::vec3(cosf(angle), sinf(angle), 0.0f);
    }
    outlineCircle20VBOIndex = vboV.vboAlloc(sides);
    vboV.vertexData(outlineCircle20VBOIndex, sides, vertex);

    sphere12VBOIndex = buildSphere(12);
    sphere32VBOIndex = buildSphere(32);
    cylinder8VBOIndex = buildCylinder(8);
    cylinder10VBOIndex = buildCylinder(10);
    cylinder16VBOIndex = buildCylinder(16);
    cylinder24VBOIndex = buildCylinder(24);
    cylinder32VBOIndex = buildCylinder(32);
    disk8VBOIndex = buildDisk(8);
}

int VBO_Drawing::buildSphere(int slices)
{
    const int maxSlices = 32;
    glm::vec3 vertex[2 * maxSlices * (maxSlices + 1)];
    glm::vec3 *pVertex = vertex;
    int i,j;
    float zLow, zHigh;
    float sintemp1, sintemp2;
    float angle;
    float x1, y1, x2, y2;

    angle    = (float)(M_PI / slices);
    zHigh    = cos(angle);
    sintemp2 = sin(angle);
    *pVertex++ = glm::vec3(0.0f, sintemp2, zHigh);
    *pVertex++ = glm::vec3(0.0f, 0.0f,     1.0f);
    for (i = 1; i < slices; i++)
    {
        angle = (float)(2.0 * M_PI * i / slices);
        x1 = sin(angle);
        y1 = cos(angle);
        x1 *= sintemp2;
        y1 *= sintemp2;
        *pVertex++ = glm::vec3(x1,   y1,   zHigh);
        *pVertex++ = glm::vec3(0.0f, 0.0f, 1.0f);
    }
    *pVertex++ = glm::vec3(0.0f, sintemp2, zHigh);
    *pVertex++ = glm::vec3(0.0f, 0.0f,     1.0f);
    for (j = 2; j < slices; j++)
    {
        zLow     = zHigh;
        sintemp1 = sintemp2;
        angle    = (float)(M_PI * j / slices);
        zHigh    = cos(angle);
        sintemp2 = sin(angle);
        *pVertex++ = glm::vec3(0.0f, sintemp2, zHigh);
        *pVertex++ = glm::vec3(0.0f, sintemp1, zLow);
        for (i = 1; i < slices; i++)
        {
            angle = (float)(2 * M_PI * i / slices);
            x1 = sin(angle);
            y1 = cos(angle);
            x2 = x1;
            y2 = y1;
            x1 *= sintemp2;
            y1 *= sintemp2;
            x2 *= sintemp1;
            y2 *= sintemp1;
            *pVertex++ = glm::vec3(x1, y1, zHigh);
            *pVertex++ = glm::vec3(x2, y2, zLow);
        }
        *pVertex++ = glm::vec3(0.0f, sintemp2, zHigh);
        *pVertex++ = glm::vec3(0.0f, sintemp1, zLow);
    }
    zLow     = zHigh;
    sintemp1 = sintemp2;

    *pVertex++ = glm::vec3(0.0f, 0.0f,     -1.0f);
    *pVertex++ = glm::vec3(0.0f, sintemp1, zLow);
    for (i = 1; i < slices; i++)
    {
        angle = (float)(2 * M_PI * i / slices);
        x2 = sin(angle);
        y2 = cos(angle);
        x2 *= sintemp1;
        y2 *= sintemp1;

        *pVertex++ = glm::vec3(0.0f, 0.0f, -1.0f);
        *pVertex++ = glm::vec3(x2,   y2,   zLow);
    }
    *pVertex++ = glm::vec3(0.0f, 0.0f,     -1.0f);
    *pVertex++ = glm::vec3(0.0f, sintemp1, zLow);
    int vertexCount = 2 * slices * (slices + 1);
    int vboIndex = vboVN.vboAlloc(vertexCount);
    vboVN.normalData(vboIndex, vertexCount, vertex);
    vboVN.vertexData(vboIndex, vertexCount, vertex);
    return vboIndex;
}

int VBO_Drawing::buildCylinder(int slices)
{
    const int maxSlices = 32;
    glm::vec3 vertex[2 * (maxSlices + 1)];
    glm::vec3 normal[2 * (maxSlices + 1)];
    glm::vec3 *pVertex = vertex;
    glm::vec3 *pNormal = normal;
    int   i;
    float angle;
    float sinCache;
    float cosCache;
    glm::vec2 currentVartex;

    currentVartex = glm::vec2(0.0f, 1.0f);
    *pNormal++ = glm::vec3(currentVartex, 0.0f);
    *pVertex++ = glm::vec3(currentVartex, 0.0f);
    *pNormal++ = glm::vec3(currentVartex, 0.0f);
    *pVertex++ = glm::vec3(currentVartex, 1.0f);
    for (i = 1; i < slices; i++)
    {
        angle = (float)(2 * M_PI * i / slices);
        sinCache  = sin(angle);
        cosCache  = cos(angle);
        currentVartex = glm::vec2(sinCache, cosCache);
        *pNormal++ = glm::vec3(currentVartex, 0.0f);
        *pVertex++ = glm::vec3(currentVartex, 0.0f);
        *pNormal++ = glm::vec3(currentVartex, 0.0f);
        *pVertex++ = glm::vec3(currentVartex, 1.0f);
    }
    currentVartex = glm::vec2(0.0f, 1.0f);
    *pNormal++ = glm::vec3(currentVartex, 0.0f);
    *pVertex++ = glm::vec3(currentVartex, 0.0f);
    *pNormal++ = glm::vec3(currentVartex, 0.0f);
    *pVertex++ = glm::vec3(currentVartex, 1.0f);
    int vertexCount = 2 * (slices + 1);
    int vboIndex = vboVN.vboAlloc(vertexCount);
    vboVN.normalData(vboIndex, vertexCount, normal);
    vboVN.vertexData(vboIndex, vertexCount, vertex);
    return vboIndex;
}

int VBO_Drawing::buildDisk(int slices)
{
    const int maxSlices = 8;
    glm::vec3 vertex[maxSlices + 2];
    glm::vec3 *pVertex = vertex;
    /* Triangle strip for inner polygons */
    *pVertex++ = glm::vec3(0.0f, 0.0f, 0.0f);
    *pVertex++ = glm::vec3(0.0f, 1.0f, 0.0f);
    for (int i = slices - 1; i > 0; i--)
    {
        float angle = (float)(2 * M_PI * i / slices);
        *pVertex++ = glm::vec3(sin(angle), cos(angle), 0.0f);
    }
    *pVertex++ = glm::vec3(0.0f, 1.0f, 0.0f);
    int vertexCount = slices + 2;
    int vboIndex = vboV.vboAlloc(vertexCount);
    vboV.vertexData(vboIndex, vertexCount, vertex);
    return vboIndex;
}

void VBO_Drawing::simmetricRect()
{
    vboV.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, simmetricRectVBOIndex, 4);
}

void VBO_Drawing::asimmetricRect()
{
    vboV.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, asimmetricRectVBOIndex, 4);
}

void VBO_Drawing::simmetricTexturedRect()
{
    vboVT.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, simmetricTexturedRectVBOIndex, 4);
}

void VBO_Drawing::asimmetricTexturedRect()
{
    vboVT.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, asimmetricTexturedRectVBOIndex, 4);
}

void VBO_Drawing::verticalTexturedRect()
{
    vboVT.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, verticalTexturedRectVBOIndex, 4);
}

void VBO_Drawing::point()
{
    vboV.enableArrays();
    glDrawArrays(GL_POINTS, shotLineVBOIndex, 1);
}

void VBO_Drawing::laggingLine()
{
    vboV.enableArrays();
    glDrawArrays(GL_LINES, shotLineVBOIndex + 2, 2);
}

void VBO_Drawing::leadingLine()
{
    vboV.enableArrays();
    glDrawArrays(GL_LINES, shotLineVBOIndex, 2);
}

void VBO_Drawing::leadlagLine()
{
    vboV.enableArrays();
    glDrawArrays(GL_LINES, shotLineVBOIndex, 4);
}

void VBO_Drawing::outlinedCircle20()
{
    vboV.enableArrays();
    glDrawArrays(GL_LINE_LOOP, outlineCircle20VBOIndex, 20);
}

void VBO_Drawing::sphere12()
{
    const int vertexCount = 2 * 12 * 13;
    vboVN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, sphere12VBOIndex, vertexCount);
}

void VBO_Drawing::sphere32()
{
    const int vertexCount = 2 * 32 * 33;
    vboVN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, sphere32VBOIndex, vertexCount);
}

void VBO_Drawing::cylinder8()
{
    const int vertexCount = 2 * 9;
    vboVN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, cylinder8VBOIndex, vertexCount);
}

void VBO_Drawing::cylinder10()
{
    const int vertexCount = 2 * 11;
    vboVN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, cylinder10VBOIndex, vertexCount);
}

void VBO_Drawing::cylinder16()
{
    const int vertexCount = 2 * 17;
    vboVN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, cylinder16VBOIndex, vertexCount);
}

void VBO_Drawing::cylinder24()
{
    const int vertexCount = 2 * 25;
    vboVN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, cylinder24VBOIndex, vertexCount);
}

void VBO_Drawing::cylinder32()
{
    const int vertexCount = 2 * 33;
    vboVN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, cylinder32VBOIndex, vertexCount);
}

void VBO_Drawing::disk8()
{
    vboV.enableArrays();
    glDrawArrays(GL_TRIANGLE_FAN, disk8VBOIndex, 10);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
