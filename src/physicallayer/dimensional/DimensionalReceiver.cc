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

#include "DimensionalReceiver.h"
#include "DimensionalReception.h"
#include "DimensionalNoise.h"
#include "DimensionalUtils.h"
#include "BandListening.h"

using namespace physicallayer;

Define_Module(DimensionalReceiver);

void DimensionalReceiver::printToStream(std::ostream &stream) const
{
    stream << "dimensional receiver, "
           << "energyDetection = " << energyDetection << ", "
           << "sensitivity = " <<  sensitivity << ", "
           << "carrierFrequency = " << carrierFrequency << ", "
           << "bandwidth = " << bandwidth;
}

const INoise *DimensionalReceiver::computeNoise(const IListening *listening, const std::vector<const IReception *> *receptions, const INoise *backgroundNoise) const
{
    std::vector<ConstMapping *> receptionPowers;
    const DimensionalNoise *dimensionalBackgroundNoise = check_and_cast<const DimensionalNoise *>(backgroundNoise);
    receptionPowers.push_back(const_cast<ConstMapping *>(dimensionalBackgroundNoise->getPower()));
    for (std::vector<const IReception *>::const_iterator it = receptions->begin(); it != receptions->end(); it++)
    {
        const DimensionalReception *reception = check_and_cast<const DimensionalReception *>(*it);
        receptionPowers.push_back(const_cast<ConstMapping *>(reception->getPower()));
    }
    const simtime_t startTime = listening->getStartTime();
    const simtime_t endTime = listening->getEndTime();
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    Hz carrierFrequency = bandListening->getCarrierFrequency();
    Hz bandwidth = bandListening->getBandwidth();
    ConstMapping *listeningMapping = DimensionalUtils::createFlatMapping(startTime, endTime, carrierFrequency, bandwidth, W(0));

    // TODO: delme
    for (std::vector<ConstMapping *>::const_iterator it = receptionPowers.begin(); it != receptionPowers.end(); it++)
    {
        std::cout << "Interference power begin " << *it << endl;
        (*it)->print(std::cout);
        std::cout << "Interference power end" << endl;
    }

    ConcatConstMapping<std::plus<double> > *noisePower = new ConcatConstMapping<std::plus<double> >(listeningMapping, receptionPowers.begin(), receptionPowers.end(), false, Argument::MappedZero);
    std::cout << "Noise power begin " << endl;
    noisePower->print(std::cout);
    std::cout << "Noise power end" << endl;
    return new DimensionalNoise(listening->getStartTime(), listening->getEndTime(), carrierFrequency, bandwidth, noisePower);
}

double DimensionalReceiver::computeMinSNIR(const IReception *reception, const INoise *noise) const
{
    const DimensionalNoise *dimensionalNoise = check_and_cast<const DimensionalNoise *>(noise);
    const DimensionalReception *dimensionalReception = check_and_cast<const DimensionalReception *>(reception);
    std::cout << "Reception power begin " << endl;
    dimensionalReception->getPower()->print(std::cout);
    std::cout << "Reception power end" << endl;
    const ConstMapping *snirMapping = MappingUtils::divide(*dimensionalReception->getPower(), *dimensionalNoise->getPower());
    const simtime_t startTime = reception->getStartTime();
    const simtime_t endTime = reception->getEndTime();
    Argument start(DimensionSet::timeFreqDomain);
    Argument end(DimensionSet::timeFreqDomain);
    std::cout << "TIMES" << startTime << " " << endTime << endl;
    start.setTime(startTime);
    start.setArgValue(Dimension::frequency, carrierFrequency.get() - bandwidth.get() / 2);
    end.setTime(endTime);
    end.setArgValue(Dimension::frequency, carrierFrequency.get() + bandwidth.get() / 2);
    std::cout << "SNIR begin " << endl;
    snirMapping->print(std::cout);
    std::cout << "SNIR end" << endl;
    return MappingUtils::findMin(*snirMapping, start, end);
}