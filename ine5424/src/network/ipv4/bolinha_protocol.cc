// EPOS ICMP Protocol Implementation
#include <network/bolinha_protocol.h>

#ifdef __ipv4__

__BEGIN_SYS

Bolinha_Protocol::Observed Bolinha_Protocol::_observed;
Bolinha_Protocol * Bolinha_Protocol::_networks[];

int Bolinha_Protocol::send(const Address::Local & from, const Address & to, const void *data, size_t size) {
    Bolinha_Protocol *bp = Bolinha_Protocol::get_by_nic(0);
    int bytes=0;
    int n = 3;
    bool status = false;
    Semaphore sem(0);
    Semaphore_Handler handler_a(&sem);
    Alarm *alarm_a = new Alarm(20*1000000, &handler_a, n);

    Frame *f = new Frame(from, &status, &data, 5);
    f->sem(&sem);

    while(!status && n > 0) {
        bytes = bp->nic()->send(to.bp(), bp->Prot_Bolinha, reinterpret_cast<void*> (f), size);
        sem.p();
        n--;
    }

    delete alarm_a;
    if (status) {
        db<Bolinha_Protocol>(WRN) << "Mensagem " << &status << " confirmada" << endl;
    } else  {
        db<Bolinha_Protocol>(WRN) << "Falha ao enviar mensagem " << &status << endl;
        bytes = 0;
    }
    return bytes;
}

int Bolinha_Protocol::receive(Buffer *buffer, Address * from, void* data, size_t size) {
    Bolinha_Protocol *bp = Bolinha_Protocol::get_by_nic(0);
    db<Bolinha_Protocol>(WRN) << "Mensagem " << buffer->frame()->data<char>() << " confirmada" << endl;
    Frame *f = reinterpret_cast<Frame*>(buffer->frame()->data<char>());
    memcpy(buffer, f->data<char>(), size);

    char ack_data = 'A';
    Frame *ack = new Frame(f->from(), f->id(), &ack_data, 0);
    ack->sem(f->sem());
    bp->nic()->send(from->bp(), bp->Prot_Bolinha, ack, size);
    bp->nic()->free(buffer);
    return size;
}

void Bolinha_Protocol::update(Observed *o, const Protocol& p, Buffer *b) {
    Frame *f = reinterpret_cast<Frame*>(b->frame()->data<char>());
    if (f->flags()) {
        db<Thread>(WRN) << "ACK " << f->id() << " recebido" << endl;
        // _status = 1;
        CPU::tsl<bool>(*(f->id()));
        f->sem()->v();
    }
    
    if (!notify(p, b)) b->nic()->free(b);
}
__END_SYS

#endif