#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    // AI-changed highlight: 120 ms in, 2 s hold, 600 ms fade
    constexpr double highlightIn { 0.12 }, highlightHold { 2.0 }, highlightOut { 0.6 };

    void collectAiTargets (juce::Component& parent, std::map<juce::String, AiTarget*>& targets)
    {
        for (auto* child : parent.getChildren())
        {
            if (auto* target = dynamic_cast<AiTarget*> (child); target != nullptr && child->getComponentID().isNotEmpty())
                targets[child->getComponentID()] = target;

            collectAiTargets (*child, targets);
        }
    }

    // The prompt as a patch name: capitalised and cut at 34 characters
    juce::String patchNameFor (const juce::String& prompt)
    {
        auto name = prompt.substring (0, 1).toUpperCase() + prompt.substring (1, 34);
        return prompt.length() > 34 ? name.trimEnd() + Tap::ellipsis : name;
    }
}

//==============================================================================
TapSynthAudioProcessorEditor::TapSynthAudioProcessorEditor (TapSynthAudioProcessor& p)
: AudioProcessorEditor (&p)
, audioProcessor (p)
, osc1 (audioProcessor.apvts, "01", "Osc 1", "OSC1", "OSC1GAIN", "OSC1PITCH", "OSC1FMFREQ", "OSC1FMDEPTH")
, osc2 (audioProcessor.apvts, "02", "Osc 2", "OSC2", "OSC2GAIN", "OSC2PITCH", "OSC2FMFREQ", "OSC2FMDEPTH")
, filter (audioProcessor.apvts, "03", "Filter", "FILTERTYPE", "FILTERCUTOFF", "FILTERRESONANCE")
, filterAdsr (audioProcessor.apvts, "04", "Filter Env", Tap::Group::filter, "FILTERATTACK", "FILTERDECAY", "FILTERSUSTAIN", "FILTERRELEASE", "FILTERADSRDEPTH")
, lfo1 (audioProcessor.apvts, "05", "LFO", "LFO1FREQ", "LFO1DEPTH")
, adsr (audioProcessor.apvts, "06", "Amp Env", Tap::Group::accent, "ATTACK", "DECAY", "SUSTAIN", "RELEASE")
, reverb (audioProcessor.apvts, "07", "Reverb", "REVERBSIZE", "REVERBDAMPING", "REVERBWIDTH", "REVERBDRY", "REVERBWET", "REVERBFREEZE")
, meter (audioProcessor.getMeter(), "08", "Out")
{
    setLookAndFeel (&lookAndFeel);

    addAndMakeVisible (promptBar);
    addAndMakeVisible (presetBox);

    for (auto* panel : getPanels())
        addAndMakeVisible (panel);

    collectAiTargets (*this, aiTargets);

    promptBar.onSubmit = [this] (const juce::String& prompt) { sendPrompt (prompt); };

    presetBox.onPrevious = [this] { stepSessionPatch (-1); };
    presetBox.onNext = [this] { stepSessionPatch (1); };
    updatePresetBox();

    // Clicking anywhere outside the prompt bar takes the focus away from it
    addMouseListener (this, true);

    lastTick = juce::Time::getMillisecondCounterHiRes();
    startTimerHz (30);
    setSize (1066, 600);
}

TapSynthAudioProcessorEditor::~TapSynthAudioProcessorEditor()
{
    stopTimer();
    removeMouseListener (this);
    setLookAndFeel (nullptr);
}

std::array<TapPanel*, 8> TapSynthAudioProcessorEditor::getPanels()
{
    return { &osc1, &osc2, &filter, &filterAdsr, &lfo1, &adsr, &reverb, &meter };
}

