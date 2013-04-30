#ifndef __ogg_stream_h
#define __ogg_stream_h
#include <stdio.h>
#include <stdlib.h>
#include <AL/al.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>
#include "../prefix.h"
#include "audio.h"

typedef struct OggStream {
    FILE *file;
    OggVorbis_File stream;
    vorbis_info *vorbis_info;
    vorbis_comment *vorbis_comment;

    ALuint buffers[2];
    ALuint source;
    ALenum format;
} OggStream;

OggStream *OggStream_create(char *file);
void OggStream_destroy(OggStream *ogg_stream);
void OggStream_debug(OggStream *ogg_stream);
int OggStream_play(OggStream *ogg_stream);
int OggStream_playing(OggStream *ogg_stream);
int OggStream_update(OggStream *ogg_stream);

#endif
