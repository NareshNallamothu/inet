//
// Copyright (C) 2011 Juan Luis Garrote Molinero
// Copyright (C) 2013 Zsolt Prontvai
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

#include "RelayRSTP.h"

#include "Ethernet.h"
#include "RSTPAccess.h"
#include "MACAddressTableAccess.h"

Define_Module(RelayRSTP);

void RelayRSTP::initialize(int stage)
{
    if(stage==2)  // "auto" MAC addresses assignment takes place in stage 0. rstpModule gets address in stage 1.
    {
        //Obtaining AddressTable, rstp modules pointers
        AddressTable = MACAddressTableAccess().get();
        rstpModule = RSTPAccess().get();

        //Gets bridge MAC address from rstpModule
        address=rstpModule->getAddress();

        //Gets parameter values
        verbose=(bool) par("verbose");
        if(verbose==true)
            ev<<"Bridge MAC address "<<address.str()<<endl;
    }
}

void RelayRSTP::handleMessage(cMessage *msg)
{
    ev << "New message"<<endl;

    //rstp module update.
    rstpModule=RSTPAccess().get();
    int ArrivalRole=rstpModule->getPortRole(msg->getArrivalGate()->getIndex());

    //Sends to correct handler after verification
    if (dynamic_cast<BPDUieee8021D *>(msg) != NULL)
    {
        ev<<"BPDU";
        if(ArrivalRole!=DISABLED)
            handleIncomingFrame(check_and_cast<BPDUieee8021D *> (msg));
        else
            delete msg;
    }
    else if (dynamic_cast<Delivery *>(msg)!=NULL)
    { //Outgoing Frame. Delivery encapsulates the frame.
        ev<<"Outgoing Frame";
        handleIncomingFrame(check_and_cast<Delivery *>(msg));
    }
    else
    {
        ev<<"Data Frame";
        handleIncomingFrame(msg);
    }
}

void RelayRSTP::handleIncomingFrame(BPDUieee8021D *frame)
{
    if((frame->getDest()==address)||(frame->getDest()==MACAddress::STP_MULTICAST_ADDRESS))
    {
        Delivery * frame2= new Delivery();
        frame2->setArrivalPort(frame->getArrivalGate()->getIndex());
        frame2->encapsulate(frame);
        send(frame2,"RSTPGate$o"); //And sends the frame to the RSTP module.
    }
    else
    {
        ev<<"Wrong formated BPDU";
        delete frame;
    }
}

void RelayRSTP::handleIncomingFrame(Delivery *frame)
{
    int sendBy=frame->getSendByPort();
    if(sendBy>gateSize("GatesOut"))
    {
        ev<<"Error. Wrong sending port.";
    }
    else
    {
        cMessage * msg=frame->decapsulate();
        send(msg,"GatesOut",sendBy);
    }
    delete frame;
}

void RelayRSTP::handleIncomingFrame(cMessage * msg)
{  //Check port state and frame type.
    int ArrivalState=rstpModule->getPortState(msg->getArrivalGate()->getIndex());

    if(dynamic_cast<EtherFrame *>(msg) != NULL)
    {
        if((ArrivalState==FORWARDING)||(ArrivalState==LEARNING))
        {
            ev<<"Handling Ethernet Frame";
            handleEtherFrame(check_and_cast<EtherFrame *> (msg));
        }
        else
            delete msg;
            ev<<"Discarding Frame";
    }
    else
    {
        error("Not supported frame type");
    }
}

void RelayRSTP::handleEtherFrame(EtherFrame *frame)
{
    bool broadcast=false;
    int ArrivalState=rstpModule->getPortState(frame->getArrivalGate()->getIndex());
    int arrival=frame->getArrivalGate()->getIndex();
    EtherFrame * EthIITemp= frame;
    if(verbose==true)
    {
        AddressTable->printState();  //Shows AddressTable info.
    }

    //Learning in case of FORWARDING or LEARNING state.
    if((ArrivalState==FORWARDING)|| (ArrivalState==LEARNING))
    {
        if(EthIITemp->getSrc()!= MACAddress::UNSPECIFIED_ADDRESS )
        {
            AddressTable->updateTableWithAddress(arrival,EthIITemp->getSrc()); //Registers source at arrival gate.
        }
    }
    //Processing in case of FORWARDING state
    if(ArrivalState==FORWARDING)
    {
        int outputPort;
        if (EthIITemp->getDest().isBroadcast())
        {
            broadcastMsg(frame);
        }
        else if((outputPort=AddressTable->getPortForAddress(EthIITemp->getDest()))!=-1)
        {
            relayMsg(frame,outputPort);
        }
        else
        {
            broadcastMsg(frame);
        }
    }
    else
    {
        delete frame;
    }
}

void RelayRSTP::relayMsg(cMessage * msg, int outputPort)
{
    int arrival=msg->getArrivalGate()->getIndex();
    int outputState=rstpModule->getPortState(outputPort);
    if((arrival!=outputPort)&&(outputState==FORWARDING))
    {
        if(verbose==true)
            ev << "Sending frame to port " << outputPort << endl;
        send(msg,"GatesOut",outputPort);
    }
}

void RelayRSTP::broadcastMsg(cMessage * msg)
{
    int arrival=msg->getArrivalGate()->getIndex();
    int gates=gateSize("GatesOut");
    for(unsigned int i=0;i<gates;i++)
    {
        int outputState=rstpModule->getPortState(i);
        if((arrival!=i)&&(outputState==FORWARDING))
        {
            if(verbose==true)
                ev << "Sending frame to port " << i << endl;
            cMessage * msg2=msg->dup();
            send(msg2,"GatesOut",i);
        }
    }
    delete msg;
}