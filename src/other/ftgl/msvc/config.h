// GLUT
#define HAVE_GL_GLUT_H

// M_PI and friends on VC
#define _USE_MATH_DEFINES

// quell spurious "'this': used in base member initializer list" warnings
#ifdef _MSC_VER
#pragma warning(disable: 4355)
#endif

// quell spurious portable-function deprecation warnings
#define _CRT_SECURE_NO_DEPRECATE 1
#define _POSIX_ 1