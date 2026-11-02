#pragma once

#include "Tolerances.h"

#include <gtest/gtest.h>
#include <string>

namespace tapsynth::test
{
    // Records `measured` in the test's XML report as "<tolerance name>|<label>" and checks it against the
    // tolerance's default value. NaN never passes. Use as EXPECT_TRUE (within (tol::pitch, cents, "osc1/69")).
    ::testing::AssertionResult within (const Tolerance& t, double measured, const std::string& label);

    // MIDI note number -> Hz (equal temperament, A4 = 440 Hz)
    double noteHz (double midiNote);

    // Signed distance between two frequencies in cents
    double cents (double measuredHz, double expectedHz);
}
