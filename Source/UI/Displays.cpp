/*
  ==============================================================================

    Displays.cpp

  ==============================================================================
*/

#include "Displays.h"

namespace
{
    constexpr float twoPi = juce::MathConstants<float>::twoPi;

    void horizontalGridLine (juce::Graphics& g, float y, float w, juce::Colour colour)
    {
        g.setColour (colour);
        g.fillRect (juce::Rectangle<float> (0.0f, std::round (y), w, 1.0f));
    }

    void verticalGridLine (juce::Graphics& g, float x, float top, float bottom, juce::Colour colour)
    {
        g.setColour (colour);
        g.fillRect (juce::Rectangle<float> (std::round (x), top, 1.0f, bottom - top));
    }

    void dashedVerticalLine (juce::Graphics& g, float x, float top, float bottom)
    {
        const float dashes[] { 2.0f, 3.0f };
        g.drawDashedLine ({ std::round (x) + 0.5f, top, std::round (x) + 0.5f, bottom }, dashes, 2, 1.0f);
    }
}

//==============================================================================
Display::Display (juce::AudioProcessorValueTreeState& apvts, const juce::StringArray& paramIds)
{
    for (const auto& id : paramIds)
    {
        auto* raw = apvts.getRawParameterValue (id);
        auto* param = apvts.getParameter (id);
        jassert (raw != nullptr && param != nullptr);

        sources.push_back (raw);
        ranges.push_back (param->getNormalisableRange());
        lastValues.push_back (raw->load());
    }

    setInterceptsMouseClicks (false, false);
}

void Display::refresh()
{
    bool changed = false;

    for (size_t i = 0; i < sources.size(); ++i)
    {
        const auto v = sources[i]->load();

        if (v != lastValues[i])
        {
            lastValues[i] = v;
            changed = true;
        }
    }

    if (changed)
        repaint();
}

void Display::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();

    g.setColour (Tap::well);
    g.fillRoundedRectangle (b, 6.0f);

    {
        juce::Graphics::ScopedSaveState save (g);
        juce::Path clip;
        clip.addRoundedRectangle (b.reduced (1.0f), 5.0f);
        g.reduceClipRegion (clip);
        g.addTransform (juce::AffineTransform::translation (1.0f, 1.0f));
        paintContent (g, b.getWidth() - 2.0f, b.getHeight() - 2.0f);
    }

    g.setColour (Tap::lineSubtle);
    g.drawRoundedRectangle (b.reduced (0.5f), 5.5f, 1.0f);
}

void Display::drawChip (juce::Graphics& g, const juce::String& text)
{
    g.setFont (Tap::font (10.0f, Tap::Weight::medium, 0.06f));
    g.setColour (Tap::inkMuted);
    g.drawText (text, 8, 6, 400, 12, juce::Justification::centredLeft, false);
}

void Display::fillUnder (juce::Graphics& g, const juce::Path& area, juce::Colour colour)
{
    const auto bounds = area.getBounds();
    g.setGradientFill (juce::ColourGradient (colour.withAlpha (0.28f), 0.0f, bounds.getY(),
                                             colour.withAlpha (0.0f), 0.0f, bounds.getBottom(), false));
    g.fillPath (area);
}

