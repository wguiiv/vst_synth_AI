// Phase 0 feasibility check: one note rendered through the real processor produces finite, audible output.
// The study's test suites (sanity, spectral, temporal, golden) live in their own folders.

#include "../common/Render.h"
#include <gtest/gtest.h>

using namespace tapsynth::test;

TEST (Smoke, RendersOneNote)
{
    RenderSpec spec;
    spec.lengthSeconds = 1.0;
    spec.notes = { Note { 69, 1.0f, 0.0, 0.5 } };

    const auto out = render (spec);

    ASSERT_EQ (out.getNumChannels(), 2);
    ASSERT_EQ (out.getNumSamples(), 48000);

    for (int ch = 0; ch < out.getNumChannels(); ++ch)
    {
        const auto* data = out.getReadPointer (ch);

        for (int i = 0; i < out.getNumSamples(); ++i)
            ASSERT_TRUE (std::isfinite (data[i])) << "channel " << ch << ", sample " << i;

        EXPECT_GT (out.getMagnitude (ch, 0, out.getNumSamples()), 0.01f) << "channel " << ch << " is silent";
    }
}
