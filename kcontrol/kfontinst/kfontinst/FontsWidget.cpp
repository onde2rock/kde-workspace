////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontsWidget
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 29/04/2001
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include "FontsWidget.h"
#include "DiskFontListWidget.h"
#include "KfiGlobal.h"
#include "FontEngine.h"
#include "Config.h"
#include "SysConfigurer.h"
#include "KfiCmModule.h"
#include <qbitmap.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qsplitter.h>
#include <kprogress.h>
#include <kapp.h>
#include <klocale.h>
#include <stdlib.h>

CFontsWidget::CFontsWidget(QWidget *parent, const char *)
            : CFontsWidgetData(parent),
              itsSysConfigurer(NULL)
{
    itsDisk=new CDiskFontListWidget(itsSplitter);
    itsInstalled=new CInstalledFontListWidget(itsSplitter);
    itsSplitter->setOrientation(CKfiGlobal::cfg().getFontListsOrientation());
    connect(itsDisk, SIGNAL(fontSelected(const QString &, const QString &)), this, SLOT(preview(const QString &, const QString &)));
    connect(itsInstalled, SIGNAL(fontSelected(const QString &, const QString &)), this, SLOT(preview(const QString &, const QString &)));
    connect(itsInstalled, SIGNAL(configureSystem()), this, SLOT(configureSystem()));
    connect(itsInstalled, SIGNAL(directorySelected(const QString &)), itsDisk, SLOT(setDestDir(const QString &)));
    connect(itsInstalled, SIGNAL(fontSelected(const QString &, const QString &)), itsDisk, SLOT(setDestDirFromFontSel(const QString &, const QString &)));
    connect(itsInstalled, SIGNAL(fontMoved(const QString &, const QString &, const QString &)), itsDisk, SLOT(fontMoved(const QString &, const QString &, const QString &)));
    connect(itsInstalled, SIGNAL(dirMoved(const QString &, const QString &)), itsDisk, SLOT(dirMoved(const QString &, const QString &)));
    connect(itsDisk, SIGNAL(installFont(const QString &, const QString &)), itsInstalled, SLOT(addFont(const QString &, const QString &)));
    connect(itsDisk, SIGNAL(installDir(const QString &, const QString &)), itsInstalled, SLOT(addSubDir(const QString &, const QString &)));
    connect(itsDisk, SIGNAL(initProgress(const QString &, int)), SLOT(initProgress(const QString &, int)));
    connect(itsDisk, SIGNAL(progress(const QString &)), SLOT(progress(const QString &)));
    connect(itsDisk, SIGNAL(stopProgress()), SLOT(stopProgress()));
    connect(itsInstalled, SIGNAL(initProgress(const QString &, int)), SLOT(initProgress(const QString &, int)));
    connect(itsInstalled, SIGNAL(progress(const QString &)), SLOT(progress(const QString &)));
    connect(itsInstalled, SIGNAL(stopProgress()), SLOT(stopProgress()));
    setPreviewMode(true);
}

CFontsWidget::~CFontsWidget()
{
    if(itsSysConfigurer)
        delete itsSysConfigurer;
}

static const QString constDefaultPreviewStr("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz1234567890");
static const QString constNoPreviewStr(i18n(" No preview available"));
static const QString constPreviewTitle(i18n("Preview:"));

void CFontsWidget::setPreviewMode(bool on)
{
    if(on)
    {
        if(QString::null!=itsPreviousTitle)
        {
            itsBox->setTitle(itsPreviousTitle);

            if(QString::null!=itsPreviousStr)
                itsLabel->setText(itsPreviousStr);
            else
                if(itsPreviousPixmap.isNull())
                    itsLabel->setText(constNoPreviewStr);
                else
                    itsLabel->setPixmap(itsPreviousPixmap);
        }
        else
        {
            itsBox->setTitle(constPreviewTitle);
            itsLabel->setText(constNoPreviewStr);
        }

        itsProgress->hide();
    }
    else
    {
        itsPreviousTitle=itsBox->title();
        itsPreviousStr=itsLabel->text();

        QPixmap *pix=itsLabel->pixmap();
        if(pix)
            itsPreviousPixmap=*pix;

        itsBox->setTitle(i18n("Progress:"));
        itsLabel->setText("");
        if(itsProgress->maxValue()>0)
            itsProgress->show();
    }
}

