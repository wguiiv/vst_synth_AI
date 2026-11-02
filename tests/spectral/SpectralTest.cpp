// Spectral strategy: "are the frequencies right?" Fundamental of every note at 44.1, 48 and 96 kHz, oscillator
// transposition, FM sidebands, filter stop band and pass band, and a characterization test of aliasing.

#include "../common/Analysis.h"
#include "../common/Check.h"
#include "../common/Render.h"

#include <gtest/gtest.h>

using namespace tapsynth::test;

namespace
{
    constexpr double sampleRates[] { 44100.0, 48000.0, 96000.0 };
    const char* waveNames[] { "Sine", "Saw", "Square" };

    std::string rateName (double sampleRate)
    {
        return std::to_string ((int) sampleRate);
    }

    // Steady part of a held note: after the attack, before the note-off
    Segment held (const juce::AudioBuffer<float>& out, double sampleRate)
    {
        return segment (out, 0, sampleRate, 0.3, 0.6);
    }

    juce::AudioBuffer<float> renderNote (std::map<juce::String, float> params, int note, double sampleRate = 48000.0)
    {
        RenderSpec spec;
        spec.sampleRate = sampleRate;
        spec.lengthSeconds = 1.0;
        spec.params = std::move (params);
        spec.notes = { Note { note, 1.0f, 0.0, 1.0 } };
        return render (spec);
    }

    double measuredCents (const juce::AudioBuffer<float>& out, double sampleRate, double expectedHz)
    {
        return std::abs (cents (fundamentalHz (held (out, sampleRate), sampleRate, 15.0, 0.45 * sampleRate), expectedHz));
    }
}

//==============================================================================
struct PitchCase
{
    double sampleRate;
    int wave;
};

class SpectralPitch : public ::testing::TestWithParam<PitchCase> {};

TEST_P (SpectralPitch, EveryNoteHasTheRightFundamental)
{
    const auto [sampleRate, wave] = GetParam();

    for (const auto note : { 21, 33, 45, 57, 69, 81, 93, 105, 108 })
    {
        const auto out = renderNote ({ { "OSC1", (float) wave }, { "OSC2", (float) wave } }, note, sampleRate);
        EXPECT_TRUE (within (tol::pitch, measuredCents (out, sampleRate, noteHz (note)), "note" + std::to_string (note)));
    }
}

INSTANTIATE_TEST_SUITE_P (RatesAndWaves, SpectralPitch,
                          ::testing::Values (PitchCase { 44100.0, 0 }, PitchCase { 44100.0, 1 }, PitchCase { 44100.0, 2 },
                                             PitchCase { 48000.0, 0 }, PitchCase { 48000.0, 1 }, PitchCase { 48000.0, 2 },
                                             PitchCase { 96000.0, 0 }, PitchCase { 96000.0, 1 }, PitchCase { 96000.0, 2 }),
                          [] (const auto& info) { return waveNames[info.param.wave] + rateName (info.param.sampleRate); });

//==============================================================================
TEST (SpectralTranspose, EachOscillatorFollowsItsPitchParameter)
{
    // The other oscillator is at -40 dB, so the transposed one dominates the spectrum
    for (const auto osc : { 1, 2 })
        for (const auto semitones : { -24, -12, -5, 7, 12, 24 })
        {
            const auto self  = "OSC" + juce::String (osc);
            const auto other = "OSC" + juce::String (3 - osc);

            const auto out = renderNote ({ { self + "PITCH", (float) semitones }, { other + "GAIN", -40.0f } }, 57);
            EXPECT_TRUE (within (tol::pitch, measuredCents (out, 48000.0, noteHz (57 + semitones)),
                                 self.toStdString() + "/" + std::to_string (semitones)));
        }
}

//==============================================================================
namespace
{
    // Bessel function of the first kind, J_n(x), by its power series (accurate for the small x used here;
    // std::cyl_bessel_j is missing from Apple's standard library)
    double besselJ (int n, double x)
    {
        double sum = 0.0, term = std::pow (x / 2.0, n) / std::tgamma (n + 1.0);

        for (int k = 0; k < 20; ++k)
        {
            sum += term;
            term *= -(x * x / 4.0) / ((k + 1.0) * (k + 1.0 + n));
        }

        return sum;
    }
}

