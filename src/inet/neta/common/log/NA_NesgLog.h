//
// Copyright (C) 2013, NESG (Network Engineering and Security Group), http://nesg.ugr.es,
// - Gabriel Maciá Fernández (gmacia@ugr.es)
// - Leovigildo Sánchez Casado (sancale@ugr.es)
// - Rafael A. Rodríguez Gómez (rodgom@ugr.es)
// - Roberto Magán Carrión (rmagan@ugr.es)
// - Pedro García Teodoro (pgteodor@ugr.es)
// - José Camacho Páez (josecamacho@ugr.es)
// - Jesús E. Díaz Verdejo (jedv@ugr.es)
//
// This file is part of NETA.
//
//    NETA is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    NETA is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with NETA.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef NESGLOG_H_
#define NESGLOG_H_

#include <fstream>
#include <omnetpp.h>
#include <stdarg.h>
#include "inet/neta/common/utils/NA_Utils.h"

using namespace std;
using namespace omnetpp;

/** LOG header*/
#define NESGLOGHEADER "[" << NA_Utils::currentDateTime() << "][" << simTime() << "][NESGLOG]: "
/** LOG macro*/
#define LOG log << NESGLOGHEADER

/**
 * @brief Specific logs for MA framework
 *
 * @details This class provides some methods and macros to make the debug process easier.
 * An example of use is showed following:
 * @code
 *  LOG << "Dropping Attack Probability received: "<< dmsg->getDroppingAttackProbability() << "\n";
 * @endcode
 *
 * @see NA_Utils
 *
 * @author Gabriel Maciá Fernández, gmacia@ugr.es
 * @date 01/22/2013
 *
 */

namespace inet {

namespace neta {

class NA_NesgLog {
public:
    /**
     * Constructor
     */
    NA_NesgLog();

    /**
     * Destructor
     */
    ~NA_NesgLog() {
    }

    /**
     * Write a log line in the simulation console
     *
     * @param logline
     */
    void write(const char* logline, ...);

    /**
     * Deactivate the logs
     */
    void unsetLog();

    /**
     * Override the operator << for strings
     *
     * @param t the string to write on log
     * @return
     */
    NA_NesgLog& operator<<(const std::string& t) {
        if (active)
            EV << t;
        return *this;
    } // For strings

    /**
     * Override the operator << for generic objects
     *
     * @param t a generic object to write on log
     * @return
     */
    template<typename T> NA_NesgLog& operator<<(const T& t) {
        if (active)
            EV << t;
        return *this;
    } // For generic objects

    /**
     * Override the operator << for objects like endl
     *
     * @param t
     * @return
     */
    NA_NesgLog& operator<<(std::ostream& (t)(std::ostream&)) {
        if (active)
            EV << t;
        return *this;
    } // For objects like endl

private:
    /**
     * Flag to active the logs or not
     */
    bool active; // Indicates if logging is active or not

};

}
}

#endif
