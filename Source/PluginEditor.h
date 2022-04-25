/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class Parametric_EQ_PluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Parametric_EQ_PluginAudioProcessorEditor (Parametric_EQ_PluginAudioProcessor&);
    ~Parametric_EQ_PluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:

    Parametric_EQ_PluginAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Parametric_EQ_PluginAudioProcessorEditor)
};
