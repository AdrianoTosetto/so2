#include <machine/nic.h>
#include <time.h>
#include <real-time.h>
#include <utility/ostream.h>
#include <synchronizer.h>


using namespace EPOS;

OStream cout;

int test_int() {
    TSC_Chronometer c;
    c.start();
    cout << "NIC Test" << endl;

    NIC<Ethernet> * nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);

    NIC<Ethernet>::Address src, dst;
    NIC<Ethernet>::Protocol prot;
    char data[nic->mtu()+1];

    NIC<Ethernet>::Address self = nic->address();
    cout << "  MAC: " << self << endl;

    if(self[5] % 2) { // sender
        //Delay (5000000);
        memset(data, '1', nic->mtu()+1);
        data[nic->mtu() - 1] = '\n';

        nic->send(nic->broadcast(), 0x8888, data, nic->mtu());
        Delay (3000000);
        nic->send(nic->broadcast(), 0x8888, data, nic->mtu());
        Delay (1000000);
        nic->send(nic->broadcast(), 0x8888, data, nic->mtu());
        Delay (4000000);
        nic->send(nic->broadcast(), 0x8888, data, nic->mtu());
    } else {
        TSC_Chronometer cr;
        cr.start();
        nic->receive(&src, &prot, data, nic->mtu());
        cr.stop();
        cout << "Time = " << cr.read() / 1000000 << endl;
        cr.reset();
        cr.start();
        nic->receive(&src, &prot, data, nic->mtu());
        cr.stop();
        cout << "Time = " << cr.read() / 1000000 << endl;
        cr.reset();
        cr.start();
        nic->receive(&src, &prot, data, nic->mtu());
        cr.stop();
        cout << "Time = " << cr.read() / 1000000 << endl;
        cr.reset();
        cr.start();
        nic->receive(&src, &prot, data, nic->mtu());
        cr.stop();
        cout << "Time = " << cr.read() / 1000000 << endl;
        cr.reset();
        cr.start();
        cout << "  Data: " << data;
        
    }

    NIC<Ethernet>::Statistics stat = nic->statistics();
    cout << "Statistics\n"
         << "Tx Packets: " << stat.tx_packets << "\n"
         << "Tx Bytes:   " << stat.tx_bytes << "\n"
         << "Rx Packets: " << stat.rx_packets << "\n"
         << "Rx Bytes:   " << stat.rx_bytes << "\n";
    c.stop();
    return c.read();
}

int main()
{
    test_int();
}