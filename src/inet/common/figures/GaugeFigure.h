//
// Copyright (C) 2015 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_GAUGEFIGURE_H
#define __INET_GAUGEFIGURE_H

#include "inet/common/INETDefs.h"
#include "inet/common/INETMath.h"
#include "inet/common/figures/IMeterFigure.h"

// for the moment commented out as omnet cannot instatiate it from a namespace
//namespace inet {

#if OMNETPP_VERSION >= 0x500

class INET_API GaugeFigure : public cOvalFigure, public inet::IMeterFigure
{
    cPathFigure *needle;
    cTextFigure *valueFigure;
    cTextFigure *labelFigure;
    const char *label;
    const char *colorStrip;
    double minValue = 0;
    double maxValue = 100;
    double tickSize = 10;
    double value = NaN;

  protected:
    virtual void parse(cProperty *property) override;
    virtual bool isAllowedPropertyKey(const char *key) const override;
    void addChildren();
    void addColorCurve(const cFigure::Color& curveColor, double startAngle, double endAngle);
    void refresh();

  public:
    GaugeFigure(const char *name = nullptr);
    virtual ~GaugeFigure() {};

    virtual void setLabel(const char *label);
    virtual void setValue(int series, simtime_t timestamp, double value) override;
};

#else

// dummy figure for OMNeT++ 4.x
class INET_API GaugeFigure : public cGroupFigure {

};

#endif // omnetpp 5

// } // namespace inet

#endif // ifndef __INET_GAUGEFIGURE_H

