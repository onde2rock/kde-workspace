/*
   Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _TRIGGERS_H_
#define _TRIGGERS_H_

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QUuid>

#include <kdemacros.h>
#include "khotkeysglobal.h"
#include "voicesignature.h"

#include "input.h"
#include "config-khotkeys.h"


class KConfig;

namespace KHotKeys
{

class Windowdef_list;
class Action_data;

class KDE_EXPORT Trigger
    {
    public:
        Trigger( Action_data* data_P );
        Trigger( KConfigGroup& cfg_P, Action_data* data_P );
        virtual ~Trigger();
        virtual void cfg_write( KConfigGroup& cfg_P ) const = 0;
        virtual Trigger* copy( Action_data* data_P ) const = 0;
        virtual const QString description() const = 0;
        static Trigger* create_cfg_read( KConfigGroup& cfg_P, Action_data* data_P );
        virtual void activate( bool activate_P ) = 0;
    protected:
        Action_data* const data;
    KHOTKEYS_DISABLE_COPY( Trigger );
    };

class KDE_EXPORT Trigger_list
    : public QList< Trigger* >
    {
    public:
        Trigger_list( const QString& comment_P ); // CHECKME nebo i data ?
        Trigger_list( KConfigGroup& cfg_P, Action_data* data_P );
        ~Trigger_list();
        void activate( bool activate_P );
        void cfg_write( KConfigGroup& cfg_P ) const;
        //! Some convenience typedef
        typedef QList< Trigger* >::Iterator Iterator;
        typedef QList< Trigger* >::ConstIterator ConstIterator;
        const QString& comment() const;
        Trigger_list* copy( Action_data* data_P ) const;
    private:
        QString _comment;
    KHOTKEYS_DISABLE_COPY( Trigger_list );
    };

class KDE_EXPORT Shortcut_trigger
    : public QObject, public Trigger
    {
    Q_OBJECT

    typedef Trigger base;
    public:
        Shortcut_trigger( 
            Action_data* data_P,
            const QString &text,
            const KShortcut& shortcut_P,
            const QUuid &uuid = QUuid::createUuid() );

        Shortcut_trigger( 
            KConfigGroup& cfg_P,
            Action_data* data_P );

        virtual ~Shortcut_trigger();
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual Shortcut_trigger* copy( Action_data* data_P ) const;
        virtual const QString description() const;
        const KShortcut& shortcut() const;
        virtual void activate( bool activate_P );

    public Q_SLOTS:

        void trigger();

    private:

        //! The shortcut
        KShortcut _shortcut;

        //! A persistent identifier for this shortcut
        QUuid _uuid;
    };

class KDE_EXPORT Window_trigger
    : public QObject, public Trigger
    {
    Q_OBJECT
    typedef Trigger base;
    public:
        enum window_action_t
            {
            WINDOW_APPEARS         = ( 1 << 0 ),
            WINDOW_DISAPPEARS      = ( 1 << 1 ),
            WINDOW_ACTIVATES       = ( 1 << 2 ),
            WINDOW_DEACTIVATES     = ( 1 << 3 )
            };
        Window_trigger( Action_data* data_P, Windowdef_list* windows_P, int window_actions_P );
        Window_trigger( KConfigGroup& cfg_P, Action_data* data_P );
        virtual ~Window_trigger();
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
#ifdef HAVE_COVARIANT_RETURN    // stupid gcc, it doesn't even warn it can't do this
        virtual Window_trigger* copy( Action_data* data_P ) const;
#else
        virtual Trigger* copy( Action_data* data_P ) const;
#endif
        virtual const QString description() const;
        const Windowdef_list* windows() const;
        bool triggers_on( window_action_t w_action_P ) const;
        virtual void activate( bool activate_P );
    protected: // CHECKME neco private ?
        Windowdef_list* _windows;
        int window_actions;
        void init();
        typedef QMap< WId, bool > Windows_map;
        Windows_map existing_windows;
        WId last_active_window;
    protected Q_SLOTS:
        void window_added( WId window_P );
        void window_removed( WId window_P );
        void active_window_changed( WId window_P );
        void window_changed( WId window_P, unsigned int dirty_P );
    protected:
        bool active;
    };

class KDE_EXPORT Gesture_trigger
    : public QObject, public Trigger
    {
    Q_OBJECT
    typedef Trigger base;
    public:
        Gesture_trigger( Action_data* data_P, const QString& gesture_P );
        Gesture_trigger( KConfigGroup& cfg_P, Action_data* data_P );
        virtual ~Gesture_trigger();
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual Trigger* copy( Action_data* data_P ) const;
        virtual const QString description() const;
        const QString& gesturecode() const;
        virtual void activate( bool activate_P );
    protected Q_SLOTS:
        void handle_gesture( const QString& gesture_P, WId window_P );
    private:
        QString _gesturecode;
    };


// FIXME: SOUND
#if 0
class KDE_EXPORT Voice_trigger
    : public QObject, public Trigger
    {
    Q_OBJECT
    typedef Trigger base;
    public:
        Voice_trigger( Action_data* data_P, const QString& Voice_P, const VoiceSignature & signature1_P, const VoiceSignature & signature2_P );
        Voice_trigger( KConfigGroup& cfg_P, Action_data* data_P );
        virtual ~Voice_trigger();
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual Trigger* copy( Action_data* data_P ) const;
        virtual const QString description() const;
        const QString& voicecode() const;
        virtual void activate( bool activate_P );
        VoiceSignature voicesignature( int ech ) const;
    public slots:
        void handle_Voice(  );
    private:
        QString _voicecode;
        VoiceSignature _voicesignature[2];
    };
#endif

//***************************************************************************
// Inline
//***************************************************************************

// Trigger

inline
Trigger::Trigger( Action_data* data_P )
    : data( data_P )
    {
    }

inline
Trigger::Trigger( KConfigGroup&, Action_data* data_P )
    : data( data_P )
    {
    }

inline
Trigger::~Trigger()
    {
    }

// Trigger_list

inline
Trigger_list::Trigger_list( const QString& comment_P )
    : QList< Trigger* >(), _comment( comment_P )
    {
    }

inline
const QString& Trigger_list::comment() const
    {
    return _comment;
    }

// Shortcut_trigger

inline
const KShortcut& Shortcut_trigger::shortcut() const
    {
    return _shortcut;
    }

// Window_trigger

inline
Window_trigger::Window_trigger( Action_data* data_P, Windowdef_list* windows_P,
     int window_actions_P )
    : Trigger( data_P ), _windows( windows_P ), window_actions( window_actions_P ),
      last_active_window( None ), active( false )
    {
    init();
    }

inline
const Windowdef_list* Window_trigger::windows() const
    {
    return _windows;
    }

inline
bool Window_trigger::triggers_on( window_action_t w_action_P ) const
    {
    return window_actions & w_action_P;
    }

// Gesture_trigger

inline
const QString& Gesture_trigger::gesturecode() const
    {
    return _gesturecode;
    }

// FIXME: SOUND
#if 0
// Voice_trigger
inline
const QString& Voice_trigger::voicecode() const
    {
    return _voicecode;
    }

inline
VoiceSignature Voice_trigger::voicesignature(int ech) const
    {
    return _voicesignature[ech-1];
    }
#endif 
} // namespace KHotKeys

#endif
