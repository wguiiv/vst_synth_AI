// Checks the measurement functions themselves on synthetic signals with a known answer, so a failing
// audio test points at the plugin and not at the oracle. These tests do not use the processor.

#include "../common/Analysis.h"
#include <gtest/gtest.h>

using namespace tapsynth::test;

namespace
{
    juce::AudioBuffer<float> sine (double freq, double sampleRate, double seconds, float amplitude = 1.0f, float dc = 0.0f)
    {
        juce::AudioBuffer<float> buffer (1, (int) std::lround (seconds * sampleRate));

        for (int i = 0; i < buffer.getNumSamples(); ++i)
            buffer.setSample (0, i, dc + amplitude * (float) std::sin (juce::MathConstants<double>::twoPi * freq * i / sampleRate));

        return buffer;
    }
}

TEST (AnalysisSelfTest, RmsPeakAndDcOfSine)
{
    const auto buffer = sine (1000.0, 48000.0, 1.0, 0.5f, 0.1f);
    const auto s = segment (buffer, 0);

    EXPECT_NEAR (peak (s), 0.6f, 1.0e-3f);
    EXPECT_NEAR (dcOffset (s), 0.1f, 1.0e-4f);
    EXPECT_NEAR (rms (s), std::sqrt (0.5f * 0.5f / 2.0f + 0.1f * 0.1f), 1.0e-4f);
}

TEST (AnalysisSelfTest, FundamentalOfSineAcrossRangeAndRates)
{
    for (const auto sampleRate : { 44100.0, 48000.0, 96000.0 })
        for (const auto freq : { 27.5, 110.0, 440.0, 1234.5, 4186.0 })
        {
            const auto buffer = sine (freq, sampleRate, 0.5);
            EXPECT_NEAR (fundamentalHz (segment (buffer, 0), sampleRate), freq, 0.05)
                << "at " << sampleRate << " Hz";
        }
}

TEST (AnalysisSelfTest, BandEnergyFindsTheTone)
{
    const auto buffer = sine (1000.0, 48000.0, 0.5);
    const auto s = segment (buffer, 0);

    EXPECT_GT (bandEnergyDb (s, 48000.0, 900.0, 1100.0) - bandEnergyDb (s, 48000.0, 4000.0, 20000.0), 60.0);
}

TEST (AnalysisSelfTest, InharmonicEnergyOfToneMixture)
{
    // Window leakage puts the floor of a pure tone near -57 dB, 25 dB below the aliasing the plugin produces
    auto buffer = sine (440.0, 48000.0, 0.5);
    EXPECT_LT (inharmonicEnergyDb (segment (buffer, 0), 48000.0, 440.0), -50.0);

    // A tone at 1000 Hz (not a multiple of 440) at -20 dB relative to the 440 Hz tone
    const auto other = sine (1000.0, 48000.0, 0.5, 0.1f);
    buffer.addFrom (0, 0, other, 0, 0, buffer.getNumSamples());
    EXPECT_NEAR (inharmonicEnergyDb (segment (buffer, 0), 48000.0, 440.0), -20.0, 0.2);
}

TEST (AnalysisSelfTest, EnvelopeFollowsAmplitude)
{
    auto buffer = sine (1000.0, 48000.0, 1.0);
    buffer.applyGain (0, 24000, 24000, 0.0f);   // silent second half

    const auto env = envelope (segment (buffer, 0), 48000.0, 0.1);
    ASSERT_EQ (env.size(), 10u);
    EXPECT_NEAR (env.front(), std::sqrt (0.5f), 1.0e-3f);
    EXPECT_FLOAT_EQ (env.back(), 0.0f);
}

TEST (AnalysisSelfTest, MaxAbsDiffDetectsShapeAndNaN)
{
    const auto a = sine (440.0, 48000.0, 0.1);
    auto b = a;
    EXPECT_FLOAT_EQ (maxAbsDiff (a, b), 0.0f);

    b.setSample (0, 100, b.getSample (0, 100) + 0.25f);
    EXPECT_NEAR (maxAbsDiff (a, b), 0.25f, 1.0e-6f);

    b.setSample (0, 200, std::numeric_limits<float>::quiet_NaN());
    EXPECT_TRUE (std::isinf (maxAbsDiff (a, b)));
    EXPECT_FALSE (allFinite (b));

    EXPECT_TRUE (std::isinf (maxAbsDiff (a, sine (440.0, 48000.0, 0.2))));
}

TEST (AnalysisSelfTest, WavRoundTripIsBitExact)
{
    const auto a = sine (440.0, 48000.0, 0.1, 0.8f);
    const auto file = juce::File::createTempFile (".wav");

    ASSERT_TRUE (saveWav (file, a, 48000.0));

    juce::AudioBuffer<float> b;
    double sampleRate = 0.0;
    ASSERT_TRUE (loadWav (file, b, sampleRate));
    file.deleteFile();

    EXPECT_EQ (sampleRate, 48000.0);
    EXPECT_EQ (maxAbsDiff (a, b), 0.0f);
}
