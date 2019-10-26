#include <communicator.h>
#include <time.h>
#include <network/bolinha_protocol.h>
using namespace EPOS;
typedef Bolinha_Protocol::Address Address;

OStream cout;

int main()
{
    cout << "NIC Test" << endl;
    Bolinha_Protocol::init(0);
    Bolinha_Protocol *bp = new Bolinha_Protocol();
    char data[1500];
    cout << "MAC " << bp->address().bp() << endl;
    if(bp->address().bp()[5] % 2) { // sender
        Delay (5000000);
        data[0] = 'H';
        data[1] = 'e';
        data[2] = 'l';
        data[3] = 'l';
        data[4] = 'o';
        data[5] = '\n';
        Address d = bp->address();
        Port<Bolinha_Protocol> * com;
        com = new Port<Bolinha_Protocol>(1);
        d[5]--;
        cout << "Dado Enviado: " << data << endl;
        com->send(d, &data, 1500);

    } else {
        Address from = bp->address();
        from[5]++;
        Port<Bolinha_Protocol> * com;
        com = new Port<Bolinha_Protocol>(1);
        com->receive(&from, &data, 1500);
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
