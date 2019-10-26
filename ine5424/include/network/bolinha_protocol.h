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



Semaphore *sem;
void timeout() {
    db<Thread>(WRN) << "Ocorreu um timeout" << endl;
    (*sem).v();
}


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
        public:
            Header(ID from, unsigned short id): 
            _id(id), _from(from), _flags(false)
            {}
            void flags(bool ack) {
                _flags = ack;
            }
        public:    
            unsigned short  _id;
            ID _from;
            bool  _flags; // ACK
    } __attribute__((packed));

    class Frame: private Header {
        public: 
            Frame(ID to, ID from, unsigned short id, void* data, size_t len): 
                Header(from, id), _data(data), _len(len) {}
            unsigned short id() {
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
            template<typename T>
            T * data() { return reinterpret_cast<T *>(_data); }
        public:
            void* _data;
            size_t _len;
    } __attribute__((packed));



public:

    Bolinha_Protocol(): _nic(Traits<Ethernet>::DEVICES::Get<0>::Result::get(0))
    {
        _address = Address(_nic->address(), 1);
        _nic->attach(this, NIC<Ethernet>::PROTO_SP);
        _networks[0] = new (SYSTEM) Bolinha_Protocol();
    }

    static Bolinha_Protocol * get_by_nic(unsigned int unit) {
        if(unit >= Traits<Ethernet>::UNITS) {
            db<Bolinha_Protocol>(WRN) << "Bolinha_Protocol::get_by_nic: requested unit (" << unit << ") does not exist!" << endl;
            return 0;
        } else
            return _networks[unit];
    }

    ~Bolinha_Protocol();
    static int send(const Address::Local & from, const Address & to, const void *data, size_t size) {
        Bolinha_Protocol *bp = Bolinha_Protocol::get_by_nic(0);
        int bytes = 0;
        int n = 3;
        sem = &bp->sem();
        Function_Handler handler_a(&timeout);
        Alarm *alarm_a = new Alarm(2000000, &handler_a, n);
        int id = 42069;

        Frame *f = new Frame(to.bp(), bp->address().local(), id, &data, 5);

        while(!bp->_status && n > 0) {
            bytes = bp->nic()->send(to.bp(), bp->Prot_Bolinha, f, size);
            bp->_sem.p();
            n--;
        }
        delete alarm_a;
        if (bp->_status) {
            db<Thread>(WRN) << "Mensagem " << id << " confirmada" << endl;
        } else  {
            db<Thread>(WRN) << "Falha ao enviar mensagem " << id << endl;
            bytes = 0;
        }
        Delay delay(2000000);
        return bytes;
    }

    static int receive(Buffer *buffer, Address * from, void* data, size_t size) {
        Bolinha_Protocol *bp = Bolinha_Protocol::get_by_nic(0);
        Frame *f = reinterpret_cast<Frame*>(buffer->frame()->data<char>());
        memcpy(buffer, f->data<char>(), size);

        char ack_data = 'A';
        Frame *ack = new Frame(f->from(), bp->address().local(), f->id(), &ack_data, 0);
        ack->flags(true);
        bp->nic()->send(f->from(), bp->Prot_Bolinha, ack, size);

        bp->nic()->free(buffer);
        return size;
    }

    static void attach(Observer * obs, const Protocol & prot) { _observed.attach(obs, prot); }
    static void detach(Observer * obs, const Protocol & prot) { _observed.detach(obs, prot); }

    static bool notify(const Protocol& p, Buffer *b) {
        return _observed.notify(p, b);
    }

    void update(Observed *o, const Protocol& p, Buffer *b) {
        Frame *f = reinterpret_cast<Frame*>(b->frame()->data<char>());
        if (f->flags()) {
            db<Thread>(WRN) << "ACK " << f->id() << " recebido" << endl;
            _status = 1;
            _sem.v();
        }
        
        if (!notify(p, b)) b->nic()->free(b);
    }

    NIC<Ethernet> *nic() {
        return _nic;
    }

    const Address & address() const {
        return _address;
    }

    unsigned int MTU() {
        return Bolinha_MTU; // header precisa ser packed pra calcular certo o mtu
    }

    Semaphore sem() {
        return _sem;
    }

protected:
    int _status;
    Semaphore _sem = Semaphore(0);
    NIC<Ethernet> * _nic;
    Address _address;
    static Observed _observed;
    static const unsigned int NIC_MTU = 1500;
    static const unsigned int Bolinha_MTU = NIC_MTU - sizeof(Header);
    static Bolinha_Protocol * _networks[Traits<Ethernet>::UNITS];

};



__END_SYS

#endif

#endif