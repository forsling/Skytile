#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
bool audio_init();
void audio_set_volume(float volume);
bool audio_load_music(const char* filename);
void audio_play_music();
void audio_pause_music();
void audio_resume_music();
void audio_unload_music();
int audio_load_sound(const char* filename);
void audio_play_sound(int sound_id, float volume);
void audio_unload_sound();
void audio_quit();

#endif // AUDIO_H
