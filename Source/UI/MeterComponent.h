#pragma once

#include <JuceHeader.h>
#include "../Data/MeterData.h"
#include "CustomComponent.h"

//==============================================================================
// Stereo meter: RMS and peak bars with a peak-hold line, clip LEDs and L / R peak readouts.
// Clicking the readouts resets the holds and the clip LEDs.
class LevelMeter : public juce::Component
{
public:
    explicit LevelMeter (MeterData& meterData);

    void update (float dt);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

    static constexpr int barHeight { 384 };

private:
    struct Channel
    {
        float rmsDb { -60.0f }, peakDb { -60.0f }, holdDb { -60.0f };
        float holdTime { 0.0f };
        bool clipped { false };
    };

    float yOf (float db) const;
    void fillLevel (juce::Graphics&, juce::Rectangle<float> bar, float db, float alpha) const;
    juce::Rectangle<int> readoutArea() const;

    MeterData& meterData;
    std::array<Channel, 2> channels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};

//==============================================================================
class MeterComponent : public TapPanel
{
public:
    MeterComponent (MeterData& meterData, juce::String index, juce::String title);

    void resized() override;
    void tick (float dt) override;

private:
    LevelMeter meter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeterComponent)
};
