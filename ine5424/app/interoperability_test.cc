#include <machine/nic.h>
#include <time.h>

#include <process.h>
#include <communicator.h>
#include <synchronizer.h>
#include <utility/list.h>
#include <utility/string.h>
#include <utility/observer.h>
#include <utility/random.h>

using namespace EPOS;


OStream cout;


int main()
{
    cout << "NIC Test" << endl;
    NIC<Ethernet> * nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
    char data[nic->mtu()];

    Dummyzao::Address self = nic->address();
    cout << "  MAC:\t" << self << endl;

    if(self[5] % 2)
    {
        new Thread(&dummy_sender);
        new Thread(&icmp_sender);
    }
    else
    {
        new Thread(&dummy_receiver);
        new Thread(&icmp_receiver);
    }

    NIC<Ethernet>::Statistics stat = nic->statistics();
    cout << "Statistics\n"
         << "Tx Packets:\t" << stat.tx_packets << "\n"
         << "Tx Bytes:   " << stat.tx_bytes << "\n"
         << "Rx Packets:\t" << stat.rx_packets << "\n"
         << "Rx Bytes:   " << stat.rx_bytes << "\n";
}
