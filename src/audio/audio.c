#include "audio.h"

ALfloat listenerPos[]={0.0,0.0,4.0};
ALfloat listenerVel[]={0.0,0.0,0.0};
ALfloat listenerOri[]={0.0,0.0,1.0,0.0,1.0,0.0};

Audio *Audio_create() {
    Audio *audio = calloc(1, sizeof(Audio));
    check(audio != NULL, "Couldn't create audio");
    audio->ogg_streams = List_create();

#ifdef DABES_IOS
#else
   int audio_rate = 44100;
    Uint16 audio_format = AUDIO_S16;
    int audio_channels = 2;
    int audio_buffers = 4096;
    if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
        printf("Unable to open audio!\n");
        return 0;
    }
#endif
    const ALCchar *defaultDevice =
        alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    if (defaultDevice == NULL) return audio;
    debug("%s", defaultDevice);
    ALCdevice *device = alcOpenDevice(defaultDevice);
    if (device == NULL) return audio;
    ALCcontext *context = alcCreateContext(device, NULL);
    if (context == NULL) return audio;

    if (alcMakeContextCurrent(context) == AL_FALSE) return audio;
    // GHETTO ERROR CHECKING

    Audio_check();
    alListenerfv(AL_POSITION,listenerPos);
    Audio_check();
    alListenerfv(AL_VELOCITY,listenerVel);
    Audio_check();
    alListenerfv(AL_ORIENTATION,listenerOri);
    Audio_check();

    return audio;
error:
    return NULL;
}

void Audio_check() {
    int error = alGetError();
    check(error == AL_NO_ERROR, "OpenAL error code: %d", error);
    return;
error:
    return;
}

void Audio_stream(Audio *audio) {
    LIST_FOREACH(audio->ogg_streams, first, next, current) {
        OggStream *ogg_stream = current->value;
        OggStream_update(ogg_stream);
    }
}

void Audio_destroy(Audio *audio) {
    check(audio != NULL, "No audio to destroy");
    LIST_FOREACH(audio->ogg_streams, first, next, current) {
        OggStream *ogg_stream = current->value;
        OggStream_destroy(ogg_stream);
    }
    List_destroy(audio->ogg_streams);
    free(audio);
error:
    return;
}
