// EPOS IP Protocol Implementation

#include <utility/string.h>
#include <network/simple_protocol.h>
#include <system.h>

#ifdef __sp__

__BEGIN_SYS
// Class attributes
Simple_Protocol * Simple_Protocol::_networks[];
Simple_Protocol::Observed Simple_Protocol::_observed;

// Methods
Simple_Protocol::~Simple_Protocol()
{
    _nic->detach(this, NIC<Ethernet>::PROTO_SP);
}

int Simple_Protocol::send(Buffer * buf)
{
    db<Simple_Protocol>(TRC) << "Simple_Protocol::send(buf=" << buf << ")" << endl;

    return buf->nic()->send(buf); // implicitly releases the pool
}

void Simple_Protocol::update(NIC<Ethernet>::Observed * obs, const NIC<Ethernet>::Protocol & prot, Buffer * buf)
{
    db<Simple_Protocol>(TRC) << "Simple_Protocol::update(obs=" << obs << ",prot=" << hex << prot << dec << ",buf=" << buf << ")" << endl;

    buf->nic(_nic);

    // The Ethernet Frame in Buffer might have been padded, so we need to adjust it to the datagram length
    //buf->size(packet->length());
    db<Simple_Protocol>(INF) << "Simple_Protocol::update: notifying whole datagram" << endl;
    buf->nic()->free(buf);
}

__END_SYS

#endif