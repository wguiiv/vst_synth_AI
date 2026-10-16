/*
  ==============================================================================

    TapLookAndFeel.h

    Design tokens (colours, type, value formats) and the LookAndFeel that draws
    knobs, sliders and tooltips. Values come from the tapSynth design system.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Tap
{
    // surfaces
    const juce::Colour window     { 0xff121212 }, header    { 0xff161616 }, panel      { 0xff1a1a1a },
                       well       { 0xff0e0e0e }, control   { 0xff262626 }, controlHi  { 0xff303030 };
    // lines
    const juce::Colour lineSubtle { 0xff262626 }, line      { 0xff333333 }, lineStrong { 0xff4a4a4a };
    // text
    const juce::Colour ink        { 0xffededed }, inkMuted  { 0xffa3a3a3 }, inkDim     { 0xff858585 };
    // module groups
    const juce::Colour osc        { 0xfff7be43 }, filter    { 0xfff65740 }, accent     { 0xff3dd6c6 };
    const juce::Colour onOsc      { 0xff1a1405 }, onAccent  { 0xff0b1f1d }, danger     { 0xffff6b81 };

    // The -tint tokens: a group colour at 16 % alpha
    inline juce::Colour tint (juce::Colour c) { return c.withAlpha ((juce::uint8) 0x29); }

    enum class Group { osc, filter, accent };
    inline juce::Colour of (Group g) { return g == Group::osc ? osc : g == Group::filter ? filter : accent; }

    //==============================================================================
    enum class Weight { regular, medium, semiBold };

    // IBM Plex Sans, embedded as BinaryData. Shared so the typefaces are only loaded once.
    struct Fonts
    {
        Fonts();
        juce::Typeface::Ptr get (Weight w) const;

        juce::Typeface::Ptr regular, medium, semiBold;
    };

    // size is the CSS font size in px (the em size), tracking is CSS letter-spacing in em
    juce::Font font (float size, Weight weight, float tracking = 0.0f);

    // The type styles of the design system
    inline juce::Font sectionTitle() { return font (11.0f, Weight::semiBold, 0.12f); }
    inline juce::Font sectionIndex() { return font (11.0f, Weight::semiBold, 0.04f); }
    inline juce::Font paramLabel()   { return font (10.0f, Weight::medium, 0.08f); }
    inline juce::Font meta()         { return font (10.0f, Weight::medium, 0.02f); }
    inline juce::Font value()        { return font (11.0f, Weight::medium); }
    inline juce::Font valueStrong()  { return font (13.0f, Weight::semiBold); }
    inline juce::Font prompt()       { return font (13.0f, Weight::regular); }
    inline juce::Font button()       { return font (12.0f, Weight::semiBold, 0.01f); }
    inline juce::Font message()      { return font (12.0f, Weight::medium); }

    //==============================================================================
    const juce::String minus { juce::String::charToString ((juce::juce_wchar) 0x2212) };
    const juce::String thinSpace { juce::String::charToString ((juce::juce_wchar) 0x2009) };
    const juce::String middleDot { juce::String::charToString ((juce::juce_wchar) 0x00b7) };
    const juce::String ellipsis { juce::String::charToString ((juce::juce_wchar) 0x2026) };

    // UI copy is Portuguese; juce::String (const char*) assumes ASCII, so accented literals go through here
    inline juce::String utf8 (const char* text) { return juce::String::fromUTF8 (text); }

    enum class Format { decibels, semitones, hertz, rate, seconds, percent, q, number };

    // Value strings: real minus sign, thin space before the unit, kHz above 999 Hz
    juce::String formatValue (float value, Format format);

    //==============================================================================
    // Line icons in a 16 × 10 box (waveforms) or 14 × 14 box (status), stroked at 1.5 px
    juce::Path sineIcon();
    juce::Path sawIcon();
    juce::Path squareIcon();
    juce::Path checkIcon();

    // The three-bar AI glyph in a 14 × 14 box. barScale scales each bar's height from its bottom.
    void drawAiGlyph (juce::Graphics& g, juce::Rectangle<float> box, juce::Colour colour, std::array<float, 3> barScale = { 1.0f, 1.0f, 1.0f });

    // The logo mark: three rounded bars in the group colours, 18 × 18
    void drawLogoMark (juce::Graphics& g, juce::Point<float> topLeft);

    // The radius-sm pill behind a value readout
    void drawPill (juce::Graphics& g, juce::Rectangle<float> pill, juce::Colour fill);
}

//==============================================================================
class TapLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TapLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    int getSliderThumbRadius (juce::Slider&) override;

    juce::Typeface::Ptr getTypefaceForFont (const juce::Font&) override;

    juce::Rectangle<int> getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea) override;
    void drawTooltip (juce::Graphics&, const juce::String& text, int width, int height) override;

    // Slider properties read by the drawing code
    static constexpr const char* groupProperty   { "group" };
    static constexpr const char* bipolarProperty { "bipolar" };
    static constexpr const char* aiAlphaProperty { "aiAlpha" };

private:
    juce::SharedResourcePointer<Tap::Fonts> fonts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapLookAndFeel)
};
