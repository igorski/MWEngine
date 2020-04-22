/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2020 Igor Zinken - https://www.igorski.nl
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
package nl.igorski.mwengine.helpers;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Build;
import android.os.PowerManager;

public final class DevicePropertyCalculator
{
    /**
     * From Android 7/Nougat onwards, as long as the application has focus, it can run in
     * sustained performance mode, promising a consistent performance over long periods of time.
     */
    public static boolean setSustainedPerformanceMode( Activity activity ) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N
                && activity.getWindow() != null
                && (( PowerManager ) activity.getSystemService( Context.POWER_SERVICE )).isSustainedPerformanceModeSupported()) {
            activity.getWindow().setSustainedPerformanceMode(true);
            return true;
        }
        return false;
    }

    public static boolean supportsLowLatency( Context context ) {
        return context.getPackageManager().hasSystemFeature( PackageManager.FEATURE_AUDIO_LOW_LATENCY );
    }

    public static int getRecommendedSampleRate( Context context ) {
        String SR_CHECK = null;

        // API level 17 available ?  Use the sample rate provided by AudioManager.getProperty(PROPERTY_OUTPUT_SAMPLE_RATE)
        // to prevent the buffers from taking a detour through the system resampler
        if ( android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1 ) {
            SR_CHECK = (( AudioManager ) context.getSystemService( Context.AUDIO_SERVICE )).getProperty( AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE );
        }
        return ( SR_CHECK != null ) ? Integer.parseInt( SR_CHECK ) : 48000;
    }

    /**
     * retrieve the recommended buffer size for the device running the application
     * you can increase / decrease the buffer size for lower latency or higher stability, but
     * note you must use multiples of this recommendation !! Otherwise the buffer callback will
     * occasionally get two calls per timeslice which can cause glitching unless CPU usage is
     * really light
     *
     * some measurements ( combined w/ recommended sample rate above ):
     *
     * Samsung Galaxy S Plus on 2.3 Gingerbread : 4800 samples per buffer ( 44.1 kHz ) ADEQUATE   ( 75 samples == SOMEWHAT STABLE
     *                                                                                            ( unless under heavy stress ), 150 == PERFECT )
     * Samsung Galaxy Nexus 4.2.1 Jelly Bean    : 144 samples per buffer  ( 44.1 kHz ) INADEQUATE ( 288 samples == OK )
     * Samsung Galaxy S3 4.1.2 Jelly Bean       : 2048 samples per buffer ( ---- kHz ) ADEQUATE   ( 512 samples == OK )
     * Asus Nexus 7 on 4.2.2 Jelly Bean         : 512 samples per buffer  ( 44.1 kHz ) ACCURATE!  ( perhaps 384 ?? )
     * HTC One V 4.0.3 Ice Cream Sandwich       : 4800 samples per buffer ( 44.1 kHz ) ADEQUATE   ( 300 samples == OK )
     */
    public static int getRecommendedBufferSize( Context context ) {
        String BS_CHECK = null;

        // API level 17 available ?
        if ( android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1 ) {
            BS_CHECK = (( AudioManager ) context.getSystemService( Context.AUDIO_SERVICE )).getProperty( AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER );
        }
        return ( BS_CHECK != null ) ? Integer.parseInt( BS_CHECK ) : AudioTrack.getMinBufferSize(
            getRecommendedSampleRate( context ), AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT
        );
    }
}
