// Temporal strategy: "does the sound evolve correctly over time?" Amplitude envelope (attack, decay, sustain,
// release), independence of simultaneous voices, and the filter modulated over time by its envelope and the LFO.

#include "../common/Analysis.h"
#include "../common/Check.h"
#include "../common/Render.h"

#include <gtest/gtest.h>

using namespace tapsynth::test;

namespace
{
    constexpr double hop = 0.001;   // envelope resolution, seconds

    juce::AudioBuffer<float> renderNote (std::map<juce::String, float> params, double noteSeconds, double lengthSeconds,
                                         double sampleRate = 48000.0, int note = 69)
    {
        RenderSpec spec;
        spec.sampleRate = sampleRate;
        spec.lengthSeconds = lengthSeconds;
        spec.params = std::move (params);
        spec.notes = { Note { note, 1.0f, 0.0, noteSeconds } };
        return render (spec);
    }

    // Amplitude envelope: RMS over a window of two periods of the note (so the waveform's phase does not make it
    // ripple), every `hop` seconds. Element i is the level centred at time (i + 0.5) * hop from the segment start.
    std::vector<float> amplitude (Segment s, double sampleRate, int note = 69)
    {
        const auto length = (int) std::lround (2.0 * sampleRate / noteHz (note));
        const auto step = hop * sampleRate;
        std::vector<float> result;

        for (int i = 0;; ++i)
        {
            const auto centre = (int) std::lround ((i + 0.5) * step);
            const auto start = centre - length / 2;

            if (start + length > s.numSamples)
                break;

            result.push_back (start < 0 ? 0.0f : rms ({ s.data + start, length }));
        }

        return result;
    }

    double timeOf (size_t index)
    {
        return ((double) index + 0.5) * hop;
    }

    // Time at which the envelope first reaches `fraction` of its maximum
    double timeToReach (const std::vector<float>& env, float fraction)
    {
        const auto max = *std::max_element (env.begin(), env.end());

        for (size_t i = 0; i < env.size(); ++i)
            if (env[i] >= fraction * max)
                return timeOf (i);

        return std::numeric_limits<double>::infinity();
    }

    // Time of the last point whose level is above `db` relative to `reference`; 0 if none is
    double lastTimeAbove (const std::vector<float>& env, float reference, float db)
    {
        for (size_t i = env.size(); i-- > 0;)
            if (gainToDb (env[i] / reference) > db)
                return timeOf (i);

        return 0.0;
    }
}

//==============================================================================
class TemporalAttack : public ::testing::TestWithParam<double> {};

TEST_P (TemporalAttack, ReachesPeakInConfiguredTime)
{
    // The amplitude ramps linearly over the attack, so it crosses 90 % of the peak at 0.9 x attack
    const auto sampleRate = GetParam();

    for (const auto attack : { 0.1f, 0.3f, 0.6f, 1.0f })
    {
        const auto out = renderNote ({ { "ATTACK", attack }, { "SUSTAIN", 1.0f } }, attack + 0.3, attack + 0.3, sampleRate);
        const auto env = amplitude (segment (out, 0), sampleRate);

        EXPECT_TRUE (within (tol::envelopeTime, std::abs (timeToReach (env, 0.9f) - 0.9 * attack),
                             "attack" + juce::String (attack, 2).toStdString()));
    }
}

TEST_P (TemporalAttack, DecaysToSustainInConfiguredTime)
{
    // After the attack the level falls linearly from 1 to the sustain level over the decay time
    const auto sampleRate = GetParam();
    constexpr float attack = 0.1f, sustain = 0.5f;

    for (const auto decay : { 0.2f, 0.5f, 1.0f })
    {
        const auto out = renderNote ({ { "ATTACK", attack }, { "DECAY", decay }, { "SUSTAIN", sustain } },
                                     attack + decay + 0.3, attack + decay + 0.3, sampleRate);
        const auto env = amplitude (segment (out, 0), sampleRate);
        const auto max = *std::max_element (env.begin(), env.end());

        // First time after the peak at which the level is 10 % of the way from sustain to the peak
        const auto target = sustain + 0.1f * (1.0f - sustain);
        auto crossing = std::numeric_limits<double>::infinity();

        for (auto i = (size_t) (attack / hop); i < env.size(); ++i)
            if (env[i] <= target * max)
            {
                crossing = timeOf (i);
                break;
            }

        EXPECT_TRUE (within (tol::envelopeTime, std::abs (crossing - (attack + 0.9 * decay)),
                             "decay" + juce::String (decay, 2).toStdString()));
    }
}

TEST_P (TemporalAttack, HoldsSustainLevel)
{
    const auto sampleRate = GetParam();
    constexpr float attack = 0.1f, decay = 0.1f;

    for (const auto sustain : { 0.25f, 0.5f, 0.8f })
    {
        const auto out = renderNote ({ { "ATTACK", attack }, { "DECAY", decay }, { "SUSTAIN", sustain } }, 1.0, 1.0, sampleRate);
        const auto env = amplitude (segment (out, 0), sampleRate);
        const auto max = *std::max_element (env.begin(), env.end());
        const auto steady = rms (segment (out, 0, sampleRate, 0.5, 0.4));

        EXPECT_TRUE (within (tol::sustainLevel, std::abs (gainToDb (steady / max) - gainToDb (sustain)),
                             "sustain" + juce::String (sustain, 2).toStdString()));
    }
}

INSTANTIATE_TEST_SUITE_P (Rates, TemporalAttack, ::testing::Values (44100.0, 48000.0, 96000.0),
                          [] (const auto& info) { return std::to_string ((int) info.param); });

//==============================================================================
class TemporalRelease : public ::testing::TestWithParam<double> {};

