set cl_flags=/Gm- /W3 /MP /MTd /Z7 /Zl /Od /EHsc
set link_flags=/out:game.exe /incremental:no
set src_files=w:\src\*.cpp w:\src\imgui\*.cpp
set lib_files="kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" "libcmtd.lib" ../lib/glew/glew32.lib opengl32.lib ../lib/sdl2/SDL2.lib ../lib/sdl2/SDL2_image.lib ../lib/sdl2/SDL2_ttf.lib

pushd w:\build
cl %cl_flags% %src_files% %lib_files% /link %link_flags% 
popd