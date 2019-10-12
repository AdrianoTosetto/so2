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
        for(int i = 0; i < 10; i++) {
            memset(data, '0' + i, sp->mtu());
            data[sp->mtu() - 1] = '\n';
            sp->send(sp->address(), 0x8888, reinterpret_cast<void*>(data), sp->mtu());
        }
    } else {
        Delay (5000000);
        for(int i = 0; i < 10; i++) {
            sp->receive(reinterpret_cast<void*>(data), sp->mtu());
        }
    }

    NIC<Ethernet>::Statistics stat = sp->nic()->statistics();
    cout << "Statistics\n"
         << "Tx Packets: " << stat.tx_packets << "\n"
         << "Tx Bytes:   " << stat.tx_bytes << "\n"
         << "Rx Packets: " << stat.rx_packets << "\n"
         << "Rx Bytes:   " << stat.rx_bytes << "\n";
    return 0;
}
