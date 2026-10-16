/*
  ==============================================================================

    MeterData.h
    Created: 27 Feb 2021 9:37:46pm
    Author:  Joshua Hodge

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

// Per-channel output levels for the editor's meter. Written on the audio thread, read on the message thread.
class MeterData
{
public:
    static constexpr int maxChannels { 2 };

    void prepare (double sampleRate);
    void process (const juce::AudioBuffer<float>& buffer);

    // RMS over a ~300 ms window, linear gain
    float getRMS (int channel) const { return rms[(size_t) juce::jlimit (0, maxChannels - 1, channel)].load(); }

    // Highest absolute sample since the previous call, linear gain. Resets the peak, so call it once per UI frame.
    float takePeak (int channel) { return peak[(size_t) juce::jlimit (0, maxChannels - 1, channel)].exchange (0.0f); }

private:
    static constexpr double rmsWindowSeconds { 0.3 };
    double sampleRate { 44100.0 };
    std::array<float, maxChannels> meanSquare {};
    std::array<std::atomic<float>, maxChannels> rms {};
    std::array<std::atomic<float>, maxChannels> peak {};
};
