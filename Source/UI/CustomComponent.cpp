/*
  ==============================================================================

    CustomComponent.cpp

  ==============================================================================
*/

#include <JuceHeader.h>
#include "CustomComponent.h"

namespace
{
    constexpr int labelHeight { 12 };
    constexpr int valueGap { 2 };
    constexpr int valueHeight { 14 };

    // The label line and the value readout under a control; labelTop is in cell coordinates
    void drawLabelAndValue (juce::Graphics& g, int width, int labelTop, const juce::String& label, const juce::String& value,
                            juce::Colour groupColour, bool dragging, float aiAlpha, bool valueInAccent = false)
    {
        g.setFont (Tap::paramLabel());
        g.setColour (dragging ? Tap::ink : Tap::inkMuted);
        g.drawText (label.toUpperCase(), 0, labelTop, width, labelHeight, juce::Justification::centred, false);

        const auto font = Tap::value();
        const float textWidth = juce::GlyphArrangement::getStringWidth (font, value);
        const auto valueArea = juce::Rectangle<float> (0.0f, (float) (labelTop + labelHeight + valueGap), (float) width, (float) valueHeight);
        const auto pill = valueArea.withSizeKeepingCentre (textWidth + 8.0f, (float) valueHeight);

        auto textColour = valueInAccent ? Tap::accent : Tap::ink;

        if (dragging)
        {
            Tap::drawPill (g, pill, Tap::tint (groupColour));
            textColour = groupColour;
        }
        else if (aiAlpha > 0.0f)
        {
            Tap::drawPill (g, pill, Tap::tint (Tap::accent).withMultipliedAlpha (aiAlpha));
            textColour = textColour.interpolatedWith (Tap::accent, aiAlpha);
        }

        g.setFont (font);
        g.setColour (textColour);
        g.drawText (value, valueArea, juce::Justification::centred, false);
    }

    void tagSlider (juce::Slider& s, Tap::Group group, bool bipolar)
    {
        s.getProperties().set (TapLookAndFeel::groupProperty, (int) group);
        s.getProperties().set (TapLookAndFeel::bipolarProperty, bipolar);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    }

    double defaultValueOf (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId)
    {
        auto* param = apvts.getParameter (paramId);
        jassert (param != nullptr);
        return param != nullptr ? param->convertFrom0to1 (param->getDefaultValue()) : 0.0;
    }
}

//==============================================================================
TapPanel::TapPanel (juce::String i, juce::String t, Tap::Group g)
    : group (g), index (std::move (i)), title (std::move (t))
{
}

void TapPanel::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();

    g.setColour (Tap::panel);
    g.fillRoundedRectangle (b.reduced (0.5f), 8.0f);
    g.setColour (Tap::lineSubtle);
    g.drawRoundedRectangle (b.reduced (0.5f), 8.0f, 1.0f);

    // 1 px highlight along the inner top edge
    g.setColour (juce::Colour (0x0dffffff));
    g.fillRect (juce::Rectangle<float> (8.0f, 1.0f, b.getWidth() - 16.0f, 1.0f));

    // Header row: the two-digit signal-flow index in the group colour, then the title
    const auto indexFont = Tap::sectionIndex();
    const auto row = juce::Rectangle<float> (12.0f, 12.0f, b.getWidth() - 24.0f, 24.0f);

    g.setFont (indexFont);
    g.setColour (Tap::of (group));
    g.drawText (index, row, juce::Justification::centredLeft, false);

    g.setFont (Tap::sectionTitle());
    g.setColour (Tap::ink);
    g.drawText (title.toUpperCase(), row.withTrimmedLeft (juce::GlyphArrangement::getStringWidth (indexFont, index) + 8.0f),
                juce::Justification::centredLeft, false);
}

void TapPanel::resized()
{
    if (headerControl != nullptr)
        headerControl->setTopLeftPosition (getWidth() - 12 - headerControl->getWidth(), 12);
}

void TapPanel::setHeaderControl (juce::Component& c)
{
    headerControl = &c;
    addAndMakeVisible (c);
    resized();
}

//==============================================================================
void TapSlider::mouseDown (const juce::MouseEvent& e)
{
    Slider::mouseDown (e);
    lastDragY = e.position.y;
    dragProportion = valueToProportionOfLength (getValue());
}

void TapSlider::mouseDrag (const juce::MouseEvent& e)
{
    if (! isRotary() || e.mods.isPopupMenu())
    {
        Slider::mouseDrag (e);
        return;
    }

    // Accumulate in proportion space so stepped parameters still move on slow drags
    const double pixelsForFullRange = e.mods.isShiftDown() ? 720.0 : 180.0;
    dragProportion = juce::jlimit (0.0, 1.0, dragProportion + (lastDragY - e.position.y) / pixelsForFullRange);
    lastDragY = e.position.y;

    setValue (proportionOfLengthToValue (dragProportion), juce::sendNotificationSync);
}

