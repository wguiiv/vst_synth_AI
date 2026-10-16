/*
  ==============================================================================

    TapLookAndFeel.cpp

  ==============================================================================
*/

#include "TapLookAndFeel.h"
#include "BinaryData.h"

namespace Tap
{
    Fonts::Fonts()
        : regular  (juce::Typeface::createSystemTypefaceFor (BinaryData::IBMPlexSansRegular_ttf,  BinaryData::IBMPlexSansRegular_ttfSize)),
          medium   (juce::Typeface::createSystemTypefaceFor (BinaryData::IBMPlexSansMedium_ttf,   BinaryData::IBMPlexSansMedium_ttfSize)),
          semiBold (juce::Typeface::createSystemTypefaceFor (BinaryData::IBMPlexSansSemiBold_ttf, BinaryData::IBMPlexSansSemiBold_ttfSize))
    {
    }

    juce::Typeface::Ptr Fonts::get (Weight w) const
    {
        return w == Weight::regular ? regular : w == Weight::medium ? medium : semiBold;
    }

    juce::Font font (float size, Weight weight, float tracking)
    {
        juce::SharedResourcePointer<Fonts> fonts;
        juce::Font f { juce::FontOptions { fonts->get (weight) }.withPointHeight (size) };

        // JUCE's kerning factor is relative to the font height, CSS letter-spacing to the em size
        if (tracking != 0.0f)
            f.setExtraKerningFactor (tracking * size / f.getHeight());

        return f;
    }

    juce::String formatValue (float v, Format format)
    {
        const auto signedText = [] (float x, int decimals, bool plusSign)
        {
            const auto text = juce::String (std::abs (x), decimals);

            if (text.getDoubleValue() == 0.0)
                return text;

            return (x < 0.0f ? minus : plusSign ? juce::String ("+") : juce::String()) + text;
        };

        switch (format)
        {
            case Format::decibels:  return signedText (v, 1, true) + thinSpace + "dB";
            case Format::semitones:
            {
                const int st = juce::roundToInt (v);
                return (st > 0 ? juce::String ("+") : st < 0 ? minus : juce::String()) + juce::String (std::abs (st)) + thinSpace + "st";
            }
            case Format::rate:      return juce::String (v, 2) + thinSpace + "Hz";
            case Format::seconds:   return juce::String (v, 2) + thinSpace + "s";
            case Format::percent:   return juce::String (juce::roundToInt (v * 100.0f)) + thinSpace + "%";
            case Format::q:         return juce::String (v, 2);
            case Format::number:    return juce::String (juce::roundToInt (v));
            case Format::hertz:
                if (v >= 1000.0f)
                    return juce::String (v / 1000.0f, v >= 10000.0f ? 1 : 2) + thinSpace + "kHz";

                return (v < 100.0f ? juce::String (v, 1) : juce::String (juce::roundToInt (v))) + thinSpace + "Hz";
        }

        return juce::String (v);
    }

    //==============================================================================
    juce::Path sineIcon()
    {
        juce::Path p;
        p.startNewSubPath (1.0f, 5.0f);
        p.quadraticTo (4.5f, -2.6f, 8.0f, 5.0f);
        p.quadraticTo (11.5f, 12.6f, 15.0f, 5.0f);
        return p;
    }

    juce::Path sawIcon()
    {
        juce::Path p;
        p.startNewSubPath (1.0f, 8.5f);
        p.lineTo (8.0f, 1.5f);
        p.lineTo (8.0f, 8.5f);
        p.lineTo (15.0f, 1.5f);
        return p;
    }

    juce::Path squareIcon()
    {
        juce::Path p;
        p.startNewSubPath (1.0f, 8.5f);
        p.lineTo (1.0f, 1.5f);
        p.lineTo (8.0f, 1.5f);
        p.lineTo (8.0f, 8.5f);
        p.lineTo (15.0f, 8.5f);
        p.lineTo (15.0f, 1.5f);
        return p;
    }

    juce::Path checkIcon()
    {
        juce::Path p;
        p.startNewSubPath (3.0f, 7.5f);
        p.lineTo (6.0f, 10.5f);
        p.lineTo (11.0f, 4.0f);
        return p;
    }

