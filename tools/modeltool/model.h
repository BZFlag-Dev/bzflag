
//
// modeltool.h : defintions of the model classes
//

#ifndef _MODEL_H_
#define _MODEL_H_

#include <string>
#include <vector>
#include <map>

extern std::string texdir;
extern std::string groupName;
extern bool useMaterials;
extern bool useAmbient;
extern bool useDiffuse;
extern bool useSpecular;
extern bool useShininess;
extern bool useEmission;
extern bool useNormals;
extern bool useTexcoords ;
extern bool flipYZ;
extern bool useSmoothBounce;
extern float shineFactor;
extern float globalScale;
extern float globalShift[3];

extern float maxShineExponent; // OpenGL minimum shininess

extern std::vector<std::string> bspMaterialSkips; // materials to skip in a bsp map

typedef std::vector<int> tvIndexList;

typedef enum
{
	eXAxis,
	eYAxis,
	eZAxis
} teModelAxis;

class CTexCoord
{
public:
	CTexCoord() {u = v = 0;}
	~CTexCoord() {};
	float	u,v;

	bool same ( const CTexCoord &c )
	{
		return u == c.u && v ==c.v;
	}
};

typedef std::vector<CTexCoord> tvTexCoordList;

class CVertex
{
public:
	CVertex() {x = y = z = 0;}
	~CVertex() {};
	float	x,y,z;

	float get ( teModelAxis axis )
	{
		switch (axis)
		{
		case eXAxis:
			return x;
		case eYAxis:
			return y;
		default:
			return z;
		}
	}

	void translate ( float val, teModelAxis axis )
	{
		switch (axis)
		{
		case eXAxis:
			x += val;
			return;
		case eYAxis:
			y += val;
			return;
		case eZAxis:
			z += val;
		}
	}

	bool same ( const CVertex &v )
	{
		return x == v.x && y == v.y && z == v.z;
	}
};

typedef std::vector<CVertex> tvVertList;

class CFace
{
public:
	CFace() {};
	~CFace() {};

	std::string	material;
	tvIndexList	verts;
	tvIndexList	normals;
	tvIndexList	texCoords;

	void clear ( void ) {verts.clear();normals.clear();texCoords.clear();}
};
typedef std::vector<CFace> tvFaceList;


class CMaterial
{
public:
	CMaterial() {clear();}
	~CMaterial() {};

	std::string	texture;
	float		ambient[4];
	float		diffuse[4];
	float		specular[4];
	float		emission[4];
	float		shine;

	void clear ( void )
	{
		texture = "";
		ambient[0] = ambient[1] = ambient[2] = 0.2f;
		ambient[3] = 1.0f;
		diffuse[0] = diffuse[1] = diffuse[2] = 1.0f;
		diffuse[3] = 1.0f;
		specular[0] = specular[1] = specular[2] = 0.0f;
		specular[3] = 1.0f;
		emission[0] = emission[1] = emission[2] = 0.0f;
		emission[3] = 1.0f;
		shine = 0.0f;
	}
};

typedef std::map<std::string,CMaterial> tmMaterialMap;

class CMesh
{
public:
	CMesh() {};
	~CMesh() {};

	tvVertList	verts;
	tvVertList	normals;
	tvTexCoordList	texCoords;

	std::string	name;
	tvFaceList	faces;

	float getMaxAxisValue ( teModelAxis axis )
	{
		if (!valid())
			return 0.0f;

		float pt = verts[0].get(axis);

		for ( unsigned int i = 0; i < verts.size(); i++ )
			if ( verts[i].get(axis) > pt)
				pt = verts[i].get(axis);

		return pt;
	}

	float getMinAxisValue ( teModelAxis axis )
	{
		if (!valid())
			return 0.0f;

		float pt = verts[0].get(axis);

		for ( unsigned int i = 0; i < verts.size(); i++ )
			if ( verts[i].get(axis) < pt)
				pt = verts[i].get(axis);

		return pt;
	}

	void translate ( float value, teModelAxis axis )
	{
		for ( unsigned int i = 0; i < verts.size(); i++ )
			verts[i].translate(value,axis);
	}

	bool valid ( void )
	{
		return faces.size() != 0;
	}

	void clear ( void )
	{
		faces.clear();
		verts.clear();
		normals.clear();
		texCoords.clear();
		name = "";
	}
	void reindex ( void );
};

typedef std::vector<CMesh> tvMeshList;


class CCustomObject
{
public:
	std::string name;
	std::vector<std::string> params;

	void clear ( void ) {params.clear();name="";}
};

typedef std::vector<CCustomObject> tvCustomObjectList;

class CModel
{
public:
	CModel() {};
	~CModel() {};

	tmMaterialMap		materials;
	tvMeshList		meshes;
	tvCustomObjectList	customObjects;

	void pushAboveAxis ( teModelAxis axis )
	{
		if (!meshes.size())
			return;

		float minValue = meshes[0].getMinAxisValue(axis);

		for ( unsigned int i = 0; i < meshes.size(); i++ )
			if ( minValue > meshes[i].getMinAxisValue(axis))
				minValue = meshes[i].getMinAxisValue(axis);

		for ( unsigned int i = 0; i < meshes.size(); i++ )
			meshes[i].translate(-minValue,axis);
	}

	void clear ( void )
	{
		meshes.clear();
		materials.clear();
		customObjects.clear();
	}
};


#endif // _MODEL_H_

