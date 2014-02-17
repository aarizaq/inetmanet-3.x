
#ifndef _DSR_OPTIONS_H
#define _DSR_OPTIONS_H
#include <omnetpp.h>
#include "compatibility_dsr.h"

typedef std::vector<uint32_t> VectorAddress;
typedef std::vector<double> VectorCost;
typedef std::vector<struct in_addr> VectorAddressIn;


/* Generic header for all options */
struct dsr_opt : public cObject
{
    u_int8_t type;
    u_int8_t length;
    virtual dsr_opt * dup() = 0;
};

/* The DSR options header (always comes first) */
struct dsr_opt_hdr : public cObject
{
    u_int8_t nh;
    u_int8_t res:7;
    u_int8_t f:1;
    u_int16_t p_len;    /* payload length */
    std::vector <dsr_opt*> option;
    dsr_opt_hdr &  operator= (const dsr_opt_hdr &m)
    {
        if (this==&m) return *this;
        nh = m.nh;
        res = m.res;
        f = m.f;
        p_len = m.p_len;

        while (!option.empty())
        {
            delete option.back();
            option.pop_back();
        }
        for (unsigned int i = 0; i < m.option.size(); i++)
        {
            option.push_back(m.option[i]->dup());
        }
        return *this;
    }
    dsr_opt_hdr()
    {
        nh = 0;
        res = 0;
        f = 0;
        p_len = 0;
    }
    dsr_opt_hdr(const dsr_opt_hdr& other)
    {
        nh = other.nh;
        res = other.res;
        f = other.f;
        p_len = other.p_len;
        for (unsigned int i = 0; i < other.option.size(); i++)
        {
            option.push_back(other.option[i]->dup());
        }
    }
    ~dsr_opt_hdr()
    {
        while (!option.empty())
        {
            delete option.back();
            option.pop_back();
        }
    }
    virtual dsr_opt_hdr* dup() {return new dsr_opt_hdr(*this);}
};

struct dsr_pad1_opt : public dsr_opt
{
};



struct dsr_rreq_opt : public dsr_opt
{
    u_int16_t id;
    u_int32_t target;
    VectorAddress addrs;

    dsr_rreq_opt &  operator= (const dsr_rreq_opt &m)
    {
        if (this==&m) return *this;
        type = m.type;
        length = m.length;
        id = m.id;
        target = m.target;
        addrs = m.addrs;
        return *this;
    }
    dsr_rreq_opt()
    {
        type = 0;
        length = 0;
        id = 0;
        target = 0;
    }

    dsr_rreq_opt(const dsr_rreq_opt& other)
    {
        type = other.type;
        length = other.length;
        id = other.id;
        target = other.target;
        addrs = other.addrs;
    }

    virtual dsr_rreq_opt * dup()
    {
        return new dsr_rreq_opt(*this);
    }
};

struct dsr_srt_opt : public dsr_opt
{
    u_int16_t f:1;
    u_int16_t l:1;
    u_int16_t res:4;
    u_int16_t salv:4;
    u_int16_t sleft:6;
    VectorAddress addrs;

    dsr_srt_opt &  operator= (const dsr_srt_opt &m)
    {
        if (this==&m) return *this;
        type = m.type;
        length = m.length;
        f = m.f;
        l = m.l;
        res = m.res;
        salv = m.salv;
        sleft = m.sleft;
        addrs = m.addrs;
        return *this;
    }
    dsr_srt_opt()
    {
        type = 0;
        length = 0;
        f =0;
        l = 0;
        res = 0;
        salv = 0;
        sleft = 0;
        addrs.clear();

    }

    dsr_srt_opt(const dsr_srt_opt& m)
    {
        type = m.type;
        length = m.length;
        f = m.f;
        l = m.l;
        res = m.res;
        salv = m.salv;
        sleft = m.sleft;
        addrs = m.addrs;
    }

    virtual dsr_srt_opt * dup()
    {
        return new dsr_srt_opt(*this);
    }
};

