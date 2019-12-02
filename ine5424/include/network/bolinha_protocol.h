// EPOS IP Protocol Declarations

#ifndef __bolinha_protocol_h
#define __bolinha_protocol_h

#include <system/config.h>
#include <synchronizer.h>

#include <utility/bitmap.h>
#include <machine/nic.h>
#include <time.h>
#include <utility/list.h>
#include <utility/ostream.h>

__BEGIN_SYS

#define SEC 1000000


inline void substr_copy(char *src, char *dst, size_t begin, size_t end) {
    unsigned int i = begin, j = 0;
    for(; i <= end; i++, j++) dst[j] = src[i];
    dst[j+1] = '\n';
}

// arg = lat | lon. T = double for lat, lon, T = Ticks for timestamp
template<typename T>
inline T scan_param(char *nmea, int arg) {
    OStream funcs;
    unsigned int end = 0, begin = 0; // begin and end of substring
    double factor = 1.0;
    if (arg == 0) { // lat
        unsigned int comma_count = 0;
        for (unsigned int i = 0; i < strlen(nmea); i++) {
            if (nmea[i] == ',') comma_count++;
            
            if (comma_count == 2 && begin == 0) begin = i + 1;
            
            if (comma_count == 3) {
                end = i - 1;
                if(nmea[i+1] == 'S') factor = -1.0;
                break;
            }
            
        }
    } else if (arg == 1) { // lon
        unsigned int comma_count = 0;
        for (unsigned int i = 0; i < strlen(nmea); i++) {
            if (nmea[i] == ',') comma_count++;
            if (comma_count == 4 && begin == 0) begin = i + 1;
            if (comma_count == 5) {
                end = i - 1;
                if(nmea[i+1] == 'W') factor = -1.0;
                break;
            }
            
        }
    } else if (arg == 2) {
        // $GPGGA,092750.000,...
        char hh[3]; // -> 09
        char mm[3]; // -> 27
        char ss[3]; // -> 50
        char ms[7] = {'0', '0', '0', '0', '0', '0', '\n'};   // -> 000, no defined size, max size = 6


        hh[0] = nmea[7];
        hh[1] = nmea[8];
        hh[2] = '\n';

        mm[0] = nmea[9];
        mm[1] = nmea[10];
        mm[2] = '\n';

        ss[0] = nmea[11];
        ss[1] = nmea[12];
        ss[2] = '\n';
        unsigned int i = 14;
        unsigned int k = 5;
        while(nmea[i] != ',') i++;
        i-=1;
        while(nmea[i] != '.') {
            ms[k] = nmea[i];
            i--;
            k--;
        }
        int i_hh = funcs.atoi(hh);
        int i_mm = funcs.atoi(mm);
        int i_ss = funcs.atoi(ss);
        int i_ms = funcs.atoi(ms);

        db<Bolinha_Protocol>(WRN) << "i_hh " << i_hh << endl;
        db<Bolinha_Protocol>(WRN) << "i_mm " << i_mm << endl;
        db<Bolinha_Protocol>(WRN) << "i_ss " << i_ss << endl;
        db<Bolinha_Protocol>(WRN) << "i_ms " << i_ms << endl;
        return (i_hh * 24 * 60 * 60 + i_mm * 60 + i_ss ) * 1000000; // usar o ms aqui
    }

    char paramstr[end - begin + 1]; // +1 = \n
    substr_copy(nmea, paramstr, begin, end);
    return funcs.atof(paramstr) * factor;
}


class Bolinha_Protocol: private NIC<Ethernet>::Observer, Concurrent_Observer<Ethernet::Buffer, Ethernet::Protocol>
{

public:
    static const unsigned int TIMEOUT = Traits<Bolinha_Protocol>::TIMEOUT;
    static const unsigned int RETRIES = Traits<Bolinha_Protocol>::RETRIES;
    
    typedef Ethernet::Address MAC_Address;
    typedef Ethernet::Address Address;
    typedef Ethernet::Buffer Buffer;
    typedef Data_Observer<Buffer, Ethernet::Protocol> Observer;
    typedef Data_Observed<Buffer, Ethernet::Protocol> Observed;
    typedef Ethernet::Protocol Protocol;
	typedef Timer::Tick Tick;

