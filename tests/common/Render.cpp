#include "Render.h"
#include "PluginProcessor.h"

namespace tapsynth::test
{
    juce::AudioBuffer<float> render (const RenderSpec& spec)
    {
        TapSynthAudioProcessor processor;
        processor.setRateAndBufferSizeDetails (spec.sampleRate, spec.blockSize);

        for (const auto& [id, value] : spec.params)
        {
            auto* param = processor.apvts.getParameter (id);
            jassert (param != nullptr);

            if (param != nullptr)
                param->setValueNotifyingHost (param->convertTo0to1 (value));
        }

        processor.prepareToPlay (spec.sampleRate, spec.blockSize);

        const auto totalSamples = (int) std::lround (spec.lengthSeconds * spec.sampleRate);
        juce::AudioBuffer<float> output (2, totalSamples);
        output.clear();

        // Absolute sample positions of every MIDI event, sorted so each block can pick its slice
        std::vector<std::pair<int, juce::MidiMessage>> events;

        for (const auto& note : spec.notes)
        {
            const auto on  = (int) std::lround (note.startSeconds * spec.sampleRate);
            const auto off = (int) std::lround ((note.startSeconds + note.durationSeconds) * spec.sampleRate);
            events.emplace_back (on,  juce::MidiMessage::noteOn  (1, note.midiNote, note.velocity));
            events.emplace_back (off, juce::MidiMessage::noteOff (1, note.midiNote));
        }

        std::stable_sort (events.begin(), events.end(),
                          [] (const auto& a, const auto& b) { return a.first < b.first; });

        juce::AudioBuffer<float> block (2, spec.blockSize);
        juce::MidiBuffer midi;
        size_t nextEvent = 0;

        for (int start = 0; start < totalSamples; start += spec.blockSize)
        {
            const auto numSamples = juce::jmin (spec.blockSize, totalSamples - start);
            block.setSize (2, numSamples, false, false, true);
            block.clear();
            midi.clear();

            while (nextEvent < events.size() && events[nextEvent].first < start + numSamples)
            {
                midi.addEvent (events[nextEvent].second, juce::jmax (0, events[nextEvent].first - start));
                ++nextEvent;
            }

            processor.processBlock (block, midi);

            for (int ch = 0; ch < 2; ++ch)
                output.copyFrom (ch, start, block, ch, 0, numSamples);
        }

        processor.releaseResources();
        return output;
    }
}
