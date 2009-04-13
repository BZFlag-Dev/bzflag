//============================================================================//
//============================================================================//

#ifndef VECTORS_H
#define VECTORS_H


#include <math.h>
#include <stdio.h>
#include <string>


template <typename T> T typed_cos(T rads);
template<> inline float  typed_cos<float> (float  rads) { return cosf(rads); }
template<> inline double typed_cos<double>(double rads) { return cos(rads);  }

template <typename T> T typed_sin(T rads);
template<> inline float  typed_sin<float> (float  rads) { return sinf(rads); }
template<> inline double typed_sin<double>(double rads) { return sin(rads);  }

template <typename T> T typed_sqrt(T rads);
template<> inline float  typed_sqrt<float> (float  rads) { return sqrtf(rads); }
template<> inline double typed_sqrt<double>(double rads) { return sqrt(rads);  }

template <typename T> inline std::string tostring(T value, const char* fmt) {
  char buf[256];
  if (fmt == NULL) { fmt = "%.9g"; }
  snprintf(buf, sizeof(buf), fmt, (double)value);
  return std::string(buf);
}


//============================================================================//
//============================================================================//
//
//  vec2
//

template <typename T>
class vec2 {
  public:
    union { T x; T r; T s; };
    union { T y; T g; T t; };

  public:
    vec2()              : x((T)0), y((T)0) {}
    vec2(const vec2& v) : x(v.x),  y(v.y)  {}
    vec2(T _x, T _y)    : x(_x),   y(_y)   {}

    inline vec2& operator=(const vec2& v) { x = v.x; y = v.y; return *this; }

    inline       T* data()       { return &x; }
    inline const T* data() const { return &x; }

    inline operator       T*()       { return &x; }
    inline operator const T*() const { return &x; }

    inline       T& operator[](int index)       { return ((T*)&x)[index]; }
    inline const T& operator[](int index) const { return ((T*)&x)[index]; }

    inline vec2 operator-() const { return vec2(-x, -y); }

    vec2& operator+=(const vec2& v) { x += v.x; y += v.y; return *this; }
    vec2& operator-=(const vec2& v) { x -= v.x; y -= v.y; return *this; }
    vec2& operator*=(const vec2& v) { x *= v.x; y *= v.y; return *this; }
    vec2& operator/=(const vec2& v) { x /= v.x; y /= v.y; return *this; }

    vec2& operator+=(T d) { x += d; y += d; return *this; }
    vec2& operator-=(T d) { x -= d; y -= d; return *this; }
    vec2& operator*=(T d) { x *= d; y *= d; return *this; }
    vec2& operator/=(T d) { x /= d; y /= d; return *this; }

    vec2 operator+(const vec2& v) const { return vec2(x + v.x, y + v.y); }
    vec2 operator-(const vec2& v) const { return vec2(x - v.x, y - v.y); }
    vec2 operator*(const vec2& v) const { return vec2(x * v.x, y * v.y); }
    vec2 operator/(const vec2& v) const { return vec2(x / v.x, y / v.y); }

    vec2 operator+(T d) const { return vec2(x + d, y + d); }
    vec2 operator-(T d) const { return vec2(x - d, y - d); }
    vec2 operator*(T d) const { return vec2(x * d, y * d); }
    vec2 operator/(T d) const { return vec2(x / d, y / d); }

    bool operator<(const vec2& v) const {
      if (x < v.x) { return true;  }
      if (x > v.x) { return false; }
      if (y < v.y) { return true;  }
      if (y > v.y) { return false; }
      return false;
    }
    bool operator==(const vec2& v) const {
      return ((x == v.x) && (y == v.y));
    }
    bool operator!=(const vec2& v) const {
      return ((x != v.x) || (y != v.y));
    }

    static T dot(const vec2& a, const vec2& b) {
      return ((a.x * b.x) + (a.y * b.y));
    }
    T dot(const vec2& v) const {
      return dot(*this, v);
    }

    T lengthSq() const { return dot(*this, *this); }
    T length()   const { return typed_sqrt(lengthSq()); }

    vec2 rotate(T radians) const {
      const T cv = typed_cos(radians);
      const T sv = typed_sin(radians);
      const T nx = (cv * x) - (sv * y);
      const T ny = (cv * y) + (sv * x);
      return vec2(nx, ny);
    }
    static void rotate(vec2& v, T radians) { v = v.rotate(radians); }

