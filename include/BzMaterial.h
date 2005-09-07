/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* TetraBuilding:
 *	Encapsulates a tetrahederon in the game environment.
 */

#ifndef	BZ_MATERIAL_H
#define	BZ_MATERIAL_H

#include "common.h"
#include <string>
#include <vector>
#include <set>
#include <iostream>


class BzMaterial {

  public:
    BzMaterial();
    BzMaterial(const BzMaterial& material);
    ~BzMaterial();

    bool operator==(const BzMaterial& material) const;
    BzMaterial& operator=(const BzMaterial& material);

    void reset();

    void setReference();
    bool getReference() const;

    //
    // Parameter setting
    //

    bool setName(const std::string&);

    void setDynamicColor(int);
    void setAmbient(const float[4]);
    void setDiffuse(const float[4]);
    void setSpecular(const float[4]);
    void setEmission(const float[4]);
    void setShininess(const float);

    void setNoRadar(bool);
    void setNoCulling(bool);
    void setNoSorting(bool);
    void setAlphaThreshold(const float);

    // the following set()'s operate on the last added texture
    void addTexture(const std::string&);
    void setTexture(const std::string&);
    void setTextureLocal(int texid, const std::string& localname);
    void setTextureMatrix(int);
    void setCombineMode(int);
    void setUseTextureAlpha(bool);
    void setUseColorOnTexture(bool);
    void setUseSphereMap(bool);
    void clearTextures(); // remove all textures

    void addShader(const std::string&);
    void setShader(const std::string&);
    void clearShaders(); // remove all shaders

    //
    // Parameter getting
    //

    const std::string& getName() const;

    int getDynamicColor() const;
    const float* getAmbient() const;
    const float* getDiffuse() const;
    const float* getSpecular() const;
    const float* getEmission() const;
    float getShininess() const;

    bool getNoRadar() const;
    bool getNoCulling() const;
    bool getNoSorting() const;
    float getAlphaThreshold() const;

    int getTextureCount() const;
    const std::string& getTexture(int) const;
    const std::string& getTextureLocal(int) const;
    int getTextureMatrix(int) const;
    int getCombineMode(int) const;
    bool getUseTextureAlpha(int) const;
    bool getUseColorOnTexture(int) const;
    bool getUseSphereMap(int) const;

    int getShaderCount() const;
    const std::string& getShader(int) const;

    //
    // Utilities
    //

    int packSize() const;
    void *pack(void *) const;
    void *unpack(void *);

    void print(std::ostream& out, const std::string& indent) const;

    static const BzMaterial* getDefault();

    // data
  private:
    std::string name;

    bool referenced;

    int dynamicColor;
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float emission[4];
    float shininess;

    bool noRadar;
    bool noCulling;
    bool noSorting;
    float alphaThreshold;

    enum CombineModes {
      replace = 0,
      modulate,
      decal,
      blend,
      add,
      combine
    };
    int textureCount;
    typedef struct {
      std::string name;
      std::string localname;
      int matrix;
      int combineMode;
      bool useAlpha;
      bool useColor;
      bool useSphereMap;
    } TextureInfo;
    TextureInfo* textures;

    int shaderCount;
    typedef struct {
      std::string name;
    } ShaderInfo;
    ShaderInfo* shaders;

  private:
    static std::string nullString;
    static BzMaterial defaultMaterial;
};

inline const BzMaterial* BzMaterial::getDefault()
{
  return &defaultMaterial;
}

inline void BzMaterial::setReference()
{
  referenced = true;
  return;
}

inline bool BzMaterial::getReference() const
{
  return referenced;
}


class BzMaterialManager {
  public:
    BzMaterialManager();
    ~BzMaterialManager();
    void update();
    void clear();
    const BzMaterial* addMaterial(const BzMaterial* material);
    const BzMaterial* findMaterial(const std::string& name) const;
    const BzMaterial* getMaterial(int id) const;
    int getIndex(const BzMaterial* material) const;

    typedef std::set<std::string> TextureSet;
    void makeTextureList(TextureSet& set, bool referenced) const;
    void setTextureLocal(const std::string& url, const std::string& local);

    void* pack(void*);
    void* unpack(void*);
    int packSize();

    void print(std::ostream& out, const std::string& indent) const;
    void printReference(std::ostream& out, const BzMaterial* mat) const;

  private:
    std::vector<BzMaterial*> materials;
};


extern BzMaterialManager MATERIALMGR;


#endif // BZ_MATERIAL_H


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

