#pragma once

#include <vector>

// Every tolerance and tolerance grid of the study, fixed before the experiment (plan, rule 4).
// Values only change with explicit approval. Each check records its measured value in the XML report,
// so the experiment can judge one run against every point of a grid.
namespace tapsynth::test
{
    enum class Bound
    {
        atMost,    // passes when measured <= value
        atLeast    // passes when measured >= value
    };

    struct Tolerance
    {
        const char* name;
        const char* unit;
        Bound bound;
        double value;                 // default used by the assertions
        std::vector<double> grid;     // values swept for QP3, strictest first
    };

    namespace tol
    {
        // Sanity
        inline const Tolerance sanityPeak       { "sanity.peak",        "linear", Bound::atMost,  1.0,    { 1.0 } };
        inline const Tolerance sanityPeakRandom { "sanity.peakRandom",  "linear", Bound::atMost,  4.0,    { 4.0 } };
        inline const Tolerance sanityAudible    { "sanity.audible",     "linear", Bound::atLeast, 1.0e-3, { 1.0e-3 } };
        inline const Tolerance sanityDc         { "sanity.dc",          "linear", Bound::atMost,  0.01,   { 0.01 } };
        inline const Tolerance sanityDcRandom   { "sanity.dcRandom",    "linear", Bound::atMost,  0.05,   { 0.05 } };

        // Spectral
        inline const Tolerance pitch            { "spectral.pitch",     "cents",  Bound::atMost,  5.0,    { 1.0, 2.0, 5.0, 10.0, 25.0, 50.0 } };
        inline const Tolerance stopband         { "spectral.stopband",  "dB",     Bound::atLeast, 18.0,   { 24.0, 18.0, 12.0, 6.0 } };
        inline const Tolerance passband         { "spectral.passband",  "dB",     Bound::atMost,  1.0,    { 0.25, 0.5, 1.0, 3.0 } };
        inline const Tolerance aliasing         { "spectral.aliasing",  "dB",     Bound::atMost,  3.0,    { 1.0, 3.0, 6.0, 10.0 } };

        // Temporal
        inline const Tolerance envelopeTime     { "temporal.time",      "s",      Bound::atMost,  0.010,  { 0.002, 0.005, 0.010, 0.020, 0.050 } };
        inline const Tolerance releaseTime      { "temporal.release",   "s",      Bound::atMost,  0.020,  { 0.005, 0.010, 0.020, 0.050, 0.100 } };
        inline const Tolerance sustainLevel     { "temporal.sustain",   "dB",     Bound::atMost,  0.5,    { 0.1, 0.25, 0.5, 1.0, 3.0 } };

        // Golden master
        inline const Tolerance golden           { "golden.maxAbsDiff",  "linear", Bound::atMost,  1.0e-4, { 0.0, 1.0e-6, 1.0e-5, 1.0e-4, 1.0e-3, 1.0e-2 } };

        inline const std::vector<const Tolerance*> all { &sanityPeak, &sanityPeakRandom, &sanityAudible, &sanityDc, &sanityDcRandom,
                                                         &pitch, &stopband, &passband, &aliasing,
                                                         &envelopeTime, &releaseTime, &sustainLevel, &golden };
    }

    // Random-patch variant of the sanity strategy
    inline constexpr int randomPatchCount = 200;
    inline constexpr int randomPatchSeed  = 1234;
}
