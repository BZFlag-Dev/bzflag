#ifndef _SIMPLE_SDL_VIEW_H_
#define _SIMPLE_SDL_VIEW_H_

#include <SDL/SDL.h>
#include <sdl/SDL_keysym.h>

#ifdef _WIN32 // this file only has windows stuff
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <time.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "sdl.lib")

#else
#include <unistd.h>
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#include <AGL/agl.h>
#include <AGL/gl.h>
#include <AGL/glu.h>
#else	// linux
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif // _WIN32

#include <vector>

class SimpleDisplayCamera 
{
public:
  SimpleDisplayCamera(float x = 0.0f, float y = 0.0f, float z = 0.0f);
  ~SimpleDisplayCamera();

  void applyView();
  void removeView();
  void applyOrtho();
  void removeOrtho();

  void moveLoc(float x, float y, float z, float distance =1 );
  void moveGlob(float x, float y, float z, float distance =1 );
  void rotateLoc(float deg, float x, float y, float z);
  void rotateGlob(float deg, float x, float y, float z);

  void setOrthoViewport ( void );
  void setOrthoHitherYon ( float hither, float yon );

private:
  float matrix[16];

  float viewport[4];
  float hitherYon[2];

  // pointers into the matrix for the various things
  float const *right, *up, *forward;
  float *pos;
};

class ModiferKeys
{
public:
  bool alt;
  bool ctl;
  bool shift;
  bool meta;

  ModiferKeys(bool a = false, bool c = false, bool s = false, bool m = false )
  {
    alt = a;
    ctl = c;
    shift = s;
    meta = m;
  }

  bool operator == ( const ModiferKeys& r ) const
  {
    return r.alt == alt && r.ctl == ctl && r.shift == shift && r.meta == meta;
  }
};

class SimpleDisplayEventCallbacks
{
public:
  virtual ~SimpleDisplayEventCallbacks(){};

  virtual void activate ( void ){};
  virtual void deactivate ( void ){};
  virtual void focus ( bool lost ){};
  virtual void resize ( size_t x, size_t y ){};
  virtual void key ( int key, bool down, const ModiferKeys& mods ){};

};

class SimpleDisplay
{
public:
  SimpleDisplay ( size_t width = 800, size_t height = 600, bool full = false, const char* caption = NULL );
  ~SimpleDisplay ();

  bool create ( size_t width = 800, size_t height = 600, bool full = false, const char* caption = NULL );
  void kill ( void );

  void clear ( void );
  void flip ( void );

  float getFOV ( void ) { return fov; }
  void setFOV ( float f ) { fov = f; }

  void getBacgroundColor ( float &r, float &g, float &b );
  void setBacgroundColor ( float r, float g, float b );

  void getDesktopRes ( size_t &x, size_t &y );
  void getCurrentRes ( size_t &x, size_t &y );

  bool update ( void );
  void yeld ( float time = 0.01f );

  void addEventCallback ( SimpleDisplayEventCallbacks *callback );
  void removeEventCallback ( SimpleDisplayEventCallbacks *callback );

protected:
  void initGL ( void );
  void setViewport ( void );

  size_t size[2];
  float aspect;
  float nearZ, farZ;
  float fov;
  float clearColor[3];
  bool fullscreen;
  bool valid;

  std::vector<SimpleDisplayEventCallbacks*> callbacks;

  void activate ( void );
  void deactivate ( void );
  void focus ( bool lost );
  void resize ( size_t x, size_t y );
  void key ( int key, bool down, const ModiferKeys& mods );

};