//==============================================================================
void TapSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Tap::window);

    // Header strip with its bottom rule
    g.setColour (Tap::header);
    g.fillRect (0, 0, getWidth(), 56);
    g.setColour (Tap::lineSubtle);
    g.fillRect (0, 55, getWidth(), 1);

    // Wordmark: the logo mark, 8 px gap, "tap" in ink-muted, "Synth" in ink
    Tap::drawLogoMark (g, { 16.0f, 19.0f });

    const auto tap = Tap::font (20.0f, Tap::Weight::medium, -0.01f);
    const auto synth = Tap::font (20.0f, Tap::Weight::semiBold, -0.01f);
    const float tapWidth = juce::GlyphArrangement::getStringWidth (tap, "tap");

    g.setFont (tap);
    g.setColour (Tap::inkMuted);
    g.drawText ("tap", juce::Rectangle<float> (42.0f, 16.0f, tapWidth + 2.0f, 24.0f), juce::Justification::centredLeft, false);
    g.setFont (synth);
    g.setColour (Tap::ink);
    g.drawText ("Synth", juce::Rectangle<float> (42.0f + tapWidth, 16.0f, 100.0f, 24.0f), juce::Justification::centredLeft, false);

    // Panel drop shadows, drawn under the panels
    for (auto* panel : getPanels())
    {
        juce::Path shape;
        shape.addRoundedRectangle (panel->getBounds().toFloat(), 8.0f);
        juce::DropShadow (juce::Colour (0x80000000), 2, { 0, 1 }).drawForPath (g, shape);
    }
}

void TapSynthAudioProcessorEditor::resized()
{
    // Header: wordmark at x 16, prompt bar at x 156 (628 wide), preset box at x 800 (250 wide)
    promptBar.setBounds (juce::Rectangle<int> (156, 10, 628, 36).expanded (PromptBar::haloSize));
    presetBox.setBounds (800, 10, 250, 36);

    // Panels in signal-flow order, 8 px gutters
    osc1.setBounds       (8,   64,  264, 260);
    osc2.setBounds       (8,   332, 264, 260);
    filter.setBounds     (280, 64,  360, 260);
    filterAdsr.setBounds (280, 332, 236, 260);
    lfo1.setBounds       (524, 332, 116, 260);
    adsr.setBounds       (648, 64,  330, 260);
    reverb.setBounds     (648, 332, 330, 260);
    meter.setBounds      (986, 64,  72,  528);
}

void TapSynthAudioProcessorEditor::timerCallback()
{
    const auto now = juce::Time::getMillisecondCounterHiRes();
    const auto dt = (float) juce::jlimit (0.0, 0.25, (now - lastTick) / 1000.0);
    lastTick = now;

    for (auto* panel : getPanels())
        panel->tick (dt);

    updateAiHighlights (now);
}

void TapSynthAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    if (e.eventComponent != &promptBar && ! promptBar.isParentOf (e.eventComponent))
        promptBar.unfocusField();
}

//==============================================================================
void TapSynthAudioProcessorEditor::highlightParameters (const juce::StringArray& ids)
{
    const auto now = juce::Time::getMillisecondCounterHiRes();

    for (const auto& id : ids)
        if (aiTargets.count (id) > 0)
            aiHighlights[id] = now;

    updateAiHighlights (now);
}

void TapSynthAudioProcessorEditor::updateAiHighlights (double nowMs)
{
    for (auto it = aiHighlights.begin(); it != aiHighlights.end();)
    {
        const auto t = (nowMs - it->second) / 1000.0;
        double alpha = 0.0;

        if (t < highlightIn)                                     alpha = t / highlightIn;
        else if (t < highlightIn + highlightHold)                alpha = 1.0;
        else if (t < highlightIn + highlightHold + highlightOut) alpha = 1.0 - (t - highlightIn - highlightHold) / highlightOut;

        aiTargets[it->first]->setAiAlpha ((float) alpha);

        if (alpha <= 0.0 && t > highlightIn)
            it = aiHighlights.erase (it);
        else
            ++it;
    }
}

