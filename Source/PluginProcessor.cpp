#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>

//==============================================================================
TapSynthAudioProcessor::TapSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts (*this, nullptr, "Parameters", createParams())
#endif
{
    synth.addSound (new SynthSound());
    
    for (int i = 0; i < numVoices; i++)
    {
        synth.addVoice (new SynthVoice());
    }
}

TapSynthAudioProcessor::~TapSynthAudioProcessor()
{
    
}

//==============================================================================
const juce::String TapSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TapSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TapSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TapSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TapSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TapSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TapSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TapSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TapSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void TapSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TapSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    
    for (int i = 0; i < synth.getNumVoices(); i++)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            voice->prepareToPlay (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
        }
    }
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = (juce::uint32) getTotalNumOutputChannels();
    
    reverb.prepare (spec);
    meter.prepare (sampleRate);
    
    reverbParams.roomSize = 0.5f;
    reverbParams.width = 1.0f;
    reverbParams.damping = 0.5f;
    reverbParams.freezeMode = 0.0f;
    reverbParams.dryLevel = 1.0f;
    reverbParams.wetLevel = 0.0f;
    
    reverb.setParameters (reverbParams);
}

void TapSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TapSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TapSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    setParams();
        
    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());
    juce::dsp::AudioBlock<float> block { buffer };
    reverb.process (juce::dsp::ProcessContextReplacing<float> (block));
    
    meter.process (buffer);
}

//==============================================================================
bool TapSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TapSynthAudioProcessor::createEditor()
{
    return new TapSynthAudioProcessorEditor (*this);
}

//==============================================================================
void TapSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void TapSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TapSynthAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout TapSynthAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // OSC select
    params.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "OSC1", 1 }, "Oscillator 1", juce::StringArray { "Sine", "Saw", "Square" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "OSC2", 1 }, "Oscillator 2", juce::StringArray { "Sine", "Saw", "Square" }, 0));
    
    // OSC Gain
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "OSC1GAIN", 1 }, "Oscillator 1 Gain", juce::NormalisableRange<float> { -40.0f, 0.2f, 0.1f }, 0.1f, "dB"));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "OSC2GAIN", 1 }, "Oscillator 2 Gain", juce::NormalisableRange<float> { -40.0f, 0.2f, 0.1f }, 0.1f, "dB"));
    
    // OSC Pitch val
    params.push_back (std::make_unique<juce::AudioParameterInt>(juce::ParameterID { "OSC1PITCH", 1 }, "Oscillator 1 Pitch", -48, 48, 0));
    params.push_back (std::make_unique<juce::AudioParameterInt>(juce::ParameterID { "OSC2PITCH", 1 }, "Oscillator 2 Pitch", -48, 48, 0));
    
    // FM Osc Freq
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "OSC1FMFREQ", 1 }, "Oscillator 1 FM Frequency", juce::NormalisableRange<float> { 0.0f, 1000.0f, 0.1f, 0.4f }, 0.0f, "Hz"));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "OSC2FMFREQ", 1 }, "Oscillator 2 FM Frequency", juce::NormalisableRange<float> { 0.0f, 1000.0f, 0.1f, 0.4f }, 0.0f, "Hz"));
    
    // FM Osc Depth
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "OSC1FMDEPTH", 1 }, "Oscillator 1 FM Depth", juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 0.0f, ""));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "OSC2FMDEPTH", 1 }, "Oscillator 2 FM Depth", juce::NormalisableRange<float> { 0.0f, 100.0f, 0.1f }, 0.0f, ""));
    
    // LFO
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "LFO1FREQ", 1 }, "LFO1 Frequency", juce::NormalisableRange<float> { 0.0f, 20.0f, 0.01f, 0.5f }, 0.0f, "Hz"));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "LFO1DEPTH", 1 }, "LFO1 Depth", juce::NormalisableRange<float> { 0.0f, 10000.0f, 0.1f, 0.5f }, 0.0f, ""));
    
    //Filter (skewed so the middle of the knob sits at the geometric centre of the range, ~632 Hz)
    juce::NormalisableRange<float> cutoffRange { 20.0f, 20000.0f, 0.1f };
    cutoffRange.setSkewForCentre (632.0f);
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "FILTERTYPE", 1 }, "Filter Type", juce::StringArray { "Low Pass", "Band Pass", "High Pass" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "FILTERCUTOFF", 1 }, "Filter Cutoff", cutoffRange, 20000.0f, "Hz"));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "FILTERRESONANCE", 1 }, "Filter Resonance", juce::NormalisableRange<float> { 0.1f, 2.0f, 0.01f }, 0.1f, ""));
    
    // ADSR
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "ATTACK", 1 }, "Attack", juce::NormalisableRange<float> { 0.1f, 1.0f, 0.01f }, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "DECAY", 1 }, "Decay", juce::NormalisableRange<float> { 0.1f, 1.0f, 0.01f }, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "SUSTAIN", 1 }, "Sustain", juce::NormalisableRange<float> { 0.1f, 1.0f, 0.01f }, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "RELEASE", 1 }, "Release", juce::NormalisableRange<float> { 0.1f, 3.0f, 0.01f }, 0.4f));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "FILTERADSRDEPTH", 1 }, "Filter ADSR Depth", juce::NormalisableRange<float> { 0.0f, 10000.0f, 0.1f, 0.5f }, 10000.0f, ""));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "FILTERATTACK", 1 }, "Filter Attack", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.01f }, 0.01f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "FILTERDECAY", 1 }, "Filter Decay", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.01f }, 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "FILTERSUSTAIN", 1 }, "Filter Sustain", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.01f }, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "FILTERRELEASE", 1 }, "Filter Release", juce::NormalisableRange<float> { 0.0f, 3.0f, 0.01f }, 0.1f));
    
    // Reverb
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "REVERBSIZE", 1 }, "Reverb Size", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.01f }, 0.0f, ""));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "REVERBWIDTH", 1 }, "Reverb Width", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.01f }, 1.0f, ""));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "REVERBDAMPING", 1 }, "Reverb Damping", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.01f }, 0.5f, ""));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "REVERBDRY", 1 }, "Reverb Dry", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.01f }, 1.0f, ""));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "REVERBWET", 1 }, "Reverb Wet", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.01f }, 0.0f, ""));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "REVERBFREEZE", 1 }, "Reverb Freeze", juce::NormalisableRange<float> { 0.0f, 1.0f, 0.1f }, 0.0f, ""));
    
    return { params.begin(), params.end() };
}

