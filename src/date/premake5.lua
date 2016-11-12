project "date"
  kind "StaticLib"
  files { "*.cxx", "*.h" }
  filter "system:macosx"
    prebuildcommands { "XCODE_VERSION=`echo $XCODE_VERSION_ACTUAL | awk '{print $1 + 0}'`\necho \"#define XCODE_VERSION \\\"$XCODE_VERSION\\\"\n#if defined __llvm__ && defined __x86_64__\n  #define BZ_BUILD_ARCH_STR \\\"64\\\"\n#elif defined __llvm__ && defined __i386__\n  #define BZ_BUILD_ARCH_STR \\\"32\\\"\n#endif\n#ifdef DEBUG\n  #define BZ_BUILD_DEBUG_STR \\\"dbg\\\"\n#else\n  #define BZ_BUILD_DEBUG_STR \\\"\\\"\n#endif\n#define BZ_BUILD_OS \\\"mac\\\" BZ_BUILD_ARCH_STR \\\"xc\\\" XCODE_VERSION BZ_BUILD_DEBUG_STR\" > ../Xcode/buildinfo.h" } -- unfortunately this has to be one line or premake eats part of it... might be a bug that prevents any repeat commands in the table
  filter { "system:macosx", "options:not disable-client" }
    prebuildcommands { -- FIXME this is really client-related, not date, but
		       -- it runs too late to generate the file; when premake
		       -- supports the "None" project kind for Xcode, create
		       -- one and move this to that
      "MAJOR_VERSION=`sed -e 's/#.*define.*BZ_MAJOR_VERSION[^0-9]*\\(.*\\)/\\1/' -e t -e d < ../src/date/buildDate.cxx`",
      "MINOR_VERSION=`sed -e 's/#.*define.*BZ_MINOR_VERSION[^0-9]*\\(.*\\)/\\1/' -e t -e d < ../src/date/buildDate.cxx`",
      "REV=`sed -e 's/#.*define.*BZ_REV[^0-9]*\\(.*\\)/\\1/' -e t -e d < ../src/date/buildDate.cxx`",
      "BZFLAG_VERSION=\"$MAJOR_VERSION.$MINOR_VERSION.$REV\"",
      "echo \"<?xml version=\\\"1.0\\\" encoding=\\\"UTF-8\\\"?>",
      "<!DOCTYPE plist PUBLIC \\\"-//Apple//DTD PLIST 1.0//EN\\\" \\\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\\\">",
      "<plist version=\\\"1.0\\\">",
      "<dict>",
      "<key>CFBundleDevelopmentRegion</key><string>en</string>",
      "<key>CFBundleExecutable</key><string>\\${EXECUTABLE_NAME}</string>",
      "<key>CFBundleIconFile</key><string>BZFlag</string>",
      "<key>CFBundleIdentifier</key><string>org.BZFlag</string>",
      "<key>CFBundleInfoDictionaryVersion</key><string>6.0</string>",
      "<key>CFBundleName</key><string>BZFlag</string>",
      "<key>CFBundlePackageType</key><string>APPL</string>",
      "<key>CFBundleShortVersionString</key><string>$BZFLAG_VERSION</string>",
      "<key>CFBundleSignature</key><string>????</string>",
      "<key>CFBundleVersion</key><string>$BZFLAG_VERSION</string>",
      "<key>LSApplicationCategoryType</key><string>public.app-category.arcade-games</string>",
      "<key>LSMinimumSystemVersion</key><string>\\${MACOSX_DEPLOYMENT_TARGET}</string>",
      "<key>NSHumanReadableCopyright</key><string>Copyright (c) 1993-2016 Tim Riker</string>",
      "</dict>",
      "</plist>\" > ../Xcode/BZFlag-Info.plist";
    }
