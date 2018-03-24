#ifndef BZF_VBO_HANDLER_H
#define BZF_VBO_HANDLER_H

#include "common.h"

/* system interface headers */
#include <list>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

/* common interface headers */
#include "bzfgl.h"

class VBOclient
{
public:
    virtual void initVBO() = 0;
};

class VBO_Handler;

class VBO_Manager
{
public:
    VBO_Manager();
    ~VBO_Manager();

    void registerHandler(VBO_Handler *handler);
    void unregisterHandler(VBO_Handler *handler);

    void registerClient(VBOclient *client);
    void unregisterClient(VBOclient *client);

    void reset();
private:
    static void initContext(void* data);
    static void freeContext(void* data);

    void destroyAll();
    void initAll();

    std::list<VBO_Handler*> handlerList;
    std::list<VBOclient*> clientList;

    static bool glContextReady;
};

class VBO_Handler
{
public:
    VBO_Handler(
        bool handleTexture,
        bool handleNormal,
        bool handleColor,
        int vertexSize,
        int indexSize);
    ~VBO_Handler();

    void vertexData(
        int index,
        int size,
        const GLfloat vertices[][3]);
    void vertexData(int index, int size, const GLfloat *vertices);
    void vertexData(int index, int size, const glm::vec3 vertices[]);
    void vertexData(int index, const std::vector<glm::vec3> vertices);
    void textureData(
        int index,
        int size,
        const GLfloat textures[][2]);
    void textureData(int index, int size, const GLfloat *textures);
    void textureData(int index, int size, const glm::vec2 textures[]);
    void textureData(int index, const std::vector<glm::vec2> textures);
    void normalData(
        int index,
        int size,
        const GLfloat normals[][3]);
    void normalData(int index, int size, const GLfloat *normals);
    void normalData(int index, int size, const glm::vec3 normals[]);
    void normalData(int index, const std::vector<glm::vec3> normals);
    void colorData(
        int index,
        int size,
        const GLfloat colors[][4]);
    void colorData(int index, int size, const GLfloat *colors);
    void colorData(int index, int size, const glm::vec4 colors[]);
    void colorData(int index, const std::vector<glm::vec4> colors);
    void elementData(
        int index,
        int size,
        const GLuint element[]);
    void drawElements(GLenum mode, GLsizei count, int index);
    static void globalArraysEnabling(bool enabled);
    void enableArrays();
    void enableArrays(bool texture, bool normal, bool color);
    int  reserveIndex(int vSize);

    int  vboAlloc(int Vsize);
    void vboFree(int vboIndex);

    void init();
    void destroy();
private:
    struct MemElement
    {
        int vboIndex;
        int Vsize;
    };

    std::list<MemElement> freeVBOList;
    std::list<MemElement> alloVBOList;

    void enableVertex();
    void enableTextures();
    void enableNormals();
    void enableColors();

    static void disableTextures();
    static void disableNormals();
    static void disableColors();

    bool handleTexture;
    bool handleNormal;
    bool handleColor;
    int  vertexSize;
    int  indexSize;

    GLuint verts;
    GLuint txcds;
    GLuint norms;
    GLuint colrs;
    GLuint elems;

    bool   savedTextureEnabled;
    bool   savedNormalEnabled;
    bool   savedColorEnabled;

    static GLuint actVerts;
    static GLuint actTxcds;
    static GLuint actNorms;
    static GLuint actColrs;

    static bool textureEnabled;
    static bool normalEnabled;
    static bool colorEnabled;

    static bool arrayEnabled;
    int arrayFillPoint;
    int indexFillPoint;
};

extern VBO_Manager vboManager;
extern VBO_Handler vboV;
extern VBO_Handler vboVC;
extern VBO_Handler vboVN;
extern VBO_Handler vboVT;
extern VBO_Handler vboVTN;
extern VBO_Handler vboVTC;
extern VBO_Handler vboVTNC;

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