void TapSlider::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    const float delta = wheel.deltaX != 0.0f ? -wheel.deltaX : wheel.deltaY;

    if (delta == 0.0f)
        return;

    // juce::Slider moves 0.15 × the wheel delta in proportion space; scale it to exactly 1 % per event
    auto stepped = wheel;
    stepped.deltaX = 0.0f;
    stepped.deltaY = (delta > 0.0f ? 1.0f : -1.0f) * 0.01f / 0.15f;
    Slider::mouseWheelMove (e, stepped);
}

//==============================================================================
KnobCell::KnobCell (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& l,
                    Tap::Group g, Tap::Format f, int d, int w, bool bipolar)
    : group (g), format (f), label (l), diameter (d), cellWidth (w)
{
    setComponentID (paramId);

    slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    // 270° sweep from 7:30 to 4:30; JUCE measures clockwise from 12 o'clock and needs positive angles
    slider.setRotaryParameters (1.25f * juce::MathConstants<float>::pi, 2.75f * juce::MathConstants<float>::pi, true);
    tagSlider (slider, group, bipolar);
    addAndMakeVisible (slider);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, slider);
    slider.setDoubleClickReturnValue (true, defaultValueOf (apvts, paramId));

    slider.onValueChange = [this] { repaint(); };
    slider.onDragStart = [this] { repaint(); };
    slider.onDragEnd = [this] { repaint(); };

    setSize (cellWidth, diameter + 10 + 4 + labelHeight + valueGap + valueHeight);
}

void KnobCell::placeAt (int centreX, int knobTop)
{
    setTopLeftPosition (centreX - cellWidth / 2, knobTop - 5);
}

void KnobCell::paint (juce::Graphics& g)
{
    drawLabelAndValue (g, getWidth(), diameter + 10 + 4, label, Tap::formatValue ((float) slider.getValue(), format),
                       Tap::of (group), slider.isMouseButtonDown(), aiAlpha);
}

void KnobCell::resized()
{
    const int s = diameter + 10;
    slider.setBounds ((getWidth() - s) / 2, 0, s, s);
}

void KnobCell::setAiAlpha (float alpha)
{
    aiAlpha = alpha;
    slider.getProperties().set (TapLookAndFeel::aiAlphaProperty, alpha);
    slider.repaint();
    repaint();
}

//==============================================================================
SliderCell::SliderCell (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& l,
                        Tap::Group g, Tap::Format f, int h, int w)
    : group (g), format (f), label (l), trackHeight (h), cellWidth (w)
{
    setComponentID (paramId);

    slider.setSliderStyle (juce::Slider::LinearVertical);
    tagSlider (slider, group, false);
    addAndMakeVisible (slider);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, slider);
    slider.setDoubleClickReturnValue (true, defaultValueOf (apvts, paramId));

    slider.onValueChange = [this] { repaint(); };
    slider.onDragStart = [this] { repaint(); };
    slider.onDragEnd = [this] { repaint(); };

    setSize (cellWidth, trackHeight + 10 + 6 + labelHeight + valueGap + valueHeight);
}

void SliderCell::placeAt (int centreX, int trackTop)
{
    setTopLeftPosition (centreX - cellWidth / 2, trackTop - 5);
}

void SliderCell::paint (juce::Graphics& g)
{
    drawLabelAndValue (g, getWidth(), trackHeight + 10 + 6, label, Tap::formatValue ((float) slider.getValue(), format),
                       Tap::of (group), slider.isMouseButtonDown(), aiAlpha);
}

void SliderCell::resized()
{
    slider.setBounds ((getWidth() - 28) / 2, 0, 28, trackHeight + 10);
}

void SliderCell::setAiAlpha (float alpha)
{
    aiAlpha = alpha;
    slider.getProperties().set (TapLookAndFeel::aiAlphaProperty, alpha);
    slider.repaint();
    repaint();
}

//==============================================================================
SegmentedSelector::Segment::Segment (SegmentedSelector& o, Item i)
    : juce::Button (i.tooltip), owner (o), item (std::move (i))
{
    setTooltip (item.tooltip);
    setClickingTogglesState (false);
}

void SegmentedSelector::Segment::paintButton (juce::Graphics& g, bool isMouseOver, bool)
{
    const auto b = getLocalBounds().toFloat();
    const bool on = getToggleState();

    if (on)
    {
        juce::Path p;
        p.addRoundedRectangle (b, 4.0f);
        juce::DropShadow (juce::Colour (0xb3000000), 2, { 0, 1 }).drawForPath (g, p);
        g.setColour (Tap::controlHi);
        g.fillPath (p);
    }

    const auto colour = on ? Tap::of (owner.group) : isMouseOver ? Tap::inkMuted : Tap::inkDim;
    g.setColour (colour);

    if (item.icon.isEmpty())
    {
        g.setFont (Tap::font (10.0f, Tap::Weight::semiBold, 0.08f));
        g.drawText (item.text, b, juce::Justification::centred, false);
    }
    else
    {
        const auto box = juce::Rectangle<float> (16.0f, 10.0f).withCentre (b.getCentre());
        g.strokePath (item.icon, { 1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded },
                      juce::AffineTransform::translation (box.getX(), box.getY()));
    }
}

