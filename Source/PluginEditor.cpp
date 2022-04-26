/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Parametric_EQ_PluginAudioProcessorEditor::Parametric_EQ_PluginAudioProcessorEditor(
	Parametric_EQ_PluginAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	setSize(1280, 600);
	

}

Parametric_EQ_PluginAudioProcessorEditor::~Parametric_EQ_PluginAudioProcessorEditor()
{
}

//==============================================================================
void Parametric_EQ_PluginAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

	g.setColour(juce::Colours::white);
	g.setFont(15.0f);
	g.drawFittedText("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void Parametric_EQ_PluginAudioProcessorEditor::resized()
{
}
