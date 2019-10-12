// EPOS NIC Test Programs

#include <machine/nic.h>
#include <network/simple_protocol.h>
#include <time.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "NIC Test" << endl;

    
    Simple_Protocol *sp = Simple_Protocol::get_by_nic(0);

    cout << " addr: " << sp->address() << endl;
    cout << "  MAC: " << sp->nic()->address() << endl;
    return 0;
}
