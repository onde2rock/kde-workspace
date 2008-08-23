/*
 *   Copyright 2008 Aike J Sommer <dev@aikesommer.name>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef KEPHAL_XRANDROUTPUTS_H
#define KEPHAL_XRANDROUTPUTS_H

#include "../backendoutputs.h"
#include "xrandr12/randrdisplay.h"


namespace kephal {

    class XRandROutputs;

    class XRandROutput : public BackendOutput {
        Q_OBJECT
        public:
            XRandROutput(XRandROutputs * parent, RROutput rrId);
            
            QString id();

            QSize size();
            QSize preferredSize();
            QList<QSize> availableSizes();
            QPoint position();
            bool isConnected();
            bool isActivated();
            QString vendor();
            int productId();
            unsigned int serialNumber();
            
            bool applyGeom(const QRect & rect);
            
            void _revert();
            void _deactivate();
            //void _activate();
            RROutput _id();
            void _changed();

        Q_SIGNALS:
            void outputConnected(kephal::Output * o);
            void outputDisconnected(kephal::Output * o);
            void outputActivated(kephal::Output * o);
            void outputDeactivated(kephal::Output * o);
            void outputResized(kephal::Output * o, QSize oldSize, QSize newSize);
            void outputMoved(kephal::Output * o, QPoint oldPosition, QPoint newPosition);
            
        private:
            RandROutput * output();
            void parseEdid();
            void saveAsPrevious();
            
            XRandROutputs * m_outputs;
            RROutput m_rrId;
            QString m_vendor;
            int m_productId;
            unsigned int m_serialNumber;
            QRect m_previousGeom;
            bool m_previousConnected;
            bool m_previousActivated;
    };
    

    class XRandROutputs : public BackendOutputs {
        Q_OBJECT
        public:
            XRandROutputs(QObject * parent, RandRDisplay * display);
            
            QList<Output *> outputs();
            void activateLayout(const QMap<Output *, QRect> & layout);
            
            RandROutput * output(RROutput rrId);
            RandRDisplay * display();
            
        public Q_SLOTS:
            void outputChanged(RROutput id, int changes);
            void pollState();
            
        private:
            void init();
            
            RandRDisplay * m_display;
            QMap<QString, XRandROutput *> m_outputs;
    };
    
}


#endif // KEPHAL_XRANDROUTPUTS_H

