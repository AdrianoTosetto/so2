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

int Simple_Protocol::send(const void * data, unsigned int size)
{
    db<Simple_Protocol>(TRC) << "Simple_Protocol::send()" << endl;
    Simple_Protocol * sp = Simple_Protocol::get_by_nic(0);
    NIC<Ethernet> * nic = sp->nic();
    db<Simple_Protocol>(WRN) << "Enviando para " << nic->address() << " o dado " << &data << endl;
    return nic->send(nic->broadcast(), 0x8888, data, size); // implicitly releases the pool
}

int Simple_Protocol::receive(void * d, unsigned int s)
{
    unsigned char * data = reinterpret_cast<unsigned char *>(d);

    Simple_Protocol * sp = Simple_Protocol::get_by_nic(0);
    NIC<Ethernet> * nic = sp->nic();
    NIC<Ethernet>::Address src = nic->address();
    NIC<Ethernet>::Protocol prot;
    return nic->receive(&src, &prot, data, s); 
}

void Simple_Protocol::update(NIC<Ethernet>::Observed * obs, const NIC<Ethernet>::Protocol & prot, Buffer * buf)
{
    db<Simple_Protocol>(TRC) << "Simple_Protocol::update(obs=" << obs << ",prot=" << hex << prot << dec << ",buf=" << buf << ")" << endl;

    buf->nic(_nic);

    //The Ethernet Frame in Buffer might have been padded, so we need to adjust it to the datagram length
    //buf->size(packet->length());
    db<Simple_Protocol>(INF) << "Simple_Protocol::update: notifying whole datagram" << endl;
    buf->nic()->free(buf);
}

__END_SYS

#endif