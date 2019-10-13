#include <communicator.h>
#include <time.h>

using namespace EPOS;


OStream cout;

int main()
{
    cout << "NIC Test" << endl;

    NIC<Ethernet> * nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);

    NIC<Ethernet>::Address src, dst;
    NIC<Ethernet>::Protocol prot;
    Simple_Protocol * sp = Simple_Protocol::get_by_nic(0);
    char data[sp->mtu()];

    cout << " addr: " << sp->address() << endl;
    cout << "  NIC: " << sp->nic()->address() << endl;
    if(sp->nic()->address()[5] % 2) { // sender
        Delay (5000000);
        memset(data, '1', sp->mtu());
        data[sp->mtu() - 1] = '\n';
        cout << " addr: " << sp->address() << endl;
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
