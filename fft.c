//////////////////////////////////////////////////////////////////////
// SPDX-License-Identifier: MIT
//
// fft.c
// =====
//
// Implementation of DFT and FFT in C99.
//
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// License: MIT
//

#include <complex.h>

#ifndef PI
#define PI      3.14159265358979323846264f
#endif

// Must be a power of two for FFT
#define FRAME_COUNT_MAX (1<<7)
#define FREQUENCY_SCALING 0.52
float frames[FRAME_COUNT_MAX] = {0};       // audio samples
float frequencies[FRAME_COUNT_MAX] = {0};  // filled by FFT

// Just copy [in_frames] to [out_frequencies], used for visualizations
// or debugging
void frames_as_frequencies(const float* in_frames, float *out_frequencies,
         unsigned int n_frames)
{
  for (unsigned int i = 0; i < n_frames; ++i)
    out_frequencies[i] = in_frames[i];
  return;
}

// Calculates the Discrete Furier Transform of [in_frames], saved in
// [out_frequencies]
void dft(const float* in_frames, float *out_frequencies,
         unsigned int n_frames)
{
  // Calculate all frequency values in [out_frequencies] The number of
  // frequencies the the same as the number of discrete points
  // (frames) since we don't have enough information to calculate
  // more.
  for (unsigned int freq = 0; freq < n_frames; ++freq)
  {
    // A single frequency value represents how much that frequency
    // contributes to the overall mixed frequency (the input). To
    // calculate this, we multiply this frequency by the input for
    // each point, then we sum them up. This is done with different
    // offsets (sine and cosine) otherwise some information may be
    // never picked up (for example, if the input is a sine, and our
    // selected frequency is a cosine, the sum of their prducts will
    // always be zero and we don't get much information).
    out_frequencies[freq] = 0;
    for (unsigned int frame = 0; frame < n_frames; ++frame)
    {
      float t = (float) frame / n_frames;
      // Remember Euler's Formula
      //     e^{ix} = cos(x) + i * sin(x)
      // Using imaginary number is just a shortcut to encode information
      // for both sine and cosine.
      out_frequencies[freq] += in_frames[frame] * cexpf(2 * I * PI * freq * t);
    }
  }
  return;
}

// The Fast Furier Transform of [in_frames], saved in [out_frequencies]
void fft(const float* in_frames, float *out_frequencies,
         unsigned int window)
{
  // Base case
  if (window <= 1)
    return;

  // Split
  float even[window / 2], odd[window / 2];
  for (unsigned int i = 0; i < window / 2; i++)
  {
    even[i] = in_frames[i * 2];
    odd[i] = in_frames[i * 2 + 1];
  }
  fft(even, out_frequencies, window / 2);
  fft(odd, out_frequencies,  window / 2);

  // Combine
  for (unsigned int i = 0; i < window / 2; ++i)
  {
    complex float twiddle = cexp(-2.0 * I * PI * (float)i / (float)window) * odd[i];
    out_frequencies[i] = even[i] + twiddle;
    out_frequencies[i + window / 2] = even[i] - twiddle;
  }
  return;
}
