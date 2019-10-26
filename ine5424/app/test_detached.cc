#include <communicator.h>
#include <time.h>
#include <network/bolinha_protocol.h>
using namespace EPOS;
typedef Ethernet::Address Address;

OStream cout;

int main()
{
    cout << "NIC Test" << endl;
    NIC<Ethernet> *nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
    cout << "Meu endereco eh " << nic->address() << endl;
    char data[1500];
    if(nic->address()[5] % 2) { // sender
        Bolinha_Protocol *bp = new Bolinha_Protocol(5000);
        Delay (5000000);
        data[0] = 'H';
        data[1] = 'e';
        data[2] = 'l';
        data[3] = 'l';
        data[4] = 'o';
        data[5] = '\n';
        char* hello = "Hello\n";
        Address d = bp->addr();
        d[5]--;
        cout << "Dado enviado: " << hello << endl;
        bp->send(hello, 1500, d, 5001);
    } else {
        Bolinha_Protocol *bp = new Bolinha_Protocol(5001);
        bp->receive(data, 1500);
        cout << "Dado recebido: " << data << endl;
    }

    /*NIC<Ethernet>::Statistics stat = sp->nic()->statistics();
    cout << "Statistics\n"
         << "Tx Packets: " << stat.tx_packets << "\n"
         << "Tx Bytes:   " << stat.tx_bytes << "\n"
         << "Rx Packets: " << stat.rx_packets << "\n"
         << "Rx Bytes:   " << stat.rx_bytes << "\n";*/
    return 0;
}
