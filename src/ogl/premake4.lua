
project   'libOGL'
  targetname 'OGL'
  kind  'StaticLib'
  objdir '.objs'
  files {
    'OpenGLContext.cpp',
    'OpenGLGState.cpp',
    'OpenGLLight.cpp',
    'OpenGLMaterial.cpp',
    'OpenGLPassState.cpp',
    'OpenGLTexture.cpp',
    'OpenGLUtils.cpp',
    'RenderNode.cpp',
  }
  includedirs { '..', '../bzflag', '../clientbase' }



