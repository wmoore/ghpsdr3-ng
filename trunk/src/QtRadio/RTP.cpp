#include <QDebug>
#include <QThread>

#include "RTP.h"

RTP::RTP() {
qDebug() << "RTP::RTP";
    initialized=0;
    remote_set=0;
    jittcomp=40;
    adapt=TRUE;
    recv_ts=0;
    send_ts=0;

    ortp_init();
    ortp_scheduler_init();
    ortp_set_log_level_mask(ORTP_DEBUG|ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR);

}

RTP::~RTP() {
}

int RTP::init(char* host,int port) {
    //signal(SIGINT,stop_handler);
    rtpSession=rtp_session_new(RTP_SESSION_SENDRECV);
    rtp_session_set_scheduling_mode(rtpSession,1);
    rtp_session_set_blocking_mode(rtpSession,1);
    rtp_session_set_local_addr(rtpSession,"0.0.0.0",5004);
    rtp_session_set_connected_mode(rtpSession,TRUE);
    rtp_session_set_remote_addr(rtpSession,host,port);
    rtp_session_set_symmetric_rtp(rtpSession,TRUE);
    rtp_session_enable_adaptive_jitter_compensation(rtpSession,adapt);
    rtp_session_set_jitter_compensation(rtpSession,jittcomp);
    rtp_session_set_payload_type(rtpSession,0);
    //rtp_session_signal_connect(rtpSession,"ssrc_changed",(RtpCallback)ssrc_cb,0);
    rtp_session_signal_connect(rtpSession,"ssrc_changed",(RtpCallback)rtp_session_reset,0);

    qDebug() << "RTP initialized socket=" <<  rtp_session_get_rtp_socket(rtpSession) << " port=" << rtp_session_get_local_port(rtpSession);
    initialized=1;
    return rtp_session_get_local_port(rtpSession);
}

void RTP::setRemote(char* host,int port) {
    qDebug() << "RTP::setRemote " << host <<":" << port;
    rtp_session_set_remote_addr(rtpSession,host,port);
    remote_set=1;
}

void RTP::shutdown() {
    qDebug() << "shutdown";
    cont=0;
}

void RTP::run() {
    int err;
    char* buffer;
    int has_more;
    char *first_buffer, *second_buffer;
    int first_length, second_length;

    int audio_buffers = 0;

#define BUFFER_LENGTH 1024

    qDebug() << "RTP::run"; 
    if(!initialized) {
        qDebug() << "RTP:run call before init";
    } else {
        cont=1;
        while(cont) {
            has_more=1;
            while(has_more) {
                buffer=(char*)malloc(BUFFER_LENGTH);
                err=rtp_session_recv_with_ts(rtpSession,(uint8_t*)buffer,BUFFER_LENGTH,recv_ts,&has_more);
                if (err>0) {
                    if (audio_buffers > 2) {
                        emit rtp_packet_received(buffer,err);
                    }
                    else if (audio_buffers == 0){       // preload 2 buffers
                        first_buffer = buffer;
                        first_length = err;
                        audio_buffers++;
                    }
                    else if (audio_buffers == 1){
                        second_buffer = buffer;
                        second_length = err;
                        audio_buffers++;
                    }
                    else if (audio_buffers == 2){
                        emit rtp_packet_received(first_buffer,first_length);
                        emit rtp_packet_received(second_buffer,second_length);
                        emit rtp_packet_received(buffer,err);
                        audio_buffers++;
                    };
                    recv_ts+=err;
                }  else if(err==0) {
                    recv_ts+=BUFFER_LENGTH;
                }  else if(err<0) {
                    qDebug() << "rtp received error:" << err;
                    recv_ts+=BUFFER_LENGTH;
                }
            }
            usleep(2000);
        }
qDebug() << "RTP::run ending";

        rtp_session_destroy(rtpSession);
        ortp_global_stats_display();
        remote_set=0;
    }
qDebug() << "RTP::run exiting";
}

void RTP::send(unsigned char* buffer,int length) {
    if(initialized && cont)  {
        rtp_session_send_with_ts(rtpSession,(uint8_t*)buffer,length,send_ts);
        send_ts+=length;
    }
    if (buffer != NULL) free(buffer);
}

void RTP::dump_buffer(unsigned char* buffer,int length) {
    int i;
    QString output;
    QString hex;
    for(i=0;i<length;i++) {
        if(i%16==0) {
            if(i>0) qDebug() << output;
            output.sprintf("%04X: ",i);
        }
        hex.sprintf("%02X",buffer[i]);
        output+=hex;
    }
    if(i>0) qDebug() << output;
}