void CFontsWidget::initProgress(const QString &title, int numSteps)
{
    emit progressActive(true);
    if(topLevelWidget())
        topLevelWidget()->setEnabled(false);
    itsProgress->setRange(0, numSteps);
    setPreviewMode(false);
    itsBox->setTitle(title);
    if(numSteps>0)
        itsProgress->setValue(0);
    itsLabel->setText("");
}

void CFontsWidget::progress(const QString &str)
{
    if(QString::null!=str)
        itsLabel->setText(" "+str);

    if(itsProgress->maxValue()>0)
        itsProgress->advance(1);

    kapp->processEvents();

    //
    // If KControl has been closed by the user, then exit the application now.
    // This is needed because we're in the middle of updating the font list, and can't continue
    // as they've been deleted...
    if(!CKfiCmModule::instance())
        ::exit(0);
}

void CFontsWidget::stopProgress()
{
    if(itsProgress->maxValue()>0)
    {
        itsProgress->setValue(0);
        itsProgress->repaint();
    }

    setPreviewMode(true);
    kapp->processEvents();

    if(topLevelWidget())
        topLevelWidget()->setEnabled(true);
    emit progressActive(false);
}

void CFontsWidget::configureSystem()
{
    kapp->processEvents();
    if(NULL==itsSysConfigurer)
    {
        itsSysConfigurer=new CSysConfigurer(this);
 
        connect(itsSysConfigurer, SIGNAL(initProgress(const QString &, int)), SLOT(initProgress(const QString &, int)));
        connect(itsSysConfigurer, SIGNAL(progress(const QString &)), SLOT(progress(const QString &)));
        connect(itsSysConfigurer, SIGNAL(stopProgress()), SLOT(stopProgress()));
        connect(itsSysConfigurer, SIGNAL(successful()), itsInstalled, SLOT(disableCfgButton()));
        connect(itsSysConfigurer, SIGNAL(successful()), SLOT(systemConfigured()));
    }
 
    itsSysConfigurer->go();
}

void CFontsWidget::systemConfigured()
{
    CKfiGlobal::cfg().clearModifiedDirs();
    emit configuredSystem();
}

void CFontsWidget::preview(const QString &dir, const QString &file)
{
    bool createdBitmap=false;

    if(CKfiGlobal::fe().openFont(dir+file, CFontEngine::PREVIEW|CFontEngine::NAME))
    {
        const int constPtSize=24;
        const int constRes=75;
        const int constBMWidth=1280;

        QPixmap pix=CKfiGlobal::fe().createPixmap(CKfiGlobal::cfg().getUseCustomPreviewStr() ?
                                                      CKfiGlobal::cfg().getCustomPreviewStr() :
                                                      constDefaultPreviewStr,
                                                  constBMWidth, itsLabel->size().height(), constPtSize, constRes,
                                                  itsBackground->backgroundColor().rgb());

        if(!pix.isNull())
        {
            itsLabel->setPixmap(pix);
            createdBitmap=true;
        }

        itsBox->setTitle(constPreviewTitle+" "+CKfiGlobal::fe().getFullName().latin1());
        itsBox->repaint();
        CKfiGlobal::fe().closeFont();
    }
    else
        itsBox->setTitle(constPreviewTitle);
    
    if(!createdBitmap)
        itsLabel->setText(constNoPreviewStr);
}

void CFontsWidget::rescan()
{
    itsDisk->setAdvanced(CKfiGlobal::cfg().getAdvancedMode());  // Don't change dirs...
    itsInstalled->rescan(CKfiGlobal::cfg().getAdvancedMode(), CKfiGlobal::cfg().getFontsDir());
}

void CFontsWidget::setOrientation(Qt::Orientation o)
{
    itsSplitter->setOrientation(o);
}

void CFontsWidget::scanDirs()
{
    itsDisk->scan();
    itsInstalled->scan();
}
#include "FontsWidget.moc"
