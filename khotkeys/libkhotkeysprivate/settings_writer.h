#ifndef SETTINGS_WRITER_H
#define SETTINGS_WRITER_H
/**
 * Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
 * Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 **/

#include "action_data/action_data_visitor.h"

#include <QtCore/QStack>

#include "settings.h"

class KConfigBase;
class KConfigGroup;

namespace KHotKeys {


/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class SettingsWriter : public ActionDataConstVisitor
    {

public:

    SettingsWriter(
            const Settings *settings,
            ActionState state,
            const QString &id = QString(),
            bool allowMerging = false);

    void exportTo(
            const ActionDataBase *element,
            KConfigBase &config);

    void writeTo(KConfigBase &cfg);

    virtual void visitActionDataBase(const ActionDataBase *base);

    virtual void visitActionData(const ActionData *group);

    virtual void visitActionDataGroup(const ActionDataGroup *group);

    virtual void visitGenericActionData(const Generic_action_data *data);

    virtual void visitMenuentryShortcutActionData(const MenuEntryShortcutActionData *data);

    virtual void visitSimpleActionData(const SimpleActionData *data);

private:

    const Settings *_settings;

    QStack<KConfigGroup*> _stack;

    ActionState _state;

    QString _importId;

    bool _allowMerging;

    bool _export;

    // Disable copying
    SettingsWriter(const SettingsWriter&);
    SettingsWriter& operator=(const SettingsWriter&);

    }; //SettingsWriter

} // namespace KHotKeys


#endif /* SETTINGS_WRITER_H */