//==============================================================================
void TapSynthAudioProcessorEditor::updatePresetBox()
{
    const auto& patches = audioProcessor.sessionPatches;
    const int index = audioProcessor.currentSessionPatch;

    if (! juce::isPositiveAndBelow (index, (int) patches.size()))
    {
        presetBox.setPreset ("Init", false, false, false);
        return;
    }

    const auto& patch = patches[(size_t) index];
    presetBox.setPreset (patch.name, patch.fromAi, index > 0, index < (int) patches.size() - 1);
}

void TapSynthAudioProcessorEditor::stepSessionPatch (int delta)
{
    auto& patches = audioProcessor.sessionPatches;
    const int index = audioProcessor.currentSessionPatch + delta;

    if (! juce::isPositiveAndBelow (index, (int) patches.size()))
        return;

    audioProcessor.currentSessionPatch = index;
    audioProcessor.applyParameterSnapshot (patches[(size_t) index].values);
    updatePresetBox();

    // "Desfazer" refers to the patch that was just left
    if (promptBar.getState() == PromptBar::State::success)
        promptBar.resetState();
}

void TapSynthAudioProcessorEditor::applyAiPatch (const juce::var& params, const juce::String& prompt)
{
    const auto before = audioProcessor.getParameterSnapshot();
    audioProcessor.applyParametersFromJson (params);
    const auto after = audioProcessor.getParameterSnapshot();

    juce::StringArray changed;
    const auto& allParams = audioProcessor.getParameters();

    for (int i = 0; i < allParams.size(); ++i)
        if (std::abs (after[(size_t) i] - before[(size_t) i]) > 1.0e-6f)
            if (auto* withId = dynamic_cast<juce::AudioProcessorParameterWithID*> (allParams[i]))
                changed.add (withId->getParameterID());

    if (changed.isEmpty())
    {
        promptBar.showSuccess (Tap::utf8 ("Nenhum parâmetro alterado"), {}, {});
        return;
    }

    // Remember the patch for the preset box, with the state before the first AI patch as "Init"
    auto& patches = audioProcessor.sessionPatches;

    if (patches.empty())
    {
        patches.push_back ({ "Init", false, before });
        audioProcessor.currentSessionPatch = 0;
    }

    const int previousIndex = audioProcessor.currentSessionPatch;
    patches.push_back ({ patchNameFor (prompt), true, after });
    audioProcessor.currentSessionPatch = (int) patches.size() - 1;
    updatePresetBox();

    highlightParameters (changed);

    const auto count = changed.size();
    const auto text = count == 1 ? juce::String (Tap::utf8 ("1 parâmetro ajustado")) : juce::String (count) + Tap::utf8 (" parâmetros ajustados");

    promptBar.showSuccess (text, "Desfazer", [this, before, previousIndex]
    {
        audioProcessor.applyParameterSnapshot (before);
        audioProcessor.sessionPatches.pop_back();
        audioProcessor.currentSessionPatch = previousIndex;
        updatePresetBox();
        promptBar.resetState();
    });
}

