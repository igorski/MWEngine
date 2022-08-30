/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2019 Igor Zinken - https://www.igorski.nl
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
#include "notifier.h"
#include "global.h"
#include <algorithm>

#ifdef USE_JNI

#include <jni/javabridge.h>

#endif

namespace MWEngine {
namespace Notifier
{
    std::map<int, std::vector<Observer*> > _observerMap;

    void registerObserver( int aNotificationType, Observer* aObserver )
    {
        // first check if given type was already registered
        std::map<int, std::vector<Observer*> >::iterator it = _observerMap.find( aNotificationType );
        bool typeRegistered = ( it != _observerMap.end());

        if ( !typeRegistered )
            _observerMap.insert( std::pair<int, std::vector<Observer*> >( aNotificationType,
                                                                          std::vector<Observer*>() ));

        it = _observerMap.find( aNotificationType );
        it->second.push_back( aObserver );
    }

    void unregisterObserver( int aNotificationType, Observer* aObserver )
    {
        std::map<int, std::vector<Observer*> >::iterator it = _observerMap.find( aNotificationType );

        if ( it != _observerMap.end() )
        {
            std::vector<Observer*> observers = it->second;

            if ( std::find( observers.begin(), observers.end(), aObserver ) != observers.end())
                observers.erase( std::find( observers.begin(), observers.end(), aObserver ));

            if ( _observerMap.size() == 0 )
                _observerMap.erase( _observerMap.find( aNotificationType ));
        }
    }

    void broadcast( int aNotificationType )
    {
#ifdef USE_JNI

        // broadcast over JNI to Java

        jmethodID native_method_id = JavaBridge::getJavaMethod( JavaAPIs::HANDLE_NOTIFICATION );

        if ( native_method_id != nullptr )
        {
            JNIEnv* env = JavaBridge::getEnvironment();

            if ( env != nullptr ) {
                env->CallStaticVoidMethod( JavaBridge::getJavaInterface(), native_method_id, aNotificationType );
            }
        }
#else
        // strictly native layer code

        std::map<int, std::vector<Observer*> >::iterator it = _observerMap.find( aNotificationType );

        if ( it != _observerMap.end() )
        {
            std::vector<Observer*> observers = it->second;

            if ( observers.size() > 0 )
            {
                for ( int i = 0, l = observers.size(); i < l; ++i )
                    observers.at( i )->handleNotification( aNotificationType );
            }
        }
#endif
    }

    void broadcast( int aNotificationType, int aNotificationValue )
    {
#ifdef USE_JNI

        // broadcast over JNI to Java

        jmethodID native_method_id = JavaBridge::getJavaMethod( JavaAPIs::HANDLE_NOTIFICATION_DATA );

        if ( native_method_id != nullptr )
        {
            JNIEnv* env = JavaBridge::getEnvironment();

            if ( env != nullptr ) {
                env->CallStaticVoidMethod( JavaBridge::getJavaInterface(), native_method_id,
                                           aNotificationType, aNotificationValue );
            }
        }
#else
        // strictly native layer code

        std::map<int, std::vector<Observer*> >::iterator it = _observerMap.find( aNotificationType );

        if ( it != _observerMap.end() )
        {
            std::vector<Observer*> observers = it->second;

            if ( observers.size() > 0 )
            {
                for ( int i = 0, l = observers.size(); i < l; ++i )
                    observers.at( i )->handleNotification( aNotificationType, aNotificationValue );
            }
        }
#endif
    }
}

} // E.O namespace MWEngine
