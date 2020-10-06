#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>


#define CHANNELS 2
#define PI 3.14159265

#define VOLUME 10000.0

static void audio_callback(void *userdata, Uint8 *stream, int len);

typedef struct {
    int size;
    double samples_per_second;
    double tone_volume;
    double previous_tone_volume;
    double tone_hz;
    double previous_tone_hz;
    double advance;
    double time;
    Sint16 *buffer;
} Audio_Data;

static SDL_AudioDeviceID device;
static Audio_Data *audio_data;

void a_set_tone(double tone_hz, double tone_volume) {
    audio_data->tone_hz = tone_hz;
    audio_data->tone_volume = tone_volume;
}

void a_stop_playing() {
    SDL_ClearQueuedAudio(device);
    a_set_tone(0.0, VOLUME);
}

void a_start_playing() {
    SDL_PauseAudioDevice(device, 0);
}

void a_init(void) {
    SDL_AudioSpec wanted_spec;
    SDL_AudioSpec obtained_spec;

    audio_data = malloc(sizeof(Audio_Data));

    audio_data->buffer = malloc(sizeof(Sint16) * 8192); // calloc here for setting to 0?
    audio_data->size = 8192;
    audio_data->samples_per_second = 44000.0;
    audio_data->advance = 1.0/audio_data->samples_per_second;
    audio_data->time = 0.0;
    audio_data->tone_volume = VOLUME;
    audio_data->previous_tone_volume = VOLUME;
    audio_data->tone_hz = 0.0;
    audio_data->previous_tone_hz = 0.0;

    wanted_spec.freq = audio_data->samples_per_second;
    wanted_spec.format = AUDIO_S16;
    wanted_spec.channels = CHANNELS;
    wanted_spec.samples = 4096; // SDL buffer sample size (same used for wav files)
    wanted_spec.callback = audio_callback;
    wanted_spec.userdata = audio_data;

    // Open the audio device
    int iscapture = 0; // We are not opening the device for recording so iscapture = 0
    int allowed_changes = 0; // See docs for info on what this is for
    const char* device_name = SDL_GetAudioDeviceName(0, iscapture); // Gets the zero'th non-capture device name (see docs for more info)
    device = SDL_OpenAudioDevice(device_name, iscapture, &wanted_spec, &obtained_spec, allowed_changes);
    if (device == 0) {
        printf("SDL_OpenAudioDevice error: %s\n", SDL_GetError());
        return;
    }
}

// @TODO: here we could just generate the samples that are needed based on what
// was asked for in the callback
static void generate_wave(int len) { // Here len is the length of the buffer in bytes not samples!
    Sint16 *sample_write = audio_data->buffer;
    Sint16 sample_value;

    // If we are switching from one tone to another only switch when the sine
    // value gets to zero to avoid audio pops. Really we should be doing actual
    // cross fades here with mixing
    bool changing = false;
    if (audio_data->previous_tone_hz != audio_data->tone_hz) {
        changing = true;
        if (audio_data->previous_tone_hz == 0.0) {
            audio_data->time = 0.0;
            changing = false;
        }
    }

    for (int sample_index = 0; sample_index < audio_data->size/CHANNELS; sample_index++) {

        // Create the sine wav
        double two_pi = 2.0 * PI;
        double t = audio_data->time;

        double sval;
        if (changing) {
            sval = audio_data->previous_tone_volume * sin(two_pi * audio_data->previous_tone_hz * t);
            double next_sval = audio_data->previous_tone_volume * sin(two_pi * audio_data->previous_tone_hz * (t+audio_data->advance));
            if ((sval >= 0.0 && next_sval < 0.0) || (sval <= 0.0 && next_sval > 0.0)) {
                audio_data->time = 0.0;
                t = audio_data->time;
                sval = 0.0;
                changing = false;
            }
        } else {
            sval = audio_data->tone_volume * sin(two_pi * audio_data->tone_hz * t);
        }

        sample_value = (Sint16) sval;

        // Write the sample_value to the buffer for each channel
        for (int channel = 0; channel < CHANNELS; channel++) {
            *sample_write++ = sample_value;
        }

        // Advance the time
        audio_data->time += audio_data->advance;
    }

    audio_data->previous_tone_volume = audio_data->tone_volume;
    audio_data->previous_tone_hz = audio_data->tone_hz;
}

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    // Cast the userdata to Audio_Data so we can use it
    Audio_Data *audio_data = (Audio_Data *) userdata;

    // Generate the next wave
    generate_wave(len);

    // @TODO: Here we should check to make sure that the len is less than the
    // buffer size. If the len was larger we'd be trying to copy mem we don't
    // have access to. We may also want to be more dynamic and allocate a new
    // larger buffer if the len asked for is larger than the current buffer
    memcpy(stream, (Uint8 *)audio_data->buffer, len);
}

void a_cleanup() {
    // Shut everything down
    SDL_PauseAudioDevice(device, 1);
    SDL_CloseAudioDevice(device);
    free(audio_data->buffer);
    free(audio_data);
}