TEST_P (TemporalRelease, FadesToSilenceInConfiguredTime)
{
    // After note-off the level falls linearly to zero over the release time, so it stays below -60 dB
    // (relative to the held level) from about the release time on, and never comes back.
    const auto sampleRate = GetParam();
    constexpr double noteOff = 0.5;

    for (const auto release : { 0.1f, 0.4f, 1.0f, 3.0f })
    {
        const auto out = renderNote ({ { "RELEASE", release } }, noteOff, noteOff + release + 1.0, sampleRate);
        const auto heldLevel = rms (segment (out, 0, sampleRate, noteOff - 0.1, 0.1));
        const auto tail = amplitude (segment (out, 0, sampleRate, noteOff, release + 1.0), sampleRate);

        EXPECT_TRUE (within (tol::releaseTime, std::abs (lastTimeAbove (tail, heldLevel, -60.0f) - release),
                             "release" + juce::String (release, 2).toStdString()));
    }
}

INSTANTIATE_TEST_SUITE_P (Rates, TemporalRelease, ::testing::Values (44100.0, 48000.0, 96000.0),
                          [] (const auto& info) { return std::to_string ((int) info.param); });

//==============================================================================
TEST (TemporalVoices, ReleasingOneNoteLeavesTheOtherUntouched)
{
    // A3 (220 Hz) is held throughout; E4 (329.6 Hz) is released early. The A3 band must keep its level.
    RenderSpec spec;
    spec.lengthSeconds = 1.5;
    spec.params = { { "RELEASE", 0.1f } };
    spec.notes = { Note { 57, 1.0f, 0.0, 1.5 }, Note { 64, 1.0f, 0.0, 0.5 } };
    const auto out = render (spec);

    const auto a3 = [&] (double start) { return bandEnergyDb (segment (out, 0, 48000.0, start, 0.3), 48000.0, 210.0, 230.0); };

    EXPECT_TRUE (within (tol::sustainLevel, std::abs (a3 (1.0) - a3 (0.15)), "heldNote"));
}

//==============================================================================
// The filter cutoff moves over time with the filter envelope and the LFO. A saw at 110 Hz goes through a
// low-pass whose base cutoff is 200 Hz, and the band 3-6 kHz is watched: closed, it sits at least two octaves
// above the cutoff (stop band); opened by the modulation, it is in the pass band.
namespace
{
    std::vector<double> brightness (const juce::AudioBuffer<float>& out, double hop, double length)
    {
        std::vector<double> result;
        for (double t = 0.0; t + hop <= length; t += hop)
            result.push_back (bandEnergyDb (segment (out, 0, 48000.0, t, hop), 48000.0, 3000.0, 6000.0));

        return result;
    }
}

TEST (TemporalFilter, EnvelopeOpensTheFilterOverItsAttack)
{
    constexpr float filterAttack = 0.5f;
    const auto out = renderNote ({ { "OSC1", 1 }, { "OSC2", 1 }, { "FILTERCUTOFF", 200 }, { "FILTERRESONANCE", 0.71f },
                                   { "FILTERADSRDEPTH", 8000 }, { "FILTERATTACK", filterAttack }, { "FILTERSUSTAIN", 1.0f } },
                                 1.2, 1.2, 48000.0, 45);

    // Start of the note (cutoff 200-520 Hz) against after the filter attack (cutoff 8.2 kHz). Each band is taken
    // relative to the fundamental's band, so the amplitude attack cancels out.
    const auto early = bandEnergyDb (segment (out, 0, 48000.0, 0.0, 0.02), 48000.0, 3000.0, 6000.0)
                     - bandEnergyDb (segment (out, 0, 48000.0, 0.0, 0.02), 48000.0, 50.0, 150.0);
    const auto late  = bandEnergyDb (segment (out, 0, 48000.0, 0.7, 0.02), 48000.0, 3000.0, 6000.0)
                     - bandEnergyDb (segment (out, 0, 48000.0, 0.7, 0.02), 48000.0, 50.0, 150.0);

    EXPECT_TRUE (within (tol::stopband, late - early, "opening"));
}

TEST (TemporalFilter, LfoSweepsTheCutoffAtItsRate)
{
    constexpr float lfoHz = 2.0f;
    constexpr double windowHop = 0.01;
    const auto out = renderNote ({ { "OSC1", 1 }, { "OSC2", 1 }, { "FILTERCUTOFF", 200 }, { "FILTERRESONANCE", 0.71f },
                                   { "FILTERADSRDEPTH", 0 }, { "LFO1FREQ", lfoHz }, { "LFO1DEPTH", 8000 } },
                                 2.0, 2.0, 48000.0, 45);

    // Skip the amplitude attack, then look at the brightness curve over three LFO periods
    auto curve = brightness (out, windowHop, 2.0);
    curve.erase (curve.begin(), curve.begin() + 20);

    const auto [minIt, maxIt] = std::minmax_element (curve.begin(), curve.end());
    EXPECT_TRUE (within (tol::stopband, *maxIt - *minIt, "depth"));

    // Period: distance between the two brightest windows that are more than half a period apart
    std::vector<size_t> peaks;
    for (size_t i = 1; i + 1 < curve.size(); ++i)
        if (curve[i] >= curve[i - 1] && curve[i] > curve[i + 1] && curve[i] > *maxIt - 3.0
            && (peaks.empty() || (double) (i - peaks.back()) * windowHop > 0.5 / lfoHz))
            peaks.push_back (i);

    ASSERT_GE (peaks.size(), 2u) << "no periodic sweep found";
    const auto period = (double) (peaks[1] - peaks[0]) * windowHop;
    EXPECT_TRUE (within (tol::envelopeTime, std::abs (period - 1.0 / lfoHz), "period"));
}
