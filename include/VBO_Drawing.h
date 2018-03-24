#ifndef VBO_DRAWING_H
#define VBO_DRAWING_H

/* interface headers */
#include "VBO_Handler.h"
#include "Singleton.h"

#define DRAWER (VBO_Drawing::instance())

class VBO_Drawing: public Singleton<VBO_Drawing>, VBOclient
{
public:
    void initVBO();

    void point();
    void simmetricRect();
    void asimmetricRect();
    void asimmetricTexturedRect();
    void simmetricTexturedRect();
    void verticalTexturedRect();
    void laggingLine();
    void leadingLine();
    void leadlagLine();
    void outlinedCircle20();
    void sphere12();
    void sphere32();
    void cylinder8();
    void cylinder10();
    void cylinder16();
    void cylinder24();
    void cylinder32();
    void disk8();
protected:
    friend class Singleton<VBO_Drawing>;
private:
    VBO_Drawing();
    virtual ~VBO_Drawing();

    int buildSphere(int slices);
    int buildCylinder(int slices);
    int buildDisk(int slices);

    int simmetricRectVBOIndex;
    int asimmetricRectVBOIndex;
    int simmetricTexturedRectVBOIndex;
    int asimmetricTexturedRectVBOIndex;
    int verticalTexturedRectVBOIndex;
    int shotLineVBOIndex;
    int outlineCircle20VBOIndex;
    int sphere12VBOIndex;
    int sphere32VBOIndex;
    int cylinder8VBOIndex;
    int cylinder10VBOIndex;
    int cylinder16VBOIndex;
    int cylinder24VBOIndex;
    int cylinder32VBOIndex;
    int disk8VBOIndex;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
