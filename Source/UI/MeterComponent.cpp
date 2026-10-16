#include <JuceHeader.h>
#include "MeterComponent.h"

namespace
{
    constexpr float floorDb { -60.0f };
    constexpr float barTop { 10.0f };
    constexpr float barWidth { 10.0f };
    constexpr float barX[] { 26.0f, 40.0f };

    constexpr float holdSeconds { 1.2f };
    constexpr float holdFallDbPerSecond { 20.0f };
    constexpr float peakFallDbPerSecond { 30.0f };
}

//==============================================================================
LevelMeter::LevelMeter (MeterData& m) : meterData (m)
{
    setSize (58, barHeight + 70);
}

void LevelMeter::update (float dt)
{
    for (int i = 0; i < 2; ++i)
    {
        auto& c = channels[(size_t) i];
        const float peakNow = juce::Decibels::gainToDecibels (meterData.takePeak (i), floorDb);

        c.rmsDb = juce::Decibels::gainToDecibels (meterData.getRMS (i), floorDb);
        c.peakDb = juce::jmax (peakNow, c.peakDb - peakFallDbPerSecond * dt);

        // Hold for 1.2 s, then fall at 20 dB/s
        if (peakNow >= c.holdDb)
        {
            c.holdDb = peakNow;
            c.holdTime = 0.0f;
        }
        else
        {
            c.holdTime += dt;

            if (c.holdTime > holdSeconds)
                c.holdDb = juce::jmax (peakNow, c.holdDb - holdFallDbPerSecond * dt);
        }

        c.clipped = c.clipped || peakNow >= 0.0f;
    }

    repaint();
}

float LevelMeter::yOf (float db) const
{
    return barTop + (float) barHeight * juce::jlimit (0.0f, 1.0f, -db / -floorDb);
}

void LevelMeter::fillLevel (juce::Graphics& g, juce::Rectangle<float> bar, float db, float alpha) const
{
    const float top = yOf (db);

    // Zones reuse the group colours: accent below −12 dBFS, amber to −3, coral above
    const std::pair<float, juce::Colour> zones[] { { -12.0f, Tap::accent }, { -3.0f, Tap::osc }, { 0.0f, Tap::filter } };
    float zoneBottom = bar.getBottom();

    for (const auto& [zoneTopDb, colour] : zones)
    {
        const float zoneTop = juce::jmax (top, yOf (zoneTopDb));

        if (zoneTop < zoneBottom)
        {
            g.setColour (colour.withAlpha (alpha));
            g.fillRect (bar.withTop (zoneTop).withBottom (zoneBottom));
        }

        zoneBottom = juce::jmin (zoneBottom, yOf (zoneTopDb));
    }
}

void LevelMeter::paint (juce::Graphics& g)
{
    // dB scale
    g.setFont (Tap::meta());
    g.setColour (Tap::inkDim);

    for (int db : { 0, -6, -12, -18, -24, -36, -48, -60 })
    {
        const auto text = db == 0 ? juce::String ("0") : Tap::minus + juce::String (-db);
        g.drawText (text, juce::Rectangle<float> (0.0f, yOf ((float) db) - 6.0f, 22.0f, 12.0f), juce::Justification::centredRight, false);
    }

    for (size_t i = 0; i < 2; ++i)
    {
        const auto& c = channels[i];
        const float x = barX[i];

        // Clip LED
        g.setColour (c.clipped ? Tap::filter : Tap::control);
        g.fillRoundedRectangle (x, 0.0f, barWidth, 4.0f, 2.0f);

        const auto bar = juce::Rectangle<float> (x, barTop, barWidth, (float) barHeight);
        g.setColour (Tap::well);
        g.fillRoundedRectangle (bar, 2.0f);

        {
            juce::Graphics::ScopedSaveState save (g);
            juce::Path clip;
            clip.addRoundedRectangle (bar, 2.0f);
            g.reduceClipRegion (clip);

            // Peak at 35 % behind the solid RMS
            fillLevel (g, bar, c.peakDb, 0.35f);
            fillLevel (g, bar, c.rmsDb, 1.0f);

            if (c.holdDb > floorDb)
            {
                g.setColour (Tap::ink);
                g.fillRect (bar.withTop (yOf (c.holdDb) - 1.0f).withHeight (2.0f));
            }
        }

        g.setColour (Tap::lineSubtle);
        g.drawRoundedRectangle (bar.reduced (0.5f), 1.5f, 1.0f);

        g.setFont (Tap::font (10.0f, Tap::Weight::semiBold));
        g.setColour (Tap::inkMuted);
        g.drawText (i == 0 ? "L" : "R", juce::Rectangle<float> (x, barTop + (float) barHeight + 6.0f, barWidth, 12.0f),
                    juce::Justification::centred, false);
    }

    // Peak-hold readouts
    for (size_t i = 0; i < 2; ++i)
    {
        const auto row = juce::Rectangle<float> (0.0f, barTop + (float) barHeight + 26.0f + 18.0f * (float) i, (float) getWidth(), 16.0f);
        const float hold = channels[i].holdDb;

        g.setFont (Tap::font (10.0f, Tap::Weight::semiBold, 0.06f));
        g.setColour (Tap::inkDim);
        g.drawText (i == 0 ? "L" : "R", row, juce::Justification::centredLeft, false);

        const auto text = hold <= floorDb ? Tap::minus + "inf"
                                          : (hold < 0.0f ? Tap::minus : juce::String()) + juce::String (std::abs (hold), 1);
        g.setFont (Tap::valueStrong());
        g.setColour (Tap::ink);
        g.drawText (text, row, juce::Justification::centredRight, false);
    }
}

juce::Rectangle<int> LevelMeter::readoutArea() const
{
    return getLocalBounds().withTop ((int) barTop + barHeight + 26);
}

void LevelMeter::mouseDown (const juce::MouseEvent& e)
{
    if (! readoutArea().contains (e.getPosition()))
        return;

    for (auto& c : channels)
    {
        c.holdDb = floorDb;
        c.holdTime = 0.0f;
        c.clipped = false;
    }

    repaint();
}

//==============================================================================
MeterComponent::MeterComponent (MeterData& meterData, juce::String index, juce::String title)
: TapPanel (index, title, Tap::Group::accent)
, meter (meterData)
{
    addAndMakeVisible (meter);
}

void MeterComponent::resized()
{
    TapPanel::resized();
    meter.setTopLeftPosition (inset + 6, inset + 46);
}

void MeterComponent::tick (float dt)
{
    meter.update (dt);
}