// key defs
#define SD_KEY_BACKSPACE	8
#define SD_KEY_TAB	9
#define SD_KEY_CLEAR	12
#define SD_KEY_RETURN	13
#define SD_KEY_PAUSE	19
#define SD_KEY_ESCAPE	27
#define SD_KEY_SPACE	32
#define SD_KEY_EXCLAIM	33
#define SD_KEY_QUOTEDBL	34
#define SD_KEY_HASH	35
#define SD_KEY_DOLLAR	36
#define SD_KEY_AMPERSAND	38
#define SD_KEY_QUOTE	39
#define SD_KEY_LEFTPAREN	40
#define SD_KEY_RIGHTPAREN	41
#define SD_KEY_ASTERISK	42
#define SD_KEY_PLUS	43
#define SD_KEY_COMMA	44
#define SD_KEY_MINUS	45
#define SD_KEY_PERIOD	46
#define SD_KEY_SLASH	47
#define SD_KEY_0		48
#define SD_KEY_1		49
#define SD_KEY_2		50
#define SD_KEY_3		51
#define SD_KEY_4		52
#define SD_KEY_5		53
#define SD_KEY_6		54
#define SD_KEY_7		55
#define SD_KEY_8		56
#define SD_KEY_9		57
#define SD_KEY_COLON	58
#define SD_KEY_SEMICOLON	59
#define SD_KEY_LESS	60
#define SD_KEY_EQUALS	61
#define SD_KEY_GREATER	62
#define SD_KEY_QUESTION	63
#define SD_KEY_AT		64
/* 
Skip uppercase letters
*/
#define SD_KEY_LEFTBRACKET	  91
#define SD_KEY_BACKSLASH	92
#define SD_KEY_RIGHTBRACKET	  93
#define SD_KEY_CARET	94
#define SD_KEY_UNDERSCORE	95
#define SD_KEY_BACKQUOTE	96
#define SD_KEY_a		97
#define SD_KEY_b		98
#define SD_KEY_c		99
#define SD_KEY_d		100
#define SD_KEY_e		101
#define SD_KEY_f		102
#define SD_KEY_g		103
#define SD_KEY_h		104
#define SD_KEY_i		105
#define SD_KEY_j		106
#define SD_KEY_k		107
#define SD_KEY_l		108
#define SD_KEY_m		109
#define SD_KEY_n		110
#define SD_KEY_o		111
#define SD_KEY_p		112
#define SD_KEY_q		113
#define SD_KEY_r		114
#define SD_KEY_s		115
#define SD_KEY_t		116
#define SD_KEY_u		117
#define SD_KEY_v		118
#define SD_KEY_w		119
#define SD_KEY_x		120
#define SD_KEY_y		121
#define SD_KEY_z		122
#define SD_KEY_DELETE	127
/* End of ASCII mapped keysyms */

