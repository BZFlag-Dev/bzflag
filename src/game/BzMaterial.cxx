/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// Interface
#include "BzMaterial.h"

// System headers
#include <string.h>
#include <glm/gtc/type_ptr.hpp>

// Common headers
#include "TextureMatrix.h"
#include "DynamicColor.h"
#include "Pack.h"


/****************************************************************************/
//
// BzMaterialManager
//

BzMaterialManager MATERIALMGR;


BzMaterialManager::BzMaterialManager()
{
}


BzMaterialManager::~BzMaterialManager()
{
    clear();
}


void BzMaterialManager::clear()
{
    for (unsigned int i = 0; i < materials.size(); i++)
        delete materials[i];
    materials.clear();
}


const BzMaterial* BzMaterialManager::addMaterial(const BzMaterial* material)
{
    for (unsigned int i = 0; i < materials.size(); i++)
    {
        if (*material == *(materials[i]))
        {
            const std::string& name = material->getName();
            if (name.size() > 0)
                materials[i]->addAlias(name);
            return materials[i];
        }
    }
    BzMaterial* newMat = new BzMaterial(*material);
    if (findMaterial(newMat->getName()) != nullptr)
        newMat->setName("");
    materials.push_back(newMat);
    return newMat;
}


const BzMaterial* BzMaterialManager::findMaterial(const std::string& target) const
{
    if (target.size() <= 0)
        return nullptr;
    else if ((target[0] >= '0') && (target[0] <= '9'))
    {
        int index = atoi (target.c_str());
        if ((index < 0) || (index >= (int)materials.size()))
            return nullptr;
        else
            return materials[index];
    }
    else
    {
        for (unsigned int i = 0; i < materials.size(); i++)
        {
            const BzMaterial* mat = materials[i];
            // check the base name
            if (target == mat->getName())
                return mat;
            // check the aliases
            const std::vector<std::string>& aliases = mat->getAliases();
            for (unsigned int j = 0; j < aliases.size(); j++)
            {
                if (target == aliases[j])
                    return mat;
            }
        }
        return nullptr;
    }
}


const BzMaterial* BzMaterialManager::getMaterial(int id) const
{
    if ((id < 0) || (id >= (int)materials.size()))
        return BzMaterial::getDefault();
    return materials[id];
}


int BzMaterialManager::getIndex(const BzMaterial* material) const
{
    for (unsigned int i = 0; i < materials.size(); i++)
    {
        if (material == materials[i])
            return i;
    }
    return -1;
}


void* BzMaterialManager::pack(void* buf)
{
    buf = nboPackUInt(buf, (unsigned int)materials.size());
    for (unsigned int i = 0; i < materials.size(); i++)
        buf = materials[i]->pack(buf);

    return buf;
}


const void* BzMaterialManager::unpack(const void* buf)
{
    unsigned int i;
    uint32_t count;
    buf = nboUnpackUInt (buf, count);
    for (i = 0; i < count; i++)
    {
        BzMaterial* mat = new BzMaterial;
        buf = mat->unpack(buf);
        materials.push_back(mat);
    }
    return buf;
}


int BzMaterialManager::packSize()
{
    int fullSize = sizeof (uint32_t);
    for (unsigned int i = 0; i < materials.size(); i++)
        fullSize += materials[i]->packSize();
    return fullSize;
}


void BzMaterialManager::print(std::ostream& out, const std::string& indent) const
{
    for (unsigned int i = 0; i < materials.size(); i++)
        materials[i]->print(out, indent);
    return;
}


void BzMaterialManager::printMTL(std::ostream& out, const std::string& indent) const
{
    for (unsigned int i = 0; i < materials.size(); i++)
        materials[i]->printMTL(out, indent);
}


void BzMaterialManager::printReference(std::ostream& out, const BzMaterial* mat) const
{
    if (mat == nullptr)
    {
        out << "-1";
        return;
    }

    int index = getIndex(mat);
    if (index == -1)
        out << "-1";
    else if (mat->getName().size() > 0)
        out << mat->getName();
    else
        out << index;
}


