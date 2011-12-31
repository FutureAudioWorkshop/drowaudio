/*
  ==============================================================================
  
  This file is part of the dRowAudio JUCE module
  Copyright 2004-12 by dRowAudio.
  
  ------------------------------------------------------------------------------
 
  dRowAudio can be redistributed and/or modified under the terms of the GNU General
  Public License (Version 2), as published by the Free Software Foundation.
  A copy of the license is included in the module distribution, or can be found
  online at www.gnu.org/licenses.
  
  dRowAudio is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
  
  ==============================================================================
*/

BEGIN_JUCE_NAMESPACE

AudioFilePlayerExt::AudioFilePlayerExt()
    : AudioFilePlayer()
{
    loopingAudioSource = new LoopingAudioSource (audioTransportSource, false);
    reversibleAudioSource = new ReversibleAudioSource (audioTransportSource, false);
    filteringAudioSource = new FilteringAudioSource (reversibleAudioSource, false);
    
    masterSource = filteringAudioSource;
}

AudioFilePlayerExt::~AudioFilePlayerExt()
{
    audioTransportSource->setSource (nullptr);
}

void AudioFilePlayerExt::setPlaybackSettings (SoundTouchProcessor::PlaybackSettings newSettings)
{
    soundTouchAudioSource->setPlaybackSettings (newSettings);
    reversibleAudioSource->setPlaybackRatio (newSettings.rate * newSettings.tempo);
    
    listeners.call (&Listener::audioFilePlayerSettingChanged, this, SoundTouchSetting);
}

void AudioFilePlayerExt::setPlayDirection (bool shouldPlayForwards)
{
    reversibleAudioSource->setPlayDirection (shouldPlayForwards);

    listeners.call (&Listener::audioFilePlayerSettingChanged, this, PlayDirectionSetting);
}

//==============================================================================
void AudioFilePlayerExt::setLoopTimes (double startTime, double endTime)
{
    loopingAudioSource->setLoopTimes (startTime, endTime);
}

void AudioFilePlayerExt::setLoopBetweenTimes (bool shouldLoop)
{
    loopingAudioSource->setLoopBetweenTimes (shouldLoop);
	listeners.call (&Listener::audioFilePlayerSettingChanged, this, LoopBeetweenTimesSetting);
}

void AudioFilePlayerExt::setPosition (double newPosition, bool ignoreAnyLoopBounds)
{
    if (ignoreAnyLoopBounds && audioFormatReaderSource != nullptr)
    {
        const double sampleRate = audioFormatReaderSource->getAudioFormatReader()->sampleRate;
        
        if (sampleRate > 0.0)
            loopingAudioSource->setNextReadPositionIgnoringLoop ((int64) (newPosition * sampleRate));
    }
    else
    {
        AudioFilePlayer::setPosition (newPosition);
    }
}

//==============================================================================
bool AudioFilePlayerExt::setSourceWithReader (AudioFormatReader* reader)
{
//	stop();
    masterSource = nullptr;
	audioTransportSource->setSource (nullptr);
    //reversibleAudioSource = nullptr;
    soundTouchAudioSource = nullptr;
    //loopingAudioSource = nullptr;
    bufferingAudioSource = nullptr;
    
	if (reader != nullptr)
	{										
		// we SHOULD let the AudioFormatReaderSource delete the reader for us..
		audioFormatReaderSource = new AudioFormatReaderSource (reader, true);
        bufferingAudioSource = new BufferingAudioSource (audioFormatReaderSource,
                                                         bufferingTimeSliceThread,
                                                         false,
                                                         65536);
        soundTouchAudioSource = new SoundTouchAudioSource (bufferingAudioSource);
        loopingAudioSource = new LoopingAudioSource (soundTouchAudioSource, false);
        audioTransportSource->setSource (loopingAudioSource,
                                         0, nullptr,
                                         reader->sampleRate);
        masterSource = filteringAudioSource;
        
        listeners.call (&Listener::fileChanged, this);

		return true;
	}
	
    setLibraryEntry (ValueTree::invalid);
    listeners.call (&Listener::fileChanged, this);

    return false;    
}

END_JUCE_NAMESPACE