#include "audio.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

#define NUM_SOUNDS 10
static Mix_Music* g_music = NULL;
static Mix_Chunk* g_sounds[NUM_SOUNDS] = {NULL};

bool audio_init() {
    // Initialize SDL audio subsystem
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "Failed to initialize SDL audio subsystem: %s\n", SDL_GetError());
        return false;
    }

    // Open the audio device
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0) {
        fprintf(stderr, "Failed to open audio device: %s\n", Mix_GetError());
        return false;
    }

    return true;
}

bool audio_load_music(const char* filename) {
    // Load music from file
    g_music = Mix_LoadMUS(filename);
    if (g_music == NULL) {
        fprintf(stderr, "Failed to load music from file '%s': %s\n", filename, Mix_GetError());
        return false;
    }

    return true;
}

void audio_play_music() {
    // Play the loaded music
    Mix_PlayMusic(g_music, -1);
}

void audio_pause_music() {
    // Pause the music
    Mix_PauseMusic();
}

void audio_resume_music() {
    // Resume playing the music
    Mix_ResumeMusic();
}

void audio_unload_music() {
    // Unload the loaded music
    if (g_music != NULL) {
        Mix_FreeMusic(g_music);
        g_music = NULL;
    }
}

int audio_load_sound(const char* filename) {
    // Load sound effect from file
    for (int i = 0; i < NUM_SOUNDS; i++) {
        if (g_sounds[i] == NULL) {
            g_sounds[i] = Mix_LoadWAV(filename);
            if (g_sounds[i] == NULL) {
                fprintf(stderr, "Failed to load sound effect from file '%s': %s\n", filename, Mix_GetError());
                return -1;
            }
            return i;
        }
    }

    fprintf(stderr, "Failed to load sound effect from file '%s': too many sounds loaded.\n", filename);
    return -1;
}

void audio_play_sound(int sound_id, float volume) {
    if (sound_id >= 0 && sound_id < NUM_SOUNDS && g_sounds[sound_id] != NULL) {
        // Set the volume of the sound effect
        Mix_VolumeChunk(g_sounds[sound_id], (int)(volume * MIX_MAX_VOLUME));

        // Play the specified sound effect
        Mix_PlayChannel(-1, g_sounds[sound_id], 0);
    }
}

void audio_unload_sound() {
    // Unload the loaded sound effects
    for (int i = 0; i < NUM_SOUNDS; i++) {
        if (g_sounds[i] != NULL) {
            Mix_FreeChunk(g_sounds[i]);
            g_sounds[i] = NULL;
        }
    }
}

void audio_quit() {
    // Clean up the audio subsystem
    audio_unload_music();
    audio_unload_sound();
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
