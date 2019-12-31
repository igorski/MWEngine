/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2014 Igor Zinken - http://www.igorski.nl
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

import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Build;

/**
 * Created by IntelliJ IDEA.
 * User: igorzinken
 * Date: 03-06-13
 * Time: 13:16
 * To change this template use File | Settings | File Templates.
 */
public final class DevicePropertyCalculator
{
    public static boolean detectLowLatency( Context aContext )
    {
        // check for low latency audio
        PackageManager pm = aContext.getPackageManager();

        return pm.hasSystemFeature( PackageManager.FEATURE_AUDIO_LOW_LATENCY );
    }

    /*
        Beginning with API level 17 (Android platform version 4.2), an application can query for the native or optimal
        output sample rate and buffer size for the device's primary output stream. When combined with the feature test
        just mentioned, an app can now configure itself appropriately for lower latency output on devices that claim support.

        The recommended sequence is:

        Check for API level 9 or higher, to confirm use of OpenSL ES.
        Check for feature "android.hardware.audio.low_latency" using code such as this:
        import android.content.pm.PackageManager;
        ...
        PackageManager pm = getContext().getPackageManager();
        boolean claimsFeature = pm.hasSystemFeature(PackageManager.FEATURE_AUDIO_LOW_LATENCY);
        Check for API level 17 or higher, to confirm use of android.media.AudioManager.getProperty().
        Get the native or optimal output sample rate and buffer size for this device's primary output stream, using code such as this:
        import android.media.AudioManager;
        ...
        AudioManager am = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        String sampleRate = am.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE));
        String framesPerBuffer = am.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER));
        Note that sampleRate and framesPerBuffer are Strings. First check for null and then convert to int using Integer.parseInt().
        Now use OpenSL ES to create an AudioPlayer with PCM buffer queue data locator.
        The number of lower latency audio players is limited. If your application requires more than a few audio sources, consider mixing your audio at application level. Be sure to destroy your audio players when your activity is paused, as they are a global resource shared with other apps.
     */
    public static int getRecommendedSampleRate( Context aContext )
    {
        String SR_CHECK = null;

        // API level 17 available ?
        if ( android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1 )
        {
            AudioManager am = ( AudioManager ) aContext.getSystemService( Context.AUDIO_SERVICE );

            // Use the sample rate provided by AudioManager.getProperty(PROPERTY_OUTPUT_SAMPLE_RATE).
            // Otherwise your buffers take a detour through the system resampler.

            SR_CHECK = am.getProperty( AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE );
        }
        final int defaultSampleRate = 44100;

        return ( SR_CHECK != null ) ? Integer.parseInt( SR_CHECK ) : defaultSampleRate;
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
     *
     * @param aContext {Context}
     * @return {int}
     */
    public static int getRecommendedBufferSize( Context aContext )
    {
        // prepare Native Audio engine
        String BS_CHECK = null;

        // API level 17 available ?
        if ( android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1 )
        {
            AudioManager am = ( AudioManager ) aContext.getSystemService( Context.AUDIO_SERVICE );

            BS_CHECK = am.getProperty( AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER );
        }
        return ( BS_CHECK != null ) ? Integer.parseInt( BS_CHECK ) : AudioTrack.getMinBufferSize( getRecommendedSampleRate( aContext ),
                                                                                                  AudioFormat.CHANNEL_OUT_MONO,
                                                                                                  AudioFormat.ENCODING_PCM_16BIT );
    }
}