    struct Frame_Track {
        short _frame_id;
        short _port;
        Address _mac;

        Frame_Track(short frame_id, short port, Address mac): _frame_id(frame_id), _port(port), _mac(mac) {}
        Frame_Track(): _frame_id(-1), _port(-1) {}
    };
    struct Sem_Track {
        short _frame_id;
        Semaphore *_sem;
        bool *_status;
        Sem_Track(short frame_id, Semaphore *sem, bool* status):
            _frame_id(frame_id), _sem(sem), _status(status){}
    };

    struct Received_Points {
        bool set = false;
        char sender_id;
        double _x;
        double _y;
        double _dist;
    };

    typedef List_Elements::Doubly_Linked_Ordered<Bolinha_Protocol::Sem_Track, short> Sem_Track_El;
    typedef struct Frame_Track FT;

    Protocol Prot_Bolinha = Ethernet::PROTO_SP;
    Bolinha_Protocol(short port = 5000, bool anchor = true, bool master=false, char id = 'A'):
        _nic(Traits<Ethernet>::DEVICES::Get<0>::Result::get(0)), _anchor(anchor), _id(id) {
        OStream funcs;
        db<Bolinha_Protocol>(WRN) << "cos(30 graus) " << funcs.cos(funcs.deg2rad(30)) << endl;
        if (port <= 0)
            return;
        bool res = CPU::tsl<char>(_ports[port]);
        if (!res) {
            db<Bolinha_Protocol>(WRN) << "Porta adquirida com sucesso!" << endl;
            _using_port = port;
            _nic->attach(this, Prot_Bolinha);
        } else {
            db<Bolinha_Protocol>(WRN) << "Falha ao adquirir porta!" << endl;
        }
    
        _master = master;
        db<Bolinha_Protocol>(WRN) <<_id << endl;
        if (_anchor)
            request_GPS_info();

    }
    void request_GPS_info() {
        OStream conversor;
        char nmea_string[100];
        size_t i = 0;
        UART uart(1, 115200, 8, 0, 1);

        db<Bolinha_Protocol>(WRN) << "Loopback transmission test (conf = 115200 8N1):" << endl;
        uart.loopback(false);
        char send = _id;
        uart.put(send);
        while (true) {
            char c = uart.get();
            nmea_string[i] = c;
            i++;
            if (c == '\n') {
                db<Bolinha_Protocol>(WRN) << "nmea: " << nmea_string << endl;
                break;
            }
        }
        _x = conversor.lon_to_x(scan_param<double>(nmea_string, 1));
        _y = conversor.lat_to_y(scan_param<double>(nmea_string, 0));
        _z = 0;
        _ts = scan_param<Tick>(nmea_string, 2);
        Alarm::elapsed() = _ts;
        db<Bolinha_Protocol>(WRN) << "nmea: " << nmea_string << endl;
        db<Bolinha_Protocol>(WRN) << "lat: " << scan_param<double>(nmea_string, 0) << endl;
        db<Bolinha_Protocol>(WRN) << "lon: " << scan_param<double>(nmea_string, 1) << endl;
        db<Bolinha_Protocol>(WRN) << "ts: " << scan_param<Tick>(nmea_string, 2) << endl;

    }
    virtual ~Bolinha_Protocol() {
        CPU::fdec<char>(_ports[_using_port]);

        Frame *f = new Frame(_nic->broadcast(), addr(), -1, 0, 0, _using_port, 0, 0);
        f->flags(2);
        _nic->send(_nic->broadcast(), Prot_Bolinha, f, sizeof(Frame));
    }
    bool master() const {
        return _master;
    }
    Address broadcast () {
        return _nic->broadcast();
    }
    int send(void *data, size_t size, Address& to, short port_receiver) {
        int bytes;
        int n = RETRIES;
        bool result;
        bool status = false;
        int id = _packet_count;
        {
            Semaphore sem(0);
            Semaphore_Handler handler_a(&sem);
            Alarm alarm_a = Alarm(TIMEOUT, &handler_a, n);

            Frame *f = new Frame(to, addr(), CPU::finc<short int>(_packet_count), &status, data, _using_port, port_receiver, 5);
            f->flags(0);
            f->time(Alarm::elapsed());
            if (_anchor) {
                f->coordinates(_x, _y);
                f->sender_id(_id);
                db<Bolinha_Protocol>(WRN) << "Enviando minhas coordenadas " << _id << " (" << _x << ", " << _y << ", " << _z << ")" << endl;
            }
            if (_master) {
                f->is_master(true);
            }
            Sem_Track st(f->frame_id(), &sem, &status);
            
            Sem_Track_El e(&st, f->frame_id());
            _sem_track_m.lock();
            sem_track.insert(&e);
            _sem_track_m.unlock();
            
            while(!status && n > 0) {
                db<Bolinha_Protocol>(WRN) << "Attempting to send " << n << " through port: " << port_receiver  << endl;
                bytes = _nic->send(to, Prot_Bolinha, f, size);
                sem.p();
                n--;
            }

            result = CPU::tsl<bool>(status);
            _sem_track_m.lock();
            sem_track.remove_rank(f->frame_id());
            _sem_track_m.unlock();
            delete f;
        }
        if (result) {
            db<Bolinha_Protocol>(WRN) << "Mensagem " << id << " confirmada" << endl;
            db<Bolinha_Protocol>(WRN) << "Mensagem confirmada, port = " << _using_port << ", ft_id = " << id << ", mac = " << addr() << endl;
        } else  {
            db<Bolinha_Protocol>(WRN) << "Falha ao enviar mensagem, port = " << _using_port << ", ft_id = " << id << ", mac = " << addr() << endl;
            //db<Bolinha_Protocol>(WRN) << "Falha ao enviar mensagem " << id << endl;
            bytes = 0;
        }
        return bytes;
    }
    static Tick time() {
        return Alarm::elapsed();
    }
    static void add_time(Tick t) {
        Alarm::elapsed() += t;
    }
    int receive(void *buffer, size_t size) {
        Buffer *rec = updated();
        Frame *f = reinterpret_cast<Frame*>(rec->frame()->data<char>());
        memcpy(buffer, f->data<char>(), size);

        _m.lock();
        Frame_Track ft(f->frame_id(), f->port_sender(), f->from());
        _tracking_messages[_frame_track_count] = ft;
        _frame_track_count = (_frame_track_count + 1) % 100;
        _m.unlock();

        db<Bolinha_Protocol>(WRN) << "Dado recebido: " << f->data<char>() << endl;
        char* ack_data = (char*) "ACK\n";
        Frame *ack = new Frame(f->from(), addr(), f->frame_id(), f->status(), ack_data, _using_port, f->port_sender(), 0);
        ack->flags(1);
        ack->time(Alarm::elapsed());
        if (_anchor) {
            ack->coordinates(_x, _y);
            ack->sender_id(_id);
            db<Bolinha_Protocol>(WRN) << "Enviando minhas coordenadas " << _id << " (" << _x << ", " << _y  << ", "<< _z << ")" << endl;
        }
        if (_master) {
            f->is_master(true);
        }
        //if (_delay_ack) Delay (5*1000000);
        _nic->send(f->from(), Prot_Bolinha, ack, sizeof(Frame));
        _nic->free(rec);

        return size;
    }
    static bool notify(const Protocol& p, Buffer *b) {
        return _observed.notify(p, b);
    }
    void update(Observed *o, const Protocol& p, Buffer *b) {
        Frame *f = reinterpret_cast<Frame*>(b->frame()->data<char>());
        short port_receiver = f->port_receiver();
        short port_sender = f->port_sender();
        short flags = f->flags();
        short frame_id = f->frame_id();
		Address from = f->from();


        if (!_rps[0].set || _rps[0].sender_id == f->sender_id()) {
            _rps[0]._x = f->X();
            _rps[0]._y = f->Y();
            _rps[0].sender_id = f->sender_id();
            _rps[0]._dist = Alarm::elapsed() % 10;
            _rps[0].set = true;
        } else if (!_rps[1].set || f->sender_id() == _rps[1].sender_id) {
            _rps[1]._x = f->X();
            _rps[1]._y = f->Y();
            _rps[1].sender_id = f->sender_id();
            _rps[1]._dist = Alarm::elapsed() % 10;
            _rps[1].set = true;
        } else if (!_rps[2].set || f->sender_id() == _rps[2].sender_id) {
            _rps[2]._x = f->X();
            _rps[2]._y = f->Y();
            _rps[2]._dist = Alarm::elapsed() % 10;
            _rps[2].sender_id = f->sender_id();
            _rps[2].set = true;
        }
        if (_rps[0].set && _rps[1].set && _rps[2].set) {
            _rps[0].set = false;
            _rps[1].set = false;
            _rps[2].set = false;
            trilaterate();
        }
        if (from == addr()) {
            db<Bolinha_Protocol>(WRN) << "Mensagem redundante" << endl;
            if (port_sender == _using_port) {
                _nic->free(b);
                return;
            }
            int pc = 0;
            for (int i = 1; i < 1000; i++) {
                pc += _ports[i];
            }
        }
        Address frame_add = f->from();
        if (port_receiver != _using_port && port_receiver != 0) {
            db<Thread>(WRN) << "Porta " << _using_port << " recebeu frame para porta " << port_receiver << endl;
            return;
        }
        if (Alarm::elapsed() - f->time() > 5000 or Alarm::elapsed() - f->time() < -5000) {
            if (!_master && f->is_master()) {
                if (ticks[3] != -1) {
                    Tick propagation_delay = ((ticks[1] - ticks[0]) + (ticks[3] - ticks[2])) / 2;
                    Tick offset = (ticks[1] - ticks[0]) - propagation_delay;
                    db<Bolinha_Protocol>(WRN) << "Offset Calculado " << offset << endl;
                    Alarm::_elapsed -= offset;
                    ticks[0] = -1;
                    ticks[1] = -1;
                    ticks[2] = -1;
                    ticks[3] = -1;
                    db<Bolinha_Protocol>(WRN) << "Novo tempo do slave: " << Alarm::_elapsed << endl;
                } else if (ticks[0] != -1) {
                    ticks[2] = Alarm::elapsed();
                    ticks[3] = f->time();
                } else {
                    ticks[1] = Alarm::elapsed();
                    ticks[0] = f->time();
                }
            }
        }
        if (flags & 1) {
            db<Thread>(WRN) << "ACK " << frame_id << " recebido" << endl;
            _sem_track_m.lock();
            Sem_Track_El * ste = sem_track.search_rank(frame_id);
            if (ste) {
                Sem_Track * semt = ste->object();
                semt->_sem->v();
                CPU::tsl<bool>(*(semt->_status));
                sem_track.remove_rank(frame_id);
            }
            _sem_track_m.unlock();
            _nic->free(b);
            return;
        }
        if (flags & 2) {
            db<Bolinha_Protocol>(WRN) << "Flush de " << frame_add << "/" << port_sender << endl;
            _m.lock();
            for (int i = 0; i < 100; i++) {
                short port = _tracking_messages[i]._port;
                Address frame_mac = _tracking_messages[i]._mac;
                if(port == port_sender && frame_add == frame_mac) {
                    _tracking_messages[i] = FT();
                }
            }
            _m.unlock();
            _nic->free(b);
            return;
        }
        for (int i = 0; i < 100; i++) {
            _m.lock();
            short port = _tracking_messages[i]._port;
            short ft_id = _tracking_messages[i]._frame_id;
            Address frame_mac = _tracking_messages[i]._mac;
            _m.unlock();
            if(port == port_sender && ft_id == frame_id && frame_add == frame_mac) {
                _nic->free(b); // nobody is listening to this buffer, so we need call free on it
                db<Bolinha_Protocol>(WRN) << "Descarte, port = " << port << ", ft_id = " << ft_id << ", mac = " << frame_mac << endl;
                return;
            }
        }
        Concurrent_Observer<Observer::Observed_Data, Protocol>::update(p, b);
    }
    const Address& addr() const {
        return _nic->address();
    }
    unsigned int MTU() {
        return Bolinha_MTU; // header precisa ser packed pra calcular certo o mtu
    }

