/*
  ==============================================================================

    MeterData.cpp
    Created: 27 Feb 2021 9:37:46pm
    Author:  Joshua Hodge

  ==============================================================================
*/

#include "MeterData.h"

void MeterData::prepare (double newSampleRate)
{
    sampleRate = newSampleRate;
    meanSquare.fill (0.0f);

    for (int ch = 0; ch < maxChannels; ++ch)
    {
        rms[(size_t) ch].store (0.0f);
        peak[(size_t) ch].store (0.0f);
    }
}

void MeterData::process (const juce::AudioBuffer<float>& buffer)
{
    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();

    if (numSamples == 0 || numChannels == 0)
        return;

    // One-pole smoothing of the mean square, so the RMS reads over a fixed time window whatever the block size
    const auto coeff = (float) (1.0 - std::exp (-numSamples / (rmsWindowSeconds * sampleRate)));

    for (int ch = 0; ch < maxChannels; ++ch)
    {
        // A mono output feeds both meter channels
        auto output = buffer.getReadPointer (juce::jmin (ch, numChannels - 1));
        float sum = 0.0f;
        float max = 0.0f;

        for (int s = 0; s < numSamples; ++s)
        {
            sum += output[s] * output[s];
            max = juce::jmax (max, std::abs (output[s]));
        }

        auto& ms = meanSquare[(size_t) ch];
        ms += coeff * (sum / (float) numSamples - ms);
        rms[(size_t) ch].store (std::sqrt (ms));

        // Keep the highest peak until the editor takes it
        auto& p = peak[(size_t) ch];
        auto current = p.load();
        while (max > current && ! p.compare_exchange_weak (current, max)) {}
    }
}
