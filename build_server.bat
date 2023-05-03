set WORKSPACE_FOLDER=%cd%

gcc -fdiagnostics-color=always -g ^
%WORKSPACE_FOLDER%/src/server/server.c ^
%WORKSPACE_FOLDER%/src/game.c ^
%WORKSPACE_FOLDER%/src/game_logic.c ^
%WORKSPACE_FOLDER%/src/world.c ^
%WORKSPACE_FOLDER%/src/settings.c ^
%WORKSPACE_FOLDER%/src/utils.c ^
%WORKSPACE_FOLDER%/src/vector.c ^
-o %WORKSPACE_FOLDER%/server.exe ^
-I%WORKSPACE_FOLDER%/include ^
-L%WORKSPACE_FOLDER%/lib ^
-lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_net -lopengl32 -lglu32

echo Build server completed.
