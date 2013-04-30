#ifndef __audio_h
#define __audio_h
#include <lcthw/list.h>
#include <al/al.h>
#include <al/alc.h>

#include "../prefix.h"
#ifdef DABES_IOS
#include "AudioBridge.h"
#endif
#include "ogg_stream.h"

typedef struct Audio {
    Object proto;
    List *ogg_streams;
} Audio;

Audio *Audio_create();
void Audio_check();
void Audio_stream(Audio *audio);
void Audio_destroy(Audio *audio);

#endif
