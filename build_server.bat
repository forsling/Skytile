set WORKSPACE_FOLDER=%cd%

gcc -fdiagnostics-color=always -g ^
%WORKSPACE_FOLDER%/src/server/server.c ^
%WORKSPACE_FOLDER%/src/shared/game.c ^
%WORKSPACE_FOLDER%/src/server/game_logic.c ^
%WORKSPACE_FOLDER%/src/server/world.c ^
%WORKSPACE_FOLDER%/src/shared/settings.c ^
%WORKSPACE_FOLDER%/src/shared/utils.c ^
%WORKSPACE_FOLDER%/src/shared/vector.c ^
-o %WORKSPACE_FOLDER%/server.exe ^
-I%WORKSPACE_FOLDER%/include ^
-L%WORKSPACE_FOLDER%/lib ^
-lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_net -lopengl32 -lglu32

echo Build server completed.
