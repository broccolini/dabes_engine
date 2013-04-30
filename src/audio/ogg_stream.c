#include "ogg_stream.h"

const int BUFFER_SIZE = 4096 * 8;

int OggStream_stream(OggStream *ogg_stream, ALuint buffer);
void OggStream_empty(OggStream *ogg_stream);
const char *OggStream_error_string(int code);

OggStream *OggStream_create(char *file) {
    OggStream *ogg_stream = calloc(1, sizeof(OggStream));
    check(ogg_stream != NULL, "Couldn't create OGG Stream");

    ogg_stream->file = fopen(resource_path(file), "rb");
    check(ogg_stream->file != NULL, "Couldn't open OGG file");

    int result = ov_open(ogg_stream->file, &ogg_stream->stream, NULL, 0);
    check(result >= 0, "Couldn't open OGG stream: %s",
            OggStream_error_string(result));

    ogg_stream->vorbis_info = ov_info(&ogg_stream->stream, -1);
    ogg_stream->vorbis_comment = ov_comment(&ogg_stream->stream, -1);

    if (ogg_stream->vorbis_info->channels == 1) {
        ogg_stream->format = AL_FORMAT_MONO16;
    } else {
        ogg_stream->format = AL_FORMAT_STEREO16;
    }

    alGenBuffers(2, ogg_stream->buffers);
    Audio_check();
    alGenSources(1, &ogg_stream->source);
    Audio_check();

    alSource3f(ogg_stream->source, AL_POSITION, 0.0, 0.0, 0.0);
    alSource3f(ogg_stream->source, AL_VELOCITY, 0.0, 0.0, 0.0);
    alSource3f(ogg_stream->source, AL_DIRECTION, 0.0, 0.0, 0.0);
    alSourcef(ogg_stream->source, AL_ROLLOFF_FACTOR, 0.0);
    alSourcei(ogg_stream->source, AL_SOURCE_RELATIVE, AL_TRUE);

    return ogg_stream;
error:
    if (ogg_stream->file) fclose(ogg_stream->file);
    if (ogg_stream) free(ogg_stream);
    return NULL;
}

void OggStream_destroy(OggStream *ogg_stream) {
    check(ogg_stream != NULL, "No OGG stream to destroy");
    alSourceStop(ogg_stream->source);
    OggStream_empty(ogg_stream);
    alDeleteSources(1, &ogg_stream->source);
    Audio_check();
    alDeleteBuffers(2, ogg_stream->buffers);
    Audio_check();

    ov_clear(&ogg_stream->stream);
    free(ogg_stream);
error:
    return;
}

void OggStream_debug(OggStream *ogg_stream) {
    printf("Version         %d\n", ogg_stream->vorbis_info->version);
    printf("Channels        %d\n", ogg_stream->vorbis_info->channels);
    printf("rate (hz)       %ld\n", ogg_stream->vorbis_info->rate);
    printf("bitrate upper   %ld\n", ogg_stream->vorbis_info->bitrate_upper);
    printf("bitrate nominal %ld\n", ogg_stream->vorbis_info->bitrate_nominal);
    printf("bitrate lower   %ld\n", ogg_stream->vorbis_info->bitrate_lower);
    printf("bitrate window  %ld\n\n", ogg_stream->vorbis_info->bitrate_window);
    printf("vendor  %s\n", ogg_stream->vorbis_comment->vendor);

    int i = 0;
    for (i = 0; i < ogg_stream->vorbis_comment->comments; i++) {
        printf("    %s\n", ogg_stream->vorbis_comment->user_comments[i]);
    }
}

int OggStream_play(OggStream *ogg_stream) {
    if (OggStream_playing(ogg_stream)) return 1;

    check(OggStream_stream(ogg_stream, ogg_stream->buffers[0]) == 1,
        "Failed to stream OGG buffer 0");
    check(OggStream_stream(ogg_stream, ogg_stream->buffers[1]) == 1,
        "Failed to stream OGG buffer 1");

    alSourceQueueBuffers(ogg_stream->source, 2, ogg_stream->buffers);
    alSourcePlay(ogg_stream->source);

    return 1;
error:
    return 0;
}

int OggStream_playing(OggStream *ogg_stream) {
    ALenum state;
    alGetSourcei(ogg_stream->source, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

int OggStream_update(OggStream *ogg_stream) {
    int processed;
    int active = 1;

    alGetSourcei(ogg_stream->source, AL_BUFFERS_PROCESSED, &processed);

    while (processed--) {
        ALuint buffer;

        alSourceUnqueueBuffers(ogg_stream->source, 1, &buffer);
        Audio_check();

        active = OggStream_stream(ogg_stream, buffer);

        alSourceQueueBuffers(ogg_stream->source, 1, &buffer);
        Audio_check();
    }

    return active;
}

int OggStream_stream(OggStream *ogg_stream, ALuint buffer) {
    char data[BUFFER_SIZE];
    int size = 0;
    int section;
    int result;

    while (size < BUFFER_SIZE) {
        result = ov_read(&ogg_stream->stream, data + size, BUFFER_SIZE - size,
                0, 2, 1, &section);

        if (result > 0) {
            size += result;
        } else {
            if (result < 0) {
                debug("%s", OggStream_error_string(result));
                return 0;
            } else {
                break;
            }
        }
    }

    if (size == 0) return 0;

    alBufferData(buffer, ogg_stream->format, data, size,
            ogg_stream->vorbis_info->rate);
    Audio_check();

    return 1;
}

void OggStream_empty(OggStream *ogg_stream) {
    int queued;

    alGetSourcei(ogg_stream->source, AL_BUFFERS_QUEUED, &queued);

    while (queued--) {
        ALuint buffer;

        alSourceUnqueueBuffers(ogg_stream->source, 1, &buffer);
        Audio_check();
    }
}

const char *OggStream_error_string(int code) {
    switch(code) {
        case OV_EREAD:
            return "Read from media.";
        case OV_ENOTVORBIS:
            return "Not Vorbis data.";
        case OV_EVERSION:
            return "Vorbis version mismatch.";
        case OV_EBADHEADER:
            return "Invalid Vorbis header.";
        case OV_EFAULT:
            return "Internal logic fault (bug or heap/stack corruption)";
        default:
            return "Unknown OGG error.";
    }
}
