#include "Analysis.h"

#include <cmath>
#include <limits>

namespace tapsynth::test
{
    namespace
    {
        // Power spectrum (|X|^2 per bin, bins 0..size/2) of a Hann-windowed segment zero-padded to at least
        // minFftSize, so short segments still give a fine bin grid for interpolation
        std::vector<float> powerSpectrum (Segment s, int& fftSize)
        {
            constexpr int minFftSize = 1 << 16;
            int order = 1;
            while ((1 << order) < juce::jmax (s.numSamples, minFftSize))
                ++order;

            fftSize = 1 << order;
            std::vector<float> data ((size_t) fftSize * 2, 0.0f);

            for (int i = 0; i < s.numSamples; ++i)
            {
                const auto w = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * (float) i / (float) (s.numSamples - 1));
                data[(size_t) i] = s.data[i] * w;
            }

            juce::dsp::FFT fft (order);
            fft.performFrequencyOnlyForwardTransform (data.data(), true);

            std::vector<float> power ((size_t) fftSize / 2 + 1);
            for (size_t k = 0; k < power.size(); ++k)
                power[k] = data[k] * data[k];

            return power;
        }
    }

    Segment segment (const juce::AudioBuffer<float>& buffer, int channel, double sampleRate,
                     double startSeconds, double lengthSeconds)
    {
        const auto start = juce::jlimit (0, buffer.getNumSamples(), (int) std::lround (startSeconds * sampleRate));
        const auto end   = juce::jlimit (start, buffer.getNumSamples(), (int) std::lround ((startSeconds + lengthSeconds) * sampleRate));
        return { buffer.getReadPointer (channel) + start, end - start };
    }

    Segment segment (const juce::AudioBuffer<float>& buffer, int channel)
    {
        return { buffer.getReadPointer (channel), buffer.getNumSamples() };
    }

    float rms (Segment s)
    {
        if (s.numSamples == 0)
            return 0.0f;

        double sum = 0.0;
        for (int i = 0; i < s.numSamples; ++i)
            sum += (double) s.data[i] * s.data[i];

        return (float) std::sqrt (sum / s.numSamples);
    }

    float peak (Segment s)
    {
        float max = 0.0f;
        for (int i = 0; i < s.numSamples; ++i)
            max = juce::jmax (max, std::abs (s.data[i]));

        return max;
    }

    float dcOffset (Segment s)
    {
        if (s.numSamples == 0)
            return 0.0f;

        double sum = 0.0;
        for (int i = 0; i < s.numSamples; ++i)
            sum += s.data[i];

        return (float) (sum / s.numSamples);
    }

    bool allFinite (const juce::AudioBuffer<float>& buffer)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            const auto* data = buffer.getReadPointer (ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                if (! std::isfinite (data[i]))
                    return false;
        }

        return true;
    }

    float gainToDb (float gain)
    {
        return juce::Decibels::gainToDecibels (gain, -200.0f);
    }

    double fundamentalHz (Segment s, double sampleRate, double minHz, double maxHz)
    {
        if (s.numSamples < 64)
            return 0.0;

        int fftSize = 0;
        const auto power = powerSpectrum (s, fftSize);
        const auto binHz = sampleRate / fftSize;

        const auto first = juce::jmax (1, (int) std::ceil (minHz / binHz));
        const auto last  = juce::jmin ((int) power.size() - 2, (int) std::floor (maxHz / binHz));

        int best = first;
        for (int k = first; k <= last; ++k)
            if (power[(size_t) k] > power[(size_t) best])
                best = k;

        // Parabolic interpolation of the log magnitude around the peak bin
        const auto a = std::log (power[(size_t) best - 1] + 1.0e-30);
        const auto b = std::log (power[(size_t) best]     + 1.0e-30);
        const auto c = std::log (power[(size_t) best + 1] + 1.0e-30);
        const auto denom = a - 2.0 * b + c;
        const auto offset = std::abs (denom) > 1.0e-12 ? 0.5 * (a - c) / denom : 0.0;

        return (best + offset) * binHz;
    }

    double bandEnergyDb (Segment s, double sampleRate, double lowHz, double highHz)
    {
        if (s.numSamples < 64)
            return -200.0;

        int fftSize = 0;
        const auto power = powerSpectrum (s, fftSize);
        const auto binHz = sampleRate / fftSize;

        const auto first = juce::jlimit (0, (int) power.size() - 1, (int) std::ceil (lowHz / binHz));
        const auto last  = juce::jlimit (0, (int) power.size() - 1, (int) std::floor (highHz / binHz));

        double sum = 0.0;
        for (int k = first; k <= last; ++k)
            sum += power[(size_t) k];

        return sum > 0.0 ? 10.0 * std::log10 (sum) : -200.0;
    }

    double inharmonicEnergyDb (Segment s, double sampleRate, double f0)
    {
        if (s.numSamples < 64 || f0 <= 0.0)
            return 0.0;

        int fftSize = 0;
        const auto power = powerSpectrum (s, fftSize);
        const auto binHz = sampleRate / fftSize;

        // The Hann main lobe is +-2 bins of the unpadded length; its side lobes still leak about -42 dB
        // beyond 3 bins, so +-6 bins keeps that leakage well below the aliasing being measured
        const auto lobeHz = 6.0 * sampleRate / s.numSamples;

        double total = 0.0, inharmonic = 0.0;
        for (size_t k = 1; k < power.size(); ++k)
        {
            const auto hz = (double) k * binHz;
            const auto harmonic = juce::jmax (1.0, std::round (hz / f0));
            total += power[k];

            if (std::abs (hz - harmonic * f0) > lobeHz)
                inharmonic += power[k];
        }

        return total > 0.0 && inharmonic > 0.0 ? 10.0 * std::log10 (inharmonic / total) : -200.0;
    }

    std::vector<float> envelope (Segment s, double sampleRate, double windowSeconds)
    {
        const auto window = juce::jmax (1, (int) std::lround (windowSeconds * sampleRate));
        std::vector<float> result;

        for (int start = 0; start + window <= s.numSamples; start += window)
            result.push_back (rms ({ s.data + start, window }));

        return result;
    }

    float maxAbsDiff (const juce::AudioBuffer<float>& a, const juce::AudioBuffer<float>& b)
    {
        if (a.getNumChannels() != b.getNumChannels() || a.getNumSamples() != b.getNumSamples())
            return std::numeric_limits<float>::infinity();

        float max = 0.0f;
        for (int ch = 0; ch < a.getNumChannels(); ++ch)
        {
            const auto* x = a.getReadPointer (ch);
            const auto* y = b.getReadPointer (ch);

            for (int i = 0; i < a.getNumSamples(); ++i)
            {
                const auto d = std::abs (x[i] - y[i]);

                // NaN in either buffer must count as a difference, not vanish in jmax
                if (! (d <= max))
                    max = std::isnan (d) ? std::numeric_limits<float>::infinity() : d;
            }
        }

        return max;
    }

    bool saveWav (const juce::File& file, const juce::AudioBuffer<float>& buffer, double sampleRate)
    {
        file.getParentDirectory().createDirectory();
        file.deleteFile();

        std::unique_ptr<juce::OutputStream> stream (file.createOutputStream());
        if (stream == nullptr)
            return false;

        juce::WavAudioFormat wav;
        std::unique_ptr<juce::AudioFormatWriter> writer (
            wav.createWriterFor (stream, juce::AudioFormatWriterOptions{}
                                            .withSampleRate (sampleRate)
                                            .withNumChannels (buffer.getNumChannels())
                                            .withBitsPerSample (32)
                                            .withSampleFormat (juce::AudioFormatWriterOptions::SampleFormat::floatingPoint)));
        if (writer == nullptr)
            return false;

        return writer->writeFromAudioSampleBuffer (buffer, 0, buffer.getNumSamples());
    }

    bool loadWav (const juce::File& file, juce::AudioBuffer<float>& buffer, double& sampleRate)
    {
        auto stream = file.createInputStream();
        if (stream == nullptr)
            return false;

        juce::WavAudioFormat wav;
        std::unique_ptr<juce::AudioFormatReader> reader (wav.createReaderFor (stream.release(), true));
        if (reader == nullptr)
            return false;

        sampleRate = reader->sampleRate;
        buffer.setSize ((int) reader->numChannels, (int) reader->lengthInSamples);
        return reader->read (&buffer, 0, (int) reader->lengthInSamples, 0, true, true);
    }
}
