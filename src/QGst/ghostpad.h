/*
    Copyright (C) 2010  George Kiagiadakis <kiagiadakis.george@gmail.com>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef QGST_GHOSTPAD_H
#define QGST_GHOSTPAD_H

#include "pad.h"

namespace QGst {

/*! \headerfile ghostpad.h <QGst/GhostPad>
 * \brief Wrapper class for GstGhostPad
 */
class QTGSTREAMER_EXPORT GhostPad : public Pad
{
    QGST_WRAPPER(GhostPad)
public:
    static GhostPadPtr create(const PadPtr & target, const char *name = NULL);
    static GhostPadPtr create(PadDirection direction, const char *name = NULL);

    PadPtr target() const;
    bool setTarget(const PadPtr & target);
};

}

QGST_REGISTER_TYPE(QGst::GhostPad)

#endif
