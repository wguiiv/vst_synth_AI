/*
  ==============================================================================

    PromptBar.cpp

  ==============================================================================
*/

#include "PromptBar.h"

namespace
{
    constexpr int iconSlotWidth { 36 };
    constexpr int buttonWidth { 104 }, buttonHeight { 28 }, buttonInset { 3 };
    constexpr float pulseSeconds { 0.9f }, sweepSeconds { 1.4f }, sweepWidth { 120.0f };

    const juce::String enterHint { "Enter" };

    // The return-key arrow (↵) in an 8 × 8 box; IBM Plex Sans has no glyph for it
    juce::Path returnIcon()
    {
        juce::Path p;
        p.startNewSubPath (7.0f, 1.0f);
        p.lineTo (7.0f, 5.0f);
        p.lineTo (1.5f, 5.0f);
        p.startNewSubPath (3.5f, 3.0f);
        p.lineTo (1.5f, 5.0f);
        p.lineTo (3.5f, 7.0f);
        return p;
    }

    float easeInOut (float t) { return 0.5f - 0.5f * std::cos (juce::MathConstants<float>::pi * t); }

    void drawAlertIcon (juce::Graphics& g, juce::Rectangle<float> box)
    {
        const auto c = box.getCentre();
        g.drawEllipse (juce::Rectangle<float> (12.0f, 12.0f).withCentre (c), 1.6f);
        g.drawLine (c.x, box.getY() + 3.8f, c.x, box.getY() + 7.6f, 1.6f);
        g.fillEllipse (juce::Rectangle<float> (2.0f, 2.0f).withCentre ({ c.x, box.getY() + 10.1f }));
    }
}

//==============================================================================
PromptBar::SendButton::SendButton (PromptBar& o) : juce::Button ("Enviar"), owner (o)
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void PromptBar::SendButton::paintButton (juce::Graphics& g, bool isMouseOver, bool)
{
    const auto b = getLocalBounds().toFloat();
    const auto state = owner.state;

    juce::Colour fill, text;

    if (state == State::idle)            { fill = Tap::control;               text = Tap::inkMuted; }
    else if (state == State::generating) { fill = Tap::tint (Tap::accent);    text = Tap::accent; }
    else                                 { fill = Tap::accent;                text = Tap::onAccent; }

    if (isMouseOver && state != State::generating)
        fill = fill.brighter (0.08f);

    g.setColour (fill);
    g.fillRoundedRectangle (b, 6.0f);

    const auto font = Tap::button();
    const auto label = state == State::generating ? juce::String ("Gerando...") : juce::String ("Enviar");
    const float labelWidth = juce::GlyphArrangement::getStringWidth (font, label);

    g.setFont (font);
    g.setColour (text);

    if (state == State::generating)
    {
        // 12 px pulsing glyph, 6 px gap, label: centred as a group
        const float x = (b.getWidth() - (12.0f + 6.0f + labelWidth)) * 0.5f;
        Tap::drawAiGlyph (g, { x, b.getCentreY() - 6.0f, 12.0f, 12.0f }, Tap::accent, owner.glyphScales());
        g.setColour (text);
        g.drawText (label, juce::Rectangle<float> (x + 18.0f, 0.0f, labelWidth + 2.0f, b.getHeight()), juce::Justification::centredLeft, false);
    }
    else
    {
        g.drawText (label, b, juce::Justification::centred, false);
    }
}

//==============================================================================
PromptBar::LinkButton::LinkButton() : juce::Button ("action")
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

int PromptBar::LinkButton::getIdealWidth() const
{
    return (int) std::ceil (juce::GlyphArrangement::getStringWidth (Tap::message(), getButtonText()));
}

void PromptBar::LinkButton::paintButton (juce::Graphics& g, bool isMouseOver, bool)
{
    const auto font = Tap::message();
    const auto b = getLocalBounds().toFloat();

    g.setFont (font);
    g.setColour (Tap::ink);
    g.drawText (getButtonText(), b, juce::Justification::centredLeft, false);

    // Underline 2 px below the baseline
    const float baseline = b.getCentreY() + (font.getAscent() - font.getDescent()) * 0.5f;
    g.setColour (isMouseOver ? Tap::ink : Tap::lineStrong);
    g.fillRect (juce::Rectangle<float> (0.0f, baseline + 2.0f, b.getWidth(), 1.0f));
}

