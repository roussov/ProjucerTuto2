/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             PlayingSoundFilesTutorial
 version:          3.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Plays audio files.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make

 type:             Component
 mainClass:        MainContentComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/


#pragma once

class AppleTahoeLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AppleTahoeLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (0xfff9f9fb));

        setColour (juce::TextButton::buttonColourId, juce::Colour (0xf3ffffff));
        setColour (juce::TextButton::textColourOffId, juce::Colour (0xff1c1c1e));
        setColour (juce::TextButton::textColourOnId, juce::Colour (0xff1c1c1e));

        setColour (juce::Label::textColourId, juce::Colour (0xff1c1c1e));
        setColour (juce::Label::backgroundColourId, juce::Colour (0x00ffffff));

        setColour (juce::ToggleButton::textColourId, juce::Colour (0xff1c1c1e));

        setColour (juce::Slider::trackColourId, juce::Colour (0xffe5e5ea));
        setColour (juce::Slider::thumbColourId, juce::Colour (0xff007aff));
    }

    juce::Font getTextButtonFont (juce::TextButton&, int height) override
    {
        return juce::Font ("SF Pro Rounded", (float) height * 0.43f, juce::Font::plain);
    }

    juce::Font getLabelFont (juce::Label&) override
    {
        return juce::Font ("SF Pro", 14.5f, juce::Font::plain);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& background,
                               bool isHighlighted, bool isDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        const float radius = 10.0f;

        auto base = background.withAlpha (isDown ? 0.85f : isHighlighted ? 0.92f : 0.88f);
        g.setColour (base);
        g.fillRoundedRectangle (bounds, radius);

        g.setColour (juce::Colour (0x22000000));
        g.drawRoundedRectangle (bounds, radius, 1.0f);

        if (isHighlighted)
        {
            g.setColour (juce::Colour (0x33ffffff));
            g.fillRoundedRectangle (bounds, radius);
        }
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool isHighlighted, bool isDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();

        float toggleWidth = 46.0f;
        float toggleHeight = 26.0f;

        juce::Rectangle<float> toggle (bounds.getX(),
                                       bounds.getCentreY() - (toggleHeight * 0.5f),
                                       toggleWidth,
                                       toggleHeight);

        bool on = button.getToggleState();

        auto trackColour = on ? juce::Colour (0xff34c759) : juce::Colour (0xffe5e5ea);
        auto knobColour = juce::Colours::white;

        g.setColour (trackColour);
        g.fillRoundedRectangle (toggle, toggleHeight * 0.5f);

        float knobSize = toggleHeight - 4.0f;
        float knobX = on ? (toggle.getRight() - knobSize - 2.0f)
                         : (toggle.getX() + 2.0f);

        juce::Rectangle<float> knob (knobX,
                                     toggle.getY() + 2.0f,
                                     knobSize,
                                     knobSize);

        g.setColour (knobColour);
        g.fillRoundedRectangle (knob, knobSize * 0.5f);

        g.setColour (juce::Colour (0x11000000));
        g.drawRoundedRectangle (knob, knobSize * 0.5f, 1.0f);

        g.setColour (juce::Colour (0xff1c1c1e));
        g.setFont (juce::Font ("SF Pro", 15.0f, juce::Font::plain));
        g.drawText (button.getButtonText(),
                    toggle.getRight() + 10.0f,
                    bounds.getY(),
                    bounds.getWidth() - toggleWidth - 10.0f,
                    bounds.getHeight(),
                    juce::Justification::centredLeft);
    }
};

