#include <communicator.h>
#include <time.h>
#include <network/bolinha_protocol.h>
using namespace EPOS;

OStream cout;

int main()
{
    cout << "NIC Test" << endl;
    Bolinha_Protocol::init(0);
    Bolinha_Protocol *bp = Bolinha_Protocol::get_by_nic(0);
    Bolinha_Protocol::Address addr = bp->address();
    Ethernet::Address mac = addr.bp();
    char data[1500];
    cout << "MAC " << mac << endl;
    if(mac[5] % 2) { // sender
        Delay (5000000);
        data[0] = 'H';
        data[1] = 'e';
        data[2] = 'l';
        data[3] = 'l';
        data[4] = 'o';
        data[5] = '\n';
        mac[5]--;
        Bolinha_Protocol::Address d = Bolinha_Protocol::Address(mac, 1);
        Port<Bolinha_Protocol> * com;
        com = new Port<Bolinha_Protocol>(1);
        cout << "Dado Enviado: " << data << " para " << d.bp() << endl;
        com->send(d, &data, 1500);
    } else {
        mac[5]++;
        Bolinha_Protocol::Address from = Bolinha_Protocol::Address(mac, 1);
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
