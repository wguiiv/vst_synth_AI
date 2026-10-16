/*
  ==============================================================================

    CustomComponent.h

    The building blocks every module panel is made of: the panel itself and
    one cell per control (knob, vertical slider, segmented selector, toggle).
    Each control cell carries its parameter ID as its component ID so the
    editor can find it to show the AI-changed highlight.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "TapLookAndFeel.h"

//==============================================================================
// A control the AI-changed highlight can be shown on. alpha runs 0..1.
struct AiTarget
{
    virtual ~AiTarget() = default;
    virtual void setAiAlpha (float alpha) = 0;
};

//==============================================================================
// Module panel: rounded card with a "01 TITLE" header row and an optional control in the header's right slot
class TapPanel : public juce::Component
{
public:
    TapPanel (juce::String index, juce::String title, Tap::Group group);

    void paint (juce::Graphics&) override;
    void resized() override;

    // Called by the editor at 30 Hz to update displays and animations; dt in seconds
    virtual void tick (float /*dt*/) {}

    // Child coordinates in the design are measured from inside the 1 px border
    static constexpr int inset { 1 };

protected:
    // Places c (already sized) at the right end of the header row
    void setHeaderControl (juce::Component& c);

    const Tap::Group group;

private:
    juce::String index, title;
    juce::Component* headerControl { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapPanel)
};

//==============================================================================
// Slider with the design's mouse behaviour: knobs drag 180 px for the full range (Shift for 4× finer)
// and every wheel step moves 1 % of the range
class TapSlider : public juce::Slider
{
public:
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    float lastDragY { 0.0f };
    double dragProportion { 0.0 };
};

//==============================================================================
// Rotary knob with its label and an always-visible value underneath
class KnobCell : public juce::Component, public AiTarget
{
public:
    KnobCell (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& label,
              Tap::Group group, Tap::Format format, int diameter = 48, int cellWidth = 68, bool bipolar = false);

    // Positions the cell by the knob's centre x and the top of the knob circle
    void placeAt (int centreX, int knobTop);

    void paint (juce::Graphics&) override;
    void resized() override;
    void setAiAlpha (float alpha) override;

private:
    const Tap::Group group;
    const Tap::Format format;
    const juce::String label;
    const int diameter, cellWidth;
    float aiAlpha { 0.0f };

    TapSlider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnobCell)
};

//==============================================================================
// Vertical fader for an envelope stage, with its label and value underneath
class SliderCell : public juce::Component, public AiTarget
{
public:
    SliderCell (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& label,
                Tap::Group group, Tap::Format format, int trackHeight, int cellWidth);

    // Positions the cell by the track's centre x and top
    void placeAt (int centreX, int trackTop);

    void paint (juce::Graphics&) override;
    void resized() override;
    void setAiAlpha (float alpha) override;

private:
    const Tap::Group group;
    const Tap::Format format;
    const juce::String label;
    const int trackHeight, cellWidth;
    float aiAlpha { 0.0f };

    TapSlider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderCell)
};

//==============================================================================
// Segmented control for a choice parameter (waveform, filter type). Sizes itself to 24 px tall.
class SegmentedSelector : public juce::Component, public AiTarget
{
public:
    struct Item
    {
        juce::String text;       // shown when icon is empty
        juce::Path icon;         // drawn in a 16 × 10 box
        juce::String tooltip;
    };

    SegmentedSelector (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
                       Tap::Group group, std::vector<Item> items, int itemWidth = 28);

    void paint (juce::Graphics&) override;
    void resized() override;
    void setAiAlpha (float alpha) override;

private:
    class Segment : public juce::Button
    {
    public:
        Segment (SegmentedSelector& owner, Item item);
        void paintButton (juce::Graphics&, bool isMouseOver, bool isButtonDown) override;

    private:
        SegmentedSelector& owner;
        Item item;
    };

    const Tap::Group group;
    const int itemWidth;
    float aiAlpha { 0.0f };
    std::vector<std::unique_ptr<Segment>> segments;
    std::unique_ptr<juce::ParameterAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SegmentedSelector)
};

//==============================================================================
// Pill switch for a binary parameter, in a knob-sized cell so its label lines up with the knobs beside it
class ToggleCell : public juce::Component, public AiTarget, private juce::Timer
{
public:
    ToggleCell (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& label, int cellWidth);

    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void setAiAlpha (float alpha) override;

private:
    void timerCallback() override;
    void setOn (bool shouldBeOn, bool animate);

    const juce::String label;
    bool on { false };
    float knobPosition { 0.0f };   // 0 = off, 1 = on; animated over 120 ms
    float aiAlpha { 0.0f };
    std::unique_ptr<juce::ParameterAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToggleCell)
};
