// EPOS PC UART Mediator Test Program

#include <machine/uart.h>

using namespace EPOS;

int main()
{
    OStream cout;

    cout << "UART test\n" << endl;

    UART uart(1, 115200, 8, 0, 1);

    cout << "Loopback transmission test (conf = 115200 8N1):";
    uart.loopback(false);
    uart.put("A");
    char c = uart.get();
    cout << c << endl;
}
