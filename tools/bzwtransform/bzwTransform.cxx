#include "common.h"

/* system headers */
#include <fstream>
#include <iostream>
#include <string>
#include <strstream>
#include <stack>
#include <vector>
#include <math.h>

typedef int foo;

#ifndef PI
#  define PI (3.1415926535897932384626433832795f)
#endif
#ifndef TWO_PI
#  define TWO_PI (2.0f*PI)
#endif


class Matrix3D
{
public:
	Matrix3D()
	{
		makeIdentity();
	}

	float *operator[]( int index )
	{
		if ((index < 0) || (index >= 4))
			throw exception();

		return matrix[index];
	}

	void makeIdentity()
	{
		memset( matrix, 0, sizeof( float ) * 4 * 4 );
		for (int i = 0; i < 4; i++)
			matrix[i][i] = 1.0f;
	}

	void makeScale( float scaleX, float scaleY, float scaleZ )
	{
		makeIdentity();
		matrix[0][0] = scaleX;
		matrix[1][1] = scaleY;
		matrix[2][2] = scaleZ;

	}

	void makeRotationZ( float angle )
	{
		makeIdentity();
		matrix[0][0] = (float)cos(angle);
		matrix[0][1] = (float)-sin(angle);
		matrix[1][0] = (float)sin(angle);
		matrix[1][1] = (float)cos(angle);
	}

	void makeTranslation( float xPos, float yPos, float zPos )
	{
		makeIdentity();
		matrix[0][3] = (float)xPos;
		matrix[1][3] = (float)yPos;
		matrix[2][3] = (float)zPos;
	}

	void postMultiply( Matrix3D &inMatrix )
	{
		// this = this * matrix

		float src[4][4];
		memcpy( src, matrix, sizeof(float)*4*4 );

		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				matrix[x][y] =	src[0][y]*inMatrix.matrix[x][0] +
						src[1][y]*inMatrix.matrix[x][1] +
						src[2][y]*inMatrix.matrix[x][2] +
						src[3][y]*inMatrix.matrix[x][3];
			}
		}

	}

	void preMultiply( Matrix3D &inMatrix )
	{
		// this = matrix * this

		float src[4][4];
		memcpy( src, matrix, sizeof(float)*4*4 );

		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				matrix[x][y] =	inMatrix.matrix[0][y]*src[x][0] +
						inMatrix.matrix[1][y]*src[x][1] +
						inMatrix.matrix[2][y]*src[x][2] +
						inMatrix.matrix[3][y]*src[x][3];
			}
		}
	}
private:
	float matrix[4][4];
	friend class Point3D;
};

class Point3D
{
public:
	Point3D()
	{
		memset( points, 0, sizeof(float)*4 );
		points[3] = 1.0f;
	}

	Point3D( float x, float y, float z )
	{
		setValue( x, y, z );
	}

	void setValue( float x, float y, float z )
	{
		points[0] = x;
		points[1] = y;
		points[2] = z;
		points[3] = 1.0f;
	}

	float operator[]( int index ) const
	{
		if ((index < 0) || (index >= 4))
			throw exception();

		return points[index];
	}

	void transform( Matrix3D &matrix )
	{
		float src[4];
		memcpy( src, points, sizeof(float)*4 );

		for (int x = 0; x < 4; x++)
		{
			points[x] = 0.0;
			for (int y = 0; y < 4; y++)
				points[x] += src[y] * matrix.matrix[x][y];
		}

	}

private:
	float points[4];
};

class BZObject
{
public:
  BZObject( std::string &type, float *position, float *size, float rotation )
	{
		this->type = type;
		this->position.setValue( position[0], position[1], position[2] );
		this->size.setValue( size[0], size[1], size[2] );
		this->rotation = rotation;
	}

	bool isBox()
	{
		return type == "box";
	}
public:

  std::string	type;
	Point3D	position;
	Point3D	size;
	float	rotation;
};


void box( std::ofstream &bzw, Point3D &position, Point3D &size, float rotation, char *comment = NULL )
{
	if (comment != NULL)
		bzw << "#" << comment << std::endl;
	bzw << "box" << std::endl;
	bzw << "position " << position[0] << " " << position[1] << " " << position[2] << std::endl;
	bzw << "size " << size[0] << " " << size[1] << " " << size[2] << std::endl;
	bzw << "rotation " << rotation << std::endl;
	bzw << "end" << std::endl;
	bzw << std::endl;
}

void pyramid( std::ofstream &bzw, Point3D &position, Point3D &size, float rotation, char *comment = NULL )
{
	if (comment != NULL)
		bzw << "#" << comment << std::endl;
	bzw << "pyramid" << std::endl;
	bzw << "position " << position[0] << " " << position[1] << " " << position[2] << std::endl;
	bzw << "size " << size[0] << " " << size[1] << " " << size[2] << std::endl;
	bzw << "rotation " << rotation << std::endl;
	bzw << "end" << std::endl;
	bzw << std::endl;
}

