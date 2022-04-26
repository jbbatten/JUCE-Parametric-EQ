/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  =========================================				=====================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Parametric_EQ_PluginAudioProcessor::Parametric_EQ_PluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	)
#endif
{
}

Parametric_EQ_PluginAudioProcessor::~Parametric_EQ_PluginAudioProcessor()
{
}

//==============================================================================
const juce::String Parametric_EQ_PluginAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool Parametric_EQ_PluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
	return false;
#endif
}

bool Parametric_EQ_PluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
	return false;
#endif
}

bool Parametric_EQ_PluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
	return false;
#endif
}

double Parametric_EQ_PluginAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int Parametric_EQ_PluginAudioProcessor::getNumPrograms()
{
	return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
	// so this should be at least 1, even if you're not really implementing programs.
}

int Parametric_EQ_PluginAudioProcessor::getCurrentProgram()
{
	return 0;
}

void Parametric_EQ_PluginAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String Parametric_EQ_PluginAudioProcessor::getProgramName(int index)
{
	return {};
}

void Parametric_EQ_PluginAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void Parametric_EQ_PluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	juce::dsp::ProcessSpec spec;

	spec.maximumBlockSize = samplesPerBlock;
	spec.numChannels = 1;
	spec.sampleRate = sampleRate;

	leftChannel.prepare(spec);
	rightChannel.prepare(spec);

}

void Parametric_EQ_PluginAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Parametric_EQ_PluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	// Some plugin hosts, such as certain GarageBand versions, will only
	// load plugins that support stereo bus layouts.
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

void Parametric_EQ_PluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	const auto totalNumInputChannels = getTotalNumInputChannels();
	const auto totalNumOutputChannels = getTotalNumOutputChannels();


	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, buffer.getNumSamples());

	ChainSettings settings = getChainSettings(apvts);

	auto lowShelfCoe = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
		getSampleRate(), settings.lowShelfFreq, settings.lowShelfQ,
		juce::Decibels::decibelsToGain(settings.lowShelfGainDB));
	auto lowMidPeakCoe = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
		getSampleRate(), settings.lowMidFreq, settings.lowMidQ, juce::Decibels::decibelsToGain(settings.lowMidGainDB));
	auto midPeakCoe = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
		getSampleRate(), settings.midFreq, settings.midQ, juce::Decibels::decibelsToGain(settings.midGainDB));
	auto hiShelfCoe = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
		getSampleRate(), settings.hiShelfFreq, settings.hiShelfQ,
		juce::Decibels::decibelsToGain(settings.hiShelfGainDB));

	auto cutCoe = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(settings.lowCutFreq, getSampleRate(), 2 * (settings.lowCutSlope + 1));
	auto& leftLowCut = leftChannel.get<ChainPos::LowCut>();

	leftLowCut.setBypassed<0>(true);
	leftLowCut.setBypassed<1>(true);
	leftLowCut.setBypassed<2>(true);
	leftLowCut.setBypassed<3>(true);

	switch (settings.lowCutSlope)
	{
	case Slope12:
		*leftLowCut.get<0>().coefficients = *cutCoe[0];
		leftLowCut.setBypassed<0>(false);
		break;
	case Slope24:
		*leftLowCut.get<0>().coefficients = *cutCoe[0];
		leftLowCut.setBypassed<0>(false);
		*leftLowCut.get<1>().coefficients = *cutCoe[1];
		leftLowCut.setBypassed<1>(false);
		break;
	case Slope36:
		*leftLowCut.get<0>().coefficients = *cutCoe[0];
		leftLowCut.setBypassed<0>(false);
		*leftLowCut.get<1>().coefficients = *cutCoe[1];
		leftLowCut.setBypassed<1>(false);
		*leftLowCut.get<2>().coefficients = *cutCoe[2];
		leftLowCut.setBypassed<2>(false);
		break;
	case Slope48:
		*leftLowCut.get<0>().coefficients = *cutCoe[0];
		leftLowCut.setBypassed<0>(false);
		*leftLowCut.get<1>().coefficients = *cutCoe[1];
		leftLowCut.setBypassed<1>(false);
		*leftLowCut.get<2>().coefficients = *cutCoe[2];
		leftLowCut.setBypassed<2>(false);
		*leftLowCut.get<3>().coefficients = *cutCoe[3];
		leftLowCut.setBypassed<3>(false);
		break;
	}

	auto& rightLowCut = rightChannel.get<ChainPos::LowCut>();

	rightLowCut.setBypassed<0>(true);
	rightLowCut.setBypassed<1>(true);
	rightLowCut.setBypassed<2>(true);
	rightLowCut.setBypassed<3>(true);

	switch (settings.lowCutSlope)
	{
	case Slope12:
		*rightLowCut.get<0>().coefficients = *cutCoe[0];
		rightLowCut.setBypassed<0>(false);
		break;
	case Slope24:
		*rightLowCut.get<0>().coefficients = *cutCoe[0];
		rightLowCut.setBypassed<0>(false);
		*rightLowCut.get<1>().coefficients = *cutCoe[1];
		rightLowCut.setBypassed<1>(false);
		break;
	case Slope36:
		*rightLowCut.get<0>().coefficients = *cutCoe[0];
		rightLowCut.setBypassed<0>(false);
		*rightLowCut.get<1>().coefficients = *cutCoe[1];
		rightLowCut.setBypassed<1>(false);
		*rightLowCut.get<2>().coefficients = *cutCoe[2];
		rightLowCut.setBypassed<2>(false);
		break;
	case Slope48:
		*rightLowCut.get<0>().coefficients = *cutCoe[0];
		rightLowCut.setBypassed<0>(false);
		*rightLowCut.get<1>().coefficients = *cutCoe[1];
		rightLowCut.setBypassed<1>(false);
		*rightLowCut.get<2>().coefficients = *cutCoe[2];
		rightLowCut.setBypassed<2>(false);
		*rightLowCut.get<3>().coefficients = *cutCoe[3];
		rightLowCut.setBypassed<3>(false);
		break;
	}

	*leftChannel.get<ChainPos::LowShelf>().coefficients = *lowShelfCoe;
	*rightChannel.get<ChainPos::LowShelf>().coefficients = *lowShelfCoe;
	*leftChannel.get<ChainPos::LowMidPeak>().coefficients = *lowMidPeakCoe;
	*rightChannel.get<ChainPos::LowMidPeak>().coefficients = *lowMidPeakCoe;
	*leftChannel.get<ChainPos::MidPeak>().coefficients = *midPeakCoe;
	*rightChannel.get<ChainPos::MidPeak>().coefficients = *midPeakCoe;
	*leftChannel.get<ChainPos::HiShelf>().coefficients = *hiShelfCoe;
	*rightChannel.get<ChainPos::HiShelf>().coefficients = *hiShelfCoe;


	const juce::dsp::AudioBlock<float> block(buffer);

	auto left_channel_block = block.getSingleChannelBlock(0);
	auto right_channel_block = block.getSingleChannelBlock(1);

	juce::dsp::ProcessContextReplacing<float> left_context(left_channel_block);
	juce::dsp::ProcessContextReplacing<float> right_context(right_channel_block);

	leftChannel.process(left_context);
	rightChannel.process(right_context);
}

