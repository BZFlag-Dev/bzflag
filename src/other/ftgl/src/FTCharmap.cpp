#include "FTFace.h"
#include "FTCharmap.h"


FTCharmap::FTCharmap( FTFace* face)
:   ftFace( *(face->Face())),
    err(0)
{
    if( !ftFace->charmap)
    {
        err = FT_Set_Charmap( ftFace, ftFace->charmaps[0]);
    }
    
    ftEncoding = ftFace->charmap->encoding;

    for (int i = 0; i < FTCharmap::MAX_PRECOMPUTED; i++)
    {
      	characterCodeIndices[i] = FT_Get_Char_Index( ftFace, i);
    }
}


FTCharmap::~FTCharmap()
{
    charMap.clear();
}


bool FTCharmap::CharMap( FT_Encoding encoding)
{
    if( ftEncoding == encoding)
    {
        return true;
    }
    
    err = FT_Select_Charmap( ftFace, encoding );
    
    if( !err)
    {
        ftEncoding = encoding;
    }
    else
    {
        ftEncoding = ft_encoding_none;
    }
        
    charMap.clear();
    return !err;
}


unsigned int FTCharmap::GlyphListIndex( unsigned int characterCode )
{
    return charMap.find( characterCode);
}


unsigned int FTCharmap::FontIndex( unsigned int characterCode )
{
    if (characterCode < FTCharmap::MAX_PRECOMPUTED)
    {
	return characterCodeIndices[characterCode];
    }
    return FT_Get_Char_Index( ftFace, characterCode);
}


void FTCharmap::InsertIndex( const unsigned int characterCode, const unsigned int containerIndex)
{
    charMap.insert( characterCode, containerIndex);
}
