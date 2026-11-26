// Golden master strategy: "did the sound change?" Each configuration is rendered and compared sample by sample
// with a reference WAV recorded once from the unmodified code (tests/references/). Rendering is deterministic,
// so no seed is needed.
//
// To record the references (only on the defect-free code, before the tests-frozen tag):
//     TAPSYNTH_RECORD_GOLDEN=1 ./tapSynth_tests --gtest_filter='Golden*'

#include "../common/Analysis.h"
#include "../common/Check.h"
#include "../common/Render.h"

#include <gtest/gtest.h>

using namespace tapsynth::test;

namespace
{
    struct Config
    {
        std::string name;
        std::map<juce::String, float> params;
        std::vector<Note> notes { Note { 57, 1.0f, 0.0, 0.8 } };
    };

    std::vector<Config> configs()
    {
        const std::vector<Note> chord { Note { 48, 1.0f, 0.0, 0.8 }, Note { 55, 1.0f, 0.05, 0.8 }, Note { 60, 1.0f, 0.1, 0.8 },
                                        Note { 64, 1.0f, 0.15, 0.8 }, Note { 67, 1.0f, 0.2, 0.8 } };

        return {
            { "Default",   {} },
            { "Saw",       { { "OSC1", 1 }, { "OSC2", 1 }, { "OSC2PITCH", 7 } } },
            { "Square",    { { "OSC1", 2 }, { "OSC2", 2 }, { "OSC2PITCH", -12 }, { "OSC2GAIN", -6 } } },
            { "LowPass",   { { "OSC1", 1 }, { "OSC2", 2 }, { "FILTERCUTOFF", 400 }, { "FILTERRESONANCE", 1.5f }, { "FILTERADSRDEPTH", 3000 },
                             { "FILTERATTACK", 0.2f }, { "FILTERDECAY", 0.3f }, { "FILTERSUSTAIN", 0.3f } } },
            { "BandPass",  { { "OSC1", 1 }, { "OSC2", 1 }, { "FILTERTYPE", 1 }, { "FILTERCUTOFF", 1200 }, { "FILTERRESONANCE", 2.0f }, { "FILTERADSRDEPTH", 0 } } },
            { "HighPass",  { { "OSC1", 1 }, { "OSC2", 1 }, { "FILTERTYPE", 2 }, { "FILTERCUTOFF", 800 }, { "FILTERADSRDEPTH", 0 } } },
            { "Fm",        { { "OSC1FMFREQ", 220 }, { "OSC1FMDEPTH", 6 }, { "OSC2FMFREQ", 5 }, { "OSC2FMDEPTH", 0.5f } } },
            { "Lfo",       { { "OSC1", 1 }, { "OSC2", 1 }, { "FILTERCUTOFF", 600 }, { "FILTERADSRDEPTH", 0 }, { "LFO1FREQ", 6 }, { "LFO1DEPTH", 3000 } } },
            { "Envelope",  { { "ATTACK", 0.3f }, { "DECAY", 0.2f }, { "SUSTAIN", 0.4f }, { "RELEASE", 0.3f } } },
            { "Reverb",    { { "REVERBSIZE", 0.8f }, { "REVERBWIDTH", 0.6f }, { "REVERBDAMPING", 0.3f }, { "REVERBDRY", 0.7f }, { "REVERBWET", 0.5f } } },
            { "Chord",     { { "OSC1", 1 }, { "FILTERCUTOFF", 2000 }, { "FILTERADSRDEPTH", 0 } }, chord },
            // Extremes: every continuous parameter at an end of its range
            { "ExtremeMax", { { "OSC1", 2 }, { "OSC2", 1 }, { "OSC1GAIN", 0.2f }, { "OSC2GAIN", 0.2f }, { "OSC1PITCH", 24 }, { "OSC2PITCH", -24 },
                              { "OSC1FMFREQ", 1000 }, { "OSC1FMDEPTH", 100 }, { "LFO1FREQ", 20 }, { "LFO1DEPTH", 10000 },
                              { "FILTERCUTOFF", 20000 }, { "FILTERRESONANCE", 2.0f }, { "ATTACK", 0.1f }, { "DECAY", 1.0f }, { "SUSTAIN", 1.0f },
                              { "RELEASE", 0.1f }, { "FILTERADSRDEPTH", 10000 }, { "REVERBSIZE", 1.0f }, { "REVERBWET", 1.0f } } },
            { "ExtremeMin", { { "OSC1GAIN", -40 }, { "OSC2GAIN", -40 }, { "OSC1PITCH", -48 }, { "OSC2PITCH", -48 }, { "FILTERCUTOFF", 20 },
                              { "FILTERRESONANCE", 0.1f }, { "FILTERADSRDEPTH", 0 }, { "REVERBDRY", 0.0f }, { "REVERBWET", 0.0f } },
                            { Note { 108, 1.0f, 0.0, 0.8 } } },
        };
    }

    juce::File referenceFile (const std::string& name)
    {
        return juce::File (TAPSYNTH_REFERENCES_DIR).getChildFile (juce::String (name) + ".wav");
    }

    bool recording()
    {
        return juce::SystemStats::getEnvironmentVariable ("TAPSYNTH_RECORD_GOLDEN", {}) == "1";
    }
}

class Golden : public ::testing::TestWithParam<Config> {};

TEST_P (Golden, MatchesReference)
{
    const auto& config = GetParam();

    RenderSpec spec;
    spec.sampleRate = 48000.0;
    spec.blockSize = 512;
    spec.lengthSeconds = 1.5;
    spec.params = config.params;
    spec.notes = config.notes;

    const auto out = render (spec);
    const auto file = referenceFile (config.name);

    if (recording())
    {
        ASSERT_TRUE (saveWav (file, out, spec.sampleRate)) << "could not write " << file.getFullPathName();
        GTEST_SKIP() << "recorded " << file.getFullPathName();
    }

    juce::AudioBuffer<float> reference;
    double referenceRate = 0.0;
    ASSERT_TRUE (loadWav (file, reference, referenceRate)) << "missing reference " << file.getFullPathName();
    ASSERT_EQ (referenceRate, spec.sampleRate);

    EXPECT_TRUE (within (tol::golden, maxAbsDiff (out, reference), config.name));
}

INSTANTIATE_TEST_SUITE_P (Configs, Golden, ::testing::ValuesIn (configs()),
                          [] (const auto& info) { return info.param.name; });
