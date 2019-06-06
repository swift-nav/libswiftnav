CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars%1%.bat"
git submodule update --init --recursive
md build
cd build
cmake ..
CALL msbuild libswiftnav.sln /p:Configuration="Release" /p:Platform="Win32"
cd ..
