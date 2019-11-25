#include <communicator.h>
#include <time.h>
#include <network/bolinha_protocol.h>
using namespace EPOS;
typedef Ethernet::Address Address;

OStream cout;
NIC<Ethernet> *nic = Traits<Ethernet>::DEVICES::Get<0>::Result::get(0);
char data[1500];
int main()
{
    cout << "NIC Test" << endl;
    cout << "Meu endereco eh " << nic->address() << endl;
    if(nic->address()[5] == 9) {
        cout << "começando sender (slave) " << endl;
        Delay (5*1000000);
        Bolinha_Protocol * bp = new Bolinha_Protocol(420, false, 'C');
        char *hello = "hello\n";
        Address d = bp->addr();
        Address d2 = bp->addr();
        Address bc = bp->broadcast();
        d[5]--;
        d2[5] -= 2;
        // cout << "Dado enviado: " << hello << ", para a porta: "<< 420 << endl;
        bp->send(hello, 1500, d, 420);
        Delay(10*1000000);
        bp->send(hello, 1500, d2, 420);
        Delay(5*1000000);
        return 0;
    } else if (nic->address()[5] == 8){
        Delay(1000000);
        Bolinha_Protocol * ancora1 = new Bolinha_Protocol(420, true, 'A'); // Master
        ancora1->receive(data, 1500);
        //cout << "Dado recebido: " << data << ", pela porta: " << 420 << endl;        return 1;
    } else {
        Delay(5*1000000);
        Bolinha_Protocol * ancora2 = new Bolinha_Protocol(420, true, 'B'); // Master
        ancora2->receive(data, 1500);
        //cout << "Dado recebido: " << data << ", pela porta: " << 420 << endl;
        Delay(1000000);
        return 1;
    }
    return 0;
}
