#include <communicator.h>
#include <time.h>
#include <network/bolinha_protocol.h>
using namespace EPOS;


OStream cout;

int main()
{
    cout << "NIC Test" << endl;
    Bolinha_Protocol *bp = new Bolinha_Protocol();
    char data[1500];
    if(bp->addr()[5] % 2) { // sender
        Delay (5000000);
        data[0] = 'H';
        data[1] = 'e';
        data[2] = 'l';
        data[3] = 'l';
        data[4] = 'o';
        data[5] = '\n';

        Ethernet::Address peer_ip = bp->addr();
        peer_ip[3]--;
        bp->send(bp->addr(), peer_ip, &data, 1500);
    } else {
        bp->receive(&data, 1500);
        cout << data << endl;
    }

    /*NIC<Ethernet>::Statistics stat = sp->nic()->statistics();
    cout << "Statistics\n"
         << "Tx Packets: " << stat.tx_packets << "\n"
         << "Tx Bytes:   " << stat.tx_bytes << "\n"
         << "Rx Packets: " << stat.rx_packets << "\n"
         << "Rx Bytes:   " << stat.rx_bytes << "\n";*/
    return 0;
}
