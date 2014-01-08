 /**
******************************************************
* @file RSTPAccess.h
* @brief RSTP access
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#ifndef __RSTP_ACCESS_H
#define __RSTP_ACCESS_H


#include "ModuleAccess.h"
#include "RSTP1Q.h"


/**
 * @brief Gives access to RSTP module
 */
class RSTPAccess : public ModuleAccess<RSTP1Q>
{
    public:
    	 RSTPAccess() : ModuleAccess<RSTP1Q>("rstp") {}
};

#endif
