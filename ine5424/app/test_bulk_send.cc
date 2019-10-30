#include <communicator.h>
#include <time.h>
#include <network/bolinha_protocol.h>
using namespace EPOS;
typedef Ethernet::Address Address;

OStream cout;


NIC<Ethernet> *nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
char data[1500];
const int SEND_AMNT = 500;
int sender(int port) {
    Delay (5000000);
    Bolinha_Protocol * bp = new Bolinha_Protocol(port);
    Address d = bp->addr();
    char * hello = "HELLO WORLD";
    d[5]--;
    for (int i = 0; i < SEND_AMNT; i++) {
        cout << "Dado enviado: " << hello << ", para a porta: "<< port << endl;
        bp->send(hello, 1500, d, port);
    }
    return 1;
}

int receiver(int rport) {
    Bolinha_Protocol * bp = new Bolinha_Protocol(rport);
    for (int i = 0; i < SEND_AMNT; i++) {
        bp->receive(data, 1500);
        cout << "Dado recebido: " << data << ", pela porta: " << rport << endl;
    }
    return 1;
}


int main()
{
    
    cout << "NIC Test" << endl;
    cout << "Meu endereco eh " << nic->address() << endl;
    if(nic->address()[5] % 2) { // sender
        sender(5000);
    } else {
        receiver(5000);
    }
    return 0;
}
