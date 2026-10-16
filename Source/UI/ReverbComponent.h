#pragma once

#include <JuceHeader.h>
#include "CustomComponent.h"

//==============================================================================
// REVERB panel: Size / Damping / Width over Dry / Wet / Freeze
class ReverbComponent : public TapPanel
{
public:
    ReverbComponent (juce::AudioProcessorValueTreeState& apvts, juce::String index, juce::String title,
                     juce::String sizeId, juce::String dampingId, juce::String widthId,
                     juce::String dryId, juce::String wetId, juce::String freezeId);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    KnobCell size;
    KnobCell damping;
    KnobCell stereoWidth;
    KnobCell dry;
    KnobCell wet;
    ToggleCell freeze;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbComponent)
};
