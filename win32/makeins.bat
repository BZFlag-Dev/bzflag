#Make sure to add the directory where the NSIS installer maker is to 
#Tools\Options\Directories\Executable Files

cd ..\package\win32\nsis
makensis bzflag.nsi