struct dsr_ack_req_opt : public dsr_opt
{
    u_int16_t id;

    dsr_ack_req_opt &  operator= (const dsr_ack_req_opt &m)
    {
        if (this==&m) return *this;
        id = m.id;
        type = m.type;
        length = m.length;
        return *this;
    }
    dsr_ack_req_opt()
    {
        type = 0;
        length = 0;
        id  = 0;
    }

    dsr_ack_req_opt(const dsr_ack_req_opt& m)
    {
        type = m.type;
        length = m.length;
        id = m.id;
    }

    virtual dsr_ack_req_opt * dup()
    {
        return new dsr_ack_req_opt(*this);
    }
};

struct dsr_ack_opt : public dsr_opt
{

    u_int16_t id;
    u_int32_t src;
    u_int32_t dst;

    dsr_ack_opt &  operator= (const dsr_ack_opt &m)
    {
        if (this==&m) return *this;
        id = m.id;
        type = m.type;
        length = m.length;
        src = m.src;
        dst = m.dst;
        return *this;
    }
    dsr_ack_opt()
    {
        type = 0;
        length = 0;
        id  = 0;
        src = 0;
        dst = 0;
    }

    dsr_ack_opt(const dsr_ack_opt& m)
    {
        type = m.type;
        length = m.length;
        id = m.id;
        src = m.src;
        dst = m.dst;
    }

    virtual dsr_ack_opt * dup()
    {
        return new dsr_ack_opt(*this);
    }
};


struct dsr_rerr_opt : public dsr_opt
{
    u_int8_t err_type;
    u_int8_t res:4;
    u_int8_t salv:4;
    u_int32_t err_src;
    u_int32_t err_dst;
    std::vector<char>info;

    dsr_rerr_opt &  operator= (const dsr_rerr_opt &m)
     {
         if (this==&m) return *this;

         type = m.type;
         length = m.length;

         err_type = m.err_type;
         res = m.res;
         salv = m.salv;
         err_src = m.err_src;
         err_dst = m.err_dst;
         info = m.info;
         return *this;
     }

    dsr_rerr_opt()
     {
         type = 0;
         length = 0;

         err_type = 0;
         res = 0;
         salv = 0;
         err_src = 0;
         err_dst = 0;
         info.clear();

     }

    dsr_rerr_opt(const dsr_rerr_opt& m)
     {
        type = m.type;
        length = m.length;

        err_type = m.err_type;
        res = m.res;
        salv = m.salv;
        err_src = m.err_src;
        err_dst = m.err_dst;
        info = m.info;
     }

     virtual dsr_rerr_opt * dup()
     {
         return new dsr_rerr_opt(*this);
     }
};

struct dsr_rrep_opt : public dsr_opt
{
    u_int8_t res:7;
    u_int8_t l:1;
    VectorAddress addrs;
    std::vector<unsigned int> cost;
    dsr_rrep_opt &  operator= (const dsr_rrep_opt &m)
    {
        if (this==&m) return *this;
        type = m.type;
        length = m.length;
        res = m.res;
        l = m.l;
        addrs = m.addrs;
        cost = m.cost;
        return *this;
    }
    dsr_rrep_opt()
    {
        type = 0;
        length = 0;
        res = 0;
        l = 0;
    }
    dsr_rrep_opt(const dsr_rrep_opt& other)
    {
        type = other.type;
        length = other.length;
        res = other.res;
        l = other.l;
        addrs = other.addrs;
        cost = other.cost;
    }
    virtual dsr_rrep_opt * dup()
    {
        return new dsr_rrep_opt(*this);
    }
};

#define DSR_SRT_HDR_LEN 4
#define DSR_RREP_HDR_LEN 3
#define DSR_OPT_HDR_LEN 4
#define DSR_RERR_HDR_LEN 12
#define DSR_RERR_OPT_LEN (DSR_RERR_HDR_LEN - 2)
#define DSR_RREQ_HDR_LEN 8



#endif
