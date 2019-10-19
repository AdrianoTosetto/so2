// EPOS IP Protocol Declarations

#ifndef __bolinha_protocol_h
#define __bolinha_protocol_h

#include <system/config.h>
#include <synchronizer.h>

#include <utility/bitmap.h>
#include <machine/nic.h>

__BEGIN_SYS

class Bolinha_Protocol: private NIC<Ethernet>::Observer, Concurrent_Observer<Ethernet::Buffer, Ethernet::Protocol>
{
public:
    typedef Ethernet::Address MAC_Address;
    typedef Ethernet::Address Address;
    typedef Ethernet::Buffer Buffer;
    typedef Data_Observer<Buffer, Ethernet::Protocol> Observer;
    typedef Data_Observed<Buffer, Ethernet::Protocol> Observed;
    typedef Ethernet::Protocol Protocol;

    Protocol Prot_Bolinha = Ethernet::PROTO_SP;
    Bolinha_Protocol(): _nic(Traits<Ethernet>::DEVICES::Get<0>::Result::get(0)) {
        _nic->attach(this, Prot_Bolinha);
        
    }
    int send(const void *data, size_t size) {
        _nic->send(_nic->broadcast(), Prot_Bolinha, data, size);
    }
    int receive(void *buffer, size_t size) {
        Buffer *rec = updated();
        memcpy(buffer, rec->frame()->data<char>(), size);
        _nic->free(rec);
        return size;
    }
    static bool notify(const Protocol& p, Buffer *b) {
        return _observed.notify(p, b);
    }
    void update(Observed *o, const Protocol& p, Buffer *b) {
        db<Bolinha_Protocol>(WRN) << "Teste" << endl;
        Concurrent_Observer<Observer::Observed_Data, Protocol>::update(p, b);
    }
    const Address& addr() const {
        return _nic->address();
    }
    /*unsigned int MTU() {
        return Bolinha_MTU; // header precisa ser packed pra calcular certo o mtu
    }*/

    class Header {
    public:
        Header() {}
        Header(const Address& from, const Address & to, unsigned size) :
        _from(from), _to(to), _length((htons(size))) {}

        unsigned short length() const { return ntohs(_length); }
        void length(unsigned short length) { _length = htons(length); }
        unsigned short id() const { return ntohs(_id); }
        const Address & from() const { return _from; }
        void from(const Address & from){ _from = from; }

        const Address & to() const { return _to; }
        void to(const Address & to){ _to = to; }

        friend Debug & operator<<(Debug & db, const Header & h) {
            db << ",len=" << h.length()
               << ",id="  << h.id()
               << ",from=" << h._from
               << ",to=" << h._to
               << "}";
            return db;
        }

    public:    
        Address _to;
        Address _from;
        int unsigned short  _flags: 1; // por agora temos só a flag isACK, se houver necessidade de mais flags
                                      // aumentar o campo de bits desse atributo _flags
        unsigned short  _checksum; // verificar se o frame chegou com todos os bits corretos
        unsigned short  _length;
        unsigned short  _id;

    } __attribute__((packed));

    static const unsigned int MTU = 1500 - sizeof(Header);
    typedef unsigned char Data[MTU];

    class Packet: private Header 
    {
    public: 
        Packet() {}
        Header * header() { return this; }

        template <typename T>
        T * data() { return reinterpret_cast<T *>(&_data); }

        friend Debug & operator<<(Debug & db, const Packet & p) {
            db << "{head=" << reinterpret_cast<const Header &>(p) << ",data=" << p._data << "}";
            return db;
        }

    private:
        Data _data;
    } __attribute__((packed));

    typedef Packet PDU;

protected:
    NIC<Ethernet> * _nic;
    Address _address;
    static Observed _observed;
    static const unsigned int NIC_MTU = 1500;
    static const unsigned int Bolinha_MTU = NIC_MTU - sizeof(Header);
};



__END_SYS

#endif
