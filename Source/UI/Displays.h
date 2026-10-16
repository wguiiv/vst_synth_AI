/*
  ==============================================================================

    Displays.h

    The recessed graphic displays inside the module panels. They read the raw
    parameter values and redraw when refresh() finds that one has changed,
    which the editor calls from its 30 Hz timer.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "TapLookAndFeel.h"

class Display : public juce::Component
{
public:
    Display (juce::AudioProcessorValueTreeState& apvts, const juce::StringArray& paramIds);

    // Repaints if any of the watched parameters moved since the last call
    void refresh();

    void paint (juce::Graphics&) override;

protected:
    // Draws the content in a coordinate space starting inside the 1 px border, w × h
    virtual void paintContent (juce::Graphics&, float w, float h) = 0;

    float value (int i) const { return sources[(size_t) i]->load(); }
    juce::NormalisableRange<float> rangeOf (int i) const { return ranges[(size_t) i]; }

    // The uppercase caption in the top-left corner
    static void drawChip (juce::Graphics&, const juce::String& text);

    // Area under a curve: the group colour from 28 % alpha at the top of the shape to 0 at its bottom
    static void fillUnder (juce::Graphics&, const juce::Path& area, juce::Colour colour);

    static void strokeCurve (juce::Graphics&, const juce::Path& curve, juce::Colour colour, float thickness = 2.0f);

private:
    std::vector<std::atomic<float>*> sources;
    std::vector<juce::NormalisableRange<float>> ranges;
    std::vector<float> lastValues;
};

//==============================================================================
// 2.5 cycles of the selected waveform; FM adds a visual wobble only
class OscDisplay : public Display
{
public:
    OscDisplay (juce::AudioProcessorValueTreeState& apvts, const juce::String& waveId, const juce::String& gainId,
                const juce::String& fmFreqId, const juce::String& fmDepthId);

private:
    void paintContent (juce::Graphics&, float w, float h) override;
};

//==============================================================================
// Frequency response of the state-variable filter on a log axis, 20 Hz – 20 kHz, −39 to +15 dB
class FilterCurve : public Display
{
public:
    FilterCurve (juce::AudioProcessorValueTreeState& apvts, const juce::String& typeId, const juce::String& cutoffId,
                 const juce::String& resonanceId);

private:
    void paintContent (juce::Graphics&, float w, float h) override;
};

//==============================================================================
// ADSR curve; each stage's width is proportional to its time over its range
class EnvelopeGraph : public Display
{
public:
    EnvelopeGraph (juce::AudioProcessorValueTreeState& apvts, Tap::Group group, const juce::String& attackId,
                   const juce::String& decayId, const juce::String& sustainId, const juce::String& releaseId);

private:
    void paintContent (juce::Graphics&, float w, float h) override;
    const Tap::Group group;
};

//==============================================================================
// LFO shape with a dot travelling along it at the LFO's rate
class LfoDisplay : public Display
{
public:
    LfoDisplay (juce::AudioProcessorValueTreeState& apvts, const juce::String& rateId, const juce::String& depthId);

    // Moves the dot; dt in seconds
    void advance (float dt);

private:
    void paintContent (juce::Graphics&, float w, float h) override;
    float amplitude (float h) const;
    float cycles() const;

    float phase { 0.3f };
};
