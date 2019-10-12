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

    return nic->send(nic->address(), 0x8888, data, size); // implicitly releases the pool
}


int Simple_Protocol::receive(void * d, unsigned int s)
{
    unsigned char * data = reinterpret_cast<unsigned char *>(d);

    //db<TCP>(TRC) << "TCP::receive(buf=" << pool << ",d=" << d << ",s=" << s << ")" << endl;
    Simple_Protocol * sp = Simple_Protocol::get_by_nic(0);
    NIC<Ethernet> * nic = sp->nic();

    Buffer * pool = nic->alloc(nic->address(), NIC<Ethernet>::PROTO_SP, 0, s, 0);
    Buffer::Element * head = pool->link();
    unsigned int size = 0;

    for(Buffer::Element * el = head; el && (size <= s); el = el->next()) {
        Buffer * buf = el->object();

        memcpy(data, buf->frame()->data<void>(), s);
        db<TCP>(INF) << "TCP::receive:buf=" << buf << " => " << *buf << endl;

        data += s;
        size += s;
    }

    pool->nic()->free(pool);

    return size;
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