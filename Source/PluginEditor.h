/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/TapLookAndFeel.h"
#include "UI/OscComponent.h"
#include "UI/FilterComponent.h"
#include "UI/AdsrComponent.h"
#include "UI/LfoComponent.h"
#include "UI/ReverbComponent.h"
#include "UI/MeterComponent.h"
#include "UI/PromptBar.h"
#include <map>
#include <thread>

//==============================================================================
/**
*/
class TapSynthAudioProcessorEditor  : public juce::AudioProcessorEditor
, public juce::Timer
{
public:
    TapSynthAudioProcessorEditor (TapSynthAudioProcessor&);
    ~TapSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

    void timerCallback() override;

private:
    TapSynthAudioProcessor& audioProcessor;

    // Declared before the components so it outlives them
    TapLookAndFeel lookAndFeel;
    juce::SharedResourcePointer<Tap::Fonts> fonts;
    juce::TooltipWindow tooltipWindow { this, 600 };

    PromptBar promptBar;
    PresetBox presetBox;
    OscComponent osc1;
    OscComponent osc2;
    FilterComponent filter;
    AdsrComponent filterAdsr;
    LfoComponent lfo1;
    AdsrComponent adsr;
    ReverbComponent reverb;
    MeterComponent meter;

    std::array<TapPanel*, 8> getPanels();

    // AI-changed highlight: parameter ID -> its control, and the start time (ms) of each running highlight
    std::map<juce::String, AiTarget*> aiTargets;
    std::map<juce::String, double> aiHighlights;
    void highlightParameters (const juce::StringArray& ids);
    void updateAiHighlights (double nowMs);

    double lastTick { 0.0 };

    // Preset box: browse the patches the AI generated in this session
    void updatePresetBox();
    void stepSessionPatch (int delta);

    // send the prompt text to the configured REST endpoint (runs network on background thread)
    void sendPrompt (const juce::String& promptText);
    void handleResponse (const juce::String& prompt, const juce::String& responseBody, const juce::String& errorMsg, int statusCode);
    void applyAiPatch (const juce::var& params, const juce::String& prompt);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapSynthAudioProcessorEditor)
};