    void trilaterate() {
        OStream func;
        db<Bolinha_Protocol>(WRN) << "Trilaterando" << endl;
        double r1 = _rps[0]._dist;
        double r2 = _rps[1]._dist;
        double r3 = _rps[2]._dist;
        double U = _rps[1]._x;
        double Vx = _rps[2]._x;
        double Vy = _rps[2]._y;
        double V2 = (Vx*Vx) + (Vy*Vy);
        _x = ((r1*r1) - (r2*r2) + (U*U))/ 2*U;
        _y = ((r1*r1) - (r3*r3) + V2 - 2*Vx*_x);
        _z = func.sqrt((r1*r1) - (_x*_x) - (_y*_y));

        db<Bolinha_Protocol>(WRN) << "Posição trilaterada (" << _x  << ", " << _y << ", "<< _z <<")" << endl; 
    }

    void delay_ack(bool d_ack) { _delay_ack = d_ack; }

    class Header {
    public:    

        Header(Address from, short frame_id, bool* status, short port_sender, 
            short port_receiver, double x, double y): 
        _from(from), _frame_id(frame_id), _status(status), _flags(0), _port_sender(port_sender),
            _port_receiver(port_receiver), _x(x), _y(y)
        {}

        void flags(char flags) {
            _flags = flags;
        }
        bool* status() {
            return _status;
        }
        Address from() {
            return _from;
        }
        char flags() {
            return _flags;
        }
        //Address _to;
        //unsigned short  _checksum; // verificar se o frame chegou com todos os bits corretos
        //unsigned short  _length;
    protected:
        Address _from;
        short _frame_id;
        bool*  _status;
        char  _flags; // ACK
        short _port_sender;
        short _port_receiver;
        Timer::Tick _time;
        double _x;
        double _y;
        char _sender_id;
        bool _is_master = false;

    } __attribute__((packed));

