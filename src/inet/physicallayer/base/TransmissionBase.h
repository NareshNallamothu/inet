//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_TRANSMISSIONBASE_H
#define __INET_TRANSMISSIONBASE_H

#include "inet/physicallayer/contract/IRadio.h"
#include "inet/physicallayer/contract/ITransmission.h"

namespace inet {

namespace physicallayer {

class INET_API TransmissionBase : public virtual ITransmission, public virtual ITransmissionAnalogModel
{
  protected:
    const int id;
    const IRadio *transmitter;
    const cPacket *macFrame;
    const simtime_t startTime;
    const simtime_t endTime;
    const Coord startPosition;
    const Coord endPosition;
    const EulerAngles startOrientation;
    const EulerAngles endOrientation;

  public:
    TransmissionBase(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation);

    virtual int getId() const override { return id; }

    virtual void printToStream(std::ostream& stream) const override;

    virtual const IRadio *getTransmitter() const { return transmitter; }
    virtual const cPacket *getPhyFrame() const { return nullptr; }
    virtual const cPacket *getMacFrame() const { return macFrame; }

    virtual const simtime_t getStartTime() const { return startTime; }
    virtual const simtime_t getEndTime() const { return endTime; }
    virtual const simtime_t getDuration() const { return endTime - startTime; }

    virtual const Coord getStartPosition() const override { return startPosition; }
    virtual const Coord getEndPosition() const override { return endPosition; }

    virtual const EulerAngles getStartOrientation() const { return startOrientation; }
    virtual const EulerAngles getEndOrientation() const { return endOrientation; }

    virtual const ITransmissionAnalogModel *getAnalogModel() const { return check_and_cast<const ITransmissionAnalogModel *>(this); }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_TRANSMISSIONBASE_H

