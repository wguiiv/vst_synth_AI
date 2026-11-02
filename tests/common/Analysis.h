#pragma once

#include <JuceHeader.h>

#include <vector>

// Audio measurements shared by every test suite. They work on one channel (a pointer plus a length),
// so a test picks the channel and the time segment it wants to measure.
namespace tapsynth::test
{
    struct Segment
    {
        const float* data { nullptr };
        int numSamples { 0 };
    };

    // Samples [startSeconds, startSeconds + lengthSeconds) of one channel, clipped to the buffer
    Segment segment (const juce::AudioBuffer<float>& buffer, int channel, double sampleRate,
                     double startSeconds, double lengthSeconds);

    // Whole channel
    Segment segment (const juce::AudioBuffer<float>& buffer, int channel);

    float rms (Segment s);
    float peak (Segment s);
    float dcOffset (Segment s);                // mean value
    bool allFinite (const juce::AudioBuffer<float>& buffer);

    float gainToDb (float gain);              // -inf guarded: returns -200 dB for silence

    // Frequency of the strongest spectral peak between minHz and maxHz (Hann window, zero-padded FFT,
    // parabolic interpolation on the log magnitude). Returns 0 if the segment is too short.
    double fundamentalHz (Segment s, double sampleRate, double minHz = 20.0, double maxHz = 20000.0);

    // Energy of the band [lowHz, highHz] in dB (Hann window, sum of bin powers). Only meaningful
    // relative to another band or another render analysed the same way.
    double bandEnergyDb (Segment s, double sampleRate, double lowHz, double highHz);

    // Share of the energy that is not at a multiple of f0, in dB relative to the total energy (0 dB = all of it).
    // Bins within the window's main lobe of a harmonic count as harmonic. Measures aliasing of a periodic tone.
    double inharmonicEnergyDb (Segment s, double sampleRate, double f0);

    // RMS of consecutive, non-overlapping windows of windowSeconds
    std::vector<float> envelope (Segment s, double sampleRate, double windowSeconds);

    // Largest absolute sample difference over all channels; infinity if the shapes differ
    float maxAbsDiff (const juce::AudioBuffer<float>& a, const juce::AudioBuffer<float>& b);

    // 32-bit float WAV, so a saved render reads back bit-exact
    bool saveWav (const juce::File& file, const juce::AudioBuffer<float>& buffer, double sampleRate);
    bool loadWav (const juce::File& file, juce::AudioBuffer<float>& buffer, double& sampleRate);
}
