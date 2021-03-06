#include "QServ.h"
#include "GeoIP/libGeoIP/GeoIPCity.h"
#include "GeoIP/libGeoIP/GeoIP.h"

//includes geoip handling, command system and a lot of useful tools

namespace server {
    clientinfo QServ::m_lastCI;
    bool m_olangcheck = false;
    QServ::QServ(bool olangcheck, int maxolangwarns,
                 char cmdprefix) {
        m_lastcommand = 0;
        m_olangcheck = olangcheck;
        m_maxolangwarns = maxolangwarns;
        m_cmdprefix = cmdprefix;
}

   QServ::~QServ() { ; }

   bool QServ::initgeoip(const char *filename) {
        m_geoip = GeoIP_open(filename, GEOIP_STANDARD);
        if(m_geoip == NULL) return false;
        return true;
    }
    
    bool QServ::initcitygeoip(const char *filename) {
        city_geoip = GeoIP_open(filename, GEOIP_STANDARD);
        if(city_geoip == NULL) return false;
        return true;
    }
    
	/*
    bool contains_number(const std::string &c)
    {
    if (c.find('0') != std::string::npos ||
        c.find('1') != std::string::npos ||
        c.find('2') != std::string::npos ||
        c.find('3') != std::string::npos ||
        c.find('4') != std::string::npos ||
        c.find('5') != std::string::npos ||
        c.find('6') != std::string::npos ||
        c.find('7') != std::string::npos ||
        c.find('8') != std::string::npos ||
        c.find('9') != std::string::npos)
    {
        return true;
    }
        return false;
    }
    char *QServ::citygeoip(const char *ip)
    {
        
        char* city = NULL;
        GeoIPRecord *gir = GeoIP_record_by_addr(city_geoip, ip);
        if(gir->city!=NULL)
        {
            city = gir->city;
            return (char*)city;
            //return city; or (char*)city 
            //GeoIPRecord_delete(gir);
        }
        else {city="Unknown";}
        
    }
    char *QServ::regiongeoip(const char *ip) {
        GeoIPRecord *gipr = GeoIP_record_by_addr(city_geoip, ip);
        if(!contains_number(gipr->region)) {
            return (char *)gipr->region;
        }
        else return "Unknown";
    }
        std::string QServ::cgip(const char *ip)  {
        std::stringstream gipi;
        char delimiter[] = ", ";
        char unknown[] = "Unknown";
        
        GeoIPRecord *gipr = GeoIP_record_by_addr(city_geoip, ip);
        
        //city
        if(gipr->city!=NULL) {
            gipi << gipr->city;
        } 
        else gipi << unknown;
        gipi << delimiter;
        
        //state
        if(!contains_number(gipr->region)) {
            gipi << gipr->region;
        }
        else gipi << unknown;
        gipi << delimiter; 
        
        //country
        if(GeoIP_country_name_by_name(m_geoip, ip)) {
            gipi << GeoIP_country_name_by_name(m_geoip, ip);
        }
        else gipi << unknown;
        GeoIPRecord_delete(gipr);
        return gipi.str();
        //reset - https://github.com/andrius4669/zeromod/blob/fc017aba9a1114c4db7553c3b3a6846a44ccc486/src/modules/geoip/main.c      
    }*/
    
    char *QServ::congeoip(const char *ip) {
		return (char*)GeoIP_country_name_by_name(m_geoip, ip);
    }
    
    std::string QServ::cgip(const char *ip)  {
    	
        std::stringstream gipi;
        
        char delimiter[] = ", ";
        char unknown[] = "Unknown";
    	
        GeoIPRecord *gipr = GeoIP_record_by_addr(city_geoip, ip);
		
        if(gipr->city != NULL && gipr->region != NULL && gipr->country_name != NULL) {
        	gipi << gipr->city << delimiter << gipr->region << delimiter << gipr->country_name; 
        	//GeoIPRecord_delete(gipr);
        	//gipr->city=NULL; gipr->region=NULL;
        	gipr->city=0;
    		gipr->region=0; 
    	
        }
        else if(gipr->city == NULL || gipr->region == NULL && gipr->country_name != NULL) {
        	gipi << gipr->country_name;
        }
        else {
        	gipi << unknown;
        }
        return gipi.str();
    } 
    
