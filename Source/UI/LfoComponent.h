#pragma once

#include <JuceHeader.h>
#include "CustomComponent.h"
#include "Displays.h"

//==============================================================================
// LFO panel: shape display with a travelling dot, Rate and Depth
class LfoComponent : public TapPanel
{
public:
    LfoComponent (juce::AudioProcessorValueTreeState& apvts, juce::String index, juce::String title,
                  juce::String lfoFreqId, juce::String lfoDepthId);

    void resized() override;
    void tick (float dt) override;

private:
    LfoDisplay display;
    KnobCell rate;
    KnobCell depth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LfoComponent)
};
