#include "SynthVoice.h"

bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*>(sound) != nullptr;
}

void SynthVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound *sound, int currentPitchWheelPosition)
{
    for (int i = 0; i < numChannelsToProcess; i++)
    {
        osc1[i].setFreq (midiNoteNumber);
        osc2[i].setFreq (midiNoteNumber);
    }
    
    adsr.noteOn();
    filterAdsr.noteOn();
}

void SynthVoice::stopNote (float velocity, bool allowTailOff)
{
    adsr.noteOff();
    filterAdsr.noteOff();
    
    if (! allowTailOff || ! adsr.isActive())
    {
        adsr.reset();
        filterAdsr.reset();
        clearCurrentNote();
    }
}

void SynthVoice::controllerMoved (int controllerNumber, int newControllerValue)
{
    
}

void SynthVoice::pitchWheelMoved (int newPitchWheelValue)
{
    
}

void SynthVoice::prepareToPlay (double sampleRate, int samplesPerBlock, int outputChannels)
{
    reset();
    
    currentSampleRate = sampleRate;
    adsr.setSampleRate (sampleRate);
    filterAdsr.setSampleRate (sampleRate);
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = (juce::uint32) outputChannels;
    
    for (int ch = 0; ch < numChannelsToProcess; ch++)
    {
        osc1[ch].prepareToPlay (sampleRate, samplesPerBlock, outputChannels);
        osc2[ch].prepareToPlay (sampleRate, samplesPerBlock, outputChannels);
        filter[ch].prepareToPlay (sampleRate, samplesPerBlock, outputChannels);
        lfo[ch].prepare (spec);
        lfo[ch].initialise ([](float x) { return std::sin (x); });
    }
    
    gain.prepare (spec);
    gain.setGainLinear (0.07f);
    
    isPrepared = true;
}

void SynthVoice::renderNextBlock (juce::AudioBuffer< float > &outputBuffer, int startSample, int numSamples)
{
    jassert (isPrepared);
    
    if (! isVoiceActive())
        return;
    
    const auto numChannels = juce::jmin (outputBuffer.getNumChannels(), numChannelsToProcess);
    synthBuffer.setSize (numChannels, numSamples, false, false, true);
    synthBuffer.clear();
    
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* buffer = synthBuffer.getWritePointer (ch, 0);
        
        for (int s = 0; s < numSamples; ++s)
            buffer[s] = osc1[ch].processNextSample (0.0f) + osc2[ch].processNextSample (0.0f);
    }
    
    juce::dsp::AudioBlock<float> audioBlock { synthBuffer };
    gain.process (juce::dsp::ProcessContextReplacing<float> (audioBlock));
    adsr.applyEnvelopeToBuffer (synthBuffer, 0, numSamples);
    
    // Cutoff is modulated per sample by the filter envelope and the LFO
    const auto maxCutoff = juce::jmin (20000.0f, (float) currentSampleRate * 0.49f);
    
    for (int s = 0; s < numSamples; ++s)
    {
        filterAdsrOutput = filterAdsr.getNextSample();
        
        for (int ch = 0; ch < numChannels; ++ch)
        {
            lfoOutput[ch] = lfo[ch].processSample (0.0f);
            
            auto cutoff = baseCutoff + (filterAdsrDepth * filterAdsrOutput) + (lfoDepth * lfoOutput[ch]);
            filter[ch].setCutoffFrequency (juce::jlimit (20.0f, maxCutoff, cutoff));
            
            auto* buffer = synthBuffer.getWritePointer (ch, 0);
            buffer[s] = filter[ch].processNextSample (ch, buffer[s]);
        }
    }
    
    for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
        outputBuffer.addFrom (channel, startSample, synthBuffer, juce::jmin (channel, numChannels - 1), 0, numSamples);
    
    if (! adsr.isActive())
        clearCurrentNote();
}

void SynthVoice::reset()
{
    gain.reset();
    adsr.reset();
    filterAdsr.reset();
}

void SynthVoice::updateModParams (const int filterType, const float filterCutoff, const float filterResonance, const float adsrDepth, const float lfoFreq, const float lfoDepthAmount)
{
    baseCutoff = filterCutoff;
    filterAdsrDepth = adsrDepth;
    lfoDepth = lfoDepthAmount;

    for (int ch = 0; ch < numChannelsToProcess; ++ch)
    {
        filter[ch].setParams (filterType, juce::jlimit (20.0f, juce::jmin (20000.0f, (float) currentSampleRate * 0.49f), filterCutoff), filterResonance);
        lfo[ch].setFrequency (lfoFreq);
    }
}
