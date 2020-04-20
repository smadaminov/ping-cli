// I got lazy and just copied (with slight modification)
// the checksum implementation that I found online (https://www.cs.utah.edu/~swalton/listings/sockets/programs/part4/chap18/myping.c).
// It seems, that pretty much everyone uses the same checksum implementation (as in the same code).
// TODO: At some point later, actually go through RFC 1071 and get familiar with the way it should be.
unsigned short checksum(void *pkt, int pkt_len) {
    unsigned short *buf = (unsigned short*)pkt;
    unsigned int sum = 0;
    unsigned short result = -1;

    for (sum; pkt_len > 1; pkt_len -= 2) {
        sum += *buf++;
    }

    if (pkt_len == 1) {
        sum += *(unsigned char*)buf;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;

    return result;
}