void generateObjects( std::ofstream &bzw, std::vector<BZObject *> objects, Matrix3D &posMatrix, Matrix3D &sizeMatrix, float rotation, int count )
{
	for (int cur = 0; cur < count; cur++)
	{
	  for (std::vector<BZObject*>::iterator it = objects.begin(); it != objects.end(); it++)
		{
			BZObject *object = *it;
			if (object->isBox())
				box( bzw, object->position, object->size, object->rotation );
			else
				pyramid( bzw, object->position, object->size, object->rotation );

			object->position.transform( posMatrix );
			object->size.transform( sizeMatrix );
			object->rotation += rotation;
		}
	}
}


void parsebzwt( std::ifstream &bzwt, std::ofstream &bzw )
{
	enum { FREE, IN_TRANSFORM, IN_BOX, IN_PYRAMID, IN_MATRIX, IN_POSITION0, IN_POSITION1, IN_POSITION2, IN_SIZE0, IN_SIZE1, IN_SIZE2, IN_ROTATION,
		IN_MATRIX_POSITION, IN_MATRIX_SIZE, IN_MATRIX_ROTATION, IN_ANGLE, IN_SCALE0, IN_SCALE1, IN_SCALE2, IN_TRANSLATE0, IN_TRANSLATE1, IN_TRANSLATE2, IN_COUNT
	};
	std::stack<int> stateStack;
	std::string line, token;
	int lineNo = 0;
	int state = FREE;
	float position[4];
	float size[4];
	float scale[4];
	float translate[4];
	float angle;

	std::vector<BZObject *> objects;
	Matrix3D posMatrix, sizeMatrix, tempMatrix;
	float rotation;
	int   count;

	try
	{
		while (!bzwt.eof())
		{
			getline( bzwt, line );
			lineNo++;

			if (line.length() == 0) continue;
			int start = line.find_first_not_of( " \t\n\r" );
			if (start == std::string::npos) continue;
			if (line.at(start) == '#') continue;

			std::istrstream lineStream( line.c_str() );

			while (!lineStream.eof())
			{
				lineStream >> token;
				if (token.length() == 0)
					break;

				switch (state)
				{
					case FREE:
						if (token == "transform")
						{
							state = IN_TRANSFORM;
							posMatrix.makeIdentity();
							sizeMatrix.makeIdentity();
							count = 1;
							rotation = 0.0f;
							for (unsigned int i = 0; i < objects.size(); i++)
								delete objects[i];
							objects.clear();
						}
						else
							throw exception( "expecting transform" );
					break;

					case IN_TRANSFORM:
						memset( position, 0, sizeof(float)*4 );
						memset( size, 0, sizeof(float)*4 );

						if (token == "box")
							state = IN_BOX;
						else if (token == "pyramid")
							state = IN_PYRAMID;
						else if (token == "matrix")
							state = IN_MATRIX;
						else if (token == "count")
							state = IN_COUNT;
						else if (token == "end")
						{
							generateObjects( bzw, objects, posMatrix, sizeMatrix, rotation, count );
							state = FREE;
						}
						else
							throw exception( "expecting transform subitem, or end" );
					break;

					case IN_BOX:
						if (token == "position")
						{
							state = IN_POSITION0;
							stateStack.push( IN_BOX );
						}
						else if (token == "size")
						{
							state = IN_SIZE0;
							stateStack.push( IN_BOX );
						}
						else if (token == "rotation")
						{
							state = IN_ROTATION;
							stateStack.push( IN_BOX );
						}
						else if (token == "end")
						{
						  BZObject *obj = new BZObject( std::string("box"), position, size, rotation );
							objects.push_back( obj );
							state = IN_TRANSFORM;
							memset( position, 0, sizeof(float)*4 );
							memset( size, 0, sizeof(float)*4 );
							rotation = 0.0f;
						}
						else
							throw exception( "expecting transform subitem, or end" );
					break;

					case IN_PYRAMID:
						if (token == "position")
						{
							state = IN_POSITION0;
							stateStack.push( IN_PYRAMID );
						}
						else if (token == "size")
						{
							state = IN_SIZE0;
							stateStack.push( IN_PYRAMID );
						}
						else if (token == "rotation")
						{
							state = IN_ROTATION;
							stateStack.push( IN_PYRAMID );
						}
						else if (token == "end")
						{
							state = IN_TRANSFORM;
							BZObject *obj = new BZObject( std::string("pyramid"), position, size, rotation );
							objects.push_back( obj );
							state = IN_TRANSFORM;
							memset( position, 0, sizeof(float)*4 );
							memset( size, 0, sizeof(float)*4 );
							rotation = 0.0f;
						}
						else
							throw exception( "expecting transform subitem, or end" );
					break;

					case IN_MATRIX:
						if (token == "position")
							state = IN_MATRIX_POSITION;
						else if (token == "size")
							state = IN_MATRIX_SIZE;
						else if (token == "rotation")
							state = IN_MATRIX_ROTATION;
						else if (token == "end")
							state = IN_TRANSFORM;
						else
							throw exception( "expecting transform matrix subitem, or end" );
					break;

					case IN_COUNT:
						count = atoi( token.c_str( ));
						state = IN_TRANSFORM;
					break;


					case IN_POSITION0:
						position[0] = (float)atof(token.c_str());
						state = IN_POSITION1;
					break;

					case IN_POSITION1:
						position[1] = (float)atof(token.c_str());
						state = IN_POSITION2;
					break;

					case IN_POSITION2:
						position[2] = (float)atof(token.c_str());
						state = stateStack.top();
						stateStack.pop();
					break;


					case IN_SIZE0:
						size[0] = (float)atof(token.c_str());
						state = IN_SIZE1;
					break;

					case IN_SIZE1:
						size[1] = (float)atof(token.c_str());
						state = IN_SIZE2;
					break;

					case IN_SIZE2:
						size[2] = (float)atof(token.c_str());
						state = stateStack.top();
						stateStack.pop();
					break;

					case IN_ROTATION:
						rotation = (float)atof(token.c_str());
						state = stateStack.top();
						stateStack.pop();
					break;

					case IN_MATRIX_POSITION:
						if (token == "rotate")
						{
							stateStack.push( IN_MATRIX_POSITION );
							state = IN_ANGLE;
						}
						else if (token == "scale")
						{
							stateStack.push( IN_MATRIX_POSITION );
							state = IN_SCALE0;
						}
						else if (token == "translate")
						{
							stateStack.push( IN_MATRIX_POSITION );
							state = IN_TRANSLATE0;
						}
						else if (token == "end")
						{
							state = IN_MATRIX;
						}
						else
							throw exception( "expecting a matrix operation, or end" );
					break;

					case IN_MATRIX_SIZE:
						if (token == "rotate")
						{
							stateStack.push( IN_MATRIX_SIZE );
							state = IN_ANGLE;
						}
						else if (token == "scale")
						{
							stateStack.push( IN_MATRIX_SIZE );
							state = IN_SCALE0;
						}
						else if (token == "translate")
						{
							stateStack.push( IN_MATRIX_SIZE );
							state = IN_TRANSLATE0;
						}
						else if (token == "end")
						{
							state = IN_MATRIX;
						}
						else
							throw exception( "expecting a matrix operation, or end" );
					break;

					case IN_MATRIX_ROTATION:
						rotation = (float)atof( token.c_str( ));
						state = IN_MATRIX;
					break;

					case IN_ANGLE:
						angle = (float)atof( token.c_str( ));
						state = stateStack.top();
						stateStack.pop();
						tempMatrix.makeRotationZ( (float)((angle * TWO_PI) / 360.0f) );
						if (state == IN_MATRIX_POSITION)
							posMatrix.postMultiply( tempMatrix );
						else if (state == IN_MATRIX_SIZE)
							sizeMatrix.postMultiply( tempMatrix );
					break;

					case IN_SCALE0:
						scale[0] = (float)atof( token.c_str( ));
						state = IN_SCALE1;
					break;

					case IN_SCALE1:
						scale[1] = (float)atof( token.c_str( ));
						state = IN_SCALE2;
					break;

					case IN_SCALE2:
						scale[2] = (float)atof( token.c_str( ));
						state = stateStack.top();
						stateStack.pop();
						tempMatrix.makeScale( scale[0], scale[1], scale[2] );
						if (state == IN_MATRIX_POSITION)
							posMatrix.postMultiply( tempMatrix );
						else if (state == IN_MATRIX_SIZE)
							sizeMatrix.postMultiply( tempMatrix );
					break;

					case IN_TRANSLATE0:
						translate[0] = (float)atof( token.c_str( ));
						state = IN_TRANSLATE1;
					break;

					case IN_TRANSLATE1:
						translate[1] = (float)atof( token.c_str( ));
						state = IN_TRANSLATE2;
					break;

					case IN_TRANSLATE2:
						translate[2] = (float)atof( token.c_str( ));
						state = stateStack.top();
						stateStack.pop();
						tempMatrix.makeTranslation( translate[0], translate[1], translate[2] );
						if (state == IN_MATRIX_POSITION)
							posMatrix.postMultiply( tempMatrix );
						else if (state == IN_MATRIX_SIZE)
							sizeMatrix.postMultiply( tempMatrix );
					break;


					default:
						throw exception( "UnHandled state" );
					break;

				}
			}
		}
	}
	catch (exception &e)
	{
	  std::cerr << "Parse error: line: " << lineNo << " " << e.what() << std::endl;
	  std::cerr << line << std::endl;
	}

}

int main( int argc, char **argv )
{
	if (argc != 3)
	{
	  std::cout << "error invalid arguments" << std::endl;
	  std::cout << "\tbzwTransform inputName outputName" << std::endl;
		return -1;
	}

	std::ifstream bzwt( argv[1] );

	if (bzwt.is_open())
	{
	  std::ofstream bzw( argv[2] );
		if (bzw.is_open())
		{
			parsebzwt( bzwt, bzw );

		}
		bzwt.close();
	}


	return 0;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
