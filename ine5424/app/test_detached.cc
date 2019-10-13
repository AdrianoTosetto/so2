#include <communicator.h>
#include <time.h>

using namespace EPOS;


OStream cout;

int main()
{
    cout << "NIC Test" << endl;
    Simple_Protocol * sp = Simple_Protocol::get_by_nic(0);
    char data[sp->mtu()];

    cout << " addr: " << sp->address() << endl;
    cout << "  NIC: " << sp->nic()->address() << endl;
    if(sp->nic()->address()[5] % 2) { // sender
        Delay (5000000);
        data[0] = 'H';
        data[1] = 'e';
        data[2] = 'l';
        data[3] = 'l';
        data[4] = 'o';
        sp->send(data, sp->mtu());
    } else {
        sp->receive(data, sp->mtu());
        cout << data << endl;
    }

    NIC<Ethernet>::Statistics stat = sp->nic()->statistics();
    cout << "Statistics\n"
         << "Tx Packets: " << stat.tx_packets << "\n"
         << "Tx Bytes:   " << stat.tx_bytes << "\n"
         << "Rx Packets: " << stat.rx_packets << "\n"
         << "Rx Bytes:   " << stat.rx_bytes << "\n";
    return 0;
}