SegmentedSelector::SegmentedSelector (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
                                      Tap::Group g, std::vector<Item> items, int w)
    : group (g), itemWidth (w)
{
    setComponentID (paramId);

    for (size_t i = 0; i < items.size(); ++i)
    {
        auto segment = std::make_unique<Segment> (*this, std::move (items[i]));
        segment->onClick = [this, i] { attachment->setValueAsCompleteGesture ((float) i); };
        addAndMakeVisible (*segment);
        segments.push_back (std::move (segment));
    }

    auto* param = apvts.getParameter (paramId);
    jassert (param != nullptr);

    attachment = std::make_unique<juce::ParameterAttachment> (*param, [this] (float value)
    {
        const auto selected = (size_t) juce::roundToInt (value);

        for (size_t i = 0; i < segments.size(); ++i)
            segments[i]->setToggleState (i == selected, juce::dontSendNotification);
    });
    attachment->sendInitialUpdate();

    // 1 px border + 2 px padding on each side, 2 px between segments
    const int n = (int) segments.size();
    setSize (6 + n * itemWidth + (n - 1) * 2, 24);
}

void SegmentedSelector::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();

    g.setColour (Tap::well);
    g.fillRoundedRectangle (b, 6.0f);
    g.setColour (Tap::lineSubtle.interpolatedWith (Tap::accent, aiAlpha));
    g.drawRoundedRectangle (b.reduced (0.5f), 5.5f, 1.0f);
}

void SegmentedSelector::resized()
{
    int x = 3;

    for (auto& segment : segments)
    {
        segment->setBounds (x, 3, itemWidth, 18);
        x += itemWidth + 2;
    }
}

void SegmentedSelector::setAiAlpha (float alpha)
{
    aiAlpha = alpha;
    repaint();
}

//==============================================================================
ToggleCell::ToggleCell (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& l, int cellWidth)
    : label (l)
{
    setComponentID (paramId);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);

    auto* param = apvts.getParameter (paramId);
    jassert (param != nullptr);

    attachment = std::make_unique<juce::ParameterAttachment> (*param, [this] (float value) { setOn (value >= 0.5f, isShowing()); });
    attachment->sendInitialUpdate();

    // A 58 px box (the height of a 48 px knob cell's knob area), then label and value
    setSize (cellWidth, 58 + 4 + labelHeight + valueGap + valueHeight);
}

void ToggleCell::paint (juce::Graphics& g)
{
    const auto sw = juce::Rectangle<float> (36.0f, 20.0f).withCentre ({ getWidth() * 0.5f, 29.0f });

    g.setColour (Tap::control.interpolatedWith (Tap::accent, knobPosition));
    g.fillRoundedRectangle (sw, 10.0f);
    g.setColour (Tap::lineStrong.interpolatedWith (Tap::accent, knobPosition));
    g.drawRoundedRectangle (sw.reduced (0.5f), 9.5f, 1.0f);

    const auto knob = juce::Rectangle<float> (sw.getX() + 2.0f + 16.0f * knobPosition, sw.getY() + 3.0f, 14.0f, 14.0f);
    juce::Path kp;
    kp.addEllipse (knob);
    juce::DropShadow (juce::Colour (0xb3000000), 2, { 0, 1 }).drawForPath (g, kp);
    g.setColour (Tap::inkMuted.interpolatedWith (Tap::onAccent, knobPosition));
    g.fillPath (kp);

    drawLabelAndValue (g, getWidth(), 58 + 4, label, on ? "ON" : "OFF", Tap::accent, false, aiAlpha, on);
}

void ToggleCell::mouseUp (const juce::MouseEvent& e)
{
    if (e.mouseWasClicked() && getLocalBounds().contains (e.getPosition()))
        attachment->setValueAsCompleteGesture (on ? 0.0f : 1.0f);
}

void ToggleCell::setAiAlpha (float alpha)
{
    aiAlpha = alpha;
    repaint();
}

void ToggleCell::setOn (bool shouldBeOn, bool animate)
{
    on = shouldBeOn;

    if (animate)
    {
        startTimerHz (60);
    }
    else
    {
        knobPosition = on ? 1.0f : 0.0f;
        repaint();
    }
}

void ToggleCell::timerCallback()
{
    // 120 ms from one end to the other at 60 Hz
    const float target = on ? 1.0f : 0.0f;
    const float step = 1.0f / (0.12f * 60.0f);

    knobPosition = std::abs (target - knobPosition) <= step ? target : knobPosition + (target > knobPosition ? step : -step);

    if (knobPosition == target)
        stopTimer();

    repaint();
}