/* International keyboard syms */
#define SD_KEY_WORLD_0	160		/* 0xA0 */
#define SD_KEY_WORLD_1	161
#define SD_KEY_WORLD_2	162
#define SD_KEY_WORLD_3	163
#define SD_KEY_WORLD_4	164
#define SD_KEY_WORLD_5	165
#define SD_KEY_WORLD_6	166
#define SD_KEY_WORLD_7	167
#define SD_KEY_WORLD_8	168
#define SD_KEY_WORLD_9	169
#define SD_KEY_WORLD_10	170
#define SD_KEY_WORLD_11	171
#define SD_KEY_WORLD_12	172
#define SD_KEY_WORLD_13	173
#define SD_KEY_WORLD_14	174
#define SD_KEY_WORLD_15	175
#define SD_KEY_WORLD_16	176
#define SD_KEY_WORLD_17	177
#define SD_KEY_WORLD_18	178
#define SD_KEY_WORLD_19	179
#define SD_KEY_WORLD_20	180
#define SD_KEY_WORLD_21	181
#define SD_KEY_WORLD_22	182
#define SD_KEY_WORLD_23	183
#define SD_KEY_WORLD_24	184
#define SD_KEY_WORLD_25	185
#define SD_KEY_WORLD_26	186
#define SD_KEY_WORLD_27	187
#define SD_KEY_WORLD_28	188
#define SD_KEY_WORLD_29	189
#define SD_KEY_WORLD_30	190
#define SD_KEY_WORLD_31	191
#define SD_KEY_WORLD_32	192
#define SD_KEY_WORLD_33	193
#define SD_KEY_WORLD_34	194
#define SD_KEY_WORLD_35	195
#define SD_KEY_WORLD_36	196
#define SD_KEY_WORLD_37	197
#define SD_KEY_WORLD_38	198
#define SD_KEY_WORLD_39	199
#define SD_KEY_WORLD_40	200
#define SD_KEY_WORLD_41	201
#define SD_KEY_WORLD_42	202
#define SD_KEY_WORLD_43	203
#define SD_KEY_WORLD_44	204
#define SD_KEY_WORLD_45	205
#define SD_KEY_WORLD_46	206
#define SD_KEY_WORLD_47	207
#define SD_KEY_WORLD_48	208
#define SD_KEY_WORLD_49	209
#define SD_KEY_WORLD_50	210
#define SD_KEY_WORLD_51	211
#define SD_KEY_WORLD_52	212
#define SD_KEY_WORLD_53	213
#define SD_KEY_WORLD_54	214
#define SD_KEY_WORLD_55	215
#define SD_KEY_WORLD_56	216
#define SD_KEY_WORLD_57	217
#define SD_KEY_WORLD_58	218
#define SD_KEY_WORLD_59	219
#define SD_KEY_WORLD_60	220
#define SD_KEY_WORLD_61	221
#define SD_KEY_WORLD_62	222
#define SD_KEY_WORLD_63	223
#define SD_KEY_WORLD_64	224
#define SD_KEY_WORLD_65	225
#define SD_KEY_WORLD_66	226
#define SD_KEY_WORLD_67	227
#define SD_KEY_WORLD_68	228
#define SD_KEY_WORLD_69	229
#define SD_KEY_WORLD_70	230
#define SD_KEY_WORLD_71	231
#define SD_KEY_WORLD_72	232
#define SD_KEY_WORLD_73	233
#define SD_KEY_WORLD_74	234
#define SD_KEY_WORLD_75	235
#define SD_KEY_WORLD_76	236
#define SD_KEY_WORLD_77	237
#define SD_KEY_WORLD_78	238
#define SD_KEY_WORLD_79	239
#define SD_KEY_WORLD_80	240
#define SD_KEY_WORLD_81	241
#define SD_KEY_WORLD_82	242
#define SD_KEY_WORLD_83	243
#define SD_KEY_WORLD_84	244
#define SD_KEY_WORLD_85	245
#define SD_KEY_WORLD_86	246
#define SD_KEY_WORLD_87	247
#define SD_KEY_WORLD_88	248
#define SD_KEY_WORLD_89	249
#define SD_KEY_WORLD_90	250
#define SD_KEY_WORLD_91	251
#define SD_KEY_WORLD_92	252
#define SD_KEY_WORLD_93	253
#define SD_KEY_WORLD_94	254
#define SD_KEY_WORLD_95	255		/* 0xFF */

/* Numeric keypad */
#define SD_KEY_KP0	256
#define SD_KEY_KP1	257
#define SD_KEY_KP2	258
#define SD_KEY_KP3	259
#define SD_KEY_KP4	260
#define SD_KEY_KP5	261
#define SD_KEY_KP6	262
#define SD_KEY_KP7	263
#define SD_KEY_KP8	264
#define SD_KEY_KP9	265
#define SD_KEY_KP_PERIOD	266
#define SD_KEY_KP_DIVIDE	267
#define SD_KEY_KP_MULTIPLY	  268
#define SD_KEY_KP_MINUS	269
#define SD_KEY_KP_PLUS	270
#define SD_KEY_KP_ENTER	271
#define SD_KEY_KP_EQUALS	272

/* Arrows + Home/End pad */
#define SD_KEY_UP		273
#define SD_KEY_DOWN	274
#define SD_KEY_RIGHT	275
#define SD_KEY_LEFT	276
#define SD_KEY_INSERT	277
#define SD_KEY_HOME	278
#define SD_KEY_END	279
#define SD_KEY_PAGEUP	280
#define SD_KEY_PAGEDOWN	281

/* Function keys */
#define SD_KEY_F1		282
#define SD_KEY_F2		283
#define SD_KEY_F3		284
#define SD_KEY_F4		285
#define SD_KEY_F5		286
#define SD_KEY_F6		287
#define SD_KEY_F7		288
#define SD_KEY_F8		289
#define SD_KEY_F9		290
#define SD_KEY_F10	291
#define SD_KEY_F11	292
#define SD_KEY_F12	293
#define SD_KEY_F13	294
#define SD_KEY_F14	295
#define SD_KEY_F15	296#endif //_SIMPLE_SDL_VIEW_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8