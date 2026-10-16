#include <JuceHeader.h>
#include "LfoComponent.h"

//==============================================================================
LfoComponent::LfoComponent (juce::AudioProcessorValueTreeState& apvts, juce::String index, juce::String title,
                            juce::String lfoFreqId, juce::String lfoDepthId)
: TapPanel (index, title, Tap::Group::filter)
, display (apvts, lfoFreqId, lfoDepthId)
, rate  (apvts, lfoFreqId,  "Rate",  Tap::Group::filter, Tap::Format::rate,  44, 92)
, depth (apvts, lfoDepthId, "Depth", Tap::Group::filter, Tap::Format::hertz, 44, 92)
{
    addAndMakeVisible (display);
    addAndMakeVisible (rate);
    addAndMakeVisible (depth);
}

void LfoComponent::resized()
{
    TapPanel::resized();

    display.setBounds (inset + 11, inset + 45, 92, 26);
    rate.placeAt  (inset + 57, inset + 86);
    depth.placeAt (inset + 57, inset + 168);
}

void LfoComponent::tick (float dt)
{
    display.advance (dt);
}
