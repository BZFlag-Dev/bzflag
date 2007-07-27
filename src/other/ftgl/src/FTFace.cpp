#include "FTFace.h"
#include "FTLibrary.h"

#include FT_TRUETYPE_TABLES_H

FTFace::FTFace( const char* fontFilePath, bool precomputeKerning)
:   numGlyphs(0),
    fontEncodingList(0),
    err(0)
{
    const FT_Long DEFAULT_FACE_INDEX = 0;
    ftFace = new FT_Face;

    err = FT_New_Face( *FTLibrary::Instance().GetLibrary(), fontFilePath, DEFAULT_FACE_INDEX, ftFace);

    if( err)
    {
        delete ftFace;
        ftFace = 0;
    }
    else
    {
        numGlyphs = (*ftFace)->num_glyphs;
        hasKerningTable = FT_HAS_KERNING((*ftFace)) != 0;

	if (hasKerningTable && precomputeKerning)
	{
	    FT_Vector kernAdvance;
	    kernAdvance.x = kernAdvance.y = 0;
	    precomputedKerningAvailable = true;
	    for (int i = 0; i < 128; i++)
	    {
		for (int j = 0; j < 128; j++)
		{
		    err = FT_Get_Kerning( *ftFace, i, j, ft_kerning_unfitted, &kernAdvance);
		    if( !err)
		    {
			precomputedKerning[i][j][0] = static_cast<float>( kernAdvance.x) / 64.0f;
			precomputedKerning[i][j][1] = static_cast<float>( kernAdvance.y) / 64.0f;
		    }
		    else
		    {
			precomputedKerningAvailable = false;
			break;
		    }
		}
	    }
	}
    }
}


FTFace::FTFace( const unsigned char *pBufferBytes, size_t bufferSizeInBytes)
:   numGlyphs(0),
    err(0)
{
    const FT_Long DEFAULT_FACE_INDEX = 0;
    ftFace = new FT_Face;

    err = FT_New_Memory_Face( *FTLibrary::Instance().GetLibrary(), (FT_Byte *)pBufferBytes, (FT_Long)bufferSizeInBytes, DEFAULT_FACE_INDEX, ftFace);

    if( err)
    {
        delete ftFace;
        ftFace = 0;
    }
    else
    {
        numGlyphs = (*ftFace)->num_glyphs;
    }
}


FTFace::~FTFace()
{
    if( ftFace)
    {
        FT_Done_Face( *ftFace);
        delete ftFace;
        ftFace = 0;
    }
}


bool FTFace::Attach( const char* fontFilePath)
{
    err = FT_Attach_File( *ftFace, fontFilePath);
    return !err;
}


bool FTFace::Attach( const unsigned char *pBufferBytes, size_t bufferSizeInBytes)
{
    FT_Open_Args open;

    open.flags = FT_OPEN_MEMORY;
    open.memory_base = (FT_Byte *)pBufferBytes;
    open.memory_size =(FT_Long) bufferSizeInBytes;

    err = FT_Attach_Stream( *ftFace, &open);
    return !err;
}


const FTSize& FTFace::Size( const unsigned int size, const unsigned int res)
{
    charSize.CharSize( ftFace, size, res, res);
    err = charSize.Error();

    return charSize;
}


unsigned int FTFace::CharMapCount()
{
    return (*ftFace)->num_charmaps;
}


FT_Encoding* FTFace::CharMapList()
{
    if( 0 == fontEncodingList)
    {
        fontEncodingList = new FT_Encoding[CharMapCount()];
        for( size_t encodingIndex = 0; encodingIndex < CharMapCount(); ++encodingIndex)
        {
            fontEncodingList[encodingIndex] = (*ftFace)->charmaps[encodingIndex]->encoding;
        }
    }
    
    return fontEncodingList;
}


FTPoint FTFace::KernAdvance( unsigned int index1, unsigned int index2)
{
    float x, y;
    x = y = 0.0f;

    if( hasKerningTable && index1 && index2)
    {
	if (precomputedKerningAvailable) {
	    x = precomputedKerning[index1][index2][0];
	    y = precomputedKerning[index1][index2][1];
	}
	else
	{
	    FT_Vector kernAdvance;
	    kernAdvance.x = kernAdvance.y = 0;
	  
	    err = FT_Get_Kerning( *ftFace, index1, index2, ft_kerning_unfitted, &kernAdvance);
	    if( !err)
	    {   
		x = static_cast<float>( kernAdvance.x) / 64.0f;
		y = static_cast<float>( kernAdvance.y) / 64.0f;
	    }
	}
    }
    
    return FTPoint( x, y, 0.0);
}


FT_GlyphSlot FTFace::Glyph( unsigned int index, FT_Int load_flags)
{
    err = FT_Load_Glyph( *ftFace, index, load_flags);
    if( err)
    {
        return NULL;
    }

    return (*ftFace)->glyph;
}

