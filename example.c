// SPDX-License-Identifier: MIT
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// Github:  @San7o

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif
#ifdef _WIN32
#error "TODO: support for windows clock"
#endif

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_timer.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "miniaudio.h"

#include "fft.c"

// Window settings
#define WINDOW_NAME   "fft.c"
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720
#define WINDOW_FLAGS  0
#define FPS 60.0

// The audio file that will be played
#define AUDIO_FILE "media/afterdark.mp3"

typedef enum {
  FRAMES = 0,
  DFT,
  FFT,
} FunctionType;

void data_callback(ma_device* pDevice, void* pOutput,
                   const void* pInput, ma_uint32 frameCount)
{
  ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
  if (pDecoder == NULL)
    return;

  ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

  for (unsigned int i = 0; i < frameCount && i < FRAME_COUNT_MAX; ++i)
  {
    frames[i] = ((float*)pOutput)[i*2];
  }

  (void)pInput;
}

int main(void)
{
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
    return 1;
  }
  
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;

  bool ret = SDL_CreateWindowAndRenderer(WINDOW_NAME, WINDOW_WIDTH,
                                         WINDOW_HEIGHT, WINDOW_FLAGS,
                                         &window, &renderer);
  if (!ret)
  {
    fprintf(stderr, "Error Creating SDL Window: %s\n", SDL_GetError());
    return 1;
  }
  
  ma_result result;
  ma_decoder decoder;
  ma_device_config deviceConfig;
  ma_device device;

  result = ma_decoder_init_file(AUDIO_FILE, NULL, &decoder);
  if (result != MA_SUCCESS)
  {
    printf("Could not load file\n");
    return -2;
  }

  deviceConfig = ma_device_config_init(ma_device_type_playback);
  deviceConfig.playback.format   = decoder.outputFormat;
  deviceConfig.playback.channels = decoder.outputChannels;
  deviceConfig.sampleRate        = decoder.outputSampleRate;
  deviceConfig.dataCallback      = data_callback;
  deviceConfig.pUserData         = &decoder;

  printf("Channels: %d\n", decoder.outputChannels);
  
  if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS)
  {
    printf("Failed to open playback device.\n");
    ma_decoder_uninit(&decoder);
    return -3;
  }

  if (ma_device_start(&device) != MA_SUCCESS)
  {
    printf("Failed to start playback device.\n");
    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);
    return -4;
  }

  double delta_time = 0.0;
  FunctionType function_type = FRAMES;
  while(1)
  {
    struct timespec frame_start;
    clock_gettime(CLOCK_MONOTONIC, &frame_start);

    if (SDL_PollEvent(&event))
    {
      if (SDL_EVENT_KEY_DOWN == event.type)
      {
        switch(event.key.key)
        {
        case 'q':
        case SDLK_ESCAPE:
          goto cleanup;
        case '1':
          function_type = FRAMES;
          break;
        case '2':
          function_type = DFT;
          break;
        case '3':
          function_type = FFT;
          break;
        default:
          break;
        }
      }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    if (!SDL_RenderClear(renderer))
    {
      fprintf(stderr, "Error Clearing SDL Window: %s\n", SDL_GetError());    
      goto cleanup;
    }

    if (delta_time > 1 / FPS) // Render frame
    {
      delta_time = 0;

      SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);    
      for (unsigned int i = 0; i < FRAME_COUNT_MAX / 2; ++i)
      {
        //if (frequencies[i] <= 0.0) continue;
        SDL_FRect rect = (SDL_FRect){
          .x = i * WINDOW_WIDTH / FRAME_COUNT_MAX * 2,
          .y = WINDOW_HEIGHT/2,
          .w = WINDOW_WIDTH / FRAME_COUNT_MAX,
          .h = WINDOW_HEIGHT * frequencies[i] / 2.0 * FREQUENCY_SCALING,
        };
        SDL_RenderFillRect(renderer, &rect);
        rect.h *= -1; // Mirror the spectrum
        SDL_RenderFillRect(renderer, &rect);
      }

      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_SetRenderScale(renderer, 2.0f, 2.0f);
      switch (function_type)
      {
      case FRAMES:
        SDL_RenderDebugText(renderer, 20, 20,
                            "Showing: Frames. Change with 1 / 2 / 3");
        frames_as_frequencies(frames, frequencies, FRAME_COUNT_MAX);
        break;
      case DFT:
        SDL_RenderDebugText(renderer, 20, 20,
                            "Showing: DFT. Change with 1 / 2 / 3");
        dft(frames, frequencies, FRAME_COUNT_MAX);
        break;
      case FFT:
        SDL_RenderDebugText(renderer, 20, 20,
                            "Showing: FFT. Change with 1 / 2 / 3");
        fft(frames, frequencies, FRAME_COUNT_MAX);
        break;
      }
      SDL_SetRenderScale(renderer, 1.0f, 1.0f);
      
      if (!SDL_RenderPresent(renderer))
      {
        fprintf(stderr, "Error Rendering SDL Window: %s\n", SDL_GetError());    
        goto cleanup;
      }
    }

    SDL_Delay(16);
    struct timespec frame_end;
    clock_gettime(CLOCK_MONOTONIC, &frame_end);
    delta_time += (frame_end.tv_sec - frame_start.tv_sec)
      + (frame_end.tv_nsec - frame_start.tv_nsec) / 1e9;
  }

 cleanup:
  ma_device_uninit(&device);
  ma_decoder_uninit(&decoder);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
