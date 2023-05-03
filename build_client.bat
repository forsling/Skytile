set WORKSPACE_FOLDER=%cd%

gcc -fdiagnostics-color=always -g ^
%WORKSPACE_FOLDER%/src/main.c ^
%WORKSPACE_FOLDER%/src/game.c ^
%WORKSPACE_FOLDER%/src/client.c ^
%WORKSPACE_FOLDER%/src/game_logic.c ^
%WORKSPACE_FOLDER%/src/texture.c ^
%WORKSPACE_FOLDER%/src/world.c ^
%WORKSPACE_FOLDER%/src/render.c ^
%WORKSPACE_FOLDER%/src/audio.c ^
%WORKSPACE_FOLDER%/src/utils.c ^
%WORKSPACE_FOLDER%/src/vector.c ^
%WORKSPACE_FOLDER%/src/settings.c ^
-o %WORKSPACE_FOLDER%/game.exe ^
-I%WORKSPACE_FOLDER%/include ^
-L%WORKSPACE_FOLDER%/lib ^
-lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_net -lopengl32 -lglu32

echo Build client completed.