    void QServ::newcommand(const char *name, const char *desc, int priv, void (*callback)(int, char **args, int),
                           int args) {
        sprintf(m_command[m_lastcommand].name, "%c%s", m_cmdprefix, name);
        sprintf(m_command[m_lastcommand].desc, "%s", desc);

        m_command[m_lastcommand].priv = priv;
        m_command[m_lastcommand].id = m_lastcommand;
        m_command[m_lastcommand].func = callback;
        m_command[m_lastcommand].args = args+1;

        if(args > 0) {
            m_command[m_lastcommand].hasargs = true;
        } else {
            m_command[m_lastcommand].hasargs = false;
        }

        m_lastcommand += 1;
    }

    bool QServ::isCommand(char *text) {
        if(text[0] == m_cmdprefix) return true;
        return false;
    }

    int QServ::getCommand(char *text, char **args) {
        int CommandId = -1;

        for(int i = 0; i < m_lastcommand; i++) {
            if(strlen(m_command[i].name) > 1) {
                if(!strcmp(m_command[i].name, args[0])) {
                    CommandId = m_command[i].id;
                    break;
                }
            }
        }
        return CommandId;
    }

    void QServ::exeCommand(int command, char **args, int argc) {
        if(command > -1) {
            m_command[command].func(command, args, argc);
        }
    }

    char QServ::findWord(char *ctext, char *text, bool reg) {
        for(int i = 0; i < strlen(ctext); i++) {
            if(text[i+1] != ctext[i]) {
                return false;
            }
        }

        if(reg) {
            if(text[strlen(ctext)+1] != ' ' && text[strlen(ctext)+1] != '\0') {
                for(int j = 0; j < 3; j++) {
                    if(strcmp(owords[j], ctext)) {
                        return true;
                        break;
                    }
                }
                return false;
            }
        }
        return true;
    }

    char *QServ::getCommandName(int command) {
        return m_command[command].name;
    }

    char *QServ::getCommandDesc(int commandid) {
        return m_command[commandid].desc;
    }

    int QServ::getCommandPriv(int commandid) {
        return m_command[commandid].priv;
    }

    bool QServ::commandHasArgs(int command) {
        return m_command[command].hasargs;
    }

    int QServ::getCommandArgCount(int command) {
        return m_command[command].args;
    }

    int QServ::getlastCommand() {
        return m_lastcommand;
    }

    static int btimes = 0;
    void QServ::checkoLang(int cn, char *text) {
        if(m_olangcheck) {
            for(int i = 0; i < 50; i++) {
                if(strlen(owords[i]) > 0) {
                    for(int x = 0; x <= strlen(text); x++) {
                        if(!strcmp(owords[i], text+x-1)) {
                            btimes++;
                        }
                    }
                }
            }

            if(btimes > 0) {
                if(m_lastCI.connected) {
                    setoLangWarn(cn);
                    if(getoLangWarn(cn) == m_maxolangwarns) {
                        dcres(cn, "Offensive language");
                    } else {
                        if(getoLangWarn(cn) <= m_maxolangwarns) {
                            defformatstring(d)("\f7Watch your language \f0%s! \f3(Warning: %d)", m_lastCI.name, getoLangWarn(cn));
                            sendf(cn, 1, "ris", N_SERVMSG, d);
                        }
                    }
                }
                btimes = 0;
            }
        }
    }

    void QServ::setoLangWarn(int cn) {
        m_oLangWarn[cn] += 1;
    }

    void QServ::resetoLangWarn(int cn) {
        m_oLangWarn[cn] = 0;
    }

    int QServ::getoLangWarn(int cn) {
        return m_oLangWarn[cn];
    }

    void QServ::initCommands(void (*init)()) {
        init();
    }

