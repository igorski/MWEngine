/**
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

#include "aaudio_io.h"
#include "../global.h"
#include "../audioengine.h"
#include <assert.h>
#include <inttypes.h>
#include <utilities/debug.h>

#define CONV16BIT 32768
#define CONVMYFLT (1./32768.)

static const int32_t audioFormatEnum[] = {
    AAUDIO_FORMAT_INVALID,
    AAUDIO_FORMAT_UNSPECIFIED,
    AAUDIO_FORMAT_PCM_I16,
    AAUDIO_FORMAT_PCM_FLOAT,
};
static const int32_t audioFormatCount = sizeof(audioFormatEnum)/
                                        sizeof(audioFormatEnum[0]);

static const uint32_t sampleFormatBPP[] = {
    0xffff,
    0xffff,
    16, //I16
    32, //FLOAT
};
uint16_t SampleFormatToBpp(aaudio_format_t format) {
    for (int32_t i = 0; i < audioFormatCount; ++i) {
      if (audioFormatEnum[i] == format)
        return sampleFormatBPP[i];
    }
    return 0xffff;
}
static const char * audioFormatStr[] = {
    "AAUDIO_FORMAT_INVALID", // = -1,
    "AAUDIO_FORMAT_UNSPECIFIED", // = 0,
    "AAUDIO_FORMAT_PCM_I16",
    "AAUDIO_FORMAT_PCM_FLOAT",
};
const char* FormatToString(aaudio_format_t format) {
    for (int32_t i = 0; i < audioFormatCount; ++i) {
        if (audioFormatEnum[i] == format)
            return audioFormatStr[i];
    }
    return "UNKNOW_AUDIO_FORMAT";
}

int64_t timestamp_to_nanoseconds(timespec ts){
  return (ts.tv_sec * (int64_t) NANOS_PER_SECOND) + ts.tv_nsec;
}

int64_t get_time_nanoseconds(clockid_t clockid){
  timespec ts;
  clock_gettime(clockid, &ts);
  return timestamp_to_nanoseconds(ts);
}

/**
 * Every time the playback stream requires data this method will be called.
 *
 * @param stream the audio stream which is requesting data, this is the playStream_ object
 * @param userData the context in which the function is being called, in this case it will be the
 * AAudio instance
 * @param audioData an empty buffer into which we can write our audio data
 * @param numFrames the number of audio frames which are required
 * @return Either AAUDIO_CALLBACK_RESULT_CONTINUE if the stream should continue requesting data
 * or AAUDIO_CALLBACK_RESULT_STOP if the stream should stop.
 *
 * @see AAudio#dataCallback
 */
aaudio_data_callback_result_t dataCallback(AAudioStream *stream, void *userData,
                                           void *audioData, int32_t numFrames) {
  assert(userData && audioData);
  AAudio_IO *audioEngine = reinterpret_cast<AAudio_IO *>(userData);
  return audioEngine->dataCallback(stream, audioData, numFrames);
}

/**
 * If there is an error with a stream this function will be called. A common example of an error
 * is when an audio device (such as headphones) is disconnected. In this case you should not
 * restart the stream within the callback, instead use a separate thread to perform the stream
 * recreation and restart.
 *
 * @param stream the stream with the error
 * @param userData the context in which the function is being called, in this case it will be the
 * AAudio instance
 * @param error the error which occured, a human readable string can be obtained using
 * AAudio_convertResultToText(error);
 *
 * @see AAudio#errorCallback
 */
void errorCallback(AAudioStream *stream,
                   void *userData,
                   aaudio_result_t error) {
  assert(userData);
  AAudio_IO *audioEngine = reinterpret_cast<AAudio_IO *>(userData);
  audioEngine->errorCallback(stream, error);
}

