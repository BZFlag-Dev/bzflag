TextTool2

This is a re-write of the TextTool program. This version uses FreeType2 and
libPNG to render font textures and write the appropriate files. This tool
requires that you provide the directory where the TrueType font files may be
found as the first argument, and the directory where the output files should
be written as the second argument.

This tool links against FreeType2, libPNG, libz, and bzip2. Although the
source code should compile on most *nix platforms, right now only a Mac OS X
project file is provided to avoid adding dependencies to the main BZFlag
project.

PLEASE NOTE: if you increase the font sizes resulting in larger texture
sizes, please make sure to edit the default "maxTextureSize" BZDB setting
as well to avoid the BZFlag client downsampling your textures.
