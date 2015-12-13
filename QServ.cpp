#include "QServ.h"

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
    
    char *QServ::congeoip(const char *ip) {
        return (char*)GeoIP_country_name_by_name(m_geoip, ip);
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
            out(ECHO_CONSOLE, "%s issued: %s",colorname(ci), text);
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
                sendf(ci->clientnum, 1, "ris", N_SERVMSG, "\f3Command not found");
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
        defformatstring(location)("%s", congeoip(ip));

        int type = 0;
        const char *types[] = {
            " \f4connected from \f3Unknown",
            " connected from \f1Unknown/Localhost",
            " \f4connected from \f0"
        };
        int typeconsole = 0;
        const char *typesconsole[] = {
            " connected from Unknown",
            " connected from Localhost\n",
            " connected from "
        };
        
        char lmsg[255];
        
        if(strlen(location) > 2 && strlen(ip) > 2) {
            const char clientip = getclientip(ci->clientnum);
            int ip = getclientip(ci->clientnum);
            if(ci->local) {
                type = 1;
                typeconsole = 1;
            } else if(!strcmp("(null)", location)) {
                type = 0;
                typeconsole =0;
            } else {
                type = 2;
                typeconsole =2;
                sprintf(lmsg, "%s\f2%s", types[type], location);
            }
            defformatstring(msg)("\f0%s\f4%s", ci->name, (type < 2) ? types[type] : lmsg);
            out(ECHO_SERV, "%s", msg);
            out(ECHO_CONSOLE, "%s%s", ci->name, (type < 2) ? typesconsole[typeconsole] : lmsg);
            out(ECHO_IRC, "%s%s. Clientnum: %d", ci->name, (type < 2) ? types[type] : lmsg, ci->clientnum);
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