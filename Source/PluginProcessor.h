/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ===========================================================		===================
*/

#pragma once

#include <JuceHeader.h>

struct ChainSettings
{
	float lowShelfQ{1.0f};
	float lowMidQ{1.0f};
	float midQ{1.0f};
	float hiShelfQ{1.0f};
	float lowShelfGainDB{0};
	float lowMidGainDB{0};
	float midGainDB;
	float hiShelfGainDB{0};
	float lowCutFreq{0};
	float hiCutFreq{0};
	float lowShelfFreq{0};
	float lowMidFreq{0};
	float midFreq{0};
	float hiShelfFreq{0};
	int lowCutSlope{0};
	int highCutSlope{0};
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class Parametric_EQ_PluginAudioProcessor : public juce::AudioProcessor
{
public:
	//==============================================================================
	Parametric_EQ_PluginAudioProcessor();
	~Parametric_EQ_PluginAudioProcessor() override;

	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

	//==============================================================================
	juce::AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;

	//==============================================================================
	const juce::String getName() const override;

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override;
	double getTailLengthSeconds() const override;

	//==============================================================================
	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	//==============================================================================
	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	static juce::AudioProcessorValueTreeState::ParameterLayout createParamLayout();
	juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Params", createParamLayout()};

private:

	enum ChainPos
	{
		LowCut,
		LowShelf,
		LowMidPeak,
		MidPeak,
		HiShelf,
		HiCut
	};

	//Low Cut > Low Shelf > Low Mid Peak > Mid Peak > Hi Shelf > Hi Cut
	using Filter = juce::dsp::IIR::Filter<float>;
	using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
	using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, Filter, Filter, Filter, CutFilter>;

	MonoChain leftChannel, rightChannel;

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Parametric_EQ_PluginAudioProcessor)
};