    void drawAiGlyph (juce::Graphics& g, juce::Rectangle<float> box, juce::Colour colour, std::array<float, 3> barScale)
    {
        const float scale = box.getWidth() / 14.0f;
        const juce::Rectangle<float> bars[] { { 1.0f, 4.0f, 3.0f, 9.0f }, { 5.5f, 1.0f, 3.0f, 12.0f }, { 10.0f, 6.0f, 3.0f, 7.0f } };

        g.setColour (colour);

        for (size_t i = 0; i < 3; ++i)
        {
            auto bar = bars[i];
            bar = bar.withTop (bar.getBottom() - bar.getHeight() * barScale[i]);
            bar = (bar * scale) + box.getPosition();
            g.fillRoundedRectangle (bar, juce::jmin (1.5f * scale, bar.getHeight() * 0.5f));
        }
    }

    void drawLogoMark (juce::Graphics& g, juce::Point<float> topLeft)
    {
        g.setColour (osc);
        g.fillRoundedRectangle (topLeft.x + 1.0f, topLeft.y + 6.0f, 4.0f, 11.0f, 2.0f);
        g.setColour (filter);
        g.fillRoundedRectangle (topLeft.x + 7.0f, topLeft.y + 1.0f, 4.0f, 16.0f, 2.0f);
        g.setColour (accent);
        g.fillRoundedRectangle (topLeft.x + 13.0f, topLeft.y + 8.0f, 4.0f, 9.0f, 2.0f);
    }

    void drawPill (juce::Graphics& g, juce::Rectangle<float> pill, juce::Colour fill)
    {
        g.setColour (fill);
        g.fillRoundedRectangle (pill, 4.0f);
    }
}

//==============================================================================
TapLookAndFeel::TapLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId,     Tap::window);
    setColour (juce::TextEditor::backgroundColourId,          juce::Colours::transparentBlack); // the PromptBar paints the frame
    setColour (juce::TextEditor::textColourId,                Tap::ink);
    setColour (juce::TextEditor::highlightColourId,           Tap::tint (Tap::accent).withAlpha (0.35f));
    setColour (juce::TextEditor::highlightedTextColourId,     Tap::ink);
    setColour (juce::CaretComponent::caretColourId,           Tap::accent);
    setColour (juce::TextEditor::outlineColourId,             juce::Colours::transparentBlack);
    setColour (juce::TextEditor::focusedOutlineColourId,      juce::Colours::transparentBlack);
    setColour (juce::TooltipWindow::backgroundColourId,       Tap::well);
    setColour (juce::TooltipWindow::outlineColourId,          Tap::line);
    setColour (juce::TooltipWindow::textColourId,             Tap::ink);
    setColour (juce::PopupMenu::backgroundColourId,           Tap::panel);
    setColour (juce::PopupMenu::textColourId,                 Tap::ink);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Tap::controlHi);
}

void TapLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                       float pos, float a0, float a1, juce::Slider& s)
{
    // The slider is the knob diameter plus 5 px on each side, room for the AI ring
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (5.0f);
    const float D = juce::jmin (b.getWidth(), b.getHeight());
    const auto c = b.getCentre();
    const float rArc = D * 0.5f - 1.5f, rBody = D * 0.5f - 7.0f;
    const float angle = a0 + pos * (a1 - a0);
    const auto col = Tap::of ((Tap::Group) (int) s.getProperties()[groupProperty]);
    const juce::PathStrokeType arcStroke (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);

    juce::Path track;
    track.addCentredArc (c.x, c.y, rArc, rArc, 0.0f, a0, a1, true);
    g.setColour (Tap::line);
    g.strokePath (track, arcStroke);

    // Bipolar parameters fill from 12 o'clock (2π in the knob's 1.25π…2.75π range)
    const float from = (bool) s.getProperties()[bipolarProperty] ? juce::MathConstants<float>::twoPi : a0;

    if (std::abs (angle - from) > 0.01f)
    {
        juce::Path v;
        v.addCentredArc (c.x, c.y, rArc, rArc, 0.0f, juce::jmin (from, angle), juce::jmax (from, angle), true);
        g.setColour (col);
        g.strokePath (v, arcStroke);
    }

    juce::Path body;
    body.addEllipse (c.x - rBody, c.y - rBody, 2.0f * rBody, 2.0f * rBody);
    juce::DropShadow (juce::Colour (0xa6000000), 4, { 0, 2 }).drawForPath (g, body);
    g.setGradientFill (juce::ColourGradient (Tap::controlHi, c.x, c.y - rBody, Tap::control, c.x, c.y + rBody, false));
    g.fillPath (body);
    g.setColour (Tap::lineStrong);
    g.strokePath (body, juce::PathStrokeType (1.0f));

    juce::Path ptr;
    ptr.startNewSubPath (c.getPointOnCircumference (rBody * 0.38f, angle));
    ptr.lineTo (c.getPointOnCircumference (rBody * 0.86f, angle));
    g.setColour (Tap::ink);
    g.strokePath (ptr, { 2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });

    const float ai = (float) s.getProperties().getWithDefault (aiAlphaProperty, 0.0f);

    if (ai > 0.0f)
    {
        const float r = D * 0.5f + 3.5f;
        g.setColour (Tap::accent.withAlpha (ai));
        g.drawEllipse (c.x - r, c.y - r, 2.0f * r, 2.0f * r, 1.5f);
    }
}

void TapLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float sliderPos,
                                       float, float, juce::Slider::SliderStyle, juce::Slider& s)
{
    const auto col = Tap::of ((Tap::Group) (int) s.getProperties()[groupProperty]);
    const float cx = (float) x + (float) w * 0.5f;

    // The slider is 5 px taller than the track at both ends so the thumb never clips
    const float pad = (float) getSliderThumbRadius (s);
    auto track = juce::Rectangle<float> (cx - 2.0f, (float) y + pad, 4.0f, (float) h - 2.0f * pad);

    g.setColour (Tap::well);
    g.fillRoundedRectangle (track, 2.0f);
    g.setColour (Tap::line);
    g.drawRoundedRectangle (track.reduced (0.5f), 1.5f, 1.0f);
    g.setColour (col);
    g.fillRoundedRectangle (track.withTop (juce::jlimit (track.getY(), track.getBottom(), sliderPos)), 2.0f);

    auto thumb = juce::Rectangle<float> (20.0f, 10.0f).withCentre ({ cx, sliderPos });
    juce::Path tp;
    tp.addRoundedRectangle (thumb, 4.0f);
    juce::DropShadow (juce::Colour (0xb3000000), 2, { 0, 1 }).drawForPath (g, tp);
    g.setColour (Tap::controlHi);
    g.fillPath (tp);

    const float ai = (float) s.getProperties().getWithDefault (aiAlphaProperty, 0.0f);
    g.setColour (Tap::lineStrong.interpolatedWith (Tap::accent, ai));
    g.drawRoundedRectangle (thumb.reduced (0.5f), 3.5f, 1.0f);

    // 12 × 2 notch in the group colour
    g.setColour (col);
    g.fillRoundedRectangle (thumb.reduced (4.0f, 4.0f), 1.0f);
}

int TapLookAndFeel::getSliderThumbRadius (juce::Slider& s)
{
    if (s.isVertical())
        return 5;

    return LookAndFeel_V4::getSliderThumbRadius (s);
}

juce::Typeface::Ptr TapLookAndFeel::getTypefaceForFont (const juce::Font& f)
{
    // Fonts that don't name a typeface (stock widgets, tooltips) use IBM Plex Sans too
    if (f.getTypefaceName() == juce::Font::getDefaultSansSerifFontName())
        return f.isBold() ? fonts->semiBold : fonts->regular;

    return LookAndFeel_V4::getTypefaceForFont (f);
}

juce::Rectangle<int> TapLookAndFeel::getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea)
{
    const int w = juce::roundToInt (juce::GlyphArrangement::getStringWidth (Tap::value(), tipText)) + 16;
    const int h = 22;

    return juce::Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 12,
                                 screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6)  : screenPos.y + 16,
                                 w, h)
               .constrainedWithin (parentArea);
}

void TapLookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& text, int width, int height)
{
    const auto b = juce::Rectangle<float> ((float) width, (float) height);

    g.setColour (Tap::well);
    g.fillRoundedRectangle (b, 4.0f);
    g.setColour (Tap::line);
    g.drawRoundedRectangle (b.reduced (0.5f), 4.0f, 1.0f);
    g.setColour (Tap::ink);
    g.setFont (Tap::value());
    g.drawText (text, b, juce::Justification::centred, false);
}