TEST (SpectralFm, SidebandsMatchFrequencyModulationTheory)
{
    // Sine carrier at 440 Hz, sine modulator at 110 Hz, depth in semitones. The first sideband sits at
    // J1(b)/J0(b) relative to the carrier, with b = peak deviation / modulator frequency and
    // peak deviation = f0 * (2^(depth/12) - 1). Depths are on the parameter's 0.1 step and small enough
    // for the semitone (exponential) modulation to behave like linear FM.
    constexpr double carrier = 440.0, modulator = 110.0;

    for (const auto depth : { 0.1, 0.5, 1.0 })
    {
        const auto out = renderNote ({ { "OSC1FMFREQ", (float) modulator }, { "OSC1FMDEPTH", (float) depth },
                                       { "OSC2FMFREQ", (float) modulator }, { "OSC2FMDEPTH", (float) depth } }, 69);
        const auto s = held (out, 48000.0);

        const auto beta = carrier * (std::pow (2.0, depth / 12.0) - 1.0) / modulator;
        const auto expectedDb = 20.0 * std::log10 (besselJ (1, beta) / besselJ (0, beta));

        const auto carrierDb = bandEnergyDb (s, 48000.0, carrier - 20.0, carrier + 20.0);
        const auto upperDb   = bandEnergyDb (s, 48000.0, carrier + modulator - 20.0, carrier + modulator + 20.0);
        const auto lowerDb   = bandEnergyDb (s, 48000.0, carrier - modulator - 20.0, carrier - modulator + 20.0);

        const auto label = "depth" + juce::String (depth, 1).toStdString();
        EXPECT_TRUE (within (tol::passband, std::abs (upperDb - carrierDb - expectedDb), label + "/upper"));
        EXPECT_TRUE (within (tol::passband, std::abs (lowerDb - carrierDb - expectedDb), label + "/lower"));
    }
}

//==============================================================================
// Filter tests use a saw at 55 Hz, so every band below has harmonics in it, and compare each band against
// the same note through the open filter. The filter envelope is off (depth 0) so the cutoff stays put.
namespace
{
    double filteredBandDb (int type, float cutoff, float resonance, double sampleRate, double lowHz, double highHz)
    {
        const auto out = renderNote ({ { "OSC1", 1 }, { "OSC2", 1 }, { "FILTERTYPE", (float) type }, { "FILTERCUTOFF", cutoff },
                                       { "FILTERRESONANCE", resonance }, { "FILTERADSRDEPTH", 0 } }, 33, sampleRate);
        return bandEnergyDb (held (out, sampleRate), sampleRate, lowHz, highHz);
    }

    // Attenuation of a band relative to the same band through the open filter (positive = quieter)
    double attenuationDb (int type, float cutoff, float resonance, double sampleRate, double lowHz, double highHz)
    {
        const auto openCutoff = type == 2 ? 20.0f : 20000.0f;   // a high-pass is open at its lowest cutoff
        const auto openType   = type == 1 ? 0 : type;            // a band-pass is compared to the open low-pass

        return filteredBandDb (openType, openCutoff, resonance, sampleRate, lowHz, highHz)
             - filteredBandDb (type, cutoff, resonance, sampleRate, lowHz, highHz);
    }

    constexpr float butterworthQ = 0.71f;
}

class SpectralFilter : public ::testing::TestWithParam<double> {};

TEST_P (SpectralFilter, LowPassAttenuatesAboveCutoffAndPassesBelow)
{
    const auto sampleRate = GetParam();

    // A 2-pole low-pass falls 12 dB/octave: about 24 dB two octaves above the cutoff
    for (const auto cutoff : { 250.0f, 500.0f, 1000.0f, 2000.0f })
    {
        const auto label = "c" + std::to_string ((int) cutoff);
        EXPECT_TRUE (within (tol::stopband, attenuationDb (0, cutoff, butterworthQ, sampleRate, 4.0 * cutoff, 8.0 * cutoff), label));
        EXPECT_TRUE (within (tol::passband, std::abs (attenuationDb (0, cutoff, butterworthQ, sampleRate, 50.0, cutoff / 4.0)), label));
    }
}