void TapSynthAudioProcessor::setParams()
{
    setVoiceParams();
    setFilterParams();
    setReverbParams();
}

void TapSynthAudioProcessor::setVoiceParams()
{
    auto& attack = *apvts.getRawParameterValue ("ATTACK");
    auto& decay = *apvts.getRawParameterValue ("DECAY");
    auto& sustain = *apvts.getRawParameterValue ("SUSTAIN");
    auto& release = *apvts.getRawParameterValue ("RELEASE");
    
    auto& osc1Choice = *apvts.getRawParameterValue ("OSC1");
    auto& osc2Choice = *apvts.getRawParameterValue ("OSC2");
    auto& osc1Gain = *apvts.getRawParameterValue ("OSC1GAIN");
    auto& osc2Gain = *apvts.getRawParameterValue ("OSC2GAIN");
    auto& osc1Pitch = *apvts.getRawParameterValue ("OSC1PITCH");
    auto& osc2Pitch = *apvts.getRawParameterValue ("OSC2PITCH");
    auto& osc1FmFreq = *apvts.getRawParameterValue ("OSC1FMFREQ");
    auto& osc2FmFreq = *apvts.getRawParameterValue ("OSC2FMFREQ");
    auto& osc1FmDepth = *apvts.getRawParameterValue ("OSC1FMDEPTH");
    auto& osc2FmDepth = *apvts.getRawParameterValue ("OSC2FMDEPTH");
    
    auto& filterAttack = *apvts.getRawParameterValue ("FILTERATTACK");
    auto& filterDecay = *apvts.getRawParameterValue ("FILTERDECAY");
    auto& filterSustain = *apvts.getRawParameterValue ("FILTERSUSTAIN");
    auto& filterRelease = *apvts.getRawParameterValue ("FILTERRELEASE");

    for (int v = 0; v < synth.getNumVoices(); ++v)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(v)))
        {
            auto& osc1 = voice->getOscillator1();
            auto& osc2 = voice->getOscillator2();
            
            for (int ch = 0; ch < numChannelsToProcess; ++ch)
            {
                osc1[ch].setParams ((int) osc1Choice, osc1Gain, (int) osc1Pitch, osc1FmFreq, osc1FmDepth);
                osc2[ch].setParams ((int) osc2Choice, osc2Gain, (int) osc2Pitch, osc2FmFreq, osc2FmDepth);
            }
            
            voice->getAdsr().update (attack, decay, sustain, release);
            voice->getFilterAdsr().update (filterAttack, filterDecay, filterSustain, filterRelease);
        }
    }
}