    std::string tostring(const char* fmt = NULL, const char* sep = NULL) const {
      if (sep == NULL) { sep = " "; }
      return ::tostring(x, fmt) + sep +
             ::tostring(y, fmt);
    }
};


template <typename T>
vec2<T> operator+(T d, const vec2<T>& in) { vec2<T> v(in); v += d; return v; }
template <typename T>
vec2<T> operator-(T d, const vec2<T>& in) { vec2<T> v(d, d); v -= in; return v; }
template <typename T>
vec2<T> operator*(T d, const vec2<T>& in) { vec2<T> v(in); v *= d; return v; }
template <typename T>
vec2<T> operator/(T d, const vec2<T>& in) { vec2<T> v(d, d); v /= in; return v; }


//============================================================================//
//============================================================================//
//
//  vec3
//

template <typename T>
class vec3 {
  public:
    union { T x; T r; T s; };
    union { T y; T g; T t; };
    union { T z; T b; T p; };

  public:
    vec3()                       : x((T)0), y((T)0), z((T)0) {}
    vec3(const vec3& v)          : x(v.x),  y(v.y),  z(v.z)  {}
    vec3(T _x, T _y, T _z)       : x(_x),   y(_y),   z(_z)   {}
    vec3(const vec2<T>& v, T _z) : x(v.x),  y(v.y),  z(_z)   {}

    inline vec3& operator=(const vec3& v) {
      x = v.x; y = v.y; z = v.z; return *this;
    }

    inline       T* data()       { return &x; }
    inline const T* data() const { return &x; }

    inline operator       T*()       { return &x; }
    inline operator const T*() const { return &x; }

    inline       T& operator[](int index)       { return ((T*)&x)[index]; }
    inline const T& operator[](int index) const { return ((T*)&x)[index]; }

    inline vec3 operator-() const { return vec3(-x, -y, -z); }

    inline       vec2<T>& xy()       { return (vec2<T>&)x; }
    inline const vec2<T>& xy() const { return (vec2<T>&)x; }
    inline       vec2<T>& yz()       { return (vec2<T>&)y; }
    inline const vec2<T>& yz() const { return (vec2<T>&)y; }

    vec3& operator+=(const vec3& v) {
      x += v.x; y += v.y; z += v.z; return *this;
    }
    vec3& operator-=(const vec3& v) {
      x -= v.x; y -= v.y; z -= v.z; return *this;
    }
    vec3& operator*=(const vec3& v) {
      x *= v.x; y *= v.y; z *= v.z; return *this;
    }
    vec3& operator/=(const vec3& v) {
      x /= v.x; y /= v.y; z /= v.z; return *this;
    }
    vec3& operator+=(T d) { x += d; y += d; z += d; return *this; }
    vec3& operator-=(T d) { x -= d; y -= d; z -= d; return *this; }
    vec3& operator*=(T d) { x *= d; y *= d; z *= d; return *this; }
    vec3& operator/=(T d) { x /= d; y /= d; z /= d; return *this; }

    vec3 operator+(const vec3& v) const {
      return vec3(x + v.x, y + v.y, z + v.z);
    }
    vec3 operator-(const vec3& v) const {
      return vec3(x - v.x, y - v.y, z - v.z);
    }
    vec3 operator*(const vec3& v) const {
      return vec3(x * v.x, y * v.y, z * v.z);
    }
    vec3 operator/(const vec3& v) const {
      return vec3(x / v.x, y / v.y, z / v.z);
    }

    vec3 operator+(T d) const { return vec3(x + d, y + d, z + d); }
    vec3 operator-(T d) const { return vec3(x - d, y - d, z - d); }
    vec3 operator*(T d) const { return vec3(x * d, y * d, z * d); }
    vec3 operator/(T d) const { return vec3(x / d, y / d, z / d); }

    bool operator<(const vec3& v) const {
      if (x < v.x) { return true;  }
      if (x > v.x) { return false; }
      if (y < v.y) { return true;  }
      if (y > v.y) { return false; }
      if (z < v.z) { return true;  }
      if (z > v.z) { return false; }
      return false;
    }
    bool operator==(const vec3& v) const {
      return ((x == v.x) && (y == v.y) && (z == v.z));
    }
    bool operator!=(const vec3& v) const {
      return ((x != v.x) || (y != v.y) || (z != v.z));
    }

    static T dot(const vec3& a, const vec3& b) {
      return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
    }
    T dot(const vec3& v) const {
      return dot(*this, v);
    }

