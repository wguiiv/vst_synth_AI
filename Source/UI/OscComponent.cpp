#include <JuceHeader.h>
#include "OscComponent.h"

//==============================================================================
OscComponent::OscComponent (juce::AudioProcessorValueTreeState& apvts, juce::String index, juce::String title,
                            juce::String oscId, juce::String gainId, juce::String pitchId, juce::String fmFreqId, juce::String fmDepthId)
: TapPanel (index, title, Tap::Group::osc)
, waveform (apvts, oscId, Tap::Group::osc, { { {}, Tap::sineIcon(), "Sine" }, { {}, Tap::sawIcon(), "Saw" }, { {}, Tap::squareIcon(), "Square" } })
, display (apvts, oscId, gainId, fmFreqId, fmDepthId)
, gain    (apvts, gainId,    "Gain",     Tap::Group::osc, Tap::Format::decibels,  48, 60)
, pitch   (apvts, pitchId,   "Pitch",    Tap::Group::osc, Tap::Format::semitones, 48, 60, true)
, fmFreq  (apvts, fmFreqId,  "FM Freq",  Tap::Group::osc, Tap::Format::hertz,     48, 60)
, fmDepth (apvts, fmDepthId, "FM Depth", Tap::Group::osc, Tap::Format::number,    48, 60)
{
    setHeaderControl (waveform);

    addAndMakeVisible (display);
    addAndMakeVisible (gain);
    addAndMakeVisible (pitch);
    addAndMakeVisible (fmFreq);
    addAndMakeVisible (fmDepth);
}

void OscComponent::resized()
{
    TapPanel::resized();

    display.setBounds (inset + 11, inset + 45, 240, 96);

    const int knobTop = inset + 153;
    gain.placeAt    (inset + 41,  knobTop);
    pitch.placeAt   (inset + 101, knobTop);
    fmFreq.placeAt  (inset + 161, knobTop);
    fmDepth.placeAt (inset + 221, knobTop);
}

void OscComponent::tick (float)
{
    display.refresh();
}
