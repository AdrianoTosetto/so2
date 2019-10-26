#include <communicator.h>
#include <time.h>
#include <network/bolinha_protocol.h>
using namespace EPOS;
typedef Ethernet::Address Address;

OStream cout;


NIC<Ethernet> *nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
char data[1500];

int sender(short port) {
    Bolinha_Protocol *bp = new Bolinha_Protocol(port);
    Delay (5000000);
    char *hello;
    if (port == 6000) {
       hello = "Hello\n";
    } else {
        hello = "olleH\n";
    }
    Address d = bp->addr();
    d[5]--;
    cout << "Dado enviado: " << hello << ", para a porta: "<< port << endl;
    return bp->send(hello, 1500, d, port);
}

int receiver(short port) {
    Bolinha_Protocol *bp = new Bolinha_Protocol(port);
    bp->receive(data, 1500);
    cout << "Dado recebido: " << data << endl;
    return 1;
}


int main()
{
    
    cout << "NIC Test" << endl;
    cout << "Meu endereco eh " << nic->address() << endl;
    if(nic->address()[5] % 2) { // sender
        sender(5000);
        sender(6000);
    } else {
       receiver(5000);
       receiver(6000);
    }

    /*NIC<Ethernet>::Statistics stat = sp->nic()->statistics();
    cout << "Statistics\n"
         << "Tx Packets: " << stat.tx_packets << "\n"
         << "Tx Bytes:   " << stat.tx_bytes << "\n"
         << "Rx Packets: " << stat.rx_packets << "\n"
         << "Rx Bytes:   " << stat.rx_bytes << "\n";*/
    return 0;
}
