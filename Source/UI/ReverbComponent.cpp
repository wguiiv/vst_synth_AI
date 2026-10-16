#include <JuceHeader.h>
#include "ReverbComponent.h"

//==============================================================================
ReverbComponent::ReverbComponent (juce::AudioProcessorValueTreeState& apvts, juce::String index, juce::String title,
                                  juce::String sizeId, juce::String dampingId, juce::String widthId,
                                  juce::String dryId, juce::String wetId, juce::String freezeId)
: TapPanel (index, title, Tap::Group::accent)
, size        (apvts, sizeId,    "Size",    Tap::Group::accent, Tap::Format::percent, 48, 90)
, damping     (apvts, dampingId, "Damping", Tap::Group::accent, Tap::Format::percent, 48, 90)
, stereoWidth (apvts, widthId,   "Width",   Tap::Group::accent, Tap::Format::percent, 48, 90)
, dry         (apvts, dryId,     "Dry",     Tap::Group::accent, Tap::Format::percent, 48, 90)
, wet         (apvts, wetId,     "Wet",     Tap::Group::accent, Tap::Format::percent, 48, 90)
, freeze      (apvts, freezeId,  "Freeze",  90)
{
    addAndMakeVisible (size);
    addAndMakeVisible (damping);
    addAndMakeVisible (stereoWidth);
    addAndMakeVisible (dry);
    addAndMakeVisible (wet);
    addAndMakeVisible (freeze);
}

void ReverbComponent::paint (juce::Graphics& g)
{
    TapPanel::paint (g);

    // Rule between the two rows
    g.setColour (Tap::lineSubtle);
    g.fillRect (inset + 11, inset + 144, 306, 1);
}

void ReverbComponent::resized()
{
    TapPanel::resized();

    const int col1 = inset + 62, col2 = inset + 164, col3 = inset + 266;

    size.placeAt        (col1, inset + 56);
    damping.placeAt     (col2, inset + 56);
    stereoWidth.placeAt (col3, inset + 56);
    dry.placeAt         (col1, inset + 158);
    wet.placeAt         (col2, inset + 158);
    freeze.setTopLeftPosition (col3 - freeze.getWidth() / 2, inset + 153);
}