    class Frame: private Header {
    public:
        Frame(Address to, Address from, int packet_id, bool* status, void* data, short port_sender, 
            short port_receiver,size_t len, double  x = 0, double y = 0): 
            Header(from, packet_id, status, port_sender, port_receiver, x, y), _len(len), _data(data) 
            {
                
            }
        typedef unsigned char Data[];
        size_t _len;
        void* _data;

        void flags(char flags) {
            _flags = flags;
        }
        void time(Timer::Tick t) {
            _time = t;
        }
        void coordinates(double x, double y) {
            _x = x;
            _y = y;
        }
        void sender_id(char id) {
            _sender_id = id;
        }
        void is_master(bool set) {
            _is_master = set;
        }
        Timer::Tick time() const {
            return _time;
        }
        short frame_id() {
            return _frame_id;
        }
        void frame_id(short id) {
            _frame_id = id;
        }
        bool* status() {
            return _status;
        }
        Address from() {
            return _from;
        }
        char flags() {
            return _flags;
        }
        template<typename T>
        T * data() { return reinterpret_cast<T *>(_data); }
        short port_sender() {
            return _port_sender;
        }
        short port_receiver() {
            return _port_receiver;
        }
        double X() {
            return _x;
        }
        double Y() {
            return _y;
        }
        char sender_id() {
            return _sender_id;
        }
        bool is_master() {
            return _is_master;
        }
    } __attribute__((packed));
protected:
    NIC<Ethernet> * _nic;
    Address _address;
    Mutex _sem_track_m;
    Mutex _m;
    bool _delay_ack = false; // for test of duplicate messages 
    static char _ports[1000];
    static Observed _observed;
    static const unsigned int NIC_MTU = 1500;
    static const unsigned int Bolinha_MTU = NIC_MTU - sizeof(Header);
    short _using_port = -1;
    short _packet_count = 0;
    short _frame_track_count = 0;
    Frame_Track _tracking_messages[100];
    Ordered_List<Sem_Track, short> sem_track;
    bool _anchor;
    bool _master;
	Tick ticks[4] = {-1, -1, -1, -1};
    double _x;
    double _y;
    double _z;
    Tick _ts;
    char _id; 
    Received_Points _rps[3];
};

// bool Bolinha_Protocol::_ports[] = {0};

__END_SYS

#endif