//==============================================================================
PromptBar::PromptBar()
{
    field.setMultiLine (false);
    field.setReturnKeyStartsNewLine (false);
    field.setFont (Tap::prompt());
    field.setTextToShowWhenEmpty ("Descreva um timbre...", Tap::inkDim);
    field.setIndents (0, 8);
    field.setInputRestrictions (200);
    field.setPopupMenuEnabled (true);
    field.setTitle ("Descreva um timbre");

    field.onReturnKey = [this] { submit(); };
    field.onEscapeKey = [this] { unfocusField(); };
    field.onTextChange = [this]
    {
        if (state != State::generating && state != State::typing)
            setState (State::typing);
    };

    sendButton.onClick = [this] { submit(); };
    actionButton.onClick = [this]
    {
        // Copy first: the callback may change the state and replace actionCallback
        if (auto callback = actionCallback)
            callback();
    };

    addAndMakeVisible (field);
    addAndMakeVisible (sendButton);
    addChildComponent (actionButton);
}

PromptBar::~PromptBar()
{
    stopTimer();
}

juce::Rectangle<float> PromptBar::frame() const
{
    return getLocalBounds().toFloat().reduced ((float) haloSize);
}

void PromptBar::submit()
{
    if (state == State::generating)
        return;

    const auto prompt = getPrompt();

    if (prompt.isEmpty())
    {
        showError ("Descreva um timbre antes de enviar");
        return;
    }

    setState (State::generating);

    if (onSubmit)
        onSubmit (prompt);
}

void PromptBar::showSuccess (const juce::String& text, const juce::String& action, std::function<void()> onAction)
{
    message = text;
    actionButton.setButtonText (action);
    actionCallback = std::move (onAction);
    setState (State::success);
    unfocusField();
}

void PromptBar::showError (const juce::String& text, const juce::String& action, std::function<void()> onAction)
{
    message = text;
    actionButton.setButtonText (action);
    actionCallback = std::move (onAction);
    setState (State::error);
}

void PromptBar::resetState()
{
    setState (getPrompt().isEmpty() ? State::idle : State::typing);
}

void PromptBar::unfocusField()
{
    if (field.hasKeyboardFocus (true))
        field.giveAwayKeyboardFocus();
}

void PromptBar::setState (State newState)
{
    state = newState;

    const bool generating = state == State::generating;
    field.setReadOnly (generating);
    field.setCaretVisible (! generating);
    field.applyColourToAllText (generating ? Tap::inkMuted : Tap::ink);

    actionButton.setVisible ((state == State::success || state == State::error) && actionButton.getButtonText().isNotEmpty());

    // The field narrows to make room for the message; show the start of the prompt rather than its end
    if (state == State::success || state == State::error)
        field.setCaretPosition (0);

    if (generating)
    {
        animationStart = juce::Time::getMillisecondCounterHiRes();
        startTimerHz (30);
    }
    else
    {
        stopTimer();
    }

    resized();
    repaint();
    sendButton.repaint();
}

void PromptBar::focusOfChildComponentChanged (FocusChangeType)
{
    if (field.hasKeyboardFocus (true))
    {
        if (state == State::idle || state == State::success || state == State::error)
            setState (State::typing);
    }
    else if (state == State::typing && getPrompt().isEmpty())
    {
        setState (State::idle);
    }
}

void PromptBar::timerCallback()
{
    repaint();
}

std::array<float, 3> PromptBar::glyphScales() const
{
    if (state != State::generating)
        return { 1.0f, 1.0f, 1.0f };

    // Each bar pulses between 35 % and 100 % every 900 ms, the second and third 300 ms and 600 ms ahead
    const auto t = (float) ((juce::Time::getMillisecondCounterHiRes() - animationStart) / 1000.0);
    std::array<float, 3> scales {};

    for (size_t i = 0; i < 3; ++i)
    {
        const float phase = std::fmod (t + 0.3f * (float) i, pulseSeconds) / pulseSeconds;
        scales[i] = 0.35f + 0.65f * (0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * phase));
    }

    return scales;
}

float PromptBar::messageWidth() const
{
    // 12 px padding, 14 px icon, 6 px gap, text, then 6 px gap and the action, 10 px padding
    float w = 12.0f + 14.0f + 6.0f + juce::GlyphArrangement::getStringWidth (Tap::message(), message) + 10.0f;

    if (actionButton.getButtonText().isNotEmpty())
        w += 6.0f + (float) actionButton.getIdealWidth();

    return std::ceil (w);
}

float PromptBar::kbdWidth()
{
    // 1 px border, 5 px padding, 8 px arrow, 3 px gap, text, 5 px padding, 1 px border
    return std::ceil (juce::GlyphArrangement::getStringWidth (Tap::meta(), enterHint)) + 23.0f;
}