AAudio_IO::AAudio_IO( int amountOfChannels ) {

  sampleChannels_ = amountOfChannels;
  sampleFormat_   = AAUDIO_FORMAT_PCM_I16;

  // Create the output stream. By not specifying an audio device id we are telling AAudio that
  // we want the stream to be created using the default playback audio device.
  createPlaybackStream();

  // created the buffer the output will be written into
  _enqueuedBuffer = new int16_t[ AudioEngineProps::BUFFER_SIZE * sampleChannels_ ]{ 0 };

  render = false;
}

AAudio_IO::~AAudio_IO(){

  closeOutputStream();
  delete _enqueuedBuffer;
}

/**
 * Set the audio device which should be used for playback. Can be set to AAUDIO_UNSPECIFIED if
 * you want to use the default playback device (which is usually the built-in speaker if
 * no other audio devices, such as headphones, are attached).
 *
 * @param deviceId the audio device id, can be obtained through an {@link AudioDeviceInfo} object
 * using Java/JNI.
 */
void AAudio_IO::setDeviceId(int32_t deviceId){

  playbackDeviceId_ = deviceId;

  // If this is a different device from the one currently in use then restart the stream
  int32_t currentDeviceId = AAudioStream_getDeviceId(playStream_);
  if (deviceId != currentDeviceId) restartStream();
}

/**
 * Creates a stream builder which can be used to construct streams
 * @return a new stream builder object
 */
AAudioStreamBuilder* AAudio_IO::createStreamBuilder() {

  AAudioStreamBuilder *builder = nullptr;
  aaudio_result_t result = AAudio_createStreamBuilder(&builder);
  if (result != AAUDIO_OK && !builder) {
    Debug::log( "AAudio_IO::Error creating stream builder: %s", AAudio_convertResultToText(result));
  }
  return builder;
}

/**
 * Creates an audio stream for playback. The audio device used will depend on playbackDeviceId_.
 */
void AAudio_IO::createPlaybackStream(){

  AAudioStreamBuilder* builder = createStreamBuilder();

  if (builder != nullptr){

    setupPlaybackStreamParameters(builder);

    aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &playStream_);

    if (result == AAUDIO_OK && playStream_ != nullptr){

      // check that we got PCM_I16 format
      if (sampleFormat_ != AAudioStream_getFormat(playStream_)) {
        Debug::log( "AAudio_IO::Sample format is not PCM_I16");
      }

      sampleRate_ = AAudioStream_getSampleRate(playStream_);
      framesPerBurst_ = AAudioStream_getFramesPerBurst(playStream_);

      // Set the buffer size to the burst size - this will give us the minimum possible latency
      AAudioStream_setBufferSizeInFrames(playStream_, framesPerBurst_);
      bufSizeInFrames_ = framesPerBurst_;

//      PrintAudioStreamInfo(playStream_);

      // Start the stream - the dataCallback function will start being called
      result = AAudioStream_requestStart(playStream_);
      if (result != AAUDIO_OK) {
        Debug::log( "AAudio_IO::Error starting stream. %s", AAudio_convertResultToText(result));
      }

      // Store the underrun count so we can tune the latency in the dataCallback
      playStreamUnderrunCount_ = AAudioStream_getXRunCount(playStream_);

    } else {
      Debug::log( "AAudio_IO::Failed to create stream. Error: %s", AAudio_convertResultToText(result));
    }

  AAudioStreamBuilder_delete(builder);

  } else {
    Debug::log( "AAudio_IO::Unable to obtain an AAudioStreamBuilder object");
  }
}

/**
 * Sets the stream parameters which are specific to playback, including device id and the
 * dataCallback function, which must be set for low latency playback.
 * @param builder The playback stream builder
 */
void AAudio_IO::setupPlaybackStreamParameters(AAudioStreamBuilder *builder) {
  AAudioStreamBuilder_setDeviceId(builder, playbackDeviceId_);
  AAudioStreamBuilder_setFormat(builder, sampleFormat_);
  AAudioStreamBuilder_setChannelCount(builder, sampleChannels_);

  // We request EXCLUSIVE mode since this will give us the lowest possible latency.
  // If EXCLUSIVE mode isn't available the builder will fall back to SHARED mode.
  AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_EXCLUSIVE);
  AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
  AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
  AAudioStreamBuilder_setDataCallback(builder, ::dataCallback, this);
  AAudioStreamBuilder_setErrorCallback(builder, ::errorCallback, this);
}

