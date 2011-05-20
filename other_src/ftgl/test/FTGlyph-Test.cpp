#include <assert.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

#include "Fontdefs.h"

#include "FTGL/ftgl.h"

class TestGlyph : public FTGlyph
{
    public:
        TestGlyph(FT_GlyphSlot glyph)
        :   FTGlyph(glyph),
            advance(FTPoint(Advance(), 0.0))
        {}

        const FTPoint& Render(const FTPoint& pen, int renderMode) { return advance; };

    private:
        FTPoint advance;
};


class FTGlyphTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(FTGlyphTest);
        CPPUNIT_TEST(testBadConstructor);
        CPPUNIT_TEST(testGoodConstructor);
    CPPUNIT_TEST_SUITE_END();

    public:
        FTGlyphTest() : CppUnit::TestCase("FTGlyph Test")
        {}

        FTGlyphTest(const std::string& name) : CppUnit::TestCase(name) {}

        void testBadConstructor()
        {
            TestGlyph testGlyph(0);

            CPPUNIT_ASSERT(0.0 == testGlyph.Advance());

            CPPUNIT_ASSERT_DOUBLES_EQUAL(0, testGlyph.BBox().Upper().Y(), 0.01);

            CPPUNIT_ASSERT_EQUAL(testGlyph.Error(), 0);
        }


        void testGoodConstructor()
        {
            setUpFreetype(CHARACTER_CODE_A);
            TestGlyph testGlyph(face->glyph);

            float testPoint = 47.0;
            float nextPoint = testGlyph.Advance();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(testPoint, nextPoint, 0.0001);

            CPPUNIT_ASSERT_DOUBLES_EQUAL(51.39, testGlyph.BBox().Upper().Y(), 0.01);

            CPPUNIT_ASSERT(testGlyph.Error() == 0);

            tearDownFreetype();
        }


        void setUp()
        {}


        void tearDown()
        {}

    private:
        FT_Library   library;
        FT_Face      face;

        void setUpFreetype(unsigned int characterIndex)
        {
            FT_Error error = FT_Init_FreeType(&library);
            assert(!error);
            error = FT_New_Face(library, ARIAL_FONT_FILE, 0, &face);
            assert(!error);

            loadGlyph(characterIndex);
        }

        void loadGlyph(unsigned int characterIndex)
        {
            long glyphIndex = FT_Get_Char_Index(face, characterIndex);

            FT_Set_Char_Size(face, 0L, FONT_POINT_SIZE * 64, RESOLUTION, RESOLUTION);

            FT_Error error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
            assert(!error);
        }

        void tearDownFreetype()
        {
            FT_Done_Face(face);
            FT_Done_FreeType(library);
        }

};

CPPUNIT_TEST_SUITE_REGISTRATION(FTGlyphTest);

