/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* BzMaterial:
 *	Encapsulates a material in the game environment.
 */

#ifndef	BZ_MATERIAL_H
#define	BZ_MATERIAL_H

#include "common.h"

#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>

#include "vectors.h"


class BzMaterial;
typedef std::set<const BzMaterial*> MaterialSet;
typedef std::map<const BzMaterial*,
		 const BzMaterial*> MaterialMap;


class BzMaterial {

  public:
    BzMaterial();
    BzMaterial(const BzMaterial& material);
    ~BzMaterial();

    bool operator==(const BzMaterial& material) const;
    BzMaterial& operator=(const BzMaterial& material);

    void reset();

    int  getID() const { return id; }
    void setID(int value) { id = value; }

    void setReference();
    bool getReference() const;

    //
    // Parameter setting
    //

    bool setName(const std::string&);
    bool addAlias(const std::string&);

    void setOrder(int);
    void setDynamicColor(int);
    void setAmbient(const fvec4&);
    void setDiffuse(const fvec4&);
    void setSpecular(const fvec4&);
    void setEmission(const fvec4&);
    void setShininess(const float);

    void setOccluder(bool);
    void setGroupAlpha(bool);
    void setNoLighting(bool);
    void setNoRadar(bool);
    void setNoShadow(bool);
    void setNoCulling(bool);
    void setNoSorting(bool);
    void setAlphaThreshold(const float);

    // the following set()'s operate on the last added texture
    void addTexture(const std::string&);
    void setTexture(const std::string&);
    void setTextureLocal(int texid, const std::string& localname);
    void setTextureMatrix(int);
    void setTextureAutoScale(const fvec2& scales);
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
    const std::vector<std::string>& getAliases() const;

    int getOrder() const;
    int getDynamicColor() const;
    const fvec4& getAmbient() const;
    const fvec4& getDiffuse() const;
    const fvec4& getSpecular() const;
    const fvec4& getEmission() const;
    float getShininess() const;

    bool getOccluder() const;
    bool getGroupAlpha() const;
    bool getNoRadar() const;
    bool getNoShadow() const;
    bool getNoCulling() const;
    bool getNoSorting() const;
    bool getNoLighting() const;
    float getAlphaThreshold() const;

    int getTextureCount() const;
    const std::string& getTexture(int) const;
    const std::string& getTextureLocal(int) const;
    int getTextureMatrix(int) const;
    int getCombineMode(int) const;
    bool getUseTextureAlpha(int) const;
    bool getUseColorOnTexture(int) const;
    bool getUseSphereMap(int) const;
    const fvec2& getTextureAutoScale(int) const;

    int getShaderCount() const;
    const std::string& getShader(int) const;

    //
    // Utilities
    //

    bool isInvisible() const;

    int packSize() const;
    void *pack(void *) const;
    void *unpack(void *);

    void print(std::ostream& out, const std::string& indent) const;
    void printMTL(std::ostream& out, const std::string& indent) const;

    static const BzMaterial* getDefault();

    static std::string convertTexture(const std::string& oldTex);

    // data
  private:
    std::string name;
    std::vector<std::string> aliases;

    int id;
    bool referenced;

    int order;

    int dynamicColor;
    fvec4 ambient;
    fvec4 diffuse;
    fvec4 specular;
    fvec4 emission;
    float shininess;

    bool occluder;
    bool groupAlpha;
    bool noRadar;
    bool noShadow;
    bool noCulling;
    bool noSorting;
    bool noLighting;
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
      fvec2 autoScale;
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

    int getCount() const { return (int)materials.size(); }

    typedef std::set<std::string> TextureSet;
    void makeTextureList(TextureSet& set, bool referenced) const;
    void setTextureLocal(const std::string& url, const std::string& local);

    void* pack(void*);
    void* unpack(void*);
    int packSize();

    void print(std::ostream& out, const std::string& indent) const;
    void printMTL(std::ostream& out, const std::string& indent) const;
    void printReference(std::ostream& out, const BzMaterial* mat) const;

  private:
    std::vector<BzMaterial*> materials;
};


extern BzMaterialManager MATERIALMGR;


#endif // BZ_MATERIAL_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
