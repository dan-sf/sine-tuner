#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>


#define CHANNELS 2
#define PI 3.14159265

void audio_callback(void *userdata, Uint8 *stream, int len);
void a_cleanup();
void generate_wave();

typedef struct {
    int size;
    double samples_per_second;
    double tone_volume;
    double tone_hz;
    double advance;
    double time;
    Sint16 *buffer;
} Audio_Data;

static SDL_AudioDeviceID device;
static Audio_Data *audio_data;

void a_set_tone(double tone_hz) {
    audio_data->time = 0.0;
    audio_data->tone_hz = tone_hz;
}

void a_stop_playing() {
    SDL_ClearQueuedAudio(device);
    //SDL_PauseAudioDevice(device, 1);
    a_set_tone(0.0);
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
    audio_data->tone_volume = 5000.0;
    audio_data->tone_hz = 0.0;

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

void generate_wave() {
    Sint16 *sample_write = audio_data->buffer;
    Sint16 sample_value;

    for (int sample_index = 0; sample_index < audio_data->size/CHANNELS; sample_index++) {

        // Create the sine wav
        double two_pi = 2.0 * PI;
        double t = audio_data->time;

        // We may want to reset the time once we hit 44K (or the set samples
        // per second), this would prevent us from overflowing, however, it'll
        // take 12 hours alone to hit 44K which still a ways away from an
        // overflow, so practically we can probably just continually increase
        // the time without this check. Leaving this comment incase we decide
        // to use it
        // if (t == audio_data->samples_per_second) {
        //     audio_data->time = 0.0;
        //     t = audio_data->time;
        // }

        double sval = audio_data->tone_volume * sin(two_pi * audio_data->tone_hz * t);
        sample_value = (Sint16) sval;

        // Write the sample_value to the buffer for each channel
        for (int channel = 0; channel < CHANNELS; channel++) {
            *sample_write++ = sample_value;
        }

        // Advance the time
        audio_data->time += audio_data->advance;
    }
}

void audio_callback(void *userdata, Uint8 *stream, int len) {
    // Cast the userdata to Audio_Data so we can use it
    Audio_Data *audio_data = (Audio_Data *) userdata;

    // Generate the next wave
    generate_wave();

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