void PromptBar::resized()
{
    const auto f = frame().toNearestInt();

    sendButton.setBounds (f.getRight() - 1 - buttonInset - buttonWidth, f.getCentreY() - buttonHeight / 2, buttonWidth, buttonHeight);

    int right = sendButton.getX();

    if (state == State::typing)
        right -= 8 + (int) kbdWidth();

    if (state == State::success || state == State::error)
    {
        right -= (int) messageWidth();

        if (actionButton.getButtonText().isNotEmpty())
        {
            const int actionWidth = actionButton.getIdealWidth();
            actionButton.setBounds (sendButton.getX() - 10 - actionWidth, f.getCentreY() - 9, actionWidth, 18);
        }
    }

    const int fieldX = f.getX() + 1 + iconSlotWidth;
    field.setBounds (fieldX, f.getY() + 1, juce::jmax (40, right - fieldX), f.getHeight() - 2);
}

void PromptBar::paint (juce::Graphics& g)
{
    const auto f = frame();

    if (state == State::typing)
    {
        g.setColour (Tap::accent.withAlpha (0.2f));
        g.fillRoundedRectangle (f.expanded ((float) haloSize), 8.0f + (float) haloSize);
    }

    g.setColour (Tap::well);
    g.fillRoundedRectangle (f, 8.0f);

    {
        juce::Graphics::ScopedSaveState save (g);
        juce::Path clip;
        clip.addRoundedRectangle (f.reduced (1.0f), 7.0f);
        g.reduceClipRegion (clip);

        const auto inner = f.reduced (1.0f);

        // Hairline across the bottom: one prompt touches every module
        const float hairlineAlpha = state == State::idle       ? 0.45f
                                  : state == State::generating ? 0.2f
                                  : state == State::error      ? 0.0f
                                                               : 1.0f;
        if (hairlineAlpha > 0.0f)
        {
            juce::ColourGradient hairline (Tap::osc.withAlpha (hairlineAlpha), inner.getX(), 0.0f,
                                           Tap::accent.withAlpha (hairlineAlpha), inner.getRight(), 0.0f, false);
            hairline.addColour (0.5, Tap::filter.withAlpha (hairlineAlpha));
            g.setGradientFill (hairline);
            g.fillRect (inner.withTop (inner.getBottom() - 1.0f));
        }

        if (state == State::generating)
        {
            const auto t = (float) std::fmod ((juce::Time::getMillisecondCounterHiRes() - animationStart) / 1000.0, (double) sweepSeconds);
            const float x = inner.getX() + easeInOut (t / sweepSeconds) * (inner.getWidth() + sweepWidth) - sweepWidth;

            juce::ColourGradient sweep (Tap::accent.withAlpha (0.0f), x, 0.0f, Tap::accent.withAlpha (0.0f), x + sweepWidth, 0.0f, false);
            sweep.addColour (0.5, Tap::accent);
            g.setGradientFill (sweep);
            g.fillRect (juce::Rectangle<float> (x, inner.getBottom() - 2.0f, sweepWidth, 2.0f));
        }
    }

    const auto border = state == State::idle       ? Tap::line
                      : state == State::generating ? Tap::lineStrong
                      : state == State::error      ? Tap::danger
                                                   : Tap::accent;
    g.setColour (border);
    g.drawRoundedRectangle (f.reduced (0.5f), 7.5f, 1.0f);

    // AI glyph in the icon slot
    const auto slot = juce::Rectangle<float> (f.getX() + 1.0f, f.getY() + 1.0f, (float) iconSlotWidth, f.getHeight() - 2.0f);
    Tap::drawAiGlyph (g, juce::Rectangle<float> (14.0f, 14.0f).withCentre (slot.getCentre()), Tap::accent, glyphScales());

    if (state == State::typing)
    {
        const float w = kbdWidth();
        const auto chip = juce::Rectangle<float> ((float) sendButton.getX() - 8.0f - w, f.getCentreY() - 9.0f, w, 18.0f);
        g.setColour (Tap::line);
        g.drawRoundedRectangle (chip.reduced (0.5f), 3.5f, 1.0f);
        g.setColour (Tap::inkDim);
        g.strokePath (returnIcon(), { 1.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded },
                      juce::AffineTransform::translation (chip.getX() + 6.0f, chip.getCentreY() - 4.0f));
        g.setFont (Tap::meta());
        g.drawText (enterHint, chip.withTrimmedLeft (17.0f), juce::Justification::centredLeft, false);
    }

    if (state == State::success || state == State::error)
    {
        const auto colour = state == State::success ? Tap::accent : Tap::danger;
        const float x = (float) sendButton.getX() - messageWidth() + 12.0f;
        const auto icon = juce::Rectangle<float> (x, f.getCentreY() - 7.0f, 14.0f, 14.0f);

        g.setColour (colour);

        if (state == State::success)
            g.strokePath (Tap::checkIcon(), { 1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded },
                          juce::AffineTransform::translation (icon.getX(), icon.getY()));
        else
            drawAlertIcon (g, icon);

        g.setFont (Tap::message());
        g.drawText (message, juce::Rectangle<float> (icon.getRight() + 6.0f, f.getY(), 400.0f, f.getHeight()),
                    juce::Justification::centredLeft, false);
    }
}

