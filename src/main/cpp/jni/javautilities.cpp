/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Igor Zinken - http://www.igorski.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "javautilities.h"
#include "../audiobuffer.h"
#include "..//wavetable.h"
#include <utilities/samplemanager.h>
#include <utilities/tablepool.h>
#include <generators/wavegenerator.h>
#include <utilities/wavereader.h>
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

namespace MWEngine {

/* SampleManager hooks */

bool JavaUtilities::createSampleFromFile( jstring aKey, jstring aWAVFilePath )
{
    std::string thePath = JavaBridge::getString( aWAVFilePath );
    waveFile WAV = WaveReader::fileToBuffer( thePath );

    // error during loading of WAV file ?

    if ( WAV.buffer == nullptr )
        return false;

    SampleManager::setSample( JavaBridge::getString( aKey ), WAV.buffer, WAV.sampleRate );

    return true;
}

bool JavaUtilities::createSampleFromAsset( jstring aKey, jobject assetManager, jstring cacheDir, jstring assetName )
{
    std::string filename   = JavaBridge::getString( assetName );
    std::string tempFolder = JavaBridge::getString( cacheDir );

    // use asset manager to open asset by filename
    AAssetManager* mgr = AAssetManager_fromJava( JavaBridge::getEnvironment(), assetManager );

    if ( mgr == NULL )
        return false;

    AAsset* asset = AAssetManager_open( mgr, filename.c_str(), AASSET_MODE_UNKNOWN );

    if ( asset == NULL )
        return false;

    std::vector<char> buffer;

    off64_t length = AAsset_getLength64( asset );
    off64_t remaining = AAsset_getRemainingLength64(asset);
    size_t Mb = 1000 * 1024; // read assets in one megabyte chunks
    size_t currChunk;
    buffer.reserve( length );

    // TODO: hackaroni. Prior to Android NDK 26 we could read an asset directly
    // as a ByteArray (well, we still can) and read the WAV data from it (in WaveReader, this
    // now fails...) for now we do the wasteful thing by creating a temporary file...

    std::string tempFile = tempFolder + "/tmp";
    FILE* tmp = fopen( tempFile.c_str(), "w" );
    bool readUsingTempFile = ( tmp != 0 );
    int nb_read = 0;

    while ( remaining != 0 ) {
        //set proper size for our next chunk
        if ( remaining >= Mb )
            currChunk = Mb;
        else
            currChunk = remaining;

        char chunk[ currChunk ];

        // read next chunk and append to temporary buffer

        if (( nb_read = AAsset_read( asset, chunk, currChunk )) > 0 ) {

            if ( !readUsingTempFile )
                buffer.insert( buffer.end(), chunk, chunk + currChunk );
            else
                fwrite( chunk, nb_read, 1, tmp );

            remaining = AAsset_getRemainingLength64( asset );
        }
    }
    AAsset_close( asset );
    waveFile WAV;

    if ( readUsingTempFile ) {
        fclose( tmp );
        WAV = WaveReader::fileToBuffer( tempFile );
        remove( tempFile.c_str() );
    }
    else {
        WAV = WaveReader::byteArrayToBuffer( buffer );
    }

    // error during loading of WAV file ?

    if ( WAV.buffer == nullptr )
        return false;

    SampleManager::setSample( JavaBridge::getString( aKey ), WAV.buffer, WAV.sampleRate );

    return true;
}

void JavaUtilities::createSampleFromBuffer( jstring aKey, jint aBufferLength, jint aChannelAmount, jdoubleArray aBuffer, jdoubleArray aOptRightBuffer )
{
    AudioBuffer* sampleBuffer = new AudioBuffer( aChannelAmount, aBufferLength );

    int i = 0;

    // get a pointer to the Java array
    jdouble* c_array;
    c_array = JavaBridge::getEnvironment()->GetDoubleArrayElements( aBuffer, 0 );

    // exception checking
    if ( c_array == NULL )
        return;

    // copy buffer contents
    SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( 0 );

    for ( i = 0; i < aBufferLength; i++ )
        channelBuffer[ i ] = ( SAMPLE_TYPE ) c_array[ i ];

    // release the memory so Java can have it again
    JavaBridge::getEnvironment()->ReleaseDoubleArrayElements( aBuffer, c_array, 0 );

    // stereo ?

    if ( aChannelAmount == 2 )
    {
        c_array = JavaBridge::getEnvironment()->GetDoubleArrayElements( aOptRightBuffer, 0 );

        // exception checking
        if ( c_array == NULL )
            return;

        // copy buffer contents
        channelBuffer = sampleBuffer->getBufferForChannel( 1 );

        for ( i = 0; i < aBufferLength; i++ )
            channelBuffer[ i ] = ( SAMPLE_TYPE ) c_array[ i ];

        // release the memory so Java can have it again
        JavaBridge::getEnvironment()->ReleaseDoubleArrayElements( aOptRightBuffer, c_array, 0 );
    }
    SampleManager::setSample(
        JavaBridge::getString( aKey ), sampleBuffer, AudioEngineProps::SAMPLE_RATE
    );
}

/* TablePool hooks */

void JavaUtilities::cacheTable( jint tableLength, jint waveformType, jstring tableId )
{
    WaveTable* table = new WaveTable( tableLength, waveformType );

    // generate table contents
    WaveGenerator::generate( table, waveformType );

    TablePool::setTable( table, JavaBridge::getString( tableId ));
}

void JavaUtilities::cacheTable( jint tableLength, jint waveformType, jstring tableId, jdoubleArray aBuffer )
{
    WaveTable* table = new WaveTable( tableLength, waveformType );
    SAMPLE_TYPE* buffer  = table->getBuffer();

    // get a pointer to the Java array
    jdouble* c_array;
    c_array = JavaBridge::getEnvironment()->GetDoubleArrayElements( aBuffer, 0 );

    // exception checking
    if ( c_array == NULL )
        return;

    // copy buffer contents

    for ( int i = 0; i < tableLength; i++ )
        buffer[ i ] = ( SAMPLE_TYPE ) c_array[ i ];

    // release the memory so Java can have it again
    JavaBridge::getEnvironment()->ReleaseDoubleArrayElements( aBuffer, c_array, 0 );

    // store the table inside the pool
    TablePool::setTable( table, JavaBridge::getString( tableId ));
}

bool JavaUtilities::createTableFromFile( jstring tableId, jstring aWAVFilePath )
{
    std::string thePath = JavaBridge::getString( aWAVFilePath );
    WaveTable* table = WaveReader::fileToTable( thePath );

    // error during loading of WAV file ?

    if ( table == NULL )
        return false;

    // store the table inside the pool
    TablePool::setTable( table, JavaBridge::getString( tableId ));

    return true;
}

} // E.O namespace MWEngine
