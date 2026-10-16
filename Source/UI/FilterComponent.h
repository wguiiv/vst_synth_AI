#pragma once

#include <JuceHeader.h>
#include "CustomComponent.h"
#include "Displays.h"

//==============================================================================
// FILTER panel: LP / BP / HP in the header, response curve, Cutoff and Resonance
class FilterComponent : public TapPanel
{
public:
    FilterComponent (juce::AudioProcessorValueTreeState& apvts, juce::String index, juce::String title,
                     juce::String filterTypeId, juce::String cutoffId, juce::String resonanceId);

    void resized() override;
    void tick (float dt) override;

private:
    SegmentedSelector filterType;
    FilterCurve curve;
    KnobCell cutoff;
    KnobCell resonance;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterComponent)
};