//==============================================================================
class MainContentComponent   : public juce::AudioAppComponent,
                               public juce::ChangeListener,
                               public juce::Timer
{
public:
    MainContentComponent()
        : state (Stopped)
    {
        juce::LookAndFeel::setDefaultLookAndFeel (&tahoeTheme);

        addAndMakeVisible (&openButton);
        openButton.setButtonText ("Open...");
        openButton.onClick = [this] { openButtonClicked(); };

        addAndMakeVisible (&playButton);
        playButton.setButtonText ("Play");
        playButton.onClick = [this] { playButtonClicked(); };
        playButton.setColour (juce::TextButton::buttonColourId, juce::Colours::green);
        playButton.setEnabled (false);

        addAndMakeVisible (&pauseButton);
        pauseButton.setButtonText ("Pause");
        pauseButton.onClick = [this] { pauseButtonClicked(); };
        pauseButton.setColour (juce::TextButton::buttonColourId, juce::Colours::orange);
        pauseButton.setEnabled (false);

        addAndMakeVisible (&stopButton);
        stopButton.setButtonText ("Stop");
        stopButton.onClick = [this] { stopButtonClicked(); };
        stopButton.setColour (juce::TextButton::buttonColourId, juce::Colours::red);
        stopButton.setEnabled (false);

        addAndMakeVisible (&loopingToggle);
        loopingToggle.setButtonText ("Loop");
        loopingToggle.onClick = [this] { loopButtonChanged(); };

        addAndMakeVisible (&currentPositionLabel);
        currentPositionLabel.setText ("Stopped", juce::dontSendNotification);

        setSize (300, 200);

        formatManager.registerBasicFormats();
        transportSource.addChangeListener (this);

        setAudioChannels (2, 2);
        startTimer (20);
    }

    ~MainContentComponent() override
    {
        juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
        shutdownAudio();
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        if (readerSource.get() == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        transportSource.getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override
    {
        transportSource.releaseResources();
    }

    void paint (juce::Graphics& g) override
    {
        juce::Colour top (0xfff9f9fb);
        juce::Colour bottom (0xffededf0);
        g.setGradientFill (juce::ColourGradient (top, 0, 0, bottom, 0, (float) getHeight(), false));
        g.fillAll();
    }

    void resized() override
    {
        openButton          .setBounds (10, 10,  getWidth() - 20, 20);
        playButton          .setBounds (10, 40,  getWidth() - 20, 20);
        pauseButton         .setBounds (10, 70,  getWidth() - 20, 20);
        stopButton          .setBounds (10, 100, getWidth() - 20, 20);
        loopingToggle       .setBounds (10, 130, getWidth() - 20, 20);
        currentPositionLabel.setBounds (10, 160, getWidth() - 20, 20);
    }

    void changeListenerCallback (juce::ChangeBroadcaster* source) override
    {
        if (source == &transportSource)
        {
            if (state == Paused)
                return;

            if (transportSource.isPlaying())
                changeState (Playing);
            else
                changeState (Stopped);
        }
    }

    void timerCallback() override
    {
        if (transportSource.isPlaying())
        {
            juce::RelativeTime position (transportSource.getCurrentPosition());

            auto minutes = ((int) position.inMinutes()) % 60;
            auto seconds = ((int) position.inSeconds()) % 60;
            auto millis  = ((int) position.inMilliseconds()) % 1000;

            auto positionString = juce::String::formatted ("%02d:%02d:%03d", minutes, seconds, millis);

            currentPositionLabel.setText (positionString, juce::dontSendNotification);
        }
        else
        {
            if (state == Paused)
                currentPositionLabel.setText ("Paused", juce::dontSendNotification);
            else
                currentPositionLabel.setText ("Stopped", juce::dontSendNotification);
        }
    }

    void updateLoopState (bool shouldLoop)
    {
        if (readerSource.get() != nullptr)
            readerSource->setLooping (shouldLoop);
    }

private:
    AppleTahoeLookAndFeel tahoeTheme;

    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Paused,
        Stopping
    };

    void changeState (TransportState newState)
    {
        if (state != newState)
        {
            state = newState;

            switch (state)
            {
                case Stopped:
                    stopButton.setEnabled (false);
                    playButton.setEnabled (true);
                    pauseButton.setEnabled (false);
                    transportSource.setPosition (0.0);
                    break;

                case Starting:
                    playButton.setEnabled (false);
                    pauseButton.setEnabled (false);
                    transportSource.start();
                    break;

                case Playing:
                    stopButton.setEnabled (true);
                    pauseButton.setEnabled (true);
                    break;

                case Paused:
                    pauseButton.setEnabled (false);
                    playButton.setEnabled (true);
                    stopButton.setEnabled (true);
                    break;

                case Stopping:
                    transportSource.stop();
                    break;
            }
        }
    }

    void openButtonClicked()
    {
        auto wildcard = formatManager.getWildcardForAllFormats();

        chooser = std::make_unique<juce::FileChooser> ("Select an audio file to play...",
                                                       juce::File{},
                                                       wildcard);
        auto chooserFlags = juce::FileBrowserComponent::openMode
                          | juce::FileBrowserComponent::canSelectFiles;

        chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();

            if (file != juce::File{})
            {
                auto* reader = formatManager.createReaderFor (file);

                if (reader != nullptr)
                {
                    auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
                    transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
                    playButton.setEnabled (true);
                    pauseButton.setEnabled (false);
                    stopButton.setEnabled (false);
                    readerSource.reset (newSource.release());
                }
            }
        });
    }

    void playButtonClicked()
    {
        updateLoopState (loopingToggle.getToggleState());
        changeState (Starting);
        pauseButton.setEnabled (true);
    }

    void stopButtonClicked()
    {
        transportSource.stop();
        transportSource.setPosition (0.0);
        changeState (Stopped);
    }

    void pauseButtonClicked()
    {
        transportSource.stop();
        changeState (Paused);
    }

    void loopButtonChanged()
    {
        updateLoopState (loopingToggle.getToggleState());
    }

    //==========================================================================
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton pauseButton;
    juce::TextButton stopButton;
    juce::ToggleButton loopingToggle;
    juce::Label currentPositionLabel;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


