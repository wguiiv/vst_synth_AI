#pragma once

#include <JuceHeader.h>

#include <map>
#include <vector>

namespace tapsynth::test
{
    struct Note
    {
        int midiNote { 69 };
        float velocity { 1.0f };
        double startSeconds { 0.0 };
        double durationSeconds { 0.5 };   // time until note-off
    };

    struct RenderSpec
    {
        double sampleRate { 48000.0 };
        int blockSize { 512 };
        double lengthSeconds { 1.0 };

        // Parameter ID -> value in the parameter's own units (Hz, dB, choice index...)
        std::map<juce::String, float> params;

        std::vector<Note> notes { Note {} };
    };

    // Renders through the real TapSynthAudioProcessor (no editor) and returns the stereo output
    juce::AudioBuffer<float> render (const RenderSpec& spec);
}