void Display::strokeCurve (juce::Graphics& g, const juce::Path& curve, juce::Colour colour, float thickness)
{
    g.setColour (colour);
    g.strokePath (curve, { thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
}

//==============================================================================
OscDisplay::OscDisplay (juce::AudioProcessorValueTreeState& apvts, const juce::String& waveId, const juce::String& gainId,
                        const juce::String& fmFreqId, const juce::String& fmDepthId)
    : Display (apvts, { waveId, gainId, fmFreqId, fmDepthId })
{
}

void OscDisplay::paintContent (juce::Graphics& g, float w, float h)
{
    const int wave = juce::roundToInt (value (0));
    const float gain = value (1), fmFreq = value (2), fmDepth = value (3);

    const float mid = h * 0.5f + 6.0f;
    horizontalGridLine (g, mid, w, Tap::line);

    const auto shape = [wave] (float p)
    {
        p -= std::floor (p);

        if (wave == 0) return std::sin (twoPi * p);
        if (wave == 2) return p < 0.5f ? 1.0f : -1.0f;
        return 1.0f - 2.0f * p;
    };

    const float cycles = 2.5f;
    const float amp = (h * 0.5f - 14.0f) * juce::jmax (0.12f, juce::Decibels::decibelsToGain (gain, -100.0f));
    const float modRatio = fmFreq > 0.0f ? juce::jlimit (0.5f, 14.0f, fmFreq / 60.0f) : 0.0f;
    const float beta = fmDepth / 100.0f * 1.4f;

    juce::Path curve;
    const int n = 360;

    for (int i = 0; i <= n; ++i)
    {
        const float t = (float) i / (float) n;
        const float phase = cycles * t + (modRatio > 0.0f ? beta * std::sin (twoPi * modRatio * cycles * t) / twoPi * 2.0f : 0.0f);
        const juce::Point<float> pt { 8.0f + (w - 16.0f) * t, mid - amp * shape (phase) };

        if (i == 0) curve.startNewSubPath (pt);
        else        curve.lineTo (pt);
    }

    auto area = curve;
    area.lineTo (w - 8.0f, mid);
    area.lineTo (8.0f, mid);
    area.closeSubPath();

    fillUnder (g, area, Tap::osc);
    strokeCurve (g, curve, Tap::osc);

    static const char* names[] { "SINE", "SAW", "SQUARE" };
    auto chip = juce::String (names[juce::jlimit (0, 2, wave)]);

    if (fmDepth > 0.0f && fmFreq > 0.0f)
        chip << "  " << Tap::middleDot << "  FM " << Tap::formatValue (fmFreq, Tap::Format::hertz);

    drawChip (g, chip);
}

//==============================================================================
FilterCurve::FilterCurve (juce::AudioProcessorValueTreeState& apvts, const juce::String& typeId, const juce::String& cutoffId,
                          const juce::String& resonanceId)
    : Display (apvts, { typeId, cutoffId, resonanceId })
{
}

void FilterCurve::paintContent (juce::Graphics& g, float w, float h)
{
    const int type = juce::roundToInt (value (0));
    const float cutoff = juce::jmax (20.0f, value (1));
    const float q = juce::jmax (0.01f, value (2));

    const float dbTop = 15.0f, dbBottom = -39.0f, axisHeight = 16.0f;
    const float plotHeight = h - axisHeight;

    const auto xOf = [w] (float f) { return w * std::log (f / 20.0f) / std::log (1000.0f); };
    const auto yOf = [=] (float db) { return plotHeight * (dbTop - db) / (dbTop - dbBottom); };

    // Magnitude of juce::dsp::StateVariableTPTFilter's analog prototype; its band pass peaks at Q, not at unity
    const auto magnitudeDb = [type, cutoff, q] (float f)
    {
        const float x = f / cutoff;
        const float den = std::sqrt ((1.0f - x * x) * (1.0f - x * x) + (x / q) * (x / q));
        const float m = type == 2 ? x * x / den : type == 1 ? x / den : 1.0f / den;
        return juce::Decibels::gainToDecibels (m, -120.0f);
    };

    for (float f : { 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f })
        verticalGridLine (g, xOf (f), 0.0f, plotHeight, Tap::lineSubtle);

    for (float db : { 12.0f, -12.0f, -24.0f, -36.0f })
        horizontalGridLine (g, yOf (db), w, Tap::lineSubtle);

    const float y0 = yOf (0.0f);
    horizontalGridLine (g, y0, w, Tap::line);
    horizontalGridLine (g, plotHeight, w, Tap::lineSubtle);

    g.setFont (Tap::meta());
    g.setColour (Tap::inkDim);

    for (auto [f, text] : { std::pair<float, const char*> { 100.0f, "100" }, { 1000.0f, "1k" }, { 10000.0f, "10k" } })
        g.drawText (text, juce::Rectangle<float> (40.0f, 12.0f).withCentre ({ xOf (f), h - 8.0f }), juce::Justification::centred, false);

    g.drawText ("0 dB", juce::Rectangle<float> (w - 66.0f, y0 - 15.0f, 60.0f, 12.0f), juce::Justification::centredRight, false);

    juce::Path curve;
    const int n = 160;

    for (int i = 0; i <= n; ++i)
    {
        const float f = 20.0f * std::pow (1000.0f, (float) i / (float) n);
        const juce::Point<float> pt { w * (float) i / (float) n, juce::jlimit (-2.0f, plotHeight + 2.0f, yOf (magnitudeDb (f))) };

        if (i == 0) curve.startNewSubPath (pt);
        else        curve.lineTo (pt);
    }

    auto area = curve;
    area.lineTo (w, plotHeight);
    area.lineTo (0.0f, plotHeight);
    area.closeSubPath();

    fillUnder (g, area, Tap::filter);

    const float xc = xOf (cutoff);
    g.setColour (Tap::filter.withAlpha (0.6f));
    dashedVerticalLine (g, xc - 0.5f, 0.0f, plotHeight);

    strokeCurve (g, curve, Tap::filter);

    // Handle on the curve at the cutoff
    const float yc = juce::jlimit (4.0f, plotHeight - 4.0f, yOf (magnitudeDb (cutoff)));
    g.setColour (Tap::well);
    g.fillEllipse (juce::Rectangle<float> (11.0f, 11.0f).withCentre ({ xc, yc }));
    g.setColour (Tap::filter);
    g.fillEllipse (juce::Rectangle<float> (7.0f, 7.0f).withCentre ({ xc, yc }));

    static const char* names[] { "LOW PASS", "BAND PASS", "HIGH PASS" };
    drawChip (g, juce::String (names[juce::jlimit (0, 2, type)]) + "  " + Tap::middleDot + "  "
                     + Tap::formatValue (cutoff, Tap::Format::hertz) + "  " + Tap::middleDot + "  Q " + juce::String (q, 2));
}

//==============================================================================
EnvelopeGraph::EnvelopeGraph (juce::AudioProcessorValueTreeState& apvts, Tap::Group g, const juce::String& attackId,
                              const juce::String& decayId, const juce::String& sustainId, const juce::String& releaseId)
    : Display (apvts, { attackId, decayId, sustainId, releaseId }), group (g)
{
}

void EnvelopeGraph::paintContent (juce::Graphics& g, float w, float h)
{
    const auto colour = Tap::of (group);
    const float px = 8.0f, top = 10.0f, bottom = h - 6.0f;
    const float iw = w - px * 2.0f, ih = bottom - top;

    // Attack ≤ 27 %, decay ≤ 25 %, a fixed 16 % sustain hold, release ≤ 32 % of the width
    const float xA = px + juce::jmax (2.0f, iw * 0.27f * value (0) / rangeOf (0).end);
    const float xD = xA + juce::jmax (2.0f, iw * 0.25f * value (1) / rangeOf (1).end);
    const float xS = xD + iw * 0.16f;
    const float xR = xS + juce::jmax (2.0f, iw * 0.32f * value (3) / rangeOf (3).end);
    const float yS = top + ih * (1.0f - juce::jlimit (0.0f, 1.0f, value (2)));

    g.setColour (Tap::lineSubtle);

    for (float x : { xA, xD, xS })
        dashedVerticalLine (g, x, top, bottom);

    horizontalGridLine (g, bottom, w, Tap::line);

    juce::Path curve;
    curve.startNewSubPath (px, bottom);
    curve.quadraticTo (px + (xA - px) * 0.25f, top + ih * 0.15f, xA, top);
    curve.quadraticTo (xA + (xD - xA) * 0.15f, yS, xD, yS);
    curve.lineTo (xS, yS);
    curve.quadraticTo (xS + (xR - xS) * 0.15f, bottom, xR, bottom);

    auto area = curve;
    area.closeSubPath();

    fillUnder (g, area, colour);
    strokeCurve (g, curve, colour);

    for (auto pt : { juce::Point<float> { xA, top }, { xD, yS }, { xS, yS } })
    {
        const auto dot = juce::Rectangle<float> (6.0f, 6.0f).withCentre (pt);
        g.setColour (Tap::well);
        g.fillEllipse (dot);
        g.setColour (colour);
        g.drawEllipse (dot, 1.5f);
    }
}

//==============================================================================
LfoDisplay::LfoDisplay (juce::AudioProcessorValueTreeState& apvts, const juce::String& rateId, const juce::String& depthId)
    : Display (apvts, { rateId, depthId })
{
}

float LfoDisplay::amplitude (float h) const
{
    return (h * 0.5f - 4.0f) * juce::jmax (0.55f, std::pow (juce::jlimit (0.0f, 1.0f, value (1) / 10000.0f), 0.3f));
}

float LfoDisplay::cycles() const
{
    return juce::jlimit (1.0f, 6.0f, value (0) * 1.5f);
}

void LfoDisplay::advance (float dt)
{
    phase = std::fmod (phase + dt * juce::jmax (0.05f, value (0)) / cycles(), 1.0f);
    repaint();
}

void LfoDisplay::paintContent (juce::Graphics& g, float w, float h)
{
    const float mid = h * 0.5f, a = amplitude (h), c = cycles();

    horizontalGridLine (g, mid, w, Tap::line);

    juce::Path curve;

    for (int i = 0; i <= 120; ++i)
    {
        const float t = (float) i / 120.0f;
        const juce::Point<float> pt { w * t, mid - a * std::sin (twoPi * c * t) };

        if (i == 0) curve.startNewSubPath (pt);
        else        curve.lineTo (pt);
    }

    strokeCurve (g, curve, Tap::filter.withAlpha (0.9f), 1.5f);

    g.setColour (Tap::filter);
    g.fillEllipse (juce::Rectangle<float> (6.0f, 6.0f).withCentre ({ w * phase, mid - a * std::sin (twoPi * c * phase) }));
}
