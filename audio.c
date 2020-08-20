#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>


#define CHANNELS 2
#define PI 3.141592
#define SECONDS 5

void audio_callback(void *userdata, Uint8 *stream, int len);
void a_cleanup();
void generate_wave();

typedef struct {
    int size;
    int play_cursor;
    int samples_per_second;
    Uint16 tone_volume;
    int last_sample_index;
    int tone_hz;
    Sint16 *buffer;
} Audio_Data;

static SDL_AudioDeviceID device;
static Audio_Data *audio_data;

void a_init(void) {
    SDL_AudioSpec wanted_spec;
    SDL_AudioSpec obtained_spec;

    audio_data = malloc(sizeof(Audio_Data)); // calloc here for setting to 0?

    audio_data->buffer = malloc(sizeof(Sint16) * 8192);
    audio_data->play_cursor = 0;
    audio_data->last_sample_index = 0;
    audio_data->size = 8192;
    audio_data->samples_per_second = 44000;
    audio_data->tone_volume = 5000;
    audio_data->tone_hz = 440;

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

    generate_wave();

    // Start playing
    SDL_PauseAudioDevice(device, 0);

    // Wait for SECONDS number of seconds
    SDL_Delay(SECONDS * 1000);

    a_cleanup();
}

void generate_wave() {
    int bytes_per_sample = sizeof(Uint16) * CHANNELS;
    int bytes_to_write = audio_data->samples_per_second * bytes_per_sample;

    Sint16 *sample_write = audio_data->buffer;

    int sine_wave_period = audio_data->samples_per_second / audio_data->tone_hz;
    int bytes_per_period = sine_wave_period * bytes_per_sample;
    int sample_count = bytes_to_write / bytes_per_sample;
    Sint16 sample_value;

    int start = audio_data->last_sample_index % bytes_per_period;

    //int sample_index;
    //for (sample_index = start; sample_index < (sample_count+start); sample_index++) {

    // Here I was overwriting the buffer :(
    for (int sample_index = 0; sample_index < audio_data->size/2; sample_index++) {

        // Create the sine wav
        double two_pi = 2.0 * PI;
        double t = (double)sample_index / (double)audio_data->samples_per_second;
        double sval = audio_data->tone_volume * sin(two_pi * (double)audio_data->tone_hz * t);
        sample_value = (Sint16) sval;

        // Write the sample_value to the buffer for each channel
        for (int channel = 0; channel < CHANNELS; channel++) {
            *sample_write++ = sample_value;
        }
    }
}


void audio_callback(void *userdata, Uint8 *stream, int len) {

    // Cast the userdata to Audio_Data so we can use it
    Audio_Data *audio_data = (Audio_Data *) userdata;

    // // If SDL asks for data past the end of the generated wave we need to just
    // // give it the point in the period that it asked for but at the beginning
    // // of the generated wave. We can do this by taking the modulus of the
    // // play_cursor and the bytes_per_period
    // if (audio_data->play_cursor + len > audio_data->size) {
    //     audio_data->play_cursor %= audio_data->bytes_per_period;
    // }

    memcpy(stream, (Uint8 *)audio_data->buffer, len);
    // audio_data->play_cursor += len;
}

void a_cleanup() {
    // Shut everything down
    SDL_PauseAudioDevice(device, 1);
    SDL_CloseAudioDevice(device);
    free(audio_data->buffer);
    free(audio_data);
}

