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

#ifndef __INET_IERRORMODEL_H
#define __INET_IERRORMODEL_H

#include "ISNIR.h"

namespace inet {

namespace physicallayer {

class INET_API IErrorModel : public IPrintableObject
{
  public:
    virtual double computePacketErrorRate(const ISNIR *snir) const = 0;

    virtual double computeBitErrorRate(const ISNIR *snir) const = 0;

    virtual double computeSymbolErrorRate(const ISNIR *snir) const = 0;
};

// TODO: move ILayeredErrorModel after the layered radio is merged in
//class INET_API ILayeredErrorModel : public IErrorModel
//{
//  public:
//    virtual IReceptionPacketModel *computePacketModel(const ISNIR *snir) const = 0;
//
//    virtual IReceptionBitModel *computeBitModel(const ISNIR *snir) const = 0;
//
//    virtual IReceptionSymbolModel *computeSymbolModel(const ISNIR *snir) const = 0;
//
//    virtual IReceptionSampleModel *computeSampleModel(const ISNIR *snir) const = 0;
//
//    virtual IReceptionAnalogModel *computeAnalogModel(const ISNIR *snir) const = 0;
//};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IERRORMODEL_H
