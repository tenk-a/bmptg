pushd %~dp0
set TGT=ruig
if not exist %TGT% mkdir %TGT%
pushd %TGT%
cmake ../../src/%TGT%
echo msbuild %TGT%.sln -Property:Configuration=Release >mk_rel.bat
echo msbuild %TGT%.sln -Property:Configuration=Debug   >mk_dbg.bat
popd
popd
