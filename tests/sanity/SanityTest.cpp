// Sanity strategy: "is the sound broken?" No NaN or infinity, peak below a ceiling, no unexpected silence, no DC.
// Runs on a fixed list of musical configurations and on random patches applied through applyParametersFromJson,
// the same path the AI responses use.

#include "../common/Analysis.h"
#include "../common/Check.h"
#include "../common/Render.h"
#include "PluginProcessor.h"

#include <gtest/gtest.h>

using namespace tapsynth::test;

namespace
{
    struct Config
    {
        std::string name;
        RenderSpec spec;
    };

    RenderSpec make (std::map<juce::String, float> params, std::vector<Note> notes = { Note { 69, 1.0f, 0.0, 0.6 } },
                     double sampleRate = 48000.0, int blockSize = 512)
    {
        RenderSpec spec;
        spec.sampleRate = sampleRate;
        spec.blockSize = blockSize;
        spec.lengthSeconds = 1.0;
        spec.params = std::move (params);
        spec.notes = std::move (notes);
        return spec;
    }

    std::vector<Config> configs()
    {
        const std::vector<Note> chord { Note { 60, 1.0f, 0.0, 0.6 }, Note { 64, 1.0f, 0.0, 0.6 }, Note { 67, 1.0f, 0.0, 0.6 } };

        return {
            { "Default",          make ({}) },
            { "Saw",              make ({ { "OSC1", 1 }, { "OSC2", 1 } }) },
            { "Square",           make ({ { "OSC1", 2 }, { "OSC2", 2 } }) },
            { "LowPassResonant",  make ({ { "OSC1", 1 }, { "OSC2", 1 }, { "FILTERCUTOFF", 300 }, { "FILTERRESONANCE", 2.0f }, { "FILTERADSRDEPTH", 0 } }) },
            { "BandPass",         make ({ { "OSC1", 1 }, { "OSC2", 1 }, { "FILTERTYPE", 1 }, { "FILTERCUTOFF", 1000 }, { "FILTERADSRDEPTH", 0 } }) },
            { "HighPass",         make ({ { "OSC1", 1 }, { "OSC2", 1 }, { "FILTERTYPE", 2 }, { "FILTERCUTOFF", 1000 }, { "FILTERADSRDEPTH", 0 } }) },
            { "FilterEnvelope",   make ({ { "OSC1", 1 }, { "OSC2", 1 }, { "FILTERCUTOFF", 100 }, { "FILTERADSRDEPTH", 10000 }, { "FILTERATTACK", 0.3f } }) },
            { "Lfo",              make ({ { "OSC1", 1 }, { "FILTERCUTOFF", 1000 }, { "FILTERADSRDEPTH", 0 }, { "LFO1FREQ", 5 }, { "LFO1DEPTH", 2000 } }) },
            { "Fm",               make ({ { "OSC1FMFREQ", 200 }, { "OSC1FMDEPTH", 12 }, { "OSC2FMFREQ", 3 }, { "OSC2FMDEPTH", 1 } }) },
            { "Transposed",       make ({ { "OSC1PITCH", -48 }, { "OSC2PITCH", 48 } }, { Note { 60, 1.0f, 0.0, 0.6 } }) },
            { "Quiet",            make ({ { "OSC1GAIN", -40 }, { "OSC2GAIN", -40 } }) },
            { "Reverb",           make ({ { "REVERBWET", 0.5f }, { "REVERBSIZE", 0.8f } }) },
            { "Chord",            make ({}, chord) },
            { "LowestNote",       make ({ { "OSC1", 1 }, { "OSC2", 1 } }, { Note { 0, 1.0f, 0.0, 0.6 } }) },
            { "HighestNote",      make ({}, { Note { 127, 1.0f, 0.0, 0.6 } }) },
            { "Rate44100",        make ({ { "OSC1", 1 } }, { Note { 69, 1.0f, 0.0, 0.6 } }, 44100.0) },
            { "Rate96000",        make ({ { "OSC1", 1 } }, { Note { 69, 1.0f, 0.0, 0.6 } }, 96000.0) },
            { "Block1",           make ({ { "OSC1", 1 } }, chord, 48000.0, 1) },
            { "Block64",          make ({ { "OSC1", 1 } }, chord, 48000.0, 64) },
            { "Block333",         make ({ { "OSC1", 1 } }, chord, 48000.0, 333) },
            { "Block4096",        make ({ { "OSC1", 1 } }, chord, 48000.0, 4096) },
        };
    }