TEST_P (SpectralFilter, HighPassAttenuatesBelowCutoffAndPassesAbove)
{
    const auto sampleRate = GetParam();

    for (const auto cutoff : { 500.0f, 1000.0f, 2000.0f })
    {
        const auto label = "c" + std::to_string ((int) cutoff);
        EXPECT_TRUE (within (tol::stopband, attenuationDb (2, cutoff, butterworthQ, sampleRate, cutoff / 8.0, cutoff / 4.0), label));
        EXPECT_TRUE (within (tol::passband, std::abs (attenuationDb (2, cutoff, butterworthQ, sampleRate, 4.0 * cutoff, 8.0 * cutoff)), label));
    }
}

TEST_P (SpectralFilter, BandPassPeaksAtCutoff)
{
    const auto sampleRate = GetParam();

    // A 2-pole band-pass falls 6 dB/octave on each side, half the low-pass slope, so its skirts are held to
    // half the stop-band tolerance. Measured relative to the band around the cutoff.
    for (const auto cutoff : { 500.0f, 1000.0f, 2000.0f })
    {
        const auto label = "c" + std::to_string ((int) cutoff);
        const auto centre = attenuationDb (1, cutoff, 2.0f, sampleRate, cutoff / 1.2, cutoff * 1.2);
        const auto below  = attenuationDb (1, cutoff, 2.0f, sampleRate, cutoff / 8.0, cutoff / 4.0) - centre;
        const auto above  = attenuationDb (1, cutoff, 2.0f, sampleRate, 4.0 * cutoff, 8.0 * cutoff) - centre;

        EXPECT_TRUE (within (tol::stopband, 2.0 * below, label + "/below"));
        EXPECT_TRUE (within (tol::stopband, 2.0 * above, label + "/above"));
    }
}

INSTANTIATE_TEST_SUITE_P (Rates, SpectralFilter, ::testing::ValuesIn (sampleRates),
                          [] (const auto& info) { return rateName (info.param); });

//==============================================================================
// Characterization: the oscillators are naive (not band-limited), so saw and square alias. These values were
// measured on the unmodified code (share of the energy away from the harmonics, in dB) and the test only checks
// that aliasing does not grow beyond them. They describe current behaviour; they are not a quality target.
namespace
{
    struct AliasingBaseline
    {
        double sampleRate;
        int wave;
        int note;
        double inharmonicDb;
    };

    constexpr AliasingBaseline aliasingBaselines[]
    {
        { 44100, 1, 81, -20.3 }, { 44100, 1, 93, -16.9 }, { 44100, 1, 105, -13.0 },
        { 44100, 2, 81, -22.2 }, { 44100, 2, 93, -18.9 }, { 44100, 2, 105, -15.3 },
        { 48000, 1, 81, -22.2 }, { 48000, 1, 93, -18.5 }, { 48000, 1, 105, -14.1 },
        { 48000, 2, 81, -24.2 }, { 48000, 2, 93, -20.7 }, { 48000, 2, 105, -16.5 },
        { 96000, 1, 81, -30.2 }, { 96000, 1, 93, -25.4 }, { 96000, 1, 105, -19.0 },
        { 96000, 2, 81, -32.4 }, { 96000, 2, 93, -27.9 }, { 96000, 2, 105, -21.2 },
    };
}

TEST (SpectralAliasing, DoesNotExceedCharacterizedLevel)
{
    for (const auto& b : aliasingBaselines)
    {
        const auto out = renderNote ({ { "OSC1", (float) b.wave }, { "OSC2", (float) b.wave } }, b.note, b.sampleRate);
        const auto measured = inharmonicEnergyDb (held (out, b.sampleRate), b.sampleRate, noteHz (b.note));

        EXPECT_TRUE (within (tol::aliasing, measured - b.inharmonicDb,
                             waveNames[b.wave] + rateName (b.sampleRate) + "/note" + std::to_string (b.note)));
    }
}
