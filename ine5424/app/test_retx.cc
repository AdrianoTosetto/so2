#include <communicator.h>
#include <time.h>
#include <network/bolinha_protocol.h>
using namespace EPOS;
typedef Ethernet::Address Address;

OStream cout;


NIC<Ethernet> *nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
char data[1500];
int sender() {
    Delay (5000000);
    Bolinha_Protocol * bp = new Bolinha_Protocol();
    char *hello = "Hello\n";
    Address d = bp->addr();
    d[5]--;
    cout << "Dado enviado: " << hello << ", para a porta: "<< 5000 << endl;
    return bp->send(hello, 1500, d, 5000);
}

int receiver() {
    Bolinha_Protocol * bp = new Bolinha_Protocol();
    bp->receive(data, 1500);
    cout << "Dado recebido: " << data << ", pela porta: " << 5000 << endl;
    return 1;
}


int main()
{
    cout << "NIC Test" << endl;
    cout << "Meu endereco eh " << nic->address() << endl;
    if(nic->address()[5] % 2) { // sender
        sender();
    } else {
        //Binds nas portas 5000 e 5001 para receber
        Delay(12000000);
        receiver();
    }

    /*NIC<Ethernet>::Statistics stat = sp->nic()->statistics();
    cout << "Statistics\n"
         << "Tx Packets: " << stat.tx_packets << "\n"
         << "Tx Bytes:   " << stat.tx_bytes << "\n"
         << "Rx Packets: " << stat.rx_packets << "\n"
         << "Rx Bytes:   " << stat.rx_bytes << "\n";*/
    return 0;
}
