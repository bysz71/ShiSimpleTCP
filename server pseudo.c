while(1)
    udt_rcv(rcvpkt);
    extractcrc(rcvpkt);
    computecrc(rcvpkt);
    if(crcmatch()&&seqnummatch(expectednum)){
        extractdata(rcvpkt,data);
        deliverdata(data);
        sndpkt = makepkt(ACK,seqnum);
        udt_send(sndpkt);
    }else{
        sndpkt = makepkt(ACK,seqnum-1);
        udt_send(sndpkt);
    }
}
