// EPOS IP Protocol Declarations

#ifndef __bolinha_protocol_h
#define __bolinha_protocol_h

#include <system/config.h>
#include <synchronizer.h>

#include <utility/bitmap.h>
#include <machine/nic.h>
#include <time.h>

__BEGIN_SYS

Semaphore *sem;

void timeout() {
    db<Thread>(WRN) << "Ocorreu um timeout" << endl;
    (*sem).v();
}

struct messages {
    int _status;
    int _id;
};
typedef struct messages messages;

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
        int bytes;// = _nic->send(_nic->broadcast(), Prot_Bolinha, data, size);
        int n = 3; // numero de tentativas 
        sem = &_sem;
        Function_Handler handler_a(&timeout);
        Alarm *alarm_a = new Alarm(2000000, &handler_a, 10000);

        /*Frame * frame = buf->frame()->data<Frame>();
        size_t s = (size >= sizeof(Frame)) ? sizeof(Frame) : size;
        memcpy(frame, data, size);*/
        db<Thread>(WRN) << "quem enviou " << addr() << "\n"
                       // << "quem recebeu " << to << "\n"
                        << "dado enviado " << data << endl;
        _nic->send(_nic->broadcast(), Prot_Bolinha, data, size);
        while(!this->_status && n > 0) {
            bytes = _nic->send(_nic->broadcast(), Prot_Bolinha, data, size);
            (*sem).p();
            n--;
        }
        delete alarm_a;
        if (_status) {
            db<Thread>(WRN) << "Mensagem confirmada" << endl;
        } else  {
            db<Thread>(WRN) << "timed out" << endl;
            bytes = 0;
        }
        return bytes;
    }
    int receive(void *buffer, size_t size) {
        Buffer *rec = updated();
        memcpy(buffer, rec->frame()->data<char>(), size);

        char ack[2];
        ack[0] = 'A'; // nosso ACK por agora é um char com c[0] = 'A'
        ack[1] = '\n';
        //Delay(20000000); // forçando um timeout
        db<Thread>(WRN) << "Mandando um ACK" << endl;
        int bytes = _nic->send(_nic->broadcast(), Prot_Bolinha, ack, size);
        
        _nic->free(rec);
        return size;
    }
    int send1(void *data, size_t size, Address& to) {
        Frame *f = new Frame(to, addr(), 42069, data, 5);
        _nic->send(to, Prot_Bolinha, f, size);
    }
    int receive1(void *buffer, size_t size) {
        Buffer *rec = updated();
        Frame *f = reinterpret_cast<Frame*>(rec->frame()->data<char>());
        // Frame *rframe = reinterpret_cast<Frame*>(vrframe);
        memcpy(buffer, f->data<char>(), size);
        _nic->free(rec);
        return size;
    }
    static bool notify(const Protocol& p, Buffer *b) {
        return _observed.notify(p, b);
    }
    void update(Observed *o, const Protocol& p, Buffer *b) {
        db<Thread>(WRN) << "passando pelo update do endereco " << addr() << endl;
        char *data = reinterpret_cast<char*>(b->frame()->data<char>());
        if (data[0] == 'A') { // Quando ocorre um timeout o proprio nodo que enviou o ACK printa "ACK Recebido"
            db<Thread>(WRN) << "ACK recebido" << endl;
            _status = 1;
            (*sem).v();
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
    } __attribute__((packed));

    class Frame: private Header {
    public: 
        Frame(Address to, Address from, unsigned short id, void* data, size_t len): 
            Header(from, id), _data(data), _len(len) {}
        typedef unsigned char Data[];
        void* _data;
        size_t _len;
        unsigned short id() {
            return _id;
        }
        Address from() {
            return _from;
        }
        template<typename T>
        T * data() { return reinterpret_cast<T *>(_data); }
    } __attribute__((packed));
protected:
    int _status;
    Semaphore _sem = Semaphore(0);
    NIC<Ethernet> * _nic;
    Address _address;
    static Observed _observed;
    static const unsigned int NIC_MTU = 1500;
    static const unsigned int Bolinha_MTU = NIC_MTU - sizeof(Header);
    static messages _messages[1000];

};



__END_SYS

#endif