void TapSynthAudioProcessor::setFilterParams()
{
    auto& filterType = *apvts.getRawParameterValue ("FILTERTYPE");
    auto& filterCutoff = *apvts.getRawParameterValue ("FILTERCUTOFF");
    auto& filterResonance = *apvts.getRawParameterValue ("FILTERRESONANCE");
    auto& adsrDepth = *apvts.getRawParameterValue ("FILTERADSRDEPTH");
    auto& lfoFreq = *apvts.getRawParameterValue ("LFO1FREQ");
    auto& lfoDepth = *apvts.getRawParameterValue ("LFO1DEPTH");
        
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            voice->updateModParams ((int) filterType, filterCutoff, filterResonance, adsrDepth, lfoFreq, lfoDepth);
        }
    }
}

void TapSynthAudioProcessor::setReverbParams()
{
    reverbParams.roomSize = *apvts.getRawParameterValue ("REVERBSIZE");
    reverbParams.width = *apvts.getRawParameterValue ("REVERBWIDTH");
    reverbParams.damping = *apvts.getRawParameterValue ("REVERBDAMPING");
    reverbParams.dryLevel = *apvts.getRawParameterValue ("REVERBDRY");
    reverbParams.wetLevel = *apvts.getRawParameterValue ("REVERBWET");
    reverbParams.freezeMode = *apvts.getRawParameterValue ("REVERBFREEZE");
    
    reverb.setParameters (reverbParams);
}

void TapSynthAudioProcessor::applyParametersFromJson (const juce::var& json)
{
    if (! juce::MessageManager::getInstance()->isThisTheMessageThread())
    {
        juce::MessageManager::callAsync ([this, json]()
        {
            applyParametersFromJson (json);
        });
        return;
    }

    auto* obj = json.getDynamicObject();
    if (obj == nullptr)
    {
        DBG ("JSON is not an object.");
        return;
    }

    for (auto& entry : obj->getProperties())
    {
        const auto id = entry.name.toString();
        const auto& value = entry.value;

        auto* param = apvts.getParameter (id);
        if (param == nullptr)
        {
            DBG ("Unknown parameter id from JSON: " << id);
            continue;
        }

        if (! (value.isInt() || value.isInt64() || value.isDouble() || value.isBool() || value.isString()))
        {
            DBG ("Non-numeric value for parameter id from JSON: " << id);
            continue;
        }

        // convertTo0to1 clamps to the parameter's range (choice parameters take their integer index)
        const auto normalised = param->convertTo0to1 ((float) static_cast<double> (value));

        param->beginChangeGesture();
        param->setValueNotifyingHost (normalised);
        param->endChangeGesture();
    }
}

std::vector<float> TapSynthAudioProcessor::getParameterSnapshot() const
{
    std::vector<float> values;

    for (auto* param : getParameters())
        values.push_back (param->getValue());

    return values;
}

void TapSynthAudioProcessor::applyParameterSnapshot (const std::vector<float>& snapshot)
{
    const auto& params = getParameters();
    jassert ((int) snapshot.size() == params.size());

    for (int i = 0; i < juce::jmin ((int) snapshot.size(), params.size()); ++i)
    {
        auto* param = params[i];

        if (std::abs (param->getValue() - snapshot[(size_t) i]) < 1.0e-6f)
            continue;

        param->beginChangeGesture();
        param->setValueNotifyingHost (snapshot[(size_t) i]);
        param->endChangeGesture();
    }
}
