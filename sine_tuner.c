#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#define DEVICE_FORMAT       ma_format_f32
#define DEVICE_CHANNELS     2
#define DEVICE_SAMPLE_RATE  48000

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    MA_ASSERT(pDevice->playback.channels == DEVICE_CHANNELS);

    ma_waveform* pSineWave = (ma_waveform*)pDevice->pUserData;
    MA_ASSERT(pSineWave != NULL);

    ma_waveform_read_pcm_frames(pSineWave, pOutput, frameCount);
}

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_EnableScreenSaver();
    atexit(SDL_Quit);

    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);

    SDL_Window *window;
    window = SDL_CreateWindow(
        "Sine Tuner", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dm.w * 0.8, dm.h * 0.8,
        //"Sine Tuner", 10, 10, 600, 600, 0
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    bool play = false;
    bool changed = false;
    bool running = true;
    ma_device device;

    // Apparently we need to poll for events to get the window to show up
    while (running) {
        SDL_Event event;
        changed = false;
        while(SDL_PollEvent(&event)) {
            SDL_PollEvent(&event);
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (play) {
                    play = false;
                    changed = true;
                } else {
                    play = true;
                    changed = true;
                }
            }

            //if (event.type == SDL_KeyboardEvent && event.SDL_keysym == 'x') {
            if (event.type == SDL_KEYDOWN) {
                running = false;
            }

        }

        if (changed) {
            if (play) {
                printf("The play value is true\n");

                ma_waveform sineWave;
                ma_device_config deviceConfig;
                ma_waveform_config sineWaveConfig;

                sineWaveConfig = ma_waveform_config_init(DEVICE_FORMAT, DEVICE_CHANNELS, DEVICE_SAMPLE_RATE, ma_waveform_type_sine, 0.2, 220);
                ma_waveform_init(&sineWaveConfig, &sineWave);

                deviceConfig = ma_device_config_init(ma_device_type_playback);
                deviceConfig.playback.format   = DEVICE_FORMAT;
                deviceConfig.playback.channels = DEVICE_CHANNELS;
                deviceConfig.sampleRate        = DEVICE_SAMPLE_RATE;
                deviceConfig.dataCallback      = data_callback;
                deviceConfig.pUserData         = &sineWave;

                if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
                    printf("Failed to open playback device.\n");
                    return -4;
                }

                // printf("Device Name: %s\n", device.playback.name);

                if (ma_device_start(&device) != MA_SUCCESS) {
                    printf("Failed to start playback device.\n");
                    ma_device_uninit(&device);
                    return -5;
                }

                //printf("Press Enter to quit...\n");
                //getchar();
                //ma_device_uninit(&device);
            } else {
                printf("The play value is false\n");

                ma_device_uninit(&device);
            }
        }

        SDL_Surface *surf = SDL_GetWindowSurface(window);
        SDL_FillRect(surf, NULL, SDL_MapRGB(surf->format, 0xFF, 0xFF, 0xFF));
        SDL_UpdateWindowSurface(window);

    }


    // ma_waveform sineWave;
    // ma_device_config deviceConfig;
    // ma_device device;
    // ma_waveform_config sineWaveConfig;

    // sineWaveConfig = ma_waveform_config_init(DEVICE_FORMAT, DEVICE_CHANNELS, DEVICE_SAMPLE_RATE, ma_waveform_type_sine, 0.2, 220);
    // ma_waveform_init(&sineWaveConfig, &sineWave);

    // deviceConfig = ma_device_config_init(ma_device_type_playback);
    // deviceConfig.playback.format   = DEVICE_FORMAT;
    // deviceConfig.playback.channels = DEVICE_CHANNELS;
    // deviceConfig.sampleRate        = DEVICE_SAMPLE_RATE;
    // deviceConfig.dataCallback      = data_callback;
    // deviceConfig.pUserData         = &sineWave;

    // if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
    //     printf("Failed to open playback device.\n");
    //     return -4;
    // }

    // printf("Device Name: %s\n", device.playback.name);

    // if (ma_device_start(&device) != MA_SUCCESS) {
    //     printf("Failed to start playback device.\n");
    //     ma_device_uninit(&device);
    //     return -5;
    // }

    //printf("Press Enter to quit...\n");
    //getchar();

    ma_device_uninit(&device);
    SDL_DestroyWindow(window);

    return EXIT_SUCCESS;
}

