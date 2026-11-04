#pragma once

// Framework-neutral part of the "every note has the right fundamental" test (tests/spectral/SpectralTest.cpp),
// shared by the GoogleTest, Catch2 and doctest ports of the framework comparison. Rendering and measuring reuse
// tests/common/Render.cpp and Analysis.cpp unchanged; noteHz and cents are repeated here because the frozen
// tests keep them in Check.cpp, which depends on GoogleTest.

#include "../../tests/common/Analysis.h"
#include "../../tests/common/Render.h"

#include <cmath>
#include <limits>

namespace pitch
{
    inline constexpr double sampleRates[] { 44100.0, 48000.0, 96000.0 };
    inline constexpr int waves[] { 0, 1, 2 };                        // sine, saw, square
    inline constexpr int notes[] { 21, 33, 45, 57, 69, 81, 93, 105, 108 };
    inline constexpr double toleranceCents = 5.0;                    // tol::pitch default (tests/common/Tolerances.h)

    inline double noteHz (double midiNote)
    {
        return 440.0 * std::pow (2.0, (midiNote - 69.0) / 12.0);
    }

    inline double cents (double measuredHz, double expectedHz)
    {
        if (measuredHz <= 0.0 || expectedHz <= 0.0)
            return std::numeric_limits<double>::infinity();

        return 1200.0 * std::log2 (measuredHz / expectedHz);
    }

    // Renders one held note with both oscillators on `wave` and returns the fundamental of its steady part
    inline double measuredHz (double sampleRate, int wave, int note)
    {
        using namespace tapsynth::test;
        RenderSpec spec;
        spec.sampleRate = sampleRate;
        spec.lengthSeconds = 1.0;
        spec.params = { { "OSC1", (float) wave }, { "OSC2", (float) wave } };
        spec.notes = { Note { note, 1.0f, 0.0, 1.0 } };
        const auto out = render (spec);
        return fundamentalHz (segment (out, 0, sampleRate, 0.3, 0.6), sampleRate, 15.0, 0.45 * sampleRate);
    }
}
