/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* TetraBuilding:
 *	Encapsulates a tetrahederon in the game environment.
 */

#ifndef	BZ_MATERIAL_H
#define	BZ_MATERIAL_H

#include "common.h"
#include <string>
#include <vector>
#include <iostream>


class BzMaterial {

  public:
    BzMaterial();
    BzMaterial(const BzMaterial& material);

    bool operator==(const BzMaterial& material) const;
    BzMaterial& operator=(const BzMaterial& material);

    void reset();

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

    // the following set()'s operate on the last added texture
    void addTexture(const std::string&);
    void setTexture(const std::string&);
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
    
    int getTextureCount() const;
    const std::string& getTexture(int) const;
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

    void *pack(void *);
    void *unpack(void *);
    int packSize();

    void print(std::ostream& out, int level) const;
    
    static const BzMaterial* getDefault();
    
    // data
  private:
    std::string name;

    int dynamicColor;
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float emission[4];
    float shininess;

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

    void* pack(void*);
    void* unpack(void*);
    int packSize();

    void print(std::ostream& out, int level) const;
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

