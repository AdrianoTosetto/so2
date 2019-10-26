#include <machine/nic.h>
#include <time.h>
#include <real-time.h>
#include <utility/ostream.h>
#include <synchronizer.h>


using namespace EPOS;

OStream cout;

int test_one() {
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
        Delay (5000000);

        for(int i = 0; i < 1000; i++) {
            memset(data, '1' + i, nic->mtu()+1);
            data[nic->mtu() - 1] = '\n';
            nic->send(nic->broadcast(), 0x8888, data, nic->mtu());
        }
    } else { // receiver
        for(int i = 0; i < 1000; i++) {
           nic->receive(&src, &prot, data, nic->mtu());
           cout << "  Data: " << data;
        }
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
int test_two() {
    int secs=20; // this task will run for about 20 seconds
    TSC_Chronometer c;
    c.start();
    while(c.read() < 1000000*secs);
    return c.read();
}

int test_three()  {
    TSC_Chronometer c;
    c.start();
    Thread *one = new Thread(&test_one);
    Thread *two = new Thread(&test_two);
    one->join();
    two->join();
    c.stop();
    return c.read();
}

int main()
{
    int ttest_one = test_one() / 1000000;
    int ttest_two = test_two() / 1000000;
    int ttest_three = test_three() / 1000000;
    cout << "Teste 1 demorou " << ttest_one << " segundos" << endl;
    cout << "Teste 2 demorou " << ttest_two << " segundos" << endl;
    cout << "Teste 3 demorou " << ttest_three << " segundos" << endl;
}