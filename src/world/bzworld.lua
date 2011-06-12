--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--
-- the lua script is sent over the wire as compressed text.
-- Also included is the map of custom map object start/end tokens,
-- and the results from every CustomObject() call. The results
-- from FetchURL and IncludeURL() are stored with size, date,
-- and md5 hashes. If the client can not retrieve the data from
-- the URL (or if it doesn't match), it will request it from the
-- server (which has to keep a copy of all URL data). Local URLs
-- (aka: "file:/"), are always embedded.


-- for .bzw maps, the following could be done: appended to the beginning
if false then
  bzworld.IncludeURL('http://www.bzflag.org/bzworld/bzw2lua.lua')
  BZW2LUA(bzworld.FetchURL('themap.bzw'))
  return
end

--------------------------------------------------------------------------------
--
--  These 3 calls are required
--

bzworld.SetAuthor('trepan')

bzworld.SetLicense('permissive')

--bzworld.SetWorldSize(800.0)

--------------------------------------------------------------------------------

bzworld.AddOption('-fb -sb')

bzworld.AddOption('-ms 10 +r')

--------------------------------------------------------------------------------

-- the following could define simple functions like:
--   Box { name = '', pos = { 0,0,0 }, size = { 1, 1, 1}, rotation = 90 }

--bzworld.IncludeURL('http://www.bzflag.org/bzworld/primitives.lua')

--------------------------------------------------------------------------------

local customObjects = bzworld.GetCustomObjects()
for k, v in pairs(customObjects) do
  print('CustomObject', k, v)
end
-- {
--   superbox = 'end',
--   lua      = 'endlua',
--   etc...
-- }


bzworld.CustomObject('specialbox', [[
  position 0 1 2
  size 6 7 8
  glitter 2.5
]])

--------------------------------------------------------------------------------
--
--  The transform matrix converts all mesh vertices and normals.
--  It also affects world text objects, and physicsDriver positions.
--

bzworld.SetTransform(a00, a01, a02, a03, -- transforms vertices and normals
                     a10, a11, a12, a13,
                     a20, a21, a22, a23,
                     a30, a31, a32, a33)

bzworld.SetTransform({ a00, a01, a02, a03, -- alternate form
                       a10, a11, a12, a13,
                       a20, a21, a22, a23,
                       a30, a31, a32, a33 })

--------------------------------------------------------------------------------

bzworld.AddPhysicsDriver('required_name', {
  linear  = { 0.0, 0.0, 20.0 },
  angular = { rate = 20.0, pos = { 0.0, 20.0 } },
  radial  = { rate = 10.0, pos = { 20.0, 0.0 } },
  slide   = 1,
  death   = '',
  linearVar  = '',
  angularVar = '',
  radialVar  = '',
  slideVar   = '',
  deathVar   = '',
})

--------------------------------------------------------------------------------

bzworld.AddDynamicColor('required_name', {
  states = {
    { time = 1.0, color = 'red'                  },
    { time = 2.0, color = 'blue 0.75'            },
    { time = 1.0, color = '0.0 1.0 1.0 1.0'      },
    { time = 3.0, color = { 'green', 0.3 }       },
    { time = 1.0, color = { 0.0, 1.0, 0.0, 0.5 } },
    -- etc...
  },
  varName = '',
  varTime = 2.0,
  varNoAlpha = true,
  delay = 7.0,
  teamMask = { 'green', 'hunter'},
})

--------------------------------------------------------------------------------

bzworld.AddTextureMatrix('required_name', {
  staticSpin   = 0.0,
  staticShift  = { 0.0, 0.0 },
  staticScale  = { 0.0, 0.0 },
  staticCenter = { 0.0, 0.0 },
  spin   = 0.0,
  shift  = { 0.0, 0.0 },
  scale  = { 0.0, 0.0 },
  center = { 0.0, 0.0 },
  spinVar  = '',
  shiftVar = '',
  scaleVar = '',
})

--------------------------------------------------------------------------------

bzworld.AddMaterial('required_name', {
  dynamicColor = '',
  ambient   = { 0.0, 0.0, 0.0, 1.0 },
  diffuse   = { 1.0, 1.0, 1.0, 1.0 },
  specular  = { 0.0, 0.0, 0.0, 1.0 },
  emisssion = { 0.0, 0.0, 0.0, 1.0 },
  shininess = 16.0,
  blending = 'additive',
  occluder      = false,
  groupAlpha    = false,
  lighting      = true,
  radar         = true,
  radarOutline  = true,
  shadowCast    = true,
  shadowReceive = true,
  textureShadow = false,
  culling       = false,
  sorting       = true,
  flatShading   = false,
  alphaThreshold = 0.5,
  polygonOffset = { factor = 2.0, units = 7.0 },
  textures = { {
      source = 'green.png',
      textureMatrix = '',
      autoScale = { 2.0, 1.0 },
      combineMode = 0, -- FIXME -- string ?
      useTextureAlpha = true,
      useColorOnTexture = false,
      useSphereMap = false,
    },{
      source = 'dirt.png',
      textureMatrix = '',
      autoScale = { -1.0, 1.0 },
      combineMode = 0, -- FIXME -- string ?
      useTextureAlpha = true,
      useColorOnTexture = false,
      useSphereMap = false,
    },
    -- etc...
  },
  shaders = {
    { source = '' },
    { source = '' },
    -- etc...
  },
})

--------------------------------------------------------------------------------

bzworld.AddText('optional_name', {
  data = 'this is a string',
  varData = false,
  font = 'http://coolfonts/uber.ttf',
  fontSize = 24.0,
  justify = 0.5,
  lineSpace = 1.0,
  fixedWidth = 100.0,
  lengthPerPixel = 2.0,
  billboard = false,
})

--------------------------------------------------------------------------------

bzworld.AddWall('optional_name', {
  pos = { 400.0, 0.0, 0.0 },
  size = 800,
  rotation = 180.0,
})

--------------------------------------------------------------------------------

bzworld.AddLink('optional_name', {
  src = '',
  dst = '',
  shotSameSpeed = false,
  tankSameSpeed = false,
  shotPosScale = { 1.0, 1.0, 1.0 },
  tankPosScale = { 1.0, 1.0, 1.0 },
  shotSrcVelScale = { 1.0, 1.0, 1.0 },
  tankSrcVelScale = { 1.0, 1.0, 1.0 },
  shotDstVelOffset = { 0.0, 0.0, 0.0},
  tankDstVelOffset = { 0.0, 0.0, 0.0 },
  tankAngle       = 0.0,
  tankAngleScale  = 1.0,
  tankAngleOffset = 0.0,
  tankAngVel       = 0.0,
  tankAngVelScale  = 1.0,
  tankAngVelOffset = 0.0,
  shotMinSpeed = 0.0,
  tankMinSpeed = 0.0,
  shotMaxSpeed = 0.0,
  tankMaxSpeed = 0.0,
  shotMinAngle = 0.0,
  tankMinAngle = 0.0,
  shotMaxAngle = 0.0,
  tankMaxAngle = 0.0,
  shotBlockTeams = { 'red', 'green' },
  tankBlockTeams = { 'red', 'green' },
  shotBlockFlags = { 'GM', 'IB' },
  tankBlockFlags = { 'GM', 'IB' },
  shotBlockVar = '',
  tankBlockVar = '',
  shotPassText = '',
  tankPassText = '',
  tankPassSound = '',
})

--------------------------------------------------------------------------------
--
--  Because of the size of the tables that may be required to build large
--  meshes, I'm starting with a functional approach to mesh definition. An
--  AddMesh() wrapper can easily be made with lua code.
--
bzworld.CreateMesh('optional_name', function(mesh, ...)
  mesh:AddVertex(x, y, z)      -- repeatable
  mesh:AddVertex({ x, y, z })  -- repeatable
  mesh:AddNormal(x, y, z)      -- repeatable
  mesh:AddNormal({ x , y, z }) -- repeatable
  mesh:AddTexCoord(u ,v)       -- repeatable
  mesh:AddTexCoord({ u ,v })   -- repeatable

  mesh:AddWeapon({ -- repeatable
    shotType  = 'GM',
    team      = 'red',
    initdelay = 0.0,
    delays    = { 1.0, 2.0, 1.0, 4.0 },
    posVertex = 2,
    dirNormal = 3,
  })

  mesh:AddFace({ -- repeatable
    vertices  = { 1.0, 2.0, 4.0, 6.0 },
    normals   = { 1.0, 2.0, 4.0, 6.0 },
    texCoords = { 1.0, 2.0, 4.0, 6.0 },

    driveThrough = false,
    shootThrough = false,
    ricochet     = true,
    smoothBounce = false,
    clustered    = true,

    baseTeam = 'purple',

    zone = {
      teams      = { 'blue', 'red' },
      safeties   = { 'blue', 'red' },
      flags      = { 'SW', 'GM' },
      fixedFlags = { GM = 1, ['R*'] = 6 },
      height     = 1.0,
      weight     = 0.0,
      center     = true,
    },

    link = {
      name        = 'myLink', -- required
      srcTouch    = true,
      srcRebound  = false,
      srcNoGlow   = true,
      srcNoSound  = false,
      srcNoEffect = true,
      srcCenter   = 0,  -- vertex index
      srcSdir     = 0,  -- normal index  ("Side"  normal)
      srcTdir     = 0,  -- normal index  ("Top"   normal)
      srcPdir     = 0,  -- normal index  ("Plane" normal)
      srcSscale   = 1.0,
      srcTscale   = 1.0,
      srcPscale   = 0.0,
      dstCenter   = 0,  -- vertex index
      dstSdir     = 0,  -- normal index  ("Side"  normal)
      dstTdir     = 0,  -- normal index  ("Top"   normal)
      dstPdir     = 0,  -- normal index  ("Plane" normal)
      dstSscale   = 1.0,
      dstTscale   = 1.0,
      dstPscale   = 0.0,
    },
  })

end, 'extra', 'args', 'go', 'here')

--------------------------------------------------------------------------------
