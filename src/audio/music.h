#ifndef __music_h
#define __music_h
#include "ogg_stream.h"

typedef struct Music {
  double volume;
#ifdef DABES_IOS
  struct AudioBridge *bridge;
#else
  Mix_Music *mix_music;
#endif
  OggStream *ogg_stream;
} Music;

Music *Music_load(Audio *audio, char *filename, char *loop);
void Music_destroy(Music *music);
void Music_play(Music *music);
void Music_pause(Music *music);
void Music_set_volume(Music *music, double volume);

#endif
