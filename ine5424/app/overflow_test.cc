// EPOS NIC Test Programs

#include <machine/nic.h>
#include <time.h>
#include <synchronizer.h>
#include <process.h>

using namespace EPOS;

OStream cout;


NIC<Ethernet> * nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
    
NIC<Ethernet>::Address src, dst;
NIC<Ethernet>::Protocol prot;

NIC<Ethernet>::Address self = nic->address();

Thread * sender;
Thread * receiver;

int send() {
    char data[nic->mtu()];
    for (int i = 0; i < 20; i++) {
        memset(data, '0' + i, nic->mtu());
        data[nic->mtu() - 1] = '\n';
        nic->send(nic->broadcast(), 0x8888, data, nic->mtu());
        cout << "Pacote enviado: " << data;
    }
    return 0;
}

int receive() {
    Delay waiting(2000000);
    for (int i = 0; i < 20; i++) {
        char data[nic->mtu()];
        nic->receive(&src, &prot, data, nic->mtu());
        cout << "Pacote recebido: " << data;
    }
    return 0;
}

int main()
{
    cout << "Overflow Test" << endl;
    NIC<Ethernet> * nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);

    NIC<Ethernet>::Address src, dst;
    NIC<Ethernet>::Protocol prot;
    char data[nic->mtu()];
    if(self[5] % 2) { // sender
        //Delay (5000000);
        for (int i = 0; i < 200; i++) {
            memset(data, '0' + i, nic->mtu());
            data[nic->mtu() - 1] = '\n';
            nic->send(nic->broadcast(), 0x8888, data, nic->mtu());
            cout << "Pacote enviado: " << data;
        }
    } else { // receiver
        Delay waiting(2000000);
        for (int i = 0; i < 200; i++) {
            nic->receive(&src, &prot, data, nic->mtu());
            cout << "Pacote recebido: " << data;
        }
    }
    NIC<Ethernet>::Statistics stat = nic->statistics();
    cout << "Statistics\n"
        << "Tx Packets: " << stat.tx_packets << "\n"
        << "Tx Bytes:   " << stat.tx_bytes << "\n"
        << "Rx Packets: " << stat.rx_packets << "\n"
        << "Rx Bytes:   " << stat.rx_bytes << "\n"
        << "Overflow in Tx: " << stat.tx_overflow << "\n"
        << "Overflow in Rx: " << stat.rx_overflow << "\n";

    return 0;
}