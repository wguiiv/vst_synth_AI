#pragma once

#include <JuceHeader.h>
#include "CustomComponent.h"
#include "Displays.h"

//==============================================================================
// OSC panel: waveform selector in the header, waveform display, then Gain / Pitch / FM Freq / FM Depth
class OscComponent : public TapPanel
{
public:
    OscComponent (juce::AudioProcessorValueTreeState& apvts, juce::String index, juce::String title,
                  juce::String oscId, juce::String gainId, juce::String pitchId, juce::String fmFreqId, juce::String fmDepthId);

    void resized() override;
    void tick (float dt) override;

private:
    SegmentedSelector waveform;
    OscDisplay display;
    KnobCell gain;
    KnobCell pitch;
    KnobCell fmFreq;
    KnobCell fmDepth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscComponent)
};
