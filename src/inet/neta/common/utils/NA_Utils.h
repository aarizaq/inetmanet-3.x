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
#include <string>

#ifndef NA_UTILS_H_
#define NA_UTILS_H_

/**
 * @brief Utility class
 *
 * @details Class that provide several utility methods for MA framework
 *
 *
 * @author Roberto Magán Carrión, gmacia@ugr.es
 * @date 01/22/2013
 *
 */


namespace inet {

namespace neta {

class NA_Utils {
public:

    /**
     * Get current date/time, format is YYYY-MM-DD.HH:mm:ss
     *
     * @return string time in the specific format
     */
    static std::string currentDateTime();

};


};
};
#endif /* MAUTILS_H_ */
