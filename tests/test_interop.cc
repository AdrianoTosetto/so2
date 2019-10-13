#include <time.h>

#include <machine/nic.h>

#include <process.h>
#include <communicator.h>
#include <synchronizer.h>


using namespace EPOS;
OStream cout;


int tcp_test()
{
    cout << "TCP Test" << endl;
    char data[6];
    Link<TCP> * com;

    IP * ip = IP::get_by_nic(0);
    if(ip->address()[3] % 2) { // sender
        cout << "Sender TCP:" << endl;
        data[0] = '1';
        data[1] = '2';
        data[2] = '3';
        data[3] = '4';
        data[4] = '5';
        data[5] = '6';

        IP::Address peer_ip = ip->address();
        peer_ip[3]--;
        com = new Link<TCP>(8000, Link<TCP>::Address(peer_ip, TCP::Port(8000))); // connect
        int sent = com->write(&data, sizeof(data));
        cout << "SENT VIA TCP" << data << endl;
    } else { // receiver
        cout << "Receiver TCP:" << endl;

        IP::Address peer_ip = ip->address();
        peer_ip[3]++;

        com = new Link<TCP>(TCP::Port(8000)); // listen
        com->read(&data, sizeof(data));
        cout << "RECEIVED VIA TCP " << data << endl;
    }
    delete com;
    return 1;
}

int udp_test()
{
    cout << "UDP Test" << endl;

    char data[6];
    Link<UDP> * com;

    IP * ip = IP::get_by_nic(0);


    if(ip->address()[3] % 2) { // sender
        cout << "Sender UDP:" << endl;
        data[0] = '1';
        data[1] = '2';
        data[2] = '3';
        data[3] = '4';
        data[4] = '5';
        data[5] = '6';
        IP::Address peer_ip = ip->address();
        peer_ip[3]--;
        com = new Link<UDP>(8080, Link<UDP>::Address(peer_ip, UDP::Port(8080)));
        int sent = com->send(&data, sizeof(data));
        cout << "SENT VIA UDP" << data << endl;
    } else { // receiver
        cout << "Receiver UDP:" << endl;

        IP::Address peer_ip = ip->address();
        peer_ip[3]++;

        com = new Link<UDP>(8080, Link<UDP>::Address(peer_ip, UDP::Port(8080)));

        int received = com->receive(&data, sizeof(data));
        cout << "RECEIVED VIA UDP: " << data << endl;
    }

    delete com;
    return 1;
}

int main()
{
    while(!tcp_test());
    while(!udp_test());
    return 0;
}