// sendPrompt implementation
void TapSynthAudioProcessorEditor::sendPrompt (const juce::String& promptText)
{
    const juce::String apiKey   = "";
    const juce::String modelId  = "gemini-flash-latest"; // HERE HERE HERE "gemini-2.5-flash" if things go bad
    const juce::String endpoint =
        "https://generativelanguage.googleapis.com/v1beta/models/"
        + modelId
        + ":generateContent";

    if (apiKey.isEmpty())
    {
        promptBar.showError (Tap::utf8 ("Chave da API não configurada"));
        return;
    }

    // The editor can be closed while the request runs, so the result goes through a SafePointer
    // (null once the editor is deleted) instead of a raw `this`.
    juce::Component::SafePointer<TapSynthAudioProcessorEditor> safeThis (this);
    const juce::String promptCopy = promptText;
    const juce::String endpointCopy = endpoint;

    std::thread([promptCopy, endpointCopy, apiKey, safeThis]()
    {
        juce::String responseBody;
        juce::String errorMsg;
        int statusCode = 0;

        // --- Build Gemini JSON ---
        juce::String jsonPrompt =
            "You are an assistant that controls a synthesizer by returning ONLY valid JSON. "
            "Your output will be parsed by a machine. YOU MUST FOLLOW THESE RULES:\n"
            "\n"
            "STRICT RULES:\n"
            "1. Output ONLY a JSON object.\n"
            "2. No text before or after the JSON.\n"
            "3. No explanations, no comments, no code blocks.\n"
            "4. Use ONLY the parameter IDs listed below.\n"
            "5. Clamp all values inside the ranges provided.\n"
            "6. If the user gives a musical description, convert it into sensible parameter values.\n"
            "\n"
            "PARAMETER DEFINITIONS:\n"
            "CHOICE PARAMETERS (INTEGER INDEX):\n"
            "\"OSC1\": 0=Sine, 1=Saw, 2=Square\n"
            "\"OSC2\": 0=Sine, 1=Saw, 2=Square\n"
            "\"FILTERTYPE\": 0=Low Pass, 1=Band Pass, 2=High Pass\n"
            "\n"
            "FLOAT PARAMETERS:\n"
            "\"OSC1GAIN\": -40.0 to 0.2\n"
            "\"OSC2GAIN\": -40.0 to 0.2\n"
            "\"OSC1PITCH\": -48 to 48\n"
            "\"OSC2PITCH\": -48 to 48\n"
            "\"OSC1FMFREQ\": 0.0 to 1000.0\n"
            "\"OSC2FMFREQ\": 0.0 to 1000.0\n"
            "\"OSC1FMDEPTH\": 0.0 to 100.0\n"
            "\"OSC2FMDEPTH\": 0.0 to 100.0\n"
            "\"LFO1FREQ\": 0.0 to 20.0\n"
            "\"LFO1DEPTH\": 0.0 to 10000.0\n"
            "\"FILTERCUTOFF\": 20.0 to 20000.0\n"
            "\"FILTERRESONANCE\": 0.1 to 2.0\n"
            "\"ATTACK\": 0.1 to 1.0\n"
            "\"DECAY\": 0.1 to 1.0\n"
            "\"SUSTAIN\": 0.1 to 1.0\n"
            "\"RELEASE\": 0.1 to 3.0\n"
            "\"FILTERADSRDEPTH\": 0.0 to 10000.0\n"
            "\"FILTERATTACK\": 0.0 to 1.0\n"
            "\"FILTERDECAY\": 0.0 to 1.0\n"
            "\"FILTERSUSTAIN\": 0.0 to 1.0\n"
            "\"FILTERRELEASE\": 0.0 to 3.0\n"
            "\"REVERBSIZE\": 0.0 to 1.0\n"
            "\"REVERBWIDTH\": 0.0 to 1.0\n"
            "\"REVERBDAMPING\": 0.0 to 1.0\n"
            "\"REVERBDRY\": 0.0 to 1.0\n"
            "\"REVERBWET\": 0.0 to 1.0\n"
            "\"REVERBFREEZE\": 0.0 to 1.0\n"
            "\n"
            "OUTPUT FORMAT:\n"
            "Return ONLY something like this:\n"
            "{\n"
            "  \"OSC1\": 1,\n"
            "  \"OSC2\": 0,\n"
            "  \"OSC1GAIN\": -12.5,\n"
            "  \"OSC2GAIN\": -9.0,\n"
            "  \"FILTERCUTOFF\": 645.0,\n"
            "  \"ATTACK\": 0.2,\n"
            "  \"REVERBWET\": 0.35\n"
            "}\n"
            "\n"
            "USER REQUEST:\n" +
            promptCopy;

        juce::DynamicObject* textObj = new juce::DynamicObject();
        textObj->setProperty("text", jsonPrompt);


        juce::DynamicObject* partObj = new juce::DynamicObject();
        partObj->setProperty("parts", juce::Array<juce::var>{ juce::var(textObj) });

        juce::Array<juce::var> contentsArr;
        contentsArr.add(juce::var(partObj));

        juce::DynamicObject* rootObj = new juce::DynamicObject();
        rootObj->setProperty("contents", contentsArr);

        juce::DynamicObject* generationConfig = new juce::DynamicObject();
        generationConfig->setProperty("responseMimeType", "application/json");
        rootObj->setProperty("generationConfig", juce::var(generationConfig));

        const juce::String body = juce::JSON::toString(juce::var(rootObj));

        try
        {
            juce::URL url(endpointCopy);

            juce::String headers;
            headers << "Content-Type: application/json\r\n";
            headers << "x-goog-api-key: " << apiKey << "\r\n";

            std::unique_ptr<juce::InputStream> stream =
                url.withPOSTData(body)
                   .createInputStream(
                       juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
                           .withHttpRequestCmd("POST")
                           .withExtraHeaders(headers)
                           .withConnectionTimeoutMs(15000)
                           .withNumRedirectsToFollow(2)
                           .withStatusCode(&statusCode)
                   );

            if (stream == nullptr)
                errorMsg = "Could not open network stream.";
            else
                responseBody = stream->readEntireStreamAsString();
        }
        catch (const std::exception& e)
        {
            errorMsg = e.what();
        }
        catch (...)
        {
            errorMsg = "Unknown network error.";
        }
        
        juce::MessageManager::callAsync([safeThis, promptCopy, responseBody, errorMsg, statusCode]()
        {
            if (safeThis == nullptr)
                return;

            safeThis->handleResponse (promptCopy, responseBody, errorMsg, statusCode);
        });

    }).detach();
}

