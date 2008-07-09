/*
Copyright (C) 2008 Grame

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef __JackAudioAdapterInterface__
#define __JackAudioAdapterInterface__

#include "ringbuffer.h"
#include "jack.h"
#include "JackError.h"
#include "JackResampler.h"
#include "JackFilters.h"
#include <samplerate.h>


namespace Jack
{

#define TABLE_MAX 100000

    struct Measure 
    {
        int delta;
        int time1;
        int time2;
        float r1;
        float r2;
        int pos1;
        int pos2;
    };

    struct MeasureTable 
    {

        Measure fTable[TABLE_MAX];
        int fCount;

        MeasureTable():fCount(0)
        {}

        void Write(int time1, int time2, float r1, float r2, int pos1, int pos2);
        void Save();
    
    };

	class JackAudioAdapterInterface
	{
    
		protected:

        #ifdef DEBUG
        	MeasureTable fTable;
        #endif
        
            int fCaptureChannels;
            int fPlaybackChannels;
            
            jack_nframes_t fBufferSize;
            jack_nframes_t fSampleRate;
             
            // DLL
            JackAtomicDelayLockedLoop fProducerDLL;
            JackAtomicDelayLockedLoop fConsumerDLL;
             
            JackResampler** fCaptureRingBuffer;
            JackResampler** fPlaybackRingBuffer;

            bool fRunning;
               
		public:
        
			JackAudioAdapterInterface(int input, int output, jack_nframes_t buffer_size, jack_nframes_t sample_rate)
                :fCaptureChannels(input), 
                fPlaybackChannels(output), 
                fBufferSize(buffer_size), 
                fSampleRate(sample_rate),
                fProducerDLL(buffer_size, sample_rate),
                fConsumerDLL(buffer_size, sample_rate),
                fRunning(false)
            {}
			virtual ~JackAudioAdapterInterface()
            {}
            
            void SetRingBuffers(JackResampler** input, JackResampler** output)
            {
                fCaptureRingBuffer = input;
                fPlaybackRingBuffer = output;
            }
             
            bool IsRunning() {return fRunning;}
            
            virtual void Reset() {fRunning = false;}
            void ResetRingBuffers();
            
            virtual int Open();
            virtual int Close();
            
            virtual int SetBufferSize(jack_nframes_t buffer_size)
            {
                fBufferSize = buffer_size;
                fConsumerDLL.Init(fBufferSize, fSampleRate);
                fProducerDLL.Init(fBufferSize, fSampleRate);
                return 0;
            }
            
            virtual int SetSampleRate(jack_nframes_t sample_rate)
            {
                fSampleRate = sample_rate;
                fConsumerDLL.Init(fBufferSize, fSampleRate);
                // Producer (Audio) keep the same SR
                return 0;
            }
      
            virtual void SetCallbackTime(jack_time_t callback_usec)
            {
                fConsumerDLL.IncFrame(callback_usec);
            }
            
            void ResampleFactor(jack_nframes_t& frame1, jack_nframes_t& frame2);
        
	};
}

#endif