//==============================================================================
PresetBox::Chevron::Chevron (bool left) : juce::Button (left ? "Previous patch" : "Next patch"), pointsLeft (left)
{
    setTooltip (left ? "Timbre anterior" : Tap::utf8 ("Próximo timbre"));
}

void PresetBox::Chevron::paintButton (juce::Graphics& g, bool isMouseOver, bool)
{
    const auto box = juce::Rectangle<float> (10.0f, 10.0f).withCentre (getLocalBounds().toFloat().getCentre());

    juce::Path p;

    if (pointsLeft)
    {
        p.startNewSubPath (6.5f, 1.5f);
        p.lineTo (3.0f, 5.0f);
        p.lineTo (6.5f, 8.5f);
    }
    else
    {
        p.startNewSubPath (3.5f, 1.5f);
        p.lineTo (7.0f, 5.0f);
        p.lineTo (3.5f, 8.5f);
    }

    g.setColour (! isEnabled() ? Tap::line : isMouseOver ? Tap::ink : Tap::inkMuted);
    g.strokePath (p, { 1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded },
                  juce::AffineTransform::translation (box.getX(), box.getY()));
}

PresetBox::PresetBox()
{
    previous.onClick = [this] { if (onPrevious) onPrevious(); };
    next.onClick = [this] { if (onNext) onNext(); };

    addAndMakeVisible (previous);
    addAndMakeVisible (next);

    setPreset ("Init", false, false, false);
}

void PresetBox::setPreset (const juce::String& newName, bool isFromAi, bool canGoBack, bool canGoForward)
{
    name = newName;
    fromAi = isFromAi;
    previous.setEnabled (canGoBack);
    next.setEnabled (canGoForward);
    repaint();
}

void PresetBox::resized()
{
    previous.setBounds (1, 1, 28, getHeight() - 2);
    next.setBounds (getWidth() - 29, 1, 28, getHeight() - 2);
}

void PresetBox::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();

    g.setColour (Tap::well);
    g.fillRoundedRectangle (b, 8.0f);
    g.setColour (Tap::lineSubtle);
    g.drawRoundedRectangle (b.reduced (0.5f), 7.5f, 1.0f);
    g.fillRect (juce::Rectangle<float> (29.0f, 1.0f, 1.0f, b.getHeight() - 2.0f));
    g.fillRect (juce::Rectangle<float> (b.getWidth() - 30.0f, 1.0f, 1.0f, b.getHeight() - 2.0f));

    const auto mid = juce::Rectangle<float> (40.0f, 4.0f, b.getWidth() - 80.0f, 28.0f);

    const auto labelFont = Tap::paramLabel();
    const juce::String label ("PRESET");
    g.setFont (labelFont);
    g.setColour (Tap::inkDim);
    g.drawText (label, mid.withHeight (12.0f), juce::Justification::centredLeft, false);

    if (fromAi)
    {
        const auto tagFont = Tap::font (10.0f, Tap::Weight::semiBold, 0.04f);
        const auto tag = juce::Rectangle<float> (mid.getX() + juce::GlyphArrangement::getStringWidth (labelFont, label) + 6.0f, mid.getY(),
                                                 juce::GlyphArrangement::getStringWidth (tagFont, "IA") + 8.0f, 12.0f);
        g.setColour (Tap::tint (Tap::accent));
        g.fillRoundedRectangle (tag, 4.0f);
        g.setColour (Tap::accent);
        g.setFont (tagFont);
        g.drawText ("IA", tag, juce::Justification::centred, false);
    }

    g.setFont (Tap::valueStrong());
    g.setColour (Tap::ink);
    g.drawText (name, mid.withTrimmedTop (12.0f), juce::Justification::centredLeft, true);
}
