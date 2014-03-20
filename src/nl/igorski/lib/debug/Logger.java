package nl.igorski.lib.debug;

import android.util.Log;

/**
 * Created with IntelliJ IDEA.
 * User: izinken
 * Date: 20/03/14
 * Time: 13:20
 * To change this template use File | Settings | File Templates.
 */
public final class Logger
{
    public static String LOG_ID = "Logger";

    public static void setLogTag( String logId )
    {
        LOG_ID = logId;
    }

    /**
     * log a debug message to the logcat
     *
     * @param aMessage {string}
     */
    public static void log( String aMessage )
    {
        Log.d( LOG_ID, aMessage );
    }
}
