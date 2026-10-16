/*
  ==============================================================================

    OscData.cpp
    Created: 14 Feb 2021 6:51:17pm
    Author:  Joshua Hodge

  ==============================================================================
*/

#include "OscData.h"

void OscData::prepareToPlay (double sampleRate, int samplesPerBlock, int outputChannels)
{
    resetAll();
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = outputChannels;
    
    nyquist = (float) sampleRate * 0.5f;
    
    prepare (spec);
    fmOsc.prepare (spec);
    gain.prepare (spec);    
}

void OscData::setType (const int oscSelection)
{
    if (oscSelection == lastType)
        return;
    
    switch (oscSelection)
    {
        // Sine
        case 0:
            initialise ([](float x) { return std::sin (x); });
            break;
            
        // Saw
        case 1:
            initialise ([] (float x) { return x / juce::MathConstants<float>::pi; });
            break;
          
        // Square
        case 2:
            initialise ([] (float x) { return x < 0.0f ? -1.0f : 1.0f; });
            break;
            
        default:
            // You shouldn't be here!
            jassertfalse;
            return;
    }
    
    lastType = oscSelection;
}

void OscData::setGain (const float levelInDecibels)
{
    gain.setGainDecibels (levelInDecibels);
}

void OscData::setOscPitch (const int pitch)
{
    lastPitch = pitch;
    updateFrequency();
}

void OscData::setFreq (const int midiNoteNumber)
{
    lastMidiNote = midiNoteNumber;
    updateFrequency();
}

void OscData::setFmOsc (const float freq, const float depth)
{
    fmDepth = depth;
    fmOsc.setFrequency (freq);
}

void OscData::renderNextBlock (juce::dsp::AudioBlock<float>& audioBlock)
{
    jassert (audioBlock.getNumSamples() > 0);
    process (juce::dsp::ProcessContextReplacing<float> (audioBlock));
    gain.process (juce::dsp::ProcessContextReplacing<float> (audioBlock));
}

float OscData::processNextSample (float input)
{
    // FM offsets the note (in semitones) every sample; forced so the oscillator's frequency smoothing doesn't swallow it
    if (fmDepth > 0.0f)
    {
        fmModulator = fmOsc.processSample (0.0f) * fmDepth;
        updateFrequency();
    }
    else
    {
        fmModulator = 0.0f;
    }
    
    return gain.processSample (processSample (input));
}

void OscData::setParams (const int oscChoice, const float oscGain, const int oscPitch, const float fmFreq, const float fmDepth)
{
    setType (oscChoice);
    setGain (oscGain);
    setFmOsc (fmFreq, fmDepth);
    setOscPitch (oscPitch);
}

void OscData::updateFrequency()
{
    const auto note = (float) (lastMidiNote + lastPitch) + fmModulator;
    const auto freq = 440.0f * std::pow (2.0f, (note - 69.0f) / 12.0f);
    setFrequency (juce::jlimit (0.0f, nyquist, freq), true);
}

void OscData::resetAll()
{
    reset();
    fmOsc.reset();
    gain.reset();
}
