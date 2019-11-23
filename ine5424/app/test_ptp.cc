#include <communicator.h>
#include <time.h>
#include <network/bolinha_protocol.h>
using namespace EPOS;
typedef Ethernet::Address Address;

OStream cout;


NIC<Ethernet> *nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
char data[1500];
int sender() {
    Delay (5*1000000);
    
    Bolinha_Protocol * bp = new Bolinha_Protocol(420);
    char *hello = "hello\n";
    Address d = bp->addr();
    Address bc = bp->broadcast();
    d[5]--;
    // cout << "Dado enviado: " << hello << ", para a porta: "<< 420 << endl;
    Bolinha_Protocol::add_time(1000000);
    bp->send(hello, 1500, d, 420);
    bp->send(hello, 1500, d, 420);
    bp->send(hello, 1500, d, 420);
    Delay(5*1000000);
    return 0;
}

int receiver() {
    // Delay (5000000);
    Bolinha_Protocol * bp = new Bolinha_Protocol(420, true);
    bp->receive(data, 1500);
    bp->receive(data, 1500);
    bp->receive(data, 1500);
    //cout << "Dado recebido: " << data << ", pela porta: " << 420 << endl;
    Delay(5*1000000);
    return 1;
}


int main()
{
    
    cout << "NIC Test" << endl;
    cout << "Meu endereco eh " << nic->address() << endl;
    if(nic->address()[5] == 9) { //slave
        cout << "começando sender (slave) " << endl;
        Thread *t = new Thread(&sender);
        t->join();
        cout << "Tempo do slave eh " << Bolinha_Protocol::time() << endl;
    } else {
        Delay(1000000);
        Thread *t = new Thread(&receiver);
        t->join();
        cout << "Tempo corrigido do mestre eh " << Bolinha_Protocol::time() << endl;
    }

    cout << "Kkk saindo" << endl;

    /*NIC<Ethernet>::Statistics stat = sp->nic()->statistics();
    cout << "Statistics\n"
         << "Tx Packets: " << stat.tx_packets << "\n"
         << "Tx Bytes:   " << stat.tx_bytes << "\n"
         << "Rx Packets: " << stat.rx_packets << "\n"
         << "Rx Bytes:   " << stat.rx_bytes << "\n";*/
    return 0;
}
