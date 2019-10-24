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
    db<Thread>(WRN) << "TIMEOUT FUNCAO" << endl;
    (*sem).v();
}
void polling() {
    db<Thread>(WRN) << "Fazendo Polling" << endl;
    (*sem).v();
}

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
        int n = 3;
        sem = &_sem;
        Function_Handler handler_a(&timeout);
        Function_Handler handler_b(&polling);
        Alarm *alarm_a = new Alarm(2000000, &handler_a, 10000);
        //Alarm alarm_b(200, &handler_b, 10000);
        while(!this->_status && n > 0) {
            bytes = _nic->send(_nic->broadcast(), Prot_Bolinha, data, size);
            db<Thread>(WRN) << "antes do sem" << endl;
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
        //db<Thread>(WRN) << "depois do while" << endl;
        return bytes;
    }
    int receive(void *buffer, size_t size) {
        Buffer *rec = updated();
        memcpy(buffer, rec->frame()->data<char>(), size);
        char *test = reinterpret_cast<char*>(buffer);
        if(test[0] == 'A') {
            db<Thread>(WRN) << "Chegou um ACK" << endl;
            this->_status = 1;
            (*sem).v();
        } else {
            char ack[2];
            ack[0] = 'A';
            ack[1] = '\n';
            Delay(20000000);
            db<Thread>(WRN) << "Mandando um ACK" << endl;
            int bytes = _nic->send(_nic->broadcast(), Prot_Bolinha, ack, size);
        }
        _nic->free(rec);
        return size;
    }
    static bool notify(const Protocol& p, Buffer *b) {
        db<Thread>(WRN) << "notified" << endl;
        return _observed.notify(p, b);
    }
    void update(Observed *o, const Protocol& p, Buffer *b) {
        char *data = reinterpret_cast<char*>(b->frame()->data<char>());
        if (data[0] == 'A') {
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
        Address _to;
        Address _from;
        int unsigned short  _flags: 1; // por agora temos só a flag isACK, se houver necessidade de mais flags
                                      // aumentar o campo de bits desse atributo _flags
        unsigned short  _checksum; // verificar se o frame chegou com todos os bits corretos
        unsigned short  _length;
        unsigned short  _id;

    } __attribute__((packed));

    class Frame: private Header {
    public: 
        Frame() {

        }
        typedef unsigned char Data[];
    } __attribute__((packed));
protected:
    int _status;
    Semaphore _sem = Semaphore(0);
    NIC<Ethernet> * _nic;
    Address _address;
    static Observed _observed;
    static const unsigned int NIC_MTU = 1500;
    static const unsigned int Bolinha_MTU = NIC_MTU - sizeof(Header);
};



__END_SYS

#endif
