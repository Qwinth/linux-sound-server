#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include "alsaLib.hpp"
#include "ssocket.hpp"
#include "strlib.hpp"
using namespace std;

void handler(SSocket sock) {
    int err;
    string recv = sock.srecv(1024);
    vector<string> netparams = split(recv, ";");
    if (netparams.size() < 3) {
        sock.sclose();
        return;
    }
    _snd_pcm_format format;
    switch (stoi(netparams[0]))
    {
    case 16:
        format = SND_PCM_FORMAT_S16_LE;
        break;
    
    case 32:
        format = SND_PCM_FORMAT_S32_LE;
        break;
    
    case 64:
        format = SND_PCM_FORMAT_FLOAT64_LE;
        break;
    
    default:
        format = SND_PCM_FORMAT_S16_LE;
        break;
    }
    
    int rate = stoi(netparams[1]);
    int channels = stoi(netparams[2]);
    PCM pcm("default", SND_PCM_STREAM_PLAYBACK, 0);
    pcm.setAccess(SND_PCM_ACCESS_RW_INTERLEAVED);
    pcm.setFormat(format);
    pcm.setChannels(channels);
    pcm.setRate(rate, 0);
    pcm.paramsApply();

    int period = pcm.getPeriod(0);
    int buff_size = period * channels * pcm.getFormatWidth() / 8;
    // buff = (char*)malloc(buff_size * 2);

    SSocket::recvdata data;
    // pcm.prepare();
    
    data.value = (char*)malloc(buff_size);
    data.value = {0};
    sock.ssend(to_string(buff_size));
    
    while (true) {
        data = sock.srecv_char(buff_size * 2);
        // snd_pcm_wait(pcm.pcm, period/100);
        if (data.length == 0) {
            break;
        }
        else if (data.length < buff_size) {
            pcm.drain();
            break;
        }
        while (err = pcm.writei(data.value, period) == -EPIPE) {
        pcm.prepare();
        // cout << "yes" << endl;
        }
        sock.ssend("ok");
    }
    sock.ssend("exit");
    pcm.drop();
    pcm.close();
    sock.sclose();
    
}

int main() {
    SSocket sock(AF_INET, SOCK_STREAM);
    sock.ssetsockopt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 1);
    sock.sbind("localhost", 53764);
    sock.slisten(0);
    while (true) {
        auto s = sock.saccept();
        thread th(handler, s);
        th.detach();
    }
}