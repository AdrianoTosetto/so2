#include <network/bolinha_protocol.h>

#ifdef  __ipv4__

__BEGIN_SYS


template <unsigned int UNIT>
Bolinha_Protocol::Bolinha_Protocol(unsigned int unit): _nic(Traits<Ethernet>::DEVICES::Get<0>::Result::get(0))
{
    _address = Address(_nic->address(), 1);
    _nic->attach(this, NIC<Ethernet>::PROTO_SP);
}

__END_SYS

#endif