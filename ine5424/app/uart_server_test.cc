// EPOS PC UART Mediator Test Program

#include <machine/uart.h>
#include <network/bolinha_protocol.h>
using namespace EPOS;

int main()
{
    OStream cout;

    cout << "UART test\n" << endl;
    Bolinha_Protocol * bp = new Bolinha_Protocol();
}