    static vec3 cross(const vec3& a, const vec3& b) {
      return vec3(
        ((a.y * b.z) - (a.z * b.y)),
        ((a.z * b.x) - (a.x * b.z)),
        ((a.x * b.y) - (a.y * b.x))
      );
    }
    vec3 cross(const vec3& v) const {
      return cross(*this, v);
    }

    T lengthSq() const { return dot(*this, *this); }
    T length()   const { return typed_sqrt(lengthSq()); }

    static bool normalize(vec3& v) {
      const T len = v.length();
      if (len == (T)0) {
        return false;
      }
      const T scale = ((T)1 / len);
      v *= scale;
      return true;
    }
    vec3 normalize() const {
      vec3 v(*this); normalize(v); return v;
    }

    vec3 rotateX(T radians) const {
      const T cv = typed_cos(radians);
      const T sv = typed_sin(radians);
      const T ny = (cv * y) - (sv * z);
      const T nz = (cv * z) + (sv * y);
      return vec3(x, ny, nz);
    }
    static void rotateX(vec3& v, T radians) { v = v.rotateX(radians); }

    vec3 rotateY(T radians) const {
      const T cv = typed_cos(radians);
      const T sv = typed_sin(radians);
      const T nz = (cv * z) - (sv * x);
      const T nx = (cv * x) + (sv * z);
      return vec3(nx, y, nz);
    }
    static void rotateY(vec3& v, T radians) { v = v.rotateY(radians); }

    vec3 rotateZ(T radians) const {
      const T cv = typed_cos(radians);
      const T sv = typed_sin(radians);
      const T nx = (cv * x) - (sv * y);
      const T ny = (cv * y) + (sv * x);
      return vec3(nx, ny, z);
    }
    static void rotateZ(vec3& v, T radians) { v = v.rotateZ(radians); }

    std::string tostring(const char* fmt = NULL, const char* sep = NULL) const {
      if (sep == NULL) { sep = " "; }
      return ::tostring(x, fmt) + sep +
             ::tostring(y, fmt) + sep +
             ::tostring(z, fmt);
    }
};


template <typename T>
vec3<T> operator+(T d, const vec3<T>& in) { vec3<T> v(in); v += d; return v; }
template <typename T>
vec3<T> operator-(T d, const vec3<T>& in) { vec3<T> v(d, d, d); v -= in; return v; }
template <typename T>
vec3<T> operator*(T d, const vec3<T>& in) { vec3<T> v(in); v *= d; return v; }
template <typename T>
vec3<T> operator/(T d, const vec3<T>& in) { vec3<T> v(d, d, d); v /= in; return v; }


//============================================================================//
//============================================================================//
//
//  vec4
//

template <typename T>
class vec4 {
  public:
    union { T x; T r; T s; };
    union { T y; T g; T t; };
    union { T z; T b; T p; };
    union { T w; T a; T q; };

  public:
    vec4()                       : x((T)0), y((T)0), z((T)0), w((T)1) {}
    vec4(const vec4& v)          : x(v.x),  y(v.y),  z(v.z),  w(v.w)  {}
    vec4(T _x, T _y, T _z, T _w) : x(_x),   y(_y),   z(_z),   w(_w)   {}
    vec4(const vec3<T>& v, T _w) : x(v.x),  y(v.y),  z(v.z),  w(_w)   {}

    inline vec4& operator=(const vec4& v) {
      x = v.x; y = v.y; z = v.z; w = v.w; return *this;
    }

    inline       T* data()       { return &x; }
    inline const T* data() const { return &x; }

    inline operator       T*()       { return &x; }
    inline operator const T*() const { return &x; }

    inline       T& operator[](int index)       { return ((T*)&x)[index]; }
    inline const T& operator[](int index) const { return ((T*)&x)[index]; }

    inline vec4 operator-() const { return vec4(-x, -y, -z, -w); }

    inline       vec2<T>&  xy()       { return (vec2<T>&)x; }
    inline const vec2<T>&  xy() const { return (vec2<T>&)x; }
    inline       vec2<T>&  yz()       { return (vec2<T>&)y; }
    inline const vec2<T>&  yz() const { return (vec2<T>&)y; }
    inline       vec2<T>&  zw()       { return (vec2<T>&)z; }
    inline const vec2<T>&  zw() const { return (vec2<T>&)z; }
    inline       vec3<T>& xyz()       { return (vec3<T>&)x; }
    inline const vec3<T>& xyz() const { return (vec3<T>&)x; }
    inline       vec3<T>& yzw()       { return (vec3<T>&)y; }
    inline const vec3<T>& yzw() const { return (vec3<T>&)y; }
    inline       vec3<T>& rgb()       { return (vec3<T>&)x; }
    inline const vec3<T>& rgb() const { return (vec3<T>&)x; }

