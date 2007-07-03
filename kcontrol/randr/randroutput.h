/*
 * Copyright (c) 2007 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __RANDROUTPUT_H__
#define __RANDROUTPUT_H__

#include <QObject>
#include <QString>
#include <QRect>
#include "randr.h"

#ifdef HAS_RANDR_1_2

class QAction;
class KConfig;

class RandROutput : public QObject
{
	Q_OBJECT

public:
	RandROutput(RandRScreen *parent, RROutput id);
	~RandROutput();

	void loadSettings();
	void handleEvent(XRROutputChangeNotifyEvent *event);
	void handlePropertyEvent(XRROutputPropertyNotifyEvent *event);

	QString name() const;

	/**
	 * Return the icon name according to the device type
	 */
	QString icon() const;

	CrtcList possibleCrtcs() const;
	RRCrtc currentCrtc() const;

	ModeList modes() const;
	RRMode currentMode() const;

	/**
	 * The list of supported sizes
	 */
	SizeList sizes() const;
	QRect rect() const;

	/**
	 * The list of refresh rates for the given size.
	 * If no size is specified, it will use the current size
	 */
	RateList refreshRates(QSize s = QSize()) const;

	/**
	 * The current refresh rate
	 */
	float refreshRate() const;

	/**
	 * Return all possible rotations for all CRTCs this output can be connected
	 * to.
	 */
	int rotations() const;

	/**
	 * Returns the curent rotation of the CRTC this output is currently connected to
	 */
	int currentRotation() const;

	bool isConnected() const;
	bool isActive() const;

	/**
	 * This function will check if the proposed config for the CRTC this output
	 * is connected to has changed from the original settings
	 *
	 * If this output is not connected to any CRTC, it will return false
	 */
	bool proposedChanged();
	bool applyProposed(int changes = 0xffffff, bool confirm = false);
	void proposeOriginal();

	void load(KConfig &config);
	void save(KConfig &config);

public slots:
	void slotChangeSize(QAction *action);
	void slotChangeRotation(QAction *action);
	void slotChangeRefreshRate(QAction *action);
	void slotDisable();

private slots:
	void slotCrtcChanged(RRCrtc c, int changes);

signals:
	void outputChanged(RROutput o, int changes);

protected:
	RandRCrtc *findEmptyCrtc();
	bool tryCrtc(RandRCrtc *crtc, int changes);

	/**
	 * The current crtc should never be set directly, it should be added through 
	 * this function to get the signals connected/disconnected correctly
	 */
	void setCurrentCrtc(RRCrtc c);

private:
	RROutput m_id;
	XRROutputInfo* m_info;
	QString m_name;

	CrtcList m_possibleCrtcs;
	RRCrtc m_currentCrtc;

	RRCrtc m_originalCrtc;
	RRCrtc m_proposedCrtc;

	//proposed stuff (mostly to read from the configuration
	QRect m_proposedRect;
	int m_proposedRotation;
	float m_proposedRate;

	ModeList m_modes;

	int m_rotations;
	bool m_connected;

	RandRScreen *m_screen;
};
#endif

#endif
