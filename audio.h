#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

// Initialize the audio subsystem
bool audio_init();

// Load music from a file
bool audio_load_music(const char* filename);

// Play the loaded music
void audio_play_music();

// Pause the music
void audio_pause_music();

// Resume playing the music
void audio_resume_music();

// Unload the music
void audio_unload_music();

// Load a sound effect from a file
int audio_load_sound(const char* filename);

// Play the specified sound effect
void audio_play_sound(int sound_id, float volume);

// Unload the sound effects
void audio_unload_sound();

// Clean up the audio subsystem
void audio_quit();

#endif // AUDIO_H
