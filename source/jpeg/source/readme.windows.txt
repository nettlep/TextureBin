
Here's how the Windows version of the library was built:

1. Extract the jpegsrc.v6b.tar.gz file somewhere

2. Open a command prompt and go into the folder where the stuff was extracted

3. Make sure your Visual Studio environment variables are setup. For VS.NET, use the following command:

   "C:\Program Files\Microsoft Visual Studio .NET\Common7\Tools\vsvars32.bat"

4. Rename "jconfig.vc" to "jconfig.h"

5. Run this command:

   nmake nodebug=1 -f makefile.vc

6. Wait for it to finish

7. Copy the following files to the jpeg-win folder:

   jconfig.h
   jmorecfg.h
   jpeglib.h
   libjpeg.lib

8. You're done