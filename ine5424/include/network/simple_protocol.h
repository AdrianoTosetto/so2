// EPOS IP Protocol Declarations

#ifndef __simple_protocol_h
#define __simple_protocol_h

#include <system/config.h>

#ifdef __sp__

#include <utility/bitmap.h>
#include <machine/nic.h>

__BEGIN_SYS

class Simple_Protocol: private NIC<Ethernet>::Observer
{

public:
    typedef Ethernet::Address MAC_Address;
    typedef NIC_Common::Address<4> Address;
    // Buffers received by the NIC, eventually linked into a list
    typedef Ethernet::Buffer Buffer;
//    typedef Data_Observer<Buffer, Protocol> Observer;
    typedef Data_Observed<Buffer> Observed;
protected:
    template<unsigned int UNIT = 0>
    Simple_Protocol(unsigned int nic = UNIT);
public:
    ~Simple_Protocol();
    NIC<Ethernet> * nic() { return _nic; }
    const Address & address() const { return _address; }

    static Simple_Protocol * get_by_nic(unsigned int unit) {
        if(unit >= Traits<Ethernet>::UNITS) {
            db<Simple_Protocol>(WRN) << "Simple_Protocol::get_by_nic: requested unit (" << unit << ") does not exist!" << endl;
            return 0;
        } else
            return _networks[unit];
    }
    //static Buffer * alloc(const Address & to, const Protocol & prot, unsigned int once, unsigned int payload);
    static int send(Buffer * buf);

    static const unsigned int mtu() { return Ethernet::MTU; }

    static void attach(Data_Observer<Buffer> * obs) { _observed.attach(obs); }
    static void detach(Data_Observer<Buffer> * obs) { _observed.detach(obs); }

    friend Debug & operator<<(Debug & db, const Simple_Protocol & sp) {
        db << "{a=" << sp._address
           << ",nic=" << &sp._nic
           << "}";
        return db;
    }

private:
    void config_by_mac() { _address[sizeof(Address) -1] = _nic->address()[sizeof(MAC_Address) - 1]; }

    void update(Ethernet::Observed * obs, const Ethernet::Protocol & prot, Buffer * buf);

    static bool notify(Buffer * buf) { return _observed.notify(buf); }

    static void init(unsigned int unit);

protected:
    NIC<Ethernet> * _nic;
    Address _address;
    static Simple_Protocol * _networks[Traits<Ethernet>::UNITS];
    static Observed _observed; 
};

__END_SYS

#endif

#endif