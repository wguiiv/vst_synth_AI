#include <JuceHeader.h>
#include "AdsrComponent.h"

//==============================================================================
AdsrComponent::AdsrComponent (juce::AudioProcessorValueTreeState& apvts, juce::String index, juce::String title, Tap::Group group,
                              juce::String attackId, juce::String decayId, juce::String sustainId, juce::String releaseId,
                              juce::String depthId)
: TapPanel (index, title, group)
, compact (depthId.isNotEmpty())
, graph (apvts, group, attackId, decayId, sustainId, releaseId)
, attack  (apvts, attackId,  compact ? "Atk" : "Attack",  group, Tap::Format::seconds, compact ? 76 : 68, compact ? 36 : 60)
, decay   (apvts, decayId,   compact ? "Dec" : "Decay",   group, Tap::Format::seconds, compact ? 76 : 68, compact ? 36 : 60)
, sustain (apvts, sustainId, compact ? "Sus" : "Sustain", group, Tap::Format::percent, compact ? 76 : 68, compact ? 36 : 60)
, release (apvts, releaseId, compact ? "Rel" : "Release", group, Tap::Format::seconds, compact ? 76 : 68, compact ? 36 : 60)
{
    addAndMakeVisible (graph);
    addAndMakeVisible (attack);
    addAndMakeVisible (decay);
    addAndMakeVisible (sustain);
    addAndMakeVisible (release);

    if (compact)
    {
        depth = std::make_unique<KnobCell> (apvts, depthId, "Depth", group, Tap::Format::hertz, 48, 72);
        addAndMakeVisible (*depth);
    }
}

void AdsrComponent::paint (juce::Graphics& g)
{
    TapPanel::paint (g);

    // Divider between the sliders and the Depth knob
    if (compact)
    {
        g.setColour (Tap::lineSubtle);
        g.fillRect (inset + 158, inset + 124, 1, 110);
    }
}

void AdsrComponent::resized()
{
    TapPanel::resized();

    if (compact)
    {
        graph.setBounds (inset + 11, inset + 45, 212, 66);

        const int trackTop = inset + 126;
        attack.placeAt  (inset + 29,  trackTop);
        decay.placeAt   (inset + 65,  trackTop);
        sustain.placeAt (inset + 101, trackTop);
        release.placeAt (inset + 137, trackTop);

        depth->placeAt (inset + 192, inset + 156);
    }
    else
    {
        graph.setBounds (inset + 11, inset + 45, 306, 78);

        const int trackTop = inset + 138;
        attack.placeAt  (inset + 49,  trackTop);
        decay.placeAt   (inset + 126, trackTop);
        sustain.placeAt (inset + 203, trackTop);
        release.placeAt (inset + 280, trackTop);
    }
}

void AdsrComponent::tick (float)
{
    graph.refresh();
}