void AAudio_IO::closeOutputStream(){

  if (playStream_ != nullptr){
    aaudio_result_t result = AAudioStream_requestStop(playStream_);
    if (result != AAUDIO_OK){
      Debug::log( "AAudio_IO::Error stopping output stream. %s", AAudio_convertResultToText(result));
    }

    result = AAudioStream_close(playStream_);
    if (result != AAUDIO_OK){
      Debug::log( "AAudio_IO::Error closing output stream. %s", AAudio_convertResultToText(result));
    }
  }
}

/**
 * @see dataCallback function at top of this file
 */
aaudio_data_callback_result_t AAudio_IO::dataCallback(AAudioStream *stream,
                                                        void *audioData,
                                                        int32_t numFrames) {
  assert(stream == playStream_);

  int32_t underrunCount = AAudioStream_getXRunCount(playStream_);
  aaudio_result_t bufferSize = AAudioStream_getBufferSizeInFrames(playStream_);
  bool hasUnderrunCountIncreased = false;
  bool shouldChangeBufferSize = false;

  if (underrunCount > playStreamUnderrunCount_){
    playStreamUnderrunCount_ = underrunCount;
    hasUnderrunCountIncreased = true;
  }

  if (hasUnderrunCountIncreased && bufferSizeSelection_ == BUFFER_SIZE_AUTOMATIC){

    /**
     * This is a buffer size tuning algorithm. If the number of underruns (i.e. instances where
     * we were unable to supply sufficient data to the stream) has increased since the last callback
     * we will try to increase the buffer size by the burst size, which will give us more protection
     * against underruns in future, at the cost of additional latency.
     */
    bufferSize += framesPerBurst_; // Increase buffer size by one burst
    shouldChangeBufferSize = true;
  } else if (bufferSizeSelection_ > 0 && (bufferSizeSelection_ * framesPerBurst_) != bufferSize){

    // If the buffer size selection has changed then update it here
    bufferSize = bufferSizeSelection_ * framesPerBurst_;
    shouldChangeBufferSize = true;
  }

  if (shouldChangeBufferSize){
    Debug::log( "AAudio_IO::Setting buffer size to %d", bufferSize);
    bufferSize = AAudioStream_setBufferSizeInFrames(stream, bufferSize);
    if (bufferSize > 0) {
      bufSizeInFrames_ = bufferSize;
    } else {
      Debug::log( "AAudio_IO::Error setting buffer size: %s", AAudio_convertResultToText(bufferSize));
    }
  }

  //Debug::log( "AAudio_IO::numFrames %d, Underruns %d, buffer size %d", numFrames, underrunCount, bufferSize);

  // rendering requested ?

  if ( render ) {

    // invoke the render() method of the engine to collect audio
    // if it returns false, we can stop this stream (render thread has stopped)

    if ( !AudioEngine::render( numFrames ))
      return AAUDIO_CALLBACK_RESULT_STOP;
  }

  // write enqueued buffer into the output buffer (both interleaved int16_t)

  int16_t* outputBuffer = static_cast<int16_t*>( audioData );
  for ( int i = 0; i < numFrames; ++i ) {
    outputBuffer[ i ] = _enqueuedBuffer[ i ];
  }

  calculateCurrentOutputLatencyMillis(stream, &currentOutputLatencyMillis_);

  return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

/**
 * enqueue a buffer for rendering in the next callback
 * this is invoked by AudioEngine::render()
 *
 * buffer already contains interleaved samples, merely need to be converted
 * from floating point values into 16-bit shorts
 */
void AAudio_IO::enqueueBuffer( float* outputBuffer, int amountOfSamples ) {
    for ( int i = 0; i < amountOfSamples; ++i ) {
        _enqueuedBuffer[ i ] = ( int16_t )( outputBuffer[ i ] * CONV16BIT );
    }
}

/**
 * Calculate the current latency between writing a frame to the output stream and
 * the same frame being presented to the audio hardware.
 *
 * Here's how the calculation works:
 *
 * 1) Get the time a particular frame was presented to the audio hardware
 * @see AAudioStream_getTimestamp
 * 2) From this extrapolate the time which the *next* audio frame written to the stream
 * will be presented
 * 3) Assume that the next audio frame is written at the current time
 * 4) currentLatency = nextFramePresentationTime - nextFrameWriteTime
 *
 * @param stream The stream being written to
 * @param latencyMillis pointer to a variable to receive the latency in milliseconds between
 * writing a frame to the stream and that frame being presented to the audio hardware.
 * @return AAUDIO_OK or a negative error. It is normal to receive an error soon after a stream
 * has started because the timestamps are not yet available.
 */
aaudio_result_t
AAudio_IO::calculateCurrentOutputLatencyMillis(AAudioStream *stream, double *latencyMillis) {

  // Get the time that a known audio frame was presented for playing
  int64_t existingFrameIndex;
  int64_t existingFramePresentationTime;
  aaudio_result_t result = AAudioStream_getTimestamp(stream,
                                                     CLOCK_MONOTONIC,
                                                     &existingFrameIndex,
                                                     &existingFramePresentationTime);

  if (result == AAUDIO_OK){

    // Get the write index for the next audio frame
    int64_t writeIndex = AAudioStream_getFramesWritten(stream);

    // Calculate the number of frames between our known frame and the write index
    int64_t frameIndexDelta = writeIndex - existingFrameIndex;

    // Calculate the time which the next frame will be presented
    int64_t frameTimeDelta = (frameIndexDelta * NANOS_PER_SECOND) / sampleRate_;
    int64_t nextFramePresentationTime = existingFramePresentationTime + frameTimeDelta;

    // Assume that the next frame will be written at the current time
    int64_t nextFrameWriteTime = get_time_nanoseconds(CLOCK_MONOTONIC);

    // Calculate the latency
    *latencyMillis = (double) (nextFramePresentationTime - nextFrameWriteTime)
                           / NANOS_PER_MILLISECOND;
  } else {
    Debug::log( "AAudio_IO::Error calculating latency: %s", AAudio_convertResultToText(result));
  }

  return result;
}

/**
 * @see errorCallback function at top of this file
 */
void AAudio_IO::errorCallback(AAudioStream *stream,
                   aaudio_result_t error){

  assert(stream == playStream_);
  Debug::log( "AAudio_IO::errorCallback result: %s", AAudio_convertResultToText(error));

  aaudio_stream_state_t streamState = AAudioStream_getState(playStream_);
  if (streamState == AAUDIO_STREAM_STATE_DISCONNECTED){

    // Handle stream restart on a separate thread
    std::function<void(void)> restartStream = std::bind(&AAudio_IO::restartStream, this);
    streamRestartThread_ = new std::thread(restartStream);
  }
}

void AAudio_IO::restartStream(){

  Debug::log( "AAudio_IO::Restarting stream");

  if (restartingLock_.try_lock()){
    closeOutputStream();
    createPlaybackStream();
    restartingLock_.unlock();
  } else {
    Debug::log( "AAudio_IO::Restart stream operation already in progress - ignoring this request");
    // We were unable to obtain the restarting lock which means the restart operation is currently
    // active. This is probably because we received successive "stream disconnected" events.
    // Internal issue b/63087953
  }
}

double AAudio_IO::getCurrentOutputLatencyMillis() {
  return currentOutputLatencyMillis_;
}

void AAudio_IO::setBufferSizeInBursts(int32_t numBursts) {
  AAudio_IO::bufferSizeSelection_ = numBursts;
}