// EPOS IP Protocol Declarations

#ifndef __bolinha_protocol_h
#define __bolinha_protocol_h

#include <system/config.h>
#include <synchronizer.h>

#include <utility/bitmap.h>
#include <machine/nic.h>
#include <time.h>

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
    int send(void *data, size_t size, Address& to) {
        int bytes;
        int n = 3;
        Semaphore sem(0);
        Semaphore_Handler handler_a(&sem);
        Alarm *alarm_a = new Alarm(20*1000000, &handler_a, n);

        _m.lock();
        int id = next_id;
        pending_messages[id] = 1;
        while (pending_messages[(++next_id) % 1000]);
        _m.unlock();

        Frame *f = new Frame(to, addr(), id, data, 5);
        f->sem(&sem);

        bool status;

        _m.lock();
        status = pending_messages[id];
        _m.unlock();

        while(status && n > 0) {
            bytes = _nic->send(to, Prot_Bolinha, f, size);
            sem.p();
            n--;
            _m.lock();
            status = pending_messages[id];
            _m.unlock();
        }

        delete alarm_a;
        if (!status) {
            db<Thread>(WRN) << "Mensagem " << id << " confirmada" << endl;
        } else  {
            db<Thread>(WRN) << "Falha ao enviar mensagem " << id << endl;
            _m.lock();
            pending_messages[id] = 0;
            _m.unlock();
            bytes = 0;
        }
        return bytes;
    }
    int receive(void *buffer, size_t size) {
        Buffer *rec = updated();
        Frame *f = reinterpret_cast<Frame*>(rec->frame()->data<char>());
        memcpy(buffer, f->data<char>(), size);

        char* ack_data = "ACK\n";
        Frame *ack = new Frame(f->from(), addr(), f->id(), ack_data, 0);
        ack->flags(true);
        ack->sem(f->sem());
        _nic->send(f->from(), Prot_Bolinha, ack, size);

        _nic->free(rec);
        return size;
    }
    static bool notify(const Protocol& p, Buffer *b) {
        return _observed.notify(p, b);
    }
    void update(Observed *o, const Protocol& p, Buffer *b) {
        Frame *f = reinterpret_cast<Frame*>(b->frame()->data<char>());
        if (f->flags()) {
            db<Thread>(WRN) << "ACK " << f->id() << " recebido" << endl;
            // _status = 1;
            _m.lock();
            pending_messages[f->id()] = 0;
            _m.unlock();
            f->sem()->v();
        }
        Concurrent_Observer<Observer::Observed_Data, Protocol>::update(p, b);
    }
    const Address& addr() const {
        return _nic->address();
    }
    unsigned int MTU() {
        return Bolinha_MTU; // header precisa ser packed pra calcular certo o mtu
    }
    class Header {
    public:    
        //Address _to;
        Address _from;
        bool  _flags; // ACK
        unsigned short  _id;
        //unsigned short  _checksum; // verificar se o frame chegou com todos os bits corretos
        //unsigned short  _length;

        Header(Address from, unsigned short id): 
        _id(id), _from(from), _flags(false)
        {}

        void flags(bool ack) {
            _flags = ack;
        }
        void sem(Semaphore * sem) {
            _sem = sem;
        }
        unsigned short id() {
            return _id;
        }
        Address from() {
            return _from;
        }
        bool flags() {
            return _flags;
        }
        Semaphore * sem() {
            return _sem;
        }
    protected:
        Semaphore * _sem;
        
    } __attribute__((packed));

    class Frame: private Header {
    public: 
        Frame(Address to, Address from, unsigned short id, void* data, size_t len): 
            Header(from, id), _data(data), _len(len) {}
        typedef unsigned char Data[];
        void* _data;
        size_t _len;

        void flags(bool ack) {
            _flags = ack;
        }
        void sem(Semaphore * sem) {
            _sem = sem;
        }
        unsigned short id() {
            return _id;
        }
        Address from() {
            return _from;
        }
        bool flags() {
            return _flags;
        }
        Semaphore * sem() {
            return _sem;
        }

        template<typename T>
        T * data() { return reinterpret_cast<T *>(_data); }
    } __attribute__((packed));
protected:
    Mutex _m = Mutex();
    NIC<Ethernet> * _nic;
    Address _address;
    bool pending_messages[1000];
    int next_id = 0;
    static Observed _observed;
    static const unsigned int NIC_MTU = 1500;
    static const unsigned int Bolinha_MTU = NIC_MTU - sizeof(Header);

};



__END_SYS

#endif
