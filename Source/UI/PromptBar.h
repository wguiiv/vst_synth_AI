/*
  ==============================================================================

    PromptBar.h

    The header's AI prompt bar (idle, typing, generating, success, error) and
    the preset box next to it.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "TapLookAndFeel.h"

//==============================================================================
class PromptBar : public juce::Component, private juce::Timer
{
public:
    enum class State { idle, typing, generating, success, error };

    PromptBar();
    ~PromptBar() override;

    // Called with the trimmed prompt when the user submits a non-empty prompt; the bar is then generating
    std::function<void (const juce::String& prompt)> onSubmit;

    void showSuccess (const juce::String& message, const juce::String& action, std::function<void()> onAction);
    void showError (const juce::String& message, const juce::String& action = {}, std::function<void()> onAction = {});

    // Back to typing (prompt kept) or idle (empty prompt)
    void resetState();

    void submit();
    void unfocusField();

    State getState() const { return state; }
    juce::String getPrompt() const { return field.getText().trim(); }

    // The component is 3 px larger than the field on every side, room for the focus halo
    static constexpr int haloSize { 3 };

    void paint (juce::Graphics&) override;
    void resized() override;
    void focusOfChildComponentChanged (FocusChangeType) override;

private:
    class SendButton : public juce::Button
    {
    public:
        explicit SendButton (PromptBar& owner);
        void paintButton (juce::Graphics&, bool isMouseOver, bool isButtonDown) override;

    private:
        PromptBar& owner;
    };

    class LinkButton : public juce::Button
    {
    public:
        LinkButton();
        void paintButton (juce::Graphics&, bool isMouseOver, bool isButtonDown) override;
        int getIdealWidth() const;
    };

    void setState (State newState);
    void timerCallback() override;

    juce::Rectangle<float> frame() const;
    std::array<float, 3> glyphScales() const;
    float messageWidth() const;
    static float kbdWidth();

    State state { State::idle };
    juce::String message;
    std::function<void()> actionCallback;
    double animationStart { 0.0 };

    juce::TextEditor field;
    SendButton sendButton { *this };
    LinkButton actionButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PromptBar)
};

//==============================================================================
// Shows the current patch name (with an IA tag for AI patches) and steps through the session's patches
class PresetBox : public juce::Component
{
public:
    PresetBox();

    std::function<void()> onPrevious, onNext;

    void setPreset (const juce::String& name, bool fromAi, bool canGoBack, bool canGoForward);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class Chevron : public juce::Button
    {
    public:
        explicit Chevron (bool pointsLeft);
        void paintButton (juce::Graphics&, bool isMouseOver, bool isButtonDown) override;

    private:
        const bool pointsLeft;
    };

    juce::String name { "Init" };
    bool fromAi { false };
    Chevron previous { true }, next { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBox)
};
