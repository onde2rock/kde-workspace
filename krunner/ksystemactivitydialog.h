#ifndef KSYSTEMACTIVITYDIALOG__H
#define KSYSTEMACTIVITYDIALOG__H

#ifndef Q_WS_WIN

#include "processui/ksysguardprocesslist.h"
#include "krunnersettings.h"

#include <QCloseEvent>
#include <QString>
#include <KConfigGroup>
#include <KDialog>
#include <KGlobal>
#include <KWindowSystem>

/** This creates a simple dialog box with a KSysguardProcessList
 *
 *  It remembers the size and position of the dialog, and sets
 *  the dialog to always be over the other windows
 */
class KSystemActivityDialog : public KDialog
{
    public:
        KSystemActivityDialog(QWidget *parent = NULL) : KDialog(parent), processList(0) {
            setWindowTitle(i18n("System Activity"));
            setWindowIcon(KIcon("utilities-system-monitor"));
            setButtons(KDialog::Close);
            setMainWidget(&processList);
            processList.setScriptingEnabled(true);

            setInitialSize(QSize(650, 420));
            KConfigGroup cg = KGlobal::config()->group("TaskDialog");
            restoreDialogSize(cg);
            processList.loadSettings(cg);
            // Since we default to forcing the window to be KeepAbove, if the user turns this off, remember this
            const bool keepAbove = KRunnerSettings::keepTaskDialogAbove();
            if (keepAbove) {
                KWindowSystem::setState(winId(), NET::KeepAbove );
            }
        }

        /** Show the dialog and set the focus
         *
         *  This can be called even when the dialog is already showing to bring it
         *  to the front again and move it to the current desktop etc.
         */
        void run() {
            show();
            raise();
            KWindowSystem::setOnDesktop(winId(), KWindowSystem::currentDesktop());
            KWindowSystem::forceActiveWindow(winId());
        }
        /** Set the text in the filter line in the process list widget */
        void setFilterText(const QString &filterText) {
            processList.filterLineEdit()->setText(filterText);
            processList.filterLineEdit()->setFocus();
        }
        virtual void closeEvent(QCloseEvent *event) {
            //When the user closes the dialog, save the position and the KeepAbove state
            KConfigGroup cg = KGlobal::config()->group("TaskDialog");
            saveDialogSize(cg);
            processList.saveSettings(cg);

            // Since we default to forcing the window to be KeepAbove, if the user turns this off, remember this
            bool keepAbove = KWindowSystem::windowInfo(winId(), NET::WMState).hasState(NET::KeepAbove);
            KRunnerSettings::setKeepTaskDialogAbove(keepAbove);
            KGlobal::config()->sync();

            event->accept();
        }

    private:
        KSysGuardProcessList processList;
};
#endif // not Q_WS_WIN

#endif // KSYSTEMACTIVITYDIALOG__H
