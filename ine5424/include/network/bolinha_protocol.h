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
    Bolinha_Protocol(short port = 5000): _nic(Traits<Ethernet>::DEVICES::Get<0>::Result::get(0)) {
        _nic->attach(this, Prot_Bolinha);
        bool res = CPU::finc<char>(_ports[port]);
        if (!res) {
            db<Thread>(WRN) << "Porta adquirida com sucesso!" << endl;
            _using_port = port;
        } else {
            db<Thread>(WRN) << "Falha ao adquirir porta!" << endl;
        }
    }
    virtual ~Bolinha_Protocol() {
        CPU::fdec<char>(_ports[_using_port]);
    }
    int send(void *data, size_t size, Address& to, short port_receiver) {
        int bytes;
        int n = 3;
        bool status = false;
        Semaphore sem(0);
        Semaphore_Handler handler_a(&sem);

        {
            Alarm alarm_a = Alarm(20*1000000, &handler_a, n);

            Frame *f = new Frame(to, addr(), &status, data, _using_port, port_receiver, 5);
            f->sem(&sem);

            while(!status && n > 0) {
                bytes = _nic->send(to, Prot_Bolinha, f, size);
                sem.p();
                n--;
            }
            delete f;
        }
        if (status) {
            db<Thread>(WRN) << "Mensagem " << &status << " confirmada" << endl;
        } else  {
            db<Thread>(WRN) << "Falha ao enviar mensagem " << &status << endl;
            bytes = 0;
        }
        return bytes;
    }
    int receive(void *buffer, size_t size) {
        Buffer *rec = updated();
        Frame *f = reinterpret_cast<Frame*>(rec->frame()->data<char>());
        memcpy(buffer, f->data<char>(), size);

        char* ack_data;// = "ACK\n";
        ack_data[0] = 'A';
        ack_data[1] = 'C';
        ack_data[2] = 'K';
        ack_data[3] = '\n';
        Frame *ack = new Frame(f->from(), addr(), f->id(), ack_data, _using_port, f->port_sender(), 0);
        ack->flags(true);
        ack->sem(f->sem());
        _nic->send(f->from(), Prot_Bolinha, ack, size);

        delete f;
        _nic->free(rec);
        return size;
    }
    static bool notify(const Protocol& p, Buffer *b) {
        return _observed.notify(p, b);
    }
    void update(Observed *o, const Protocol& p, Buffer *b) {
        Frame *f = reinterpret_cast<Frame*>(b->frame()->data<char>());
        if (f->port_receiver() != _using_port) {
            delete f;
            return;
        }
        if (f->flags()) {
            db<Thread>(WRN) << "ACK " << f->id() << " recebido" << endl;
            CPU::tsl<bool>(*(f->id()));
            f->sem()->v();
            delete f;
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

        Header(Address from, bool* id, short port_sender, short port_receiver): 
        _from(from), _id(id), _flags(false), _port_sender(port_sender), _port_receiver(port_receiver)
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
        Address from() {
            return _from;
        }
        bool flags() {
            return _flags;
        }
        Semaphore * sem() {
            return _sem;
        }
        //Address _to;
        Address _from;
        bool*  _id;
        bool  _flags; // ACK
        //unsigned short  _checksum; // verificar se o frame chegou com todos os bits corretos
        //unsigned short  _length;
    protected:
        Semaphore * _sem;
        short _port_sender;
        short _port_receiver;
    } __attribute__((packed));

    class Frame: private Header {
    public: 
        Frame(Address to, Address from, bool* id, void* data, short port_sender, short port_receiver,size_t len): 
            Header(from, id, port_sender, port_receiver), _len(len), _data(data) {}
        typedef unsigned char Data[];
        size_t _len;
        void* _data;

        void flags(bool ack) {
            _flags = ack;
        }
        void sem(Semaphore * sem) {
            _sem = sem;
        }
        bool* id() {
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
        short port_sender() {
            return _port_sender;
        }
        short port_receiver() {
            return _port_receiver;
        }
    } __attribute__((packed));
protected:
    NIC<Ethernet> * _nic;
    Address _address;
    static Mutex _m;
    static char _ports[1000];
    static Observed _observed;
    static const unsigned int NIC_MTU = 1500;
    static const unsigned int Bolinha_MTU = NIC_MTU - sizeof(Header);
    short _using_port = -1;

};

// bool Bolinha_Protocol::_ports[] = {0};

__END_SYS

#endif