    vec4& operator+=(const vec4& v) {
      x += v.x; y += v.y; z += v.z; w += v.w; return *this;
    }
    vec4& operator-=(const vec4& v) {
      x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this;
    }
    vec4& operator*=(const vec4& v) {
      x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this;
    }
    vec4& operator/=(const vec4& v) {
      x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this;
    }

    vec4& operator+=(T d) { x += d; y += d; z += d; w += d; return *this; }
    vec4& operator-=(T d) { x -= d; y -= d; z -= d; w -= d; return *this; }
    vec4& operator*=(T d) { x *= d; y *= d; z *= d; w *= d; return *this; }
    vec4& operator/=(T d) { x /= d; y /= d; z /= d; w /= d; return *this; }

    vec4 operator+(const vec4& v) const {
      return vec4(x + v.x, y + v.y, z + v.z, w + v.w);
    }
    vec4 operator-(const vec4& v) const {
      return vec4(x - v.x, y - v.y, z - v.z, w - v.w);
    }
    vec4 operator*(const vec4& v) const {
      return vec4(x * v.x, y * v.y, z * v.z, w * v.w);
    }
    vec4 operator/(const vec4& v) const {
      return vec4(x / v.x, y / v.y, z / v.z, w / v.w);
    }

    vec4 operator+(T d) const { return vec4(x + d, y + d, z + d, w + d); }
    vec4 operator-(T d) const { return vec4(x - d, y - d, z - d, w - d); }
    vec4 operator*(T d) const { return vec4(x * d, y * d, z * d, w * d); }
    vec4 operator/(T d) const { return vec4(x / d, y / d, z / d, w / d); }

    bool operator<(const vec4& v) const {
      if (x < v.x) { return true;  }
      if (x > v.x) { return false; }
      if (y < v.y) { return true;  }
      if (y > v.y) { return false; }
      if (z < v.z) { return true;  }
      if (z > v.z) { return false; }
      if (w < v.w) { return true;  }
      if (w > v.w) { return false; }
      return false;
    }
    bool operator==(const vec4& v) const {
      return ((x == v.x) && (y == v.y) && (z == v.z) && (w == v.w));
    }
    bool operator!=(const vec4& v) const {
      return ((x != v.x) || (y != v.y) || (z != v.z) || (w != v.w));
    }

    static T dot(const vec4& a, const vec4& b) {
      return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w));
    }
    T dot(const vec4& v) const {
      return dot(*this, v);
    }

    T lengthSq() const { return dot(*this, *this); }
    T length()   const { return typed_sqrt(lengthSq()); }

    // for the plane equation,
    // (x * p.x) + (y * p.y) + (z * p.z) + w = 0  {for a point on the plane}
    T planeDist(const vec3<T>& point) const {
      return vec3<T>::dot(point, xyz()) + w;
    }

    std::string tostring(const char* fmt = NULL, const char* sep = NULL) const {
      if (sep == NULL) { sep = " "; }
      return ::tostring(x, fmt) + sep +
             ::tostring(y, fmt) + sep +
             ::tostring(z, fmt) + sep +
             ::tostring(w, fmt);
    }
};


template <typename T>
vec4<T> operator+(T d, const vec4<T>& in) { vec4<T> v(in); v += d; return v; }
template <typename T>
vec4<T> operator-(T d, const vec4<T>& in) { vec4<T> v(d, d, d, d); v -= in; return v; }
template <typename T>
vec4<T> operator*(T d, const vec4<T>& in) { vec4<T> v(in); v *= d; return v; }
template <typename T>
vec4<T> operator/(T d, const vec4<T>& in) { vec4<T> v(d, d, d, d); v /= in; return v; }


//============================================================================//
//============================================================================//
//
//  Easier to type
//

typedef vec2<float> fvec2;
typedef vec3<float> fvec3;
typedef vec4<float> fvec4;

typedef vec2<double> dvec2;
typedef vec3<double> dvec3;
typedef vec4<double> dvec4;


//============================================================================//
//============================================================================//

#endif // VECTORS_H

//============================================================================//
//============================================================================//

