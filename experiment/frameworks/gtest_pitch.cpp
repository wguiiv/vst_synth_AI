// GoogleTest port: value-parameterized test (TEST_P + INSTANTIATE_TEST_SUITE_P), SCOPED_TRACE for context,
// EXPECT_NEAR for the absolute tolerance in cents, RecordProperty to keep the measured value in the XML report.

#include "PitchCommon.h"

#include <gtest/gtest.h>
#include <string>
#include <tuple>

using Case = std::tuple<double, int>;   // sample rate, wave

class Pitch : public ::testing::TestWithParam<Case> {};

TEST_P (Pitch, EveryNoteHasTheRightFundamental)
{
    const auto [sampleRate, wave] = GetParam();

    for (const auto note : pitch::notes)
    {
        SCOPED_TRACE ("note " + std::to_string (note));
        const auto c = pitch::cents (pitch::measuredHz (sampleRate, wave, note), pitch::noteHz (note));
        RecordProperty ("spectral.pitch|note" + std::to_string (note), std::to_string (c));
        EXPECT_NEAR (c, 0.0, pitch::toleranceCents);
    }
}

INSTANTIATE_TEST_SUITE_P (RatesAndWaves, Pitch,
                          ::testing::Combine (::testing::ValuesIn (pitch::sampleRates), ::testing::ValuesIn (pitch::waves)));

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juceInit;   // the processor's parameters need JUCE's message manager
    ::testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS();
}