    void QServ::setFullText(const char *text) {
        strcpy(m_fulltext, text);
    }

    char *QServ::getFullText() {
        return m_fulltext;
    }

    void QServ::setSender(int cn) {
        m_sender = cn;
    }

    int QServ::getSender() {
        return m_sender;
    }

    void QServ::setlastCI(clientinfo *ci) {
        m_lastCI = *ci;
    }

    clientinfo QServ::getlastCI() {
        return QServ::m_lastCI;
    }

    void QServ::setlastSA(bool lastsa) {
        m_lastSA = lastsa;
    }

    bool QServ::getlastSA() {
        return m_lastSA;
    }

    clientinfo *QServ::getClient(int cn) {
        return (clientinfo*)getclientinfo(cn);
    }

    char *QServ::cntoip(int cn) {
        static char ip[32];
        unsigned char by[4];

        for(int i = 0; i < 4; i++) {
            by[i] = (getclientip(cn) >> i*8) & 0xFF;
            sprintf(ip, "%d.%d.%d.%d", by[0], by[1], by[2], by[3]);
        }
        return ip;
    }

    bool QServ::handleTextCommands(clientinfo *ci, char *text) {
        setSender(ci->clientnum);
        setlastCI(ci);

        char ftb[1024] = {0};

        sprintf(ftb, "%s", text);
        setFullText(ftb);
        
        if(isCommand(text)) {
        char *args[20];
        
            
            int argc = 0;
            char *token = 0;

            token = strtok(text, " ");
            while(token != NULL) {
                args[argc] = token;
                token = strtok(NULL, " ");
                argc++;
            }
            
            int command = getCommand(0, args);
            out(ECHO_CONSOLE, "%s issued: %s",colorname(ci), getFullText());
            if(command >= 0) {
                
                char fulltext[1024];
            for(int j = 0; j <= strlen(ftb); j++) {
                    fulltext[j] = ftb[j+strlen(args[0])+1];
                }
                setFullText(fulltext);

                bool fargs = false;
                if(ci->privilege >= getCommandPriv(command)) {
                    if(commandHasArgs(command)) {
                        if(getCommandArgCount(command) == argc) {
                            fargs = true;
                        } else {
                            fargs = false;
                        }
                    } else {
                        fargs = true;
                    }

                    setlastSA(fargs);
                    exeCommand(command, args, argc);
                    return false;
                } else {
                    sendf(ci->clientnum, 1, "ris", N_SERVMSG, "\f3Insufficient permission");
                    return false;
                }
            } else {
                sendf(ci->clientnum, 1, "ris", N_SERVMSG, "\f3Error: Command not found. Use \f2\"#cmd\" \f3for a list of commands.");
                return false;
            }
        } else {
        
            if(strlen(ftb) < 1024 && irc.isConnected()) {
                irc.speak("%s(%d): %s\r\n", ci->name, ci->clientnum, ftb);
                printf("%s(%d): %s\r\n", ci->name, ci->clientnum, ftb);
            } else {
                //disconnect_client(ci->clientnum, DISC_OVERFLOW);
            }
            //checkoLang(ci->clientnum, text);
        }
        //memset(ftb,'\0',1000);

        return false;
    }

    bool QServ::isLangWarnOn() {
        return m_olangcheck;
    }

    void QServ::setoLang(bool on) {
        m_olangcheck = on;
    }

    void QServ::setCmdPrefix(unsigned char cp) {
        m_cmdprefix = cp;
    }

    char QServ::getCmdPrefix() {
        return m_cmdprefix;
    }
    