void BzMaterialManager::makeTextureList(TextureSet& set, bool referenced) const
{
    set.clear();
    for (unsigned int i = 0; i < materials.size(); i++)
    {
        const BzMaterial* mat = materials[i];
        for (int j = 0; j < mat->getTextureCount(); j++)
        {
            if (mat->getReference() || !referenced)
                set.insert(mat->getTexture(j));
        }

        if (mat->hasNormalMap())
            set.insert(mat->getNormalMap());

        if (mat->hasSpecularMap())
            set.insert(mat->getSpecularMap());
    }
}


void BzMaterialManager::setTextureLocal(const std::string& url,
                                        const std::string& local)
{
    for (unsigned int i = 0; i < materials.size(); i++)
    {
        BzMaterial* mat = materials[i];
        for (int j = 0; j < mat->getTextureCount(); j++)
        {
            if (mat->getTexture(j) == url)
                mat->setTextureLocal(j, local);
        }
    }
}


/****************************************************************************/
//
// BzMaterial
//

BzMaterial BzMaterial::defaultMaterial;
std::string BzMaterial::nullString = "";


void BzMaterial::reset()
{
    dynamicColor = -1;
    const float defAmbient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    const auto defDiffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    const float defSpecular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    const float defEmission[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    memcpy (ambient, defAmbient, sizeof(ambient));
    diffuse = defDiffuse;
    memcpy (specular, defSpecular, sizeof(specular));
    memcpy (emission, defEmission, sizeof(emission));

    shininess = 0.0f;
    alphaThreshold = 0.0f;
    occluder = false;
    groupAlpha = false;
    noRadar = false;
    noShadow = false;
    noCulling = false;
    noSorting = false;
    noLighting = false;

    delete[] textures;
    textures = nullptr;
    textureCount = 0;

    delete[] shaders;
    shaders = nullptr;
    shaderCount = 0;

    referenced = false;
}


BzMaterial::BzMaterial()
{
    textures = nullptr;
    shaders = nullptr;
    reset();
}


BzMaterial::~BzMaterial()
{
    delete[] textures;
    delete[] shaders;

    delete (normalMap);
    delete (specularMap);
}


BzMaterial::BzMaterial(const BzMaterial& m)
{
    textures = nullptr;
    shaders = nullptr;
    *this = m;
}


BzMaterial& BzMaterial::operator=(const BzMaterial& m)
{
    int i;

    referenced = false;

    name = m.name;
    aliases = m.aliases;

    dynamicColor = m.dynamicColor;
    memcpy (ambient, m.ambient, sizeof(ambient));
    diffuse = m.diffuse;
    memcpy (specular, m.specular, sizeof(specular));
    memcpy (emission, m.emission, sizeof(emission));
    shininess = m.shininess;
    alphaThreshold = m.alphaThreshold;
    occluder = m.occluder;
    groupAlpha = m.groupAlpha;
    noRadar = m.noRadar;
    noShadow = m.noShadow;
    noCulling = m.noCulling;
    noSorting = m.noSorting;
    noLighting = m.noLighting;

    delete[] textures;
    textureCount = m.textureCount;
    if (textureCount > 0)
        textures = new TextureInfo[textureCount];
    else
        textures = nullptr;
    for (i = 0; i < textureCount; i++)
        textures[i] = m.textures[i];

    delete[] shaders;
    shaderCount = m.shaderCount;
    if (shaderCount > 0)
        shaders = new ShaderInfo[shaderCount];
    else
        shaders = nullptr;
    for (i = 0; i < shaderCount; i++)
        shaders[i] = m.shaders[i];

    return *this;
}


bool BzMaterial::operator==(const BzMaterial& m) const
{
    int i;

    if ((dynamicColor != m.dynamicColor) ||
            (memcmp (ambient, m.ambient, sizeof(float[4])) != 0) ||
            (diffuse != m.diffuse) ||
            (memcmp (specular, m.specular, sizeof(float[4])) != 0) ||
            (memcmp (emission, m.emission, sizeof(float[4])) != 0) ||
            (shininess != m.shininess) || (alphaThreshold != m.alphaThreshold) ||
            (occluder != m.occluder) || (groupAlpha != m.groupAlpha) ||
            (noRadar != m.noRadar) || (noShadow != m.noShadow) ||
            (noCulling != m.noCulling) || (noSorting != m.noSorting) ||
            (noLighting != m.noLighting))
        return false;

    if (textureCount != m.textureCount)
        return false;
    for (i = 0; i < textureCount; i++)
    {
        if ((textures[i].name != m.textures[i].name) ||
                (textures[i].matrix != m.textures[i].matrix) ||
                (textures[i].combineMode != m.textures[i].combineMode) ||
                (textures[i].useAlpha != m.textures[i].useAlpha) ||
                (textures[i].useColor != m.textures[i].useColor) ||
                (textures[i].useSphereMap != m.textures[i].useSphereMap))
            return false;
    }

    if (shaderCount != m.shaderCount)
        return false;
    for (i = 0; i < shaderCount; i++)
    {
        if (shaders[i].name != m.shaders[i].name)
            return false;
    }

    return true;
}


static void* pack4Float(void *buf, const float values[4])
{
    int i;
    for (i = 0; i < 4; i++)
        buf = nboPackFloat(buf, values[i]);
    return buf;
}


static const void* unpack4Float(const void *buf, float values[4])
{
    int i;
    for (i = 0; i < 4; i++)
        buf = nboUnpackFloat(buf, values[i]);
    return buf;
}


void* BzMaterial::pack(void* buf) const
{
    int i;

    buf = nboPackStdString(buf, name);

    uint8_t modeByte = 0;
    if (noCulling)    modeByte |= (1 << 0);
    if (noSorting)    modeByte |= (1 << 1);
    if (noRadar)      modeByte |= (1 << 2);
    if (noShadow)     modeByte |= (1 << 3);
    if (occluder)  modeByte |= (1 << 4);
    if (groupAlpha)       modeByte |= (1 << 5);
    if (noLighting)       modeByte |= (1 << 6);
    buf = nboPackUByte(buf, modeByte);

    buf = nboPackInt(buf, dynamicColor);
    buf = pack4Float(buf, ambient);
    buf = pack4Float(buf, glm::value_ptr(diffuse));
    buf = pack4Float(buf, specular);
    buf = pack4Float(buf, emission);
    buf = nboPackFloat(buf, shininess);
    buf = nboPackFloat(buf, alphaThreshold);

    buf = nboPackUByte(buf, textureCount);
    for (i = 0; i < textureCount; i++)
    {
        const TextureInfo* texinfo = &textures[i];

        buf = nboPackStdString(buf, texinfo->name);
        buf = nboPackInt(buf, texinfo->matrix);
        buf = nboPackInt(buf, texinfo->combineMode);
        unsigned char stateByte = 0;
        if (texinfo->useAlpha)
            stateByte = stateByte | (1 << 0);
        if (texinfo->useColor)
            stateByte = stateByte | (1 << 1);
        if (texinfo->useSphereMap)
            stateByte = stateByte | (1 << 2);
        buf = nboPackUByte(buf, stateByte);
    }

    buf = nboPackUByte(buf, shaderCount);
    for (i = 0; i < shaderCount; i++)
        buf = nboPackStdString(buf, shaders[i].name);

    if (normalMap != nullptr)
        buf = nboPackStdString(buf, normalMap->name);
    else
        buf = nboPackStdString(buf, std::string(""));

    if (specularMap != nullptr)
        buf = nboPackStdString(buf, specularMap->name);
    else
        buf = nboPackStdString(buf, std::string(""));

    return buf;
}


const void* BzMaterial::unpack(const void* buf)
{
    int i;
    int32_t inTmp;

    buf = nboUnpackStdString(buf, name);

    uint8_t modeByte;
    buf = nboUnpackUByte(buf, modeByte);
    noCulling = (modeByte & (1 << 0)) != 0;
    noSorting = (modeByte & (1 << 1)) != 0;
    noRadar   = (modeByte & (1 << 2)) != 0;
    noShadow  = (modeByte & (1 << 3)) != 0;
    occluder  = (modeByte & (1 << 4)) != 0;
    groupAlpha    = (modeByte & (1 << 5)) != 0;
    noLighting    = (modeByte & (1 << 6)) != 0;

    buf = nboUnpackInt(buf, inTmp);
    dynamicColor = int(inTmp);
    buf = unpack4Float(buf, ambient);
    buf = unpack4Float(buf, glm::value_ptr(diffuse));
    buf = unpack4Float(buf, specular);
    buf = unpack4Float(buf, emission);
    buf = nboUnpackFloat(buf, shininess);
    buf = nboUnpackFloat(buf, alphaThreshold);

    unsigned char tCount;
    buf = nboUnpackUByte(buf, tCount);
    textureCount = tCount;
    textures = new TextureInfo[textureCount];
    for (i = 0; i < textureCount; i++)
    {
        TextureInfo* texinfo = &textures[i];
        buf = nboUnpackStdString(buf, texinfo->name);
        texinfo->localname = texinfo->name;
        buf = nboUnpackInt(buf, inTmp);
        texinfo->matrix = int(inTmp);
        buf = nboUnpackInt(buf, inTmp);
        texinfo->combineMode = int(inTmp);
        texinfo->useAlpha = false;
        texinfo->useColor = false;
        texinfo->useSphereMap = false;
        unsigned char stateByte;
        buf = nboUnpackUByte(buf, stateByte);
        if (stateByte & (1 << 0))
            texinfo->useAlpha = true;
        if (stateByte & (1 << 1))
            texinfo->useColor = true;
        if (stateByte & (1 << 2))
            texinfo->useSphereMap = true;
    }

    unsigned char sCount;
    buf = nboUnpackUByte(buf, sCount);
    shaderCount = sCount;
    shaders = new ShaderInfo[shaderCount];
    for (i = 0; i < shaderCount; i++)
        buf = nboUnpackStdString(buf, shaders[i].name);

    std::string normalName;
    buf = nboUnpackStdString(buf, normalName);

    std::string specularName;
    buf = nboUnpackStdString(buf, specularName);

    delete (normalMap);
    normalMap = nullptr;
    if (normalName != "")
    {
        normalMap = new TextureInfo();
        normalMap->localname = normalMap->name = normalName;;
    }

    delete (specularMap);
    specularMap = nullptr;
    if (specularName != "")
    {
        specularMap = new TextureInfo();
        specularMap->localname = specularMap->name = specularName;;
    }

    return buf;
}


int BzMaterial::packSize() const
{
    int i;

    const int modeSize = sizeof(uint8_t);

    const int colorSize = sizeof(int32_t) + (4 * sizeof(float[4])) +
                          sizeof(float) + sizeof(float);

    int textureSize = sizeof(unsigned char);
    for (i = 0; i < textureCount; i++)
    {
        textureSize += nboStdStringPackSize(textures[i].name);
        textureSize += sizeof(int32_t);
        textureSize += sizeof(int32_t);
        textureSize += sizeof(unsigned char);
    }

    int shaderSize = sizeof(unsigned char);
    for (i = 0; i < shaderCount; i++)
        shaderSize += nboStdStringPackSize(shaders[i].name);

    int mapsSize = 0;
    if (normalMap != nullptr)
        mapsSize += nboStdStringPackSize(normalMap->name);
    else
        mapsSize += nboStdStringPackSize(std::string(""));

    if (specularMap != nullptr)
        mapsSize += nboStdStringPackSize(specularMap->name);
    else
        mapsSize += nboStdStringPackSize(std::string(""));

    return nboStdStringPackSize(name) + modeSize + colorSize + textureSize + shaderSize + mapsSize;
}


static void printColor(std::ostream& out, const char *name,
                       const float color[4], const float reference[4])
{
    if (memcmp(color, reference, sizeof(float[4])) != 0)
    {
        out << name << color[0] << " " << color[1] << " "
            << color[2] << " " << color[3] << std::endl;
    }
}


void BzMaterial::print(std::ostream& out, const std::string& indent) const
{
    int i;

    out << indent << "material" << std::endl;

    if (name.size() > 0)
        out << indent << "  name " << name << std::endl;

    if (dynamicColor != defaultMaterial.dynamicColor)
    {
        out << indent << "  dyncol ";
        const DynamicColor* dyncol = DYNCOLORMGR.getColor(dynamicColor);
        if ((dyncol != nullptr) && (dyncol->getName().size() > 0))
            out << dyncol->getName();
        else
            out << dynamicColor;

        out << std::endl;
    }
    printColor(out, "  ambient ",  ambient,  defaultMaterial.ambient);
    printColor(out, "  diffuse ",  glm::value_ptr(diffuse),  glm::value_ptr(defaultMaterial.diffuse));
    printColor(out, "  specular ", specular, defaultMaterial.specular);
    printColor(out, "  emission ", emission, defaultMaterial.emission);

    if (shininess != defaultMaterial.shininess)
        out << indent << "  shininess " << shininess << std::endl;
    if (alphaThreshold != defaultMaterial.alphaThreshold)
        out << indent << "  alphathresh " << alphaThreshold << std::endl;
    if (occluder)
        out << indent << "  occluder" << std::endl;
    if (groupAlpha)
        out << indent << "  groupAlpha" << std::endl;
    if (noRadar)
        out << indent << "  noradar" << std::endl;
    if (noShadow)
        out << indent << "  noshadow" << std::endl;
    if (noCulling)
        out << indent << "  noculling" << std::endl;
    if (noSorting)
        out << indent << "  nosorting" << std::endl;
    if (noLighting)
        out << indent << "  nolighting" << std::endl;

    for (i = 0; i < textureCount; i++)
    {
        const TextureInfo* texinfo = &textures[i];
        out << indent << "  addtexture " << texinfo->name << std::endl;
        if (texinfo->matrix != -1)
        {
            out << indent << "    texmat ";
            const TextureMatrix* texmat = TEXMATRIXMGR.getMatrix(texinfo->matrix);
            if ((texmat != nullptr) && (texmat->getName().size() > 0))
                out << texmat->getName();
            else
                out << texinfo->matrix;
            out << std::endl;
        }

        if (!texinfo->useAlpha)
            out << indent << "    notexalpha" << std::endl;
        if (!texinfo->useColor)
            out << indent << "    notexcolor" << std::endl;
        if (texinfo->useSphereMap)
            out << indent << "    spheremap" << std::endl;
    }

    for (i = 0; i < shaderCount; i++)
    {
        const ShaderInfo* shdinfo = &shaders[i];
        out << indent << "  addshader " << shdinfo->name << std::endl;
    }

    out << indent << "end" << std::endl << std::endl;

}


void BzMaterial::printMTL(std::ostream& out, const std::string& UNUSED(indent)) const
{
    out << "newmtl ";
    if (name.size() > 0)
        out << name << std::endl;
    else
        out << MATERIALMGR.getIndex(this) << std::endl;
    if (noLighting)
        out << "illum 0" << std::endl;
    else
        out << "illum 2" << std::endl;

    out << "d " << diffuse[3] << std::endl;

    const float* c;

    c = ambient; // not really used
    out << "#Ka " << c[0] << " " << c[1] << " " << c[2] << std::endl;
    c = glm::value_ptr(diffuse);
    out << "Kd " << c[0] << " " << c[1] << " " << c[2] << std::endl;
    c = emission;
    out << "Ke " << c[0] << " " << c[1] << " " << c[2] << std::endl;
    c = specular;
    out << "Ks " << c[0] << " " << c[1] << " " << c[2] << std::endl;
    out << "Ns " << (1000.0f * (shininess / 128.0f)) << std::endl;
    if (textureCount > 0)
    {
        const TextureInfo* ti = &textures[0];
        const unsigned int nlen = (unsigned int)ti->name.size();
        if (nlen > 0)
        {
            std::string texname = ti->name;
            const char* cname = texname.c_str();
            if ((nlen < 4) || (strcasecmp(cname + (nlen - 4), ".png") != 0))
                texname += ".png";
            out << "map_Kd " << texname << std::endl;
        }
    }
    out << std::endl;
}


/****************************************************************************/
//
// Parameter setting
//

bool BzMaterial::setName(const std::string& matname)
{
    if (matname.size() <= 0)
    {
        name = "";
        return false;
    }
    else if ((matname[0] >= '0') && (matname[0] <= '9'))
    {
        name = "";
        return false;
    }
    else
        name = matname;
    return true;
}

bool BzMaterial::addAlias(const std::string& alias)
{
    if (alias.size() <= 0)
    {
        name = "";
        return false;
    }
    else if ((alias[0] >= '0') && (alias[0] <= '9'))
    {
        name = "";
        return false;
    }
    else
    {
        for ( unsigned int i = 0; i < (unsigned int)aliases.size(); i++)
        {
            if ( aliases[i] == alias )
                return true;
        }
        aliases.push_back(alias); // only add it if it's new
    }
    return true;
}

void BzMaterial::setDynamicColor(int dyncol)
{
    dynamicColor = dyncol;
}

void BzMaterial::setAmbient(const float color[4])
{
    memcpy (ambient, color, sizeof(float[4]));
}

void BzMaterial::setDiffuse(const float color[4])
{
    diffuse = glm::make_vec4(color);
}

void BzMaterial::setSpecular(const float color[4])
{
    memcpy (specular, color, sizeof(float[4]));
}

void BzMaterial::setEmission(const float color[4])
{
    memcpy (emission, color, sizeof(float[4]));
}

void BzMaterial::setShininess(const float shine)
{
    shininess = shine;
}

void BzMaterial::setAlphaThreshold(const float thresh)
{
    alphaThreshold = thresh;
}

void BzMaterial::setOccluder(bool value)
{
    occluder = value;
}

void BzMaterial::setGroupAlpha(bool value)
{
    groupAlpha = value;
}

void BzMaterial::setNoRadar(bool value)
{
    noRadar = value;
}

void BzMaterial::setNoShadow(bool value)
{
    noShadow = value;
}

void BzMaterial::setNoCulling(bool value)
{
    noCulling = value;
}

void BzMaterial::setNoSorting(bool value)
{
    noSorting = value;
}

void BzMaterial::setNoLighting(bool value)
{
    noLighting = value;
}


void BzMaterial::addTexture(const std::string& texname)
{
    textureCount++;
    TextureInfo* tmpinfo = new TextureInfo[textureCount];
    for (int i = 0; i < (textureCount - 1); i++)
        tmpinfo[i] = textures[i];
    delete[] textures;
    textures = tmpinfo;

    TextureInfo* texinfo = &textures[textureCount - 1];
    texinfo->name = texname;
    texinfo->localname = texname;
    texinfo->matrix = -1;
    texinfo->combineMode = decal;
    texinfo->useAlpha = true;
    texinfo->useColor = true;
    texinfo->useSphereMap = false;
}

void BzMaterial::setTexture(const std::string& texname)
{
    if (textureCount <= 0)
        addTexture(texname);
    else
        textures[textureCount - 1].name = texname;
}

void BzMaterial::setTextureLocal(int texid, const std::string& localname)
{
    if ((texid >= 0) && (texid < textureCount))
        textures[texid].localname = localname;
}

void BzMaterial::setNormalMap(const std::string& texname)
{
    if (normalMap != nullptr)
        delete (normalMap);

    normalMap = new TextureInfo();
    normalMap->name = texname;
    normalMap->localname = texname;
    normalMap->matrix = -1;
    normalMap->combineMode = decal;
    normalMap->useAlpha = true;
    normalMap->useColor = true;
    normalMap->useSphereMap = false;
}

void BzMaterial::setNormalMapLocal(const std::string& localname)
{
    if (normalMap == nullptr)
        setNormalMap(localname);

    normalMap->localname = localname;
}

void BzMaterial::setSpecularMap(const std::string& texname)
{
    if (specularMap != nullptr)
        delete (specularMap);

    specularMap = new TextureInfo();
    specularMap->name = texname;
    specularMap->localname = texname;
    specularMap->matrix = -1;
    specularMap->combineMode = decal;
    specularMap->useAlpha = true;
    specularMap->useColor = true;
    specularMap->useSphereMap = false;
}

void BzMaterial::setSpecularMapLocal(const std::string& localname)
{
    if (normalMap == nullptr)
        setSpecularMap(localname);

    specularMap->localname = localname;
}


void BzMaterial::setTextureMatrix(int matrix)
{
    if (textureCount > 0)
        textures[textureCount - 1].matrix = matrix;
    return;
}

void BzMaterial::setCombineMode(int mode)
{
    if (textureCount > 0)
        textures[textureCount - 1].combineMode = mode;
    return;
}

void BzMaterial::setUseTextureAlpha(bool value)
{
    if (textureCount > 0)
        textures[textureCount - 1].useAlpha = value;
}

void BzMaterial::setUseColorOnTexture(bool value)
{
    if (textureCount > 0)
        textures[textureCount - 1].useColor = value;
}

void BzMaterial::setUseSphereMap(bool value)
{
    if (textureCount > 0)
        textures[textureCount - 1].useSphereMap = value;
}


void BzMaterial::clearTextures()
{
    delete[] textures;
    textures = nullptr;
    textureCount = 0;

    delete (specularMap);
    delete (normalMap);

    specularMap = nullptr;
    normalMap = nullptr;
}


void BzMaterial::setShader(const std::string& shadername)
{
    if (shaderCount <= 0)
        addShader(shadername);
    else
        shaders[shaderCount - 1].name = shadername;

}

void BzMaterial::addShader(const std::string& shaderName)
{
    shaderCount++;
    ShaderInfo* tmpinfo = new ShaderInfo[shaderCount];
    for (int i = 0; i < (shaderCount - 1); i++)
        tmpinfo[i] = shaders[i];
    delete[] shaders;
    shaders = tmpinfo;
    shaders[shaderCount - 1].name = shaderName;
}


void BzMaterial::clearShaders()
{
    delete[] shaders;
    shaders = nullptr;
    shaderCount = 0;
}


/****************************************************************************/
//
// Parameter retrieval
//

const std::string& BzMaterial::getName() const
{
    return name;
}

const std::vector<std::string>& BzMaterial::getAliases() const
{
    return aliases;
}

int BzMaterial::getDynamicColor() const
{
    return dynamicColor;
}

const float* BzMaterial::getAmbient() const
{
    return ambient;
}

const glm::vec4 &BzMaterial::getDiffuse() const
{
    return diffuse;
}

const float* BzMaterial::getSpecular() const
{
    return specular;
}

const float* BzMaterial::getEmission() const
{
    return emission;
}

float BzMaterial::getShininess() const
{
    return shininess;
}

float BzMaterial::getAlphaThreshold() const
{
    return alphaThreshold;
}

bool BzMaterial::getOccluder() const
{
    return occluder;
}

bool BzMaterial::getGroupAlpha() const
{
    return groupAlpha;
}

bool BzMaterial::getNoRadar() const
{
    return noRadar;
}

bool BzMaterial::getNoShadow() const
{
    return noShadow;
}

bool BzMaterial::getNoCulling() const
{
    return noCulling;
}

bool BzMaterial::getNoSorting() const
{
    return noSorting;
}

bool BzMaterial::getNoLighting() const
{
    return noLighting;
}


int BzMaterial::getTextureCount() const
{
    return textureCount;
}

const std::string& BzMaterial::getTexture(int texid) const
{
    if ((texid >= 0) && (texid < textureCount))
        return textures[texid].name;
    else
        return nullString;
}

const std::string& BzMaterial::getTextureLocal(int texid) const
{
    if ((texid >= 0) && (texid < textureCount))
        return textures[texid].localname;
    else
        return nullString;
}

const std::string& BzMaterial::getNormalMap() const
{
    if (normalMap == nullptr)
        return nullString;

    return normalMap->name;
}

const std::string& BzMaterial::getSpecularMap() const
{
    if (specularMap == nullptr)
        return nullString;

    return specularMap->name;
}

const std::string& BzMaterial::getNormalMapLocal() const
{
    if (normalMap == nullptr)
        return nullString;

    return normalMap->localname;
}

const std::string& BzMaterial::getSpecularMapLocal() const
{
    if (specularMap == nullptr)
        return nullString;

    return specularMap->localname;
}

int BzMaterial::getTextureMatrix(int texid) const
{
    if ((texid >= 0) && (texid < textureCount))
        return textures[texid].matrix;
    else
        return -1;
}

int BzMaterial::getCombineMode(int texid) const
{
    if ((texid >= 0) && (texid < textureCount))
        return textures[texid].combineMode;
    else
        return -1;
}

bool BzMaterial::getUseTextureAlpha(int texid) const
{
    if ((texid >= 0) && (texid < textureCount))
        return textures[texid].useAlpha;
    else
        return false;
}

bool BzMaterial::getUseColorOnTexture(int texid) const
{
    if ((texid >= 0) && (texid < textureCount))
        return textures[texid].useColor;
    else
        return false;
}

bool BzMaterial::getUseSphereMap(int texid) const
{
    if ((texid >= 0) && (texid < textureCount))
        return textures[texid].useSphereMap;
    else
        return false;
}


int BzMaterial::getShaderCount() const
{
    return shaderCount;
}

const std::string& BzMaterial::getShader(int shdid) const
{
    if ((shdid >= 0) && (shdid < shaderCount))
        return shaders[shdid].name;
    else
        return nullString;
}


bool BzMaterial::isInvisible() const
{
    const DynamicColor* dyncol = DYNCOLORMGR.getColor(dynamicColor);
    if ((diffuse[3] == 0.0f) && (dyncol == nullptr) &&
            !((textureCount > 0) && !textures[0].useColor))
        return true;
    return false;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
