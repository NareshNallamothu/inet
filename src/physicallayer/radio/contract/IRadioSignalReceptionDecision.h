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

#ifndef __INET_IRADIOSIGNALRECEPTIONDECISION_H_
#define __INET_IRADIOSIGNALRECEPTIONDECISION_H_

#include "IPhysicalLayerIndication.h"
#include "IRadioSignalReception.h"

/**
 * This interface represents the result of a receiver's reception process.
 */
class INET_API IRadioSignalReceptionDecision : public IPhysicalLayerIndication, public IPrintableObject
{
    public:
        virtual ~IRadioSignalReceptionDecision() {}

        /**
         * Returns the corresponding reception that also specifies the receiver
         * and the received transmission. This function never returns NULL.
         */
        virtual const IRadioSignalReception *getReception() const = 0;

        /**
         * Returns whether synchronization is possible according to the physical
         * properties of the received radio signal.
         */
        virtual bool isSynchronizationPossible() const = 0;

        /**
         * Returns whether the receiver decided to attempt the synchronization or
         * it decided to ignore it.
         */
        virtual bool isSynchronizationAttempted() const = 0;

        /**
         * Returns whether the synchronization was completely successful or not.
         */
        virtual bool isSynchronizationSuccessful() const = 0;

        /**
         * Returns whether reception is possible according to the physical
         * properties of the received radio signal.
         */
        virtual bool isReceptionPossible() const = 0;

        /**
         * Returns whether the receiver decided to attempt the reception or
         * it decided to ignore it.
         */
        virtual bool isReceptionAttempted() const = 0;

        /**
         * Returns whether the reception was completely successful or not.
         */
        virtual bool isReceptionSuccessful() const = 0;
};

#endif