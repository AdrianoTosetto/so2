#include <communicator.h>
#include <time.h>
#include <network/bolinha_protocol.h>
using namespace EPOS;
typedef Bolinha_Protocol::Address Address;

OStream cout;

int main()
{
    cout << "NIC Test" << endl;
    Bolinha_Protocol *bp = new Bolinha_Protocol();
    Communicator_Common<Bolinha_Protocol, true> * com;
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
        char* hello = "Hello\n";
        Ethernet::Address d = bp->address().bp();
        d[5]--;
        Address bp_addr = bp->address();
        com = new Communicator_Common<Bolinha_Protocol, true>(bp_addr.local());
        cout << "Dado enviado: " << hello << endl;
        com->send(bp->address().local(), d, &data, 1500);
    } else {
        Ethernet::Address d = bp->address().bp();
        d[5]++;
        com = new Communicator_Common<Bolinha_Protocol, true>(bp->address().local());
        com->receive(&data, 1500);
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