//==============================================================================
bool Parametric_EQ_PluginAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Parametric_EQ_PluginAudioProcessor::createEditor()
{
	// return new Parametric_EQ_PluginAudioProcessorEditor(*this);
	return new juce::GenericAudioProcessorEditor(*this);
}


//==============================================================================
void Parametric_EQ_PluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
}

void Parametric_EQ_PluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
	ChainSettings settings;

	settings.lowCutFreq = apvts.getRawParameterValue("LOWCUTFREQ")->load();
	settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LOWCUTSLOPE")->load());

	settings.lowShelfFreq = apvts.getRawParameterValue("LOWSHELFFREQ")->load();
	settings.lowShelfGainDB = apvts.getRawParameterValue("LOWSHELFGAIN")->load();
	settings.lowShelfQ = apvts.getRawParameterValue("LOWSHELFQ")->load();

	settings.lowMidFreq = apvts.getRawParameterValue("LOWMIDPEAKFREQ")->load();
	settings.lowMidGainDB = apvts.getRawParameterValue("LOWMIDPEAKGAIN")->load();
	settings.lowMidQ = apvts.getRawParameterValue("LOWMIDPEAKQ")->load();

	settings.midFreq = apvts.getRawParameterValue("MIDPEAKFREQ")->load();
	settings.midGainDB = apvts.getRawParameterValue("MIDPEAKGAIN")->load();
	settings.midQ = apvts.getRawParameterValue("MIDPEAKQ")->load();

	settings.hiShelfFreq = apvts.getRawParameterValue("HISHELFFREQ")->load();
	settings.hiShelfGainDB = apvts.getRawParameterValue("HISHELFGAIN")->load();
	settings.hiShelfQ = apvts.getRawParameterValue("HISHELFQ")->load();

	settings.hiCutFreq = apvts.getRawParameterValue("HICUTFREQ")->load();
	settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HICUTSLOPE")->load());

	return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout Parametric_EQ_PluginAudioProcessor::createParamLayout()
{
	//createing parameter layout, mapping all values to sliders
	juce::AudioProcessorValueTreeState::ParameterLayout param_layout;
	juce::StringArray choicesArray;

	for (int i = 0; i < 4; i++)
	{
		juce::String str;
		str << (12 + i * 12);
		str << " db/Octave";
		choicesArray.add(str);
	}
	//Low Cut > Low Shelf > Low Mid Peak > Mid Peak > Hi Shelf > Hi Cut

	//Low Cut Filter
	param_layout.add(std::make_unique<juce::AudioParameterFloat>("LOWCUTFREQ", "Low Cut Freq",juce::NormalisableRange<float>(20.0f, 20000.f, 1.0f, 0.2f), 20.0f));
	param_layout.add(std::make_unique<juce::AudioParameterChoice>("LOWCUTSLOPE", "Low Cut Slope", choicesArray, 0.0f));

	//Low Shelf
	param_layout.add(std::make_unique<juce::AudioParameterFloat>("LOWSHELFFREQ", "Low Shelf Freq",juce::NormalisableRange<float>(20.0f, 20000.f, 1.0f, 0.3f), 150.0f));
	param_layout.add(std::make_unique<juce::AudioParameterFloat>("LOWSHELFGAIN", "Low Shelf Gain",juce::NormalisableRange<float>(-24.0f, 24.0f, 0.5f, 1.0f), 0.0f));
	param_layout.add(std::make_unique<juce::AudioParameterFloat>("LOWSHELFQ", "Low Shelf Q",juce::NormalisableRange<float>(0.1f, 10.0f, 0.05f, 1.0f), 1.0f));

	//Low Mid Peak
	param_layout.add(std::make_unique<juce::AudioParameterFloat>("LOWMIDPEAKFREQ", "Low Mid Peak_Freq",juce::NormalisableRange<float>(20.0f, 20000.f, 1.0f, 0.3f), 750.f));
	param_layout.add(std::make_unique<juce::AudioParameterFloat>("LOWMIDPEAKGAIN", "Low Mid Gain",juce::NormalisableRange<float>(-24.0f, 24.0f, 0.5f, 1.0f), 0.0f));
	param_layout.add(std::make_unique<juce::AudioParameterFloat>("LOWMIDPEAKQ", "Low Mid Q",juce::NormalisableRange<float>(0.1f, 10.0f, 0.05f, 1.0f), 1.0f));

	//Mid Peak
	param_layout.add(std::make_unique<juce::AudioParameterFloat>("MIDPEAKFREQ", "Mid Peak Freq",juce::NormalisableRange<float>(20.0f, 20000.f, 1.0f, 0.4f), 3000.0f));
	param_layout.add(std::make_unique<juce::AudioParameterFloat>("MIDPEAKGAIN", "Mid Gain",juce::NormalisableRange<float>(-24.0f, 24.0f, 0.5f, 1.0f), 0.0f));
	param_layout.add(std::make_unique<juce::AudioParameterFloat>("MIDPEAKQ", "Mid Q",juce::NormalisableRange<float>(0.1f, 10.0f, 0.05f, 1.0f), 1.0f));

	//Hi Shelf
	param_layout.add(std::make_unique<juce::AudioParameterFloat>("HISHELFFREQ", "Hi Shelf Freq",juce::NormalisableRange<float>(20.0f, 20000.f, 1.0f, 0.4f), 8000.0f));
	param_layout.add(std::make_unique<juce::AudioParameterFloat>("HISHELFGAIN", "Hi Shelf Gain",juce::NormalisableRange<float>(-24.0f, 24.0f, 0.5f, 1.0f), 0.0f));
	param_layout.add(std::make_unique<juce::AudioParameterFloat>("HISHELFQ", "Hi Shelf Q",juce::NormalisableRange<float>(0.1f, 10.0f, 0.05f, 1.0f), 1.0f));

	//Hi Cut Filter
	param_layout.add(std::make_unique<juce::AudioParameterFloat>("HICUTFREQ", "Hi Cut Freq",juce::NormalisableRange<float>(20.0f, 20000.f, 1.0f, 0.8f), 20000.0f));
	param_layout.add(std::make_unique<juce::AudioParameterChoice>("HICUTSLOPE", "Hi Cut Slope", choicesArray, 0.0f));


	return param_layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new Parametric_EQ_PluginAudioProcessor();
}
