#pragma once

#include <JuceHeader.h>
#include "CustomComponent.h"
#include "Displays.h"

//==============================================================================
// Envelope panel: ADSR graph over four vertical sliders. With a depth parameter it uses the narrow
// FILTER ENV layout (short labels, plus a Depth knob); without one, the wide AMP ENV layout.
class AdsrComponent : public TapPanel
{
public:
    AdsrComponent (juce::AudioProcessorValueTreeState& apvts, juce::String index, juce::String title, Tap::Group group,
                   juce::String attackId, juce::String decayId, juce::String sustainId, juce::String releaseId,
                   juce::String depthId = {});

    void paint (juce::Graphics&) override;
    void resized() override;
    void tick (float dt) override;

private:
    const bool compact;
    EnvelopeGraph graph;
    SliderCell attack;
    SliderCell decay;
    SliderCell sustain;
    SliderCell release;
    std::unique_ptr<KnobCell> depth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AdsrComponent)
};