    // Random value for every automatable parameter, in its own units, as the AI would send it
    juce::var randomPatch (juce::Random& rng)
    {
        TapSynthAudioProcessor processor;
        auto* patch = new juce::DynamicObject();

        for (auto* p : processor.getParameters())
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (p))
                patch->setProperty (ranged->getParameterID(), ranged->convertFrom0to1 (rng.nextFloat()));

        return juce::var (patch);
    }

    // Every parameter at the same normalised position (0 = all minimum, 1 = all maximum)
    juce::var cornerPatch (float normalised)
    {
        TapSynthAudioProcessor processor;
        auto* patch = new juce::DynamicObject();

        for (auto* p : processor.getParameters())
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (p))
                patch->setProperty (ranged->getParameterID(), ranged->convertFrom0to1 (normalised));

        return juce::var (patch);
    }

    RenderSpec patchSpec (juce::var patch)
    {
        RenderSpec spec;
        spec.lengthSeconds = 1.5;
        spec.jsonPatch = std::move (patch);
        spec.notes = { Note { 48, 1.0f, 0.0, 0.6 }, Note { 60, 1.0f, 0.1, 0.6 }, Note { 67, 1.0f, 0.2, 0.6 } };
        return spec;
    }

    void checkPatch (const RenderSpec& spec, const std::string& label)
    {
        const auto out = render (spec);
        ASSERT_TRUE (allFinite (out)) << label << ": output has NaN or infinity";

        for (int ch = 0; ch < 2; ++ch)
        {
            const auto channel = label + "/ch" + std::to_string (ch);
            EXPECT_TRUE (within (tol::sanityPeakRandom, peak (segment (out, ch)), channel));
            EXPECT_TRUE (within (tol::sanityDcRandom, std::abs (dcOffset (segment (out, ch, spec.sampleRate, 0.3, 0.5))), channel));
        }
    }
}

class Sanity : public ::testing::TestWithParam<Config> {};

TEST_P (Sanity, OutputIsFiniteBoundedAudibleAndCentred)
{
    const auto& [name, spec] = GetParam();
    const auto out = render (spec);

    ASSERT_TRUE (allFinite (out)) << "output has NaN or infinity";

    for (int ch = 0; ch < 2; ++ch)
    {
        const auto label = name + "/ch" + std::to_string (ch);

        // The whole render, the note and its release
        EXPECT_TRUE (within (tol::sanityPeak, peak (segment (out, ch)), label));

        // While the note is held (after the shortest attack)
        const auto held = segment (out, ch, spec.sampleRate, 0.2, 0.35);
        EXPECT_TRUE (within (tol::sanityAudible, peak (held), label));
        EXPECT_TRUE (within (tol::sanityDc, std::abs (dcOffset (held)), label));
    }
}

INSTANTIATE_TEST_SUITE_P (Configs, Sanity, ::testing::ValuesIn (configs()),
                          [] (const auto& info) { return info.param.name; });

TEST (SanityRandom, RandomPatchesThroughJson)
{
    juce::Random rng (randomPatchSeed);

    for (int i = 0; i < randomPatchCount; ++i)
    {
        SCOPED_TRACE ("patch " + std::to_string (i));
        checkPatch (patchSpec (randomPatch (rng)), "patch" + std::to_string (i));
    }
}

TEST (SanityRandom, AllParametersAtMinimum)
{
    checkPatch (patchSpec (cornerPatch (0.0f)), "allMin");
}

TEST (SanityRandom, AllParametersAtMaximum)
{
    checkPatch (patchSpec (cornerPatch (1.0f)), "allMax");
}
