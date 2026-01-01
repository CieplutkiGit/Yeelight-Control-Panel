#ifndef BULT_T_H
#define BULT_T_H

#include <iostream>
#include <string>

class bulb_t
{
    std::string ip_str;
    static int port;
    std::string id_str;

    int brightness;
public:
    bulb_t(void);
    bulb_t(std::string ip, std::string id, int pt = 55443);
    std::string get_ip_str();
    std::string get_id_str();
    int get_port();
    bool operator == (const bulb_t &x) { return (this->ip_str == x.ip_str) && (this->port == x.port) && (this->id_str == x.id_str); }
    void set_brightness(int brn_value);
    int get_brightness();
};

#endif // BULT_T_H
