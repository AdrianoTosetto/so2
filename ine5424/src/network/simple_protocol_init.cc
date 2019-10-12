#include <network/simple_protocol.h>
#ifdef __sp__

__BEGIN_SYS

template<unsigned int UNIT>
Simple_Protocol::Simple_Protocol(unsigned int unit)
:_nic(Traits<Ethernet>::DEVICES::Get<UNIT>::Result::get(unit)),
_address(Traits<IP>::Config<UNIT>::ADDRESS)
{
    db<Simple_Protocol>(TRC) << "Simple_Protocol::Simple_Protocol(nic=" << _nic << ") => " << this << endl;

    _nic->attach(this, NIC<Ethernet>::PROTO_SP);
}
void Simple_Protocol::init(unsigned int unit)
{
    db<Init, Simple_Protocol>(TRC) << "Simple_Protocol::init(u=" << unit << ")" << endl;
    _networks[unit] = new (SYSTEM) Simple_Protocol(unit);
}

__END_SYS
#endif