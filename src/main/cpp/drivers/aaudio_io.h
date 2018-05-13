/*
 * Copyright 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef AAUDIO_PLAYAUDIOENGINE_H
#define AAUDIO_PLAYAUDIOENGINE_H

#include <aaudio/AAudio.h>
#include <thread>
#include <mutex>

#define BUFFER_SIZE_AUTOMATIC 0
// Time constants
#define NANOS_PER_SECOND 1000000000L
#define NANOS_PER_MILLISECOND 1000000L

uint16_t SampleFormatToBpp(aaudio_format_t format);
/*
 * GetSystemTicks(void):  return the time in micro sec
 */
__inline__ uint64_t GetSystemTicks(void) {
    struct timeval Time;
    gettimeofday( &Time, NULL );

    return (static_cast<uint64_t>(1000000) * Time.tv_sec + Time.tv_usec);
}

void PrintAudioStreamInfo(const AAudioStream * stream);

int64_t timestamp_to_nanoseconds(timespec ts);
int64_t get_time_nanoseconds(clockid_t clockid);

class AAudio_IO {

public:
  AAudio_IO( int amountOfChannels );
  ~AAudio_IO();
  void setDeviceId(int32_t deviceId);
  void setBufferSizeInBursts(int32_t numBursts);
  aaudio_data_callback_result_t dataCallback(AAudioStream *stream,
                                             void *audioData,
                                             int32_t numFrames);
  void errorCallback(AAudioStream *stream,
                     aaudio_result_t  __unused error);
  double getCurrentOutputLatencyMillis();
  void enqueueBuffer( float* outputBuffer, int amountOfSamples );
  bool render;

private:

  int32_t playbackDeviceId_ = AAUDIO_UNSPECIFIED;
  int32_t sampleRate_;
  int16_t sampleChannels_;
  int16_t* _enqueuedBuffer;
  aaudio_format_t sampleFormat_;

  AAudioStream *playStream_;

  int32_t playStreamUnderrunCount_;
  int32_t bufSizeInFrames_;
  int32_t framesPerBurst_;
  double currentOutputLatencyMillis_ = 0;
  int32_t bufferSizeSelection_ = BUFFER_SIZE_AUTOMATIC;

  std::thread* streamRestartThread_;
  std::mutex restartingLock_;

  void createPlaybackStream();
  void closeOutputStream();
  void restartStream();

  AAudioStreamBuilder* createStreamBuilder();
  void setupPlaybackStreamParameters(AAudioStreamBuilder *builder);

  aaudio_result_t calculateCurrentOutputLatencyMillis(AAudioStream *stream, double *latencyMillis);
};

#endif
