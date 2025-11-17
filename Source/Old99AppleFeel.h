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
        // Classic Mac OS 9 "Platinum" greys
        setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (0xffd4d4d4));

        // Default button face
        setColour (juce::TextButton::buttonColourId, juce::Colour (0xffb5b5b5));
        setColour (juce::TextButton::textColourOffId, juce::Colours::black);
        setColour (juce::TextButton::textColourOnId, juce::Colours::black);

        // Labels and text
        setColour (juce::Label::textColourId, juce::Colours::black);
        setColour (juce::Label::backgroundColourId, juce::Colour (0x00d4d4d4));

        // Toggle text
        setColour (juce::ToggleButton::textColourId, juce::Colours::black);

        // Sliders (if any)
        setColour (juce::Slider::trackColourId, juce::Colour (0xffb0b0b0));
        setColour (juce::Slider::thumbColourId, juce::Colour (0xff707070));
    }

    juce::Font getTextButtonFont (juce::TextButton&, int height) override
    {
        // Approximate Mac OS 9 feel: Lucida Grande / Charcoal style
        return juce::Font ("Lucida Grande", (float) height * 0.42f, juce::Font::plain);
    }

    juce::Font getLabelFont (juce::Label&) override
    {
        return juce::Font ("Lucida Grande", 13.0f, juce::Font::plain);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& background,
                               bool isHighlighted, bool isDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);

        auto base = background;
        if (isDown)
            base = base.darker (0.25f);
        else if (isHighlighted)
            base = base.brighter (0.08f);

        g.setColour (base);
        g.fillRect (bounds);

        auto light = juce::Colours::white;
        auto shadow = juce::Colour (0xff808080);
        auto darkShadow = juce::Colour (0xff606060);

        if (! isDown)
        {
            // top/left highlight, bottom/right shadow
            g.setColour (light);
            g.drawLine (bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getY());
            g.drawLine (bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom());

            g.setColour (darkShadow);
            g.drawLine (bounds.getX(), bounds.getBottom(), bounds.getRight(), bounds.getBottom());
            g.drawLine (bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getBottom());
        }
        else
        {
            // pressed: invert bevel
            g.setColour (darkShadow);
            g.drawLine (bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getY());
            g.drawLine (bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom());

            g.setColour (light);
            g.drawLine (bounds.getX(), bounds.getBottom(), bounds.getRight(), bounds.getBottom());
            g.drawLine (bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getBottom());
        }
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool isHighlighted, bool isDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();

        const float boxSize = 16.0f;
        juce::Rectangle<float> box (bounds.getX(),
                                    bounds.getCentreY() - boxSize * 0.5f,
                                    boxSize,
                                    boxSize);

        // Box background
        g.setColour (juce::Colour (0xffb5b5b5));
        g.fillRect (box);

        // Bevel
        auto light = juce::Colours::white;
        auto shadow = juce::Colour (0xff808080);
        auto darkShadow = juce::Colour (0xff606060);

        g.setColour (light);
        g.drawLine (box.getX(), box.getY(), box.getRight(), box.getY());
        g.drawLine (box.getX(), box.getY(), box.getX(), box.getBottom());

        g.setColour (darkShadow);
        g.drawLine (box.getX(), box.getBottom(), box.getRight(), box.getBottom());
        g.drawLine (box.getRight(), box.getY(), box.getRight(), box.getBottom());

        // Checkmark when ON
        if (button.getToggleState())
        {
            g.setColour (juce::Colours::black);
            g.drawLine (box.getX() + 3.0f, box.getCentreY(),
                        box.getX() + 7.0f, box.getBottom() - 4.0f, 1.5f);
            g.drawLine (box.getX() + 7.0f, box.getBottom() - 4.0f,
                        box.getRight() - 3.0f, box.getY() + 3.0f, 1.5f);
        }

        // Text on the right
        g.setColour (juce::Colours::black);
        g.setFont (juce::Font ("Lucida Grande", 13.0f, juce::Font::plain));
        g.drawText (button.getButtonText(),
                    box.getRight() + 6.0f,
                    bounds.getY(),
                    bounds.getWidth() - boxSize - 6.0f,
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
        playButton.setEnabled (false);

        addAndMakeVisible (&pauseButton);
        pauseButton.setButtonText ("Pause");
        pauseButton.onClick = [this] { pauseButtonClicked(); };
        pauseButton.setEnabled (false);

        addAndMakeVisible (&stopButton);
        stopButton.setButtonText ("Stop");
        stopButton.onClick = [this] { stopButtonClicked(); };
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
        auto w = getWidth();
        auto h = getHeight();

        // Base platinum grey
        auto base = juce::Colour (0xffd4d4d4);
        g.fillAll (base);

        // Subtle vertical pinstripes like classic Mac OS
        auto stripe = base.brighter (0.10f);
        g.setColour (stripe);

        for (int x = 0; x < w; x += 4)
            g.drawVerticalLine (x, 0.0f, (float) h);
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
