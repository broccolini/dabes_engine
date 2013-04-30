#include "music.h"

Music *Music_load(Audio *audio, char *filename, char *loop) {
  Music *music = malloc(sizeof(Music));
  music->volume = 1;

#ifdef DABES_IOS
    music->bridge = calloc(1, sizeof(struct AudioBridge));
    AudioBridge_load(music->bridge, filename);
    if (loop != NULL) {
      AudioBridge_loop(music->bridge, loop);
    }
#else
    // music->mix_music = Mix_LoadMUS(filename);

    // if (music->mix_music == NULL) {
    //     printf("Mix_LoadMUS: %s\n", Mix_GetError());
    // }
#endif
    music->ogg_stream = OggStream_create(filename);
    List_push(audio->ogg_streams, music->ogg_stream); // SO GHETTO
    // TODO: Loop
    Music_play(music);
    return music;
}

void Music_destroy(Music *music) {
#ifdef DABES_IOS
    AudioBridge_destroy(music->bridge);
#else
    Mix_FreeMusic(music->mix_music);
#endif
    OggStream_destroy(music->ogg_stream);
    free(music);
}

void Music_play(Music *music) {
#ifdef DABES_IOS
    AudioBridge_play(music->bridge);
#else
    //Mix_PlayMusic(music->mix_music, -1);
#endif
    OggStream_play(music->ogg_stream);
}

void Music_pause(Music *music) {
#ifdef DABES_IOS
    AudioBridge_pause(music->bridge);
#else
    (void)(music);
    //Mix_HaltMusic();
#endif
    // TODO: Pause OGG
}

void Music_set_volume(Music *music, double volume) {
    music->volume = volume;
#ifdef DABES_IOS
    AudioBridge_set_volume(music->bridge, volume);
#else
    // Mix_VolumeMusic(volume * 128.f);
#endif
    // TODO: OGG Volume
}

