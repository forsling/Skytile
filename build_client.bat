set WORKSPACE_FOLDER=%cd%

gcc -fdiagnostics-color=always -g ^
%WORKSPACE_FOLDER%/src/client/main.c ^
%WORKSPACE_FOLDER%/src/client/client.c ^
%WORKSPACE_FOLDER%/src/shared/game.c ^
%WORKSPACE_FOLDER%/src/client/texture.c ^
%WORKSPACE_FOLDER%/src/client/render.c ^
%WORKSPACE_FOLDER%/src/client/audio.c ^
%WORKSPACE_FOLDER%/src/shared/utils.c ^
%WORKSPACE_FOLDER%/src/shared/vector.c ^
%WORKSPACE_FOLDER%/src/shared/settings.c ^
-o %WORKSPACE_FOLDER%/game.exe ^
-I%WORKSPACE_FOLDER%/include ^
-L%WORKSPACE_FOLDER%/lib ^
-lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_net -lopengl32 -lglu32

echo Build client completed.