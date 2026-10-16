#include <JuceHeader.h>
#include "FilterComponent.h"

//==============================================================================
FilterComponent::FilterComponent (juce::AudioProcessorValueTreeState& apvts, juce::String index, juce::String title,
                                  juce::String filterTypeId, juce::String cutoffId, juce::String resonanceId)
: TapPanel (index, title, Tap::Group::filter)
, filterType (apvts, filterTypeId, Tap::Group::filter, { { "LP", {}, "Low Pass" }, { "BP", {}, "Band Pass" }, { "HP", {}, "High Pass" } }, 30)
, curve (apvts, filterTypeId, cutoffId, resonanceId)
, cutoff    (apvts, cutoffId,    "Cutoff",    Tap::Group::filter, Tap::Format::hertz, 56, 84)
, resonance (apvts, resonanceId, "Resonance", Tap::Group::filter, Tap::Format::q,     48, 84)
{
    setHeaderControl (filterType);

    addAndMakeVisible (curve);
    addAndMakeVisible (cutoff);
    addAndMakeVisible (resonance);
}

void FilterComponent::resized()
{
    TapPanel::resized();

    curve.setBounds (inset + 11, inset + 45, 240, 202);
    cutoff.placeAt    (inset + 300, inset + 52);
    resonance.placeAt (inset + 300, inset + 152);
}

void FilterComponent::tick (float)
{
    curve.refresh();
}
