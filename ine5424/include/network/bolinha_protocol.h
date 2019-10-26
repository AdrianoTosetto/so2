// EPOS IP Protocol Declarations

#ifndef __bolinha_protocol_h
#define __bolinha_protocol_h

#include <system/config.h>
#include <synchronizer.h>


#ifdef __ipv4__

#include <utility/bitmap.h>
#include <machine/nic.h>
#include <time.h>

__BEGIN_SYS


class Bolinha_Protocol: private NIC<Ethernet>::Observer
{
public:
    static const bool connectionless = true;

    typedef Ethernet::Buffer Buffer;
    typedef Ethernet::Protocol Protocol;

    Protocol Prot_Bolinha = Ethernet::PROTO_SP;

    typedef Data_Observer<Buffer, Protocol> Observer;
    typedef Data_Observed<Buffer, Protocol> Observed;

    typedef unsigned short ID;
    typedef unsigned char Data[1500];
    
    class Address {
        public:
            typedef ID Local;
        public:
            Address() {}
            Address(const Ethernet::Address & mac, const Local id): _bp(mac), _prot_id(id){}

            const Ethernet::Address & bp() const { return _bp; }
            const ID & port() const { return _prot_id; }
            const Local & local() const { return _prot_id; }

            unsigned char & operator[](const size_t i) { return _bp[i]; }
        private:
            Ethernet::Address _bp;
            Local _prot_id;
    };

    class Header {
            //unsigned short  _checksum; // verificar se o frame chegou com todos os bits corretos
            //unsigned short  _length;
        public:
            Header(ID from, bool *id): 
            _id(id), _from(from), _flags(false)
            {}
            void flags(bool ack) {
                _flags = ack;
            }
            void sem(Semaphore * sem) {
                _sem = sem;
            }
            bool* id() {
                return _id;
            }
            ID from() {
                return _from;
            }
            bool flags() {
                return _flags;
            }
            Semaphore * sem() {
                return _sem;
            }
        public:    
            //Address _to;
            bool* _id;
            ID _from;
            bool _flags; // ACK
        protected:
            Semaphore * _sem;

    } __attribute__((packed));

    class Frame: private Header {
        public: 
            Frame(ID from, bool *id, void* data, size_t len): 
                Header(from, id), _data(data), _len(len) {}
            bool *id() {
                return _id;
            }
            ID from() {
                return _from;
            }
            bool flags() {
                return _flags;
            }
            void flags(bool ack) {
                _flags = ack;
            }
            void sem(Semaphore * sem) {
                _sem = sem;
            }
            Semaphore * sem() {
                return _sem;
            }
            template<typename T>
            T * data() { return reinterpret_cast<T *>(_data); }
        public:
            void* _data;
            size_t _len;
    } __attribute__((packed));
public:
    template <unsigned int UNIT=0>
    Bolinha_Protocol(unsigned int unit=0);

    Bolinha_Protocol(): _nic(Traits<Ethernet>::DEVICES::Get<0>::Result::get(0))
    {
        _address = Address(_nic->address(), 1);
        _nic->attach(this, NIC<Ethernet>::PROTO_SP);
    }

    static Bolinha_Protocol * get_by_nic(unsigned int unit) {
        if(unit >= Traits<Ethernet>::UNITS) {
            db<Bolinha_Protocol>(WRN) << "Bolinha_Protocol::get_by_nic: requested unit (" << unit << ") does not exist!" << endl;
            return 0;
        } else
            return _networks[unit];
    }

    ~Bolinha_Protocol() {
        _nic->detach(this, Prot_Bolinha);
    }
    static int send(const Address::Local & from, const Address & to, const void *data, size_t size);

    static int receive(Buffer *buffer, Address * from, void* data, size_t size);

    static void attach(Observer * obs, const Protocol & prot) { _observed.attach(obs, prot); }
    static void detach(Observer * obs, const Protocol & prot) { _observed.detach(obs, prot); }

    static bool notify(const Protocol& p, Buffer *b) {
        return _observed.notify(p, b);
    }

    void update(Observed *o, const Protocol& p, Buffer *b);

    NIC<Ethernet> *nic() {
        return _nic;
    }

    const Address & address() const {
        return _address;
    }

    unsigned int MTU() {
        return Bolinha_MTU; // header precisa ser packed pra calcular certo o mtu
    }

    static void init(unsigned int unit) {
        _networks[unit] = new (SYSTEM) Bolinha_Protocol();
    }
 
protected:
    Mutex _m = Mutex();
    NIC<Ethernet> * _nic;
    Address _address;
    bool pending_messages[1000];
    int next_id = 0;
    static Observed _observed;
    static const unsigned int NIC_MTU = 1500;
    static const unsigned int Bolinha_MTU = NIC_MTU - sizeof(Header);
    static Bolinha_Protocol * _networks[Traits<Ethernet>::UNITS];

};

__END_SYS

#endif

#endif