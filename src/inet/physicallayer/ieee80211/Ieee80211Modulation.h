//
// Copyright (C) 2005,2006 INRIA, 2014 OpenSim Ltd.
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
// Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
//

#ifndef __INET_IEEE80211MODULATION_H
#define __INET_IEEE80211MODULATION_H

#include "inet/physicallayer/common/ModulationType.h"

namespace inet {

namespace physicallayer {

/**
 * See IEEE Std 802.11-2007 section 18.2.2.
 */
enum Ieee80211PreambleMode {
    IEEE80211_PREAMBLE_LONG,
    IEEE80211_PREAMBLE_SHORT,
    IEEE80211_PREAMBLE_HT_MF,
    IEEE80211_PREAMBLE_HT_GF
};

class Ieee80211Modulation
{
  public:
    static ModulationType GetDsssRate1Mbps();
    static ModulationType GetDsssRate2Mbps();
    static ModulationType GetDsssRate5_5Mbps();
    static ModulationType GetDsssRate11Mbps();
    static ModulationType GetErpOfdmRate6Mbps();
    static ModulationType GetErpOfdmRate9Mbps();
    static ModulationType GetErpOfdmRate12Mbps();
    static ModulationType GetErpOfdmRate18Mbps();
    static ModulationType GetErpOfdmRate24Mbps();
    static ModulationType GetErpOfdmRate36Mbps();
    static ModulationType GetErpOfdmRate48Mbps();
    static ModulationType GetErpOfdmRate54Mbps();
    static ModulationType GetOfdmRate6Mbps();
    static ModulationType GetOfdmRate9Mbps();
    static ModulationType GetOfdmRate12Mbps();
    static ModulationType GetOfdmRate18Mbps();
    static ModulationType GetOfdmRate24Mbps();
    static ModulationType GetOfdmRate36Mbps();
    static ModulationType GetOfdmRate48Mbps();
    static ModulationType GetOfdmRate54Mbps();
    static ModulationType GetOfdmRate3MbpsCS10MHz();
    static ModulationType GetOfdmRate4_5MbpsCS10MHz();
    static ModulationType GetOfdmRate6MbpsCS10MHz();
    static ModulationType GetOfdmRate9MbpsCS10MHz();
    static ModulationType GetOfdmRate12MbpsCS10MHz();
    static ModulationType GetOfdmRate18MbpsCS10MHz();
    static ModulationType GetOfdmRate24MbpsCS10MHz();
    static ModulationType GetOfdmRate27MbpsCS10MHz();
    static ModulationType GetOfdmRate1_5MbpsCS5MHz();
    static ModulationType GetOfdmRate2_25MbpsCS5MHz();
    static ModulationType GetOfdmRate3MbpsCS5MHz();
    static ModulationType GetOfdmRate4_5MbpsCS5MHz();
    static ModulationType GetOfdmRate6MbpsCS5MHz();
    static ModulationType GetOfdmRate9MbpsCS5MHz();
    static ModulationType GetOfdmRate12MbpsCS5MHz();
    static ModulationType GetOfdmRate13_5MbpsCS5MHz();

    static simtime_t getPlcpHeaderDuration(ModulationType payloadMode, Ieee80211PreambleMode preamble);
    static simtime_t getPlcpPreambleDuration(ModulationType payloadMode, Ieee80211PreambleMode preamble);
    static simtime_t getPreambleAndHeader(ModulationType payloadMode, Ieee80211PreambleMode preamble);
    static simtime_t getPayloadDuration(uint64_t size, ModulationType payloadMode);
    static simtime_t calculateTxDuration(uint64_t size, ModulationType payloadMode, Ieee80211PreambleMode preamble);
    static simtime_t getSlotDuration(ModulationType payloadMode, Ieee80211PreambleMode preamble);
    static simtime_t getSifsTime(ModulationType payloadMode, Ieee80211PreambleMode preamble);
    static simtime_t get_aPHY_RX_START_Delay(ModulationType payloadMode, Ieee80211PreambleMode preamble);
    static ModulationType getPlcpHeaderMode(ModulationType payloadMode, Ieee80211PreambleMode preamble);
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211MODULATION_H

