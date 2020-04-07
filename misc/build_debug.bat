set cl_flags=/Gm- /W3 /MP /MTd /Zi /Zl /EHsc /Od
set link_flags=/link /out:game.exe /incremental:no
set src_files=w:\src\*.cpp w:\src\imgui\*.cpp
set lib_files=comdlg32.lib ../lib/glew/glew32.lib opengl32.lib ../lib/sdl2/SDL2.lib ../lib/sdl2/SDL2_image.lib ../lib/sdl2/SDL2_ttf.lib ../lib/chipmunk/chipmunk.lib

pushd w:\build
cl %cl_flags% %src_files% %lib_files% %link_flags% 
del *.obj
popd