    void QServ::getLocation(clientinfo *ci) {
       char *ip = toip(ci->clientnum);
       /*defformatstring(location)("%s",congeoip(ip));
       out(ECHO_SERV, "\f0%s \f4connected: \f2%s", colorname(ci), cgip(ip).c_str());     
       out(ECHO_NOCOLOR, "%s connected: %s", colorname(ci), cgip(ip).c_str()); 
       */
       const char *location;
       //const char *localip[] = {"192.168.1.1", "192.168.1.2", "192.168.1.3", "192.168.1.4", "192.168.1.5", "192.168.1.6", "192.168.1.7", "192.168.1.8", "192.168.1.9", "192.168.1.10", "192.168.1.11", "192.168.1.12"};
	   if(!strcmp("127.0.0.1", ip) || !strcmp("192.168.1.1",ip) || !strcmp("192.168.1.2",ip) || !strcmp("192.168.1.3",ip)  || !strcmp("192.168.1.4",ip)  || !strcmp("192.168.1.5",ip)  || !strcmp("192.168.1.6",ip)  || !strcmp("192.168.1.7",ip)  || !strcmp("192.168.1.8",ip)  || !strcmp("192.168.1.9",ip)  || !strcmp("192.168.1.10",ip)  || !strcmp("192.168.1.11",ip)  || !strcmp("192.168.1.12",ip)  || !strcmp("192.168.1.13",ip)  || !strcmp("192.168.1.14",ip)  || !strcmp("192.168.1.15",ip)  || !strcmp("192.168.1.16",ip)  || !strcmp("192.168.1.17",ip)
	    || !strcmp("192.168.1.18",ip)  || !strcmp("192.168.1.19",ip)  || !strcmp("192.168.1.20",ip)  || !strcmp("192.168.1.21",ip)  || !strcmp("192.168.1.22",ip) || !strcmp("192.168.0.1",ip) || !strcmp("192.168.0.2",ip) || !strcmp("192.168.0.3",ip) || !strcmp("192.168.0.4",ip) || !strcmp("192.168.0.5",ip) || !strcmp("192.168.0.6",ip) || !strcmp("192.168.0.7",ip) || !strcmp("192.168.0.8",ip) || !strcmp("192.168.0.9",ip) || !strcmp("192.168.0.10",ip) || !strcmp("192.168.0.11",ip) || !strcmp("192.168.0.12",ip) || !strcmp("192.168.0.13",ip) 
	    || !strcmp("192.168.0.14",ip) || !strcmp("192.168.0.15",ip) || !strcmp("192.168.0.16",ip) || !strcmp("192.168.0.17",ip) || !strcmp("192.168.0.18",ip) || !strcmp("192.168.0.19",ip) || !strcmp("192.168.0.20",ip) || !strcmp("192.168.0.21",ip) || !strcmp("192.168.0.22",ip)) {
            location = (char*)"localhost";
        } else {
            location =  cgip(ip).c_str();
        }
      int type = 0;
        const char *types[] = {
            " connected from \f3Unknown",
            " \f4connected from \f1Localhost",
            " \f4connected near\f0"
        };
        int typeconsole = 0;
        const char *typesconsole[] = {
            " connected from Unknown",
            " connected from Localhost",
            " connected near "
        };
        
        char lmsg[255];
        char pmsg[255];
        const char clientip = getclientip(ci->clientnum);

        if(strlen(location) > 2 && strlen(ip) > 2 && strcmp("(null)", location)) {
  			if(!strcmp("(null)", location)) {
                type = 0;
                typeconsole = 0;
            } else if(ci->local) {
                type = 1;
                typeconsole = 1;
            } else {
                type = 2;
                typeconsole = 2;
                sprintf(lmsg, "%s %s", types[type], location);
                sprintf(pmsg, "%s%s", typesconsole[typeconsole], location);
                
            }
                //code below moved up into else
                defformatstring(msg)("\f0%s\f4%s", ci->name, (type < 2) ? types[type] : lmsg);
            	defformatstring(nocolormsg)("%s%s", ci->name, (typeconsole < 2) ? typesconsole[typeconsole] : pmsg);
           		out(ECHO_SERV,"%s",msg);
            	out(ECHO_NOCOLOR, "%s",nocolormsg);
        }
    }

    void QServ::checkMsg(int cn) {
        ms[cn].count += 1;
    }

    int QServ::getMsgC(int cn) {
        return ms[cn].count;
    }

    void QServ::resetMsg(int cn) {
        ms[cn].count = 0;
    }
}
