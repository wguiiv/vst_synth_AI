// doctest port: no value-parameterized tests, so the sample rates and waves are loops; CAPTURE adds the current
// values to any failure message. doctest's Approx accepts |a - b| < epsilon * (scale + max(|a|, |b|)), a margin
// that grows with the measured value, so it cannot hold an absolute tolerance around 0: the error in cents is
// checked with a plain comparison (see COMPARACAO.md and the demonstration at the end).

#include "PitchCommon.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <cmath>

TEST_CASE ("Every note has the right fundamental")
{
    for (const auto sampleRate : pitch::sampleRates)
        for (const auto wave : pitch::waves)
            for (const auto note : pitch::notes)
            {
                CAPTURE (sampleRate);
                CAPTURE (wave);
                CAPTURE (note);
                const auto c = pitch::cents (pitch::measuredHz (sampleRate, wave, note), pitch::noteHz (note));
                CHECK (std::abs (c) <= pitch::toleranceCents);
            }
}

// Not part of the port: shows why Approx is not used above. With target 0 and epsilon 5 the margin is
// 5 * (1 + |error|), larger than any error, so this "5 cents" check passes even for an error of 1000 cents.
TEST_CASE ("Demonstration: Approx(0).epsilon(5) accepts any error" * doctest::test_suite ("demonstration"))
{
    CHECK (1000.0 == doctest::Approx (0.0).epsilon (5.0));
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juceInit;   // the processor's parameters need JUCE's message manager
    doctest::Context context (argc, argv);
    return context.run();
}
