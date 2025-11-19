/*/*
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

class UnixMatrixLookAndFeel : public juce::LookAndFeel_V4
{
public:
   UnixMatrixLookAndFeel()
   {
       using namespace juce;

       // Fond global façon terminal Unix / Matrix
       setColour (ResizableWindow::backgroundColourId, background);

       // Boutons
       setColour (TextButton::buttonColourId,          background);
       setColour (TextButton::buttonOnColourId,        background);
       setColour (TextButton::textColourOnId,          textColour);
       setColour (TextButton::textColourOffId,         textColour);

       // Labels
       setColour (Label::textColourId,                 textColour);
       setColour (Label::backgroundColourId,           Colours::transparentBlack);

       // ToggleButton
       setColour (ToggleButton::textColourId,          textColour);

       // Sliders
       setColour (Slider::backgroundColourId,          background);
       setColour (Slider::thumbColourId,               darkShadow);
       setColour (Slider::trackColourId,               darkShadow);
       setColour (Slider::textBoxTextColourId,         textColour);

       // Scrollbars
       setColour (ScrollBar::thumbColourId,            darkShadow);
   }

   juce::Font getTextButtonFont (juce::TextButton&, int height) override
   {
       auto mono  = juce::Font::getDefaultMonospacedFontName();
       auto size  = (float) juce::jmin (height - 4, 14);
       return juce::Font (juce::FontOptions (mono, size, juce::Font::plain));
   }

   juce::Font getLabelFont (juce::Label&) override
   {
       auto mono = juce::Font::getDefaultMonospacedFontName();
       return juce::Font (juce::FontOptions (mono, 12.0f, juce::Font::plain));
   }

   void drawButtonBackground (juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool isHovered,
                              bool isDown) override
   {
       auto bounds = button.getLocalBounds().toFloat();
       auto base   = backgroundColour;

       if (isDown)
           base = base.darker (0.2f);
       else if (isHovered)
           base = base.brighter (0.1f);

       g.setColour (base);
       g.fillRect (bounds);

       g.setColour (lightEdge);
       g.drawRect (bounds, 1.0f);
   }

private:
   // Palette Matrix / terminal
   juce::Colour background { juce::Colours::black };
   juce::Colour darkShadow { juce::Colour::fromRGB (0, 120, 40) };  // vert plus sombre
   juce::Colour lightEdge  { juce::Colour::fromRGB (0, 220, 80) };  // vert lumineux
   juce::Colour textColour { juce::Colour::fromRGB (0, 255, 70) };  // vert Matrix
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
       juce::LookAndFeel::setDefaultLookAndFeel (&unixMatrixTheme);

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

       addAndMakeVisible (&volumeSlider);
       volumeSlider.setRange (0.0, 1.0, 0.01);
       volumeSlider.setValue (1.0);
       volumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
       volumeSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
       volumeSlider.onValueChange = [this]
       {
           targetGain = (float) volumeSlider.getValue();
       };

       addAndMakeVisible (&currentPositionLabel);
       currentPositionLabel.setText ("Stopped", juce::dontSendNotification);
       currentPositionLabel.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 12.0f, juce::Font::plain)));
       currentPositionLabel.setJustificationType (juce::Justification::centred);

       setSize (300, 200);

       formatManager.registerBasicFormats();

       transportSource.addChangeListener (this);

       setAudioChannels (2, 2);
       startTimer (50);
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

       if (auto* buffer = bufferToFill.buffer)
       {
           const auto gain = targetGain.get();
           if (gain != 1.0f)
               buffer->applyGain (bufferToFill.startSample, bufferToFill.numSamples, gain);
       }

       auto* buffer = bufferToFill.buffer;
       if (buffer != nullptr && bufferToFill.numSamples > 0 && buffer->getNumChannels() > 0)
       {
           const int numChannels = buffer->getNumChannels();
           const int startSample = bufferToFill.startSample;
           const int numSamples  = bufferToFill.numSamples;

           float maxSample = 0.0f;

           for (int ch = 0; ch < numChannels; ++ch)
           {
               auto* data = buffer->getReadPointer (ch, startSample);
               for (int i = 0; i < numSamples; ++i)
               {
                   auto s = std::abs (data[i]);
                   if (s > maxSample)
                       maxSample = s;
               }
           }

           lastLevel = maxSample;
       }
       else
       {
           lastLevel = 0.0f;
       }
   }

   void releaseResources() override
   {
       transportSource.releaseResources();
   }

   void paint (juce::Graphics& g) override
   {
       g.fillAll (juce::Colours::black); // fond terminal / Matrix

       auto bounds = getLocalBounds();
       const int meterHeight = 40;
       auto meterArea = bounds.removeFromBottom (meterHeight);

       g.setColour (juce::Colour::fromRGB (0, 255, 70)); // vert Matrix

       const int numBars = meterHistorySize;
       if (numBars > 0)
       {
           const float barWidth = (float) meterArea.getWidth() / (float) numBars;
           for (int i = 0; i < numBars; ++i)
           {
               const int index = (meterWriteIndex + i) % meterHistorySize;
               const float level = juce::jlimit (0.0f, 1.0f, meterLevels[index]);

               const int barH = (int) ((float) meterArea.getHeight() * level);
               const int x    = meterArea.getX() + (int) (barWidth * (float) i);
               const int y    = meterArea.getBottom() - barH;

               if (barH > 0)
                   g.fillRect (x, y, (int) juce::jmax (1.0f, barWidth - 1.0f), barH);
           }
       }
   }

   void resized() override
   {
       const int margin = 10;
       const int h      = 22;
       const int gap    = 5;
       int y = margin;

       openButton.setBounds           (margin, y, getWidth() - 2 * margin, h); y += h + gap;
       playButton.setBounds           (margin, y, getWidth() - 2 * margin, h); y += h + gap;
       pauseButton.setBounds          (margin, y, getWidth() - 2 * margin, h); y += h + gap;
       stopButton.setBounds           (margin, y, getWidth() - 2 * margin, h); y += h + gap;
       loopingToggle.setBounds        (margin, y, getWidth() - 2 * margin, h); y += h + gap;
       volumeSlider.setBounds         (margin, y, getWidth() - 2 * margin, h); y += h + gap;
       currentPositionLabel.setBounds (margin, y, getWidth() - 2 * margin, h);
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
       {
           const auto level = lastLevel.get();
           meterLevels[meterWriteIndex] = level;
           meterWriteIndex = (meterWriteIndex + 1) % meterHistorySize;
       }

       repaint();

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
   static constexpr int meterHistorySize = 64;
   float meterLevels[meterHistorySize] = {};
   int meterWriteIndex = 0;
   juce::Atomic<float> lastLevel { 0.0f };
   juce::Atomic<float> targetGain { 1.0f };

   UnixMatrixLookAndFeel unixMatrixTheme;

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
   juce::Slider volumeSlider;
   juce::Label currentPositionLabel;

   std::unique_ptr<juce::FileChooser> chooser;

   juce::AudioFormatManager formatManager;
   std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
   juce::AudioTransportSource transportSource;
   TransportState state;

   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