void TapSynthAudioProcessorEditor::handleResponse (const juce::String& prompt, const juce::String& responseBody,
                                                   const juce::String& errorMsg, int statusCode)
{
    // Errors stay inside the prompt bar; the details go to the debug log
    const auto fail = [this] (const juce::String& message, const juce::String& details)
    {
        DBG ("AI request failed: " << details);
        promptBar.showError (message, "Tentar de novo", [this] { promptBar.submit(); });
    };

    if (! errorMsg.isEmpty())
    {
        fail (Tap::utf8 ("Sem conexão com a API"), errorMsg);
        return;
    }

    juce::var parsed = juce::JSON::parse(responseBody);

    // Some platforms can't report the status code (0); Gemini also describes failures in an "error" object
    if ((statusCode != 0 && statusCode != 200) || parsed.hasProperty ("error"))
    {
        if (statusCode == 0)
            statusCode = (int) parsed.getProperty ("error", {}).getProperty ("code", 0);

        const auto message = statusCode == 400 || statusCode == 401 || statusCode == 403 ? Tap::utf8 ("Chave da API inválida")
                           : statusCode == 429                                           ? "Limite de uso da API atingido"
                           : statusCode >= 500                                           ? Tap::utf8 ("Serviço de IA indisponível")
                                                                                         : Tap::utf8 ("Não foi possível gerar o timbre");
        fail (message, "HTTP " + juce::String (statusCode) + ": " + responseBody);
        return;
    }

    auto candidates = parsed.getProperty("candidates", {});
    auto parts = candidates.isArray() && ! candidates.getArray()->isEmpty()
               ? candidates[0].getProperty("content", {}).getProperty("parts", {})
               : juce::var();

    if (! parts.isArray() || parts.getArray()->isEmpty())
    {
        fail (Tap::utf8 ("A IA não retornou um timbre"), "Unexpected response: " + responseBody);
        return;
    }

    juce::String jsonParams = parts[0].getProperty("text", {}).toString().trim();

    // Models sometimes wrap the JSON in a markdown code fence despite the instructions
    if (jsonParams.startsWith ("```"))
        jsonParams = jsonParams.fromFirstOccurrenceOf ("\n", false, false).upToLastOccurrenceOf ("```", false, false).trim();

    juce::var params = juce::JSON::parse(jsonParams);

    if (! params.isObject())
    {
        fail (Tap::utf8 ("Resposta inválida da IA"), "Returned text was not valid JSON: " + jsonParams);
        return;
    }

    applyAiPatch (params, prompt);
}
