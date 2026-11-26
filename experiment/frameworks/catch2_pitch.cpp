// Catch2 v3 port: GENERATE turns one test case into one run per value (the cross product of the generators),
// CAPTURE adds the current values to any failure message, and the WithinAbs matcher holds the absolute
// tolerance in cents.

#include "PitchCommon.h"

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <iterator>

using Catch::Matchers::WithinAbs;

TEST_CASE ("Every note has the right fundamental", "[spectral][pitch]")
{
    const auto sampleRate = GENERATE (from_range (std::begin (pitch::sampleRates), std::end (pitch::sampleRates)));
    const auto wave = GENERATE (from_range (std::begin (pitch::waves), std::end (pitch::waves)));

    for (const auto note : pitch::notes)
    {
        CAPTURE (sampleRate, wave, note);
        const auto c = pitch::cents (pitch::measuredHz (sampleRate, wave, note), pitch::noteHz (note));
        CHECK_THAT (c, WithinAbs (0.0, pitch::toleranceCents));
    }
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juceInit;   // the processor's parameters need JUCE's message manager
    return Catch::Session().run (argc, argv);
}
