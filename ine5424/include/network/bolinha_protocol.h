// EPOS IP Protocol Declarations

#ifndef __bolinha_protocol_h
#define __bolinha_protocol_h

#include <system/config.h>
#include <synchronizer.h>

#include <utility/bitmap.h>
#include <machine/nic.h>
#include <time.h>
#include <utility/list.h>


__BEGIN_SYS
// teste

#define SEC 1000000

enum MESSAGE_TYPE {
    SYN,
    FOLLOW_UP,
    DELAY_REQ,
    DELAY_RES,
    REQUEST_TS,
    APPLICATION
};


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
    typedef List_Elements::Doubly_Linked_Ordered<Bolinha_Protocol::Sem_Track, short> Sem_Track_El;
    typedef struct Frame_Track FT;

    Protocol Prot_Bolinha = Ethernet::PROTO_SP;
    Bolinha_Protocol(short port = 5000, bool master = false):
        _nic(Traits<Ethernet>::DEVICES::Get<0>::Result::get(0)), _master(master) {
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
		if(port == 420 && _nic->address()[5] == 9) {
			db<Bolinha_Protocol>(WRN) << "Começando o PTP... " << endl;
			ptp_handler = new Thread(&init_ptp, this);
		}
    }
	static int init_ptp(Bolinha_Protocol * _this) {
		while (!_this->finish_ptp) {
            Tick time = Alarm::elapsed();
			Frame *f = new Frame(_this->_nic->broadcast(), _this->addr(), -1, 0, nullptr, 420, 420, 0, MESSAGE_TYPE::SYN);
			_this->_nic->send(_this->_nic->broadcast(), _this->Prot_Bolinha, f, sizeof(Frame));
			Delay(1*SEC);
			Frame *f_follow_up = new Frame(_this->_nic->broadcast(), _this->addr(), -1, 0, nullptr, 420, 420, 0, MESSAGE_TYPE::FOLLOW_UP);
			f_follow_up->time(time);
            _this->_nic->send(_this->_nic->broadcast(), _this->Prot_Bolinha, f_follow_up, sizeof(Frame));
            Delay(20*SEC);
		}
		return 0;
	}
    virtual ~Bolinha_Protocol() {
        CPU::fdec<char>(_ports[_using_port]);

        Frame *f = new Frame(_nic->broadcast(), addr(), -1, 0, 0, _using_port, 0, 0);
        f->flags(2);
        _nic->send(_nic->broadcast(), Prot_Bolinha, f, sizeof(Frame));

		finish_ptp = true;
		ptp_handler->join();
		delete ptp_handler;
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
            f->sem(&sem);
            f->flags(0);
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
            // db<Bolinha_Protocol>(WRN) << "Falha ao enviar mensagem " << id << endl;
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

        char* ack_data = (char*) "ACK\n";
        Frame *ack = new Frame(f->from(), addr(), f->frame_id(), f->status(), ack_data, _using_port, f->port_sender(), 0);
        ack->flags(1);
        //if (_delay_ack) Delay (5*1000000);
        _nic->send(f->from(), Prot_Bolinha, ack, sizeof(Frame));

        _nic->free(rec);

        return size;
    }
    static bool notify(const Protocol& p, Buffer *b) {
        return _observed.notify(p, b);
    }
    void start_PTP_Slave() {

    }
    void update(Observed *o, const Protocol& p, Buffer *b) {
        Frame *f = reinterpret_cast<Frame*>(b->frame()->data<char>());
        short port_receiver = f->port_receiver();
        short port_sender = f->port_sender();
        short flags = f->flags();
        short frame_id = f->frame_id();
		Address from = f->from();
		
        if (!f->is_Application()) {
			db<Bolinha_Protocol>(WRN) << "Identificando pacote de PTP" << endl;
			if (f->is_Syn()) {
				db<Bolinha_Protocol>(WRN) << "Identificando pacote de PTP SYN" << endl;
				ticks[1] = Alarm::elapsed();
				
				//Frame *f_follow_up = new Frame(_nic->broadcast(), addr(), -1, 0, nullptr, 420, 420, 0, MESSAGE_TYPE::FOLLOW_UP);
				//_nic->send(from, Prot_Bolinha, f_follow_up, sizeof(Frame));
			}
			if (f->is_Follow_Up()) {
				db<Bolinha_Protocol>(WRN) << "Identificando pacote de PTP FollowUP" << endl;
				// recuperar o T1
				ticks[0] = f->time();
				Frame *f_delay_req = new Frame(_nic->broadcast(), addr(), -1, 0, nullptr, 420, 420, 0, MESSAGE_TYPE::DELAY_REQ);
				Delay(2*SEC);
				db<Bolinha_Protocol>(WRN) << "mandando delay req" << endl;
				ticks[2] = Alarm::elapsed();
				_nic->send(from, Prot_Bolinha, f_delay_req, sizeof(Frame));

			}
			if (f->is_Delay_Req()) {
				db<Bolinha_Protocol>(WRN) << "Identificando pacote de PTP Delay Req" << endl;
				Tick time = f->time();
				Delay(2*SEC);
				Frame *f_delay_res = new Frame(from, addr(), -1, 0, nullptr, 420, 420, 0, MESSAGE_TYPE::DELAY_RES);
				// ticks[3] = Alarm::elapsed();
                f_delay_res->time(time);
				_nic->send(from, Prot_Bolinha, f_delay_res, sizeof(Frame));
			}
			if (f->is_Delay_Res()) {
				db<Bolinha_Protocol>(WRN) << "Identificando pacote de PTP Delay Res" << endl;
				ticks[3] = f->time();
				// TODO: Sincronização
                // PD =  (T2 - T1) + (T4 - T3) / 2
                // OFFSET = T2 - T1 - PDS
                auto propagation_delay = ((ticks[1] - ticks[0]) + (ticks[3] - ticks[2])) / 2;
                auto offset = (ticks[1] - ticks[0]) - propagation_delay;
                db<Bolinha_Protocol>(WRN) << "Offset Calculado " << offset <<endl;
                Alarm::_elapsed -= offset;

			}
			Concurrent_Observer<Observer::Observed_Data, Protocol>::update(p, b);
        }
        Address frame_add = f->from();
        if (port_receiver != _using_port && port_receiver != 0) {
            db<Thread>(WRN) << "Porta " << _using_port << " recebeu frame para porta " << port_receiver << endl;
            return;
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
            // if (!i)
            //     db<Bolinha_Protocol>(WRN) << "Tracking, port = " << port << ", ft_id = " << ft_id << ", mac = " << frame_mac << endl;

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

    void delay_ack(bool d_ack) { _delay_ack = d_ack; }

    class Header {
    public:    

        Header(Address from, short frame_id, bool* status, short port_sender, short port_receiver): 
        _from(from), _frame_id(frame_id), _status(status), _flags(0), _port_sender(port_sender), _port_receiver(port_receiver)
        {}

        void flags(char flags) {
            _flags = flags;
        }
        void sem(Semaphore * sem) {
            _sem = sem;
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
        Semaphore * sem() {
            return _sem;
        }
        //Address _to;
        //unsigned short  _checksum; // verificar se o frame chegou com todos os bits corretos
        //unsigned short  _length;
    protected:
        Address _from;
        Semaphore * _sem;
        short _frame_id;
        bool*  _status;
        char  _flags; // ACK
        short _port_sender;
        short _port_receiver;
        int _ptp_flags:3;           // 000 -> SYN
                                    // 001 -> Follow_UP
                                    // 010 -> Delay_Req
                                    // 011 -> Delay_Res
                                    // 100 -> Request_TS
                                    // 111 -> APPLICATION
        Timer::Tick _time;
    } __attribute__((packed));

    class Frame: private Header {
    public:
        Frame(Address to, Address from, int packet_id, bool* status, void* data, short port_sender, 
            short port_receiver,size_t len, MESSAGE_TYPE type = MESSAGE_TYPE::APPLICATION): 
            Header(from, packet_id, status, port_sender, port_receiver), _len(len), _data(data) {
                set_ptp_flags(type);
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
        Timer::Tick time() const {
            return _time;
        }
        void sem(Semaphore * sem) {
            _sem = sem;
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
        Semaphore * sem() {
            return _sem;
        }
        template<typename T>
        T * data() { return reinterpret_cast<T *>(_data); }
        short port_sender() {
            return _port_sender;
        }
        short port_receiver() {
            return _port_receiver;
        }
        bool is_Syn() {
            return _ptp_flags == 0b000;
        }
        bool is_Follow_Up() {
            return _ptp_flags == 0b001;
        }
        bool is_Delay_Req() {
            return _ptp_flags == 0b010;
        }
        bool is_Delay_Res() {
            return _ptp_flags == 0b011;
        }
        bool is_Request_TS() {
            return _ptp_flags == 0b100;
        }
        bool is_Application() {
            return _ptp_flags == 0b111;
        }
    private:
        void set_ptp_flags(MESSAGE_TYPE type) {
            switch (type)
            {
            case MESSAGE_TYPE::SYN:
                _ptp_flags = 0b000;
                break;
            case MESSAGE_TYPE::FOLLOW_UP:
                _ptp_flags = 0b001;
                break;
            case MESSAGE_TYPE::DELAY_REQ:
                _ptp_flags = 0b010;
                break;
            case MESSAGE_TYPE::DELAY_RES:
                _ptp_flags = 0b011;
                break;
            case MESSAGE_TYPE::REQUEST_TS:
                _ptp_flags = 0b100;
            case MESSAGE_TYPE::APPLICATION:
                _ptp_flags = 0b111;
                break;
            default:
                break;
            }
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
    bool _master;
	Tick ticks[4] = {-1, -1, -1, -1};
	Thread * ptp_handler;
	bool finish_ptp = false;
};

// bool Bolinha_Protocol::_ports[] = {0};

__END_SYS

#endif