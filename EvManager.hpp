#pragma once
#include	<sys/types.h>
#include	<sys/event.h>
#include	<sys/time.h>
#include    <utility>
#include    <iostream>


class EvManager
{
    private:
        EvManager();
        EvManager(const EvManager &rhs);
        EvManager &operator=(const EvManager &rhs);
        ~EvManager();
    

    public:
        enum Flag {
            read,
            write,
            eof,
            error,
            def
        };

        static bool start();

        static int getFlag(Flag flag);

        static bool addEvent(int fd, Flag flag);

        static bool delEvent(int fd, Flag flag);

        static std::pair<Flag, int> listen();

    private:
        static int _i;
        static int _numEvents;
        static const int CLIENT_LIMIT = 1000;
        static int _kq;
        static struct kevent _evList[CLIENT_LIMIT];

};














// #pragma once
// #include	<sys/types.h>
// #include	<sys/event.h>
// #include	<sys/time.h>
// #include <utility>


// class EvManager
// {
//     private:
//         EvManager();
//         EvManager(const EvManager &rhs);
//         EvManager &operator=(const EvManager &rhs);
//         ~EvManager();
    
//     private:
//         static int _i;
//         static int _numEvents;
//         static const int CLIENT_LIMIT = 1000;
//         static int _kq;
//         static struct kevent _evList[CLIENT_LIMIT];

//     public:
//         static const int READ = EVFILT_READ;
//         static const int WRITE = EVFILT_WRITE;
//         static const int ERROR = EV_ERROR;

//         static bool start() {
//             _kq = kqueue();
//             if (_kq == -1) {
//                 std::runtime_error(std::string("kqueue: ") + strerror(errno));
//             }
//             return (true);
//         }

//         static bool addEvent(int fd, int flag) {
//             struct kevent evSet;
//             EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
//             if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1) {
//                 std::runtime_error(std::string("kevent: ") + strerror(errno));
//             };
//             return (true);
//         }

//         static bool delEvent(int fd, int flag) {
//             struct kevent evSet;
//             EV_SET(&evSet, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
//             if (kevent(_kq, &evSet, 1, NULL, 0, NULL) == -1) {
//                 std::runtime_error(std::string("kevent: ") + strerror(errno));
//             };
//             return (true);
//         }


//         static std::pair<int, int> listen() {
//             if (_numEvents == 0) {
//                 _numEvents = kevent(_kq, NULL, 0, _evList, CLIENT_LIMIT, NULL);
//             }
//             if (_numEvents == -1) {
//                 std::runtime_error(std::string("kevent: ") + strerror(errno));
//             }
//             std::pair<int, int> result = std::make_pair<int, int>(_evList[_i].filter, _evList[_i].ident);
//             ++_i;
//             --_numEvents;
//             return (result);
//         }

// };