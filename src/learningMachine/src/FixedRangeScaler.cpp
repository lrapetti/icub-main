/*
 * Copyright (C) 2007-2009 Arjan Gijsberts @ Italian Institute of Technology
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 *
 * Implementation of a fixed range scaler.
 *
 */

#include <cfloat>
#include <sstream>
#include <iostream>

#include <yarp/os/Value.h>

#include "iCub/FixedRangeScaler.h"

namespace iCub {
namespace contrib {
namespace learningmachine {

FixedRangeScaler::FixedRangeScaler(double li, double ui, double lo, double uo) {
    this->setName("Fixed");
    this->setLowerBoundIn(li);
    this->setUpperBoundIn(ui);
    this->setLowerBoundOut(lo);
    this->setUpperBoundOut(uo);
}

void FixedRangeScaler::updateScales() {
    this->scale = (this->getUpperBoundIn() - this->getLowerBoundIn()) / (this->getUpperBoundOut() - this->getLowerBoundOut());
    this->offset = this->getLowerBoundIn() - (this->getLowerBoundOut() * this->scale);
}

std::string FixedRangeScaler::getInfo() {
    std::ostringstream buffer;
    buffer << this->IScaler::getInfo() << ", ";
    buffer << "In Bounds: [" << this->getLowerBoundIn() << "," << this->getUpperBoundIn() << "], ";
    buffer << "Out Bounds: [" << this->getLowerBoundOut() << "," << this->getUpperBoundOut() << "]";
    return buffer.str();
}

bool FixedRangeScaler::configure(Searchable& config) {
    bool success = this->IScaler::configure(config);

    // set the expected incoming lower bound (double)
    if(config.find("lowerin").isDouble() || config.find("lowerin").isInt()) {
        this->setLowerBoundIn(config.find("lowerin").asDouble());
        success = true;
    }
    // set the expected incoming upper bound (double)
    if(config.find("upperin").isDouble() || config.find("upperin").isInt()) {
        this->setUpperBoundIn(config.find("upperin").asDouble());
        success = true;
    }

    // set the desired outgoing lower bound (double)
    if(config.find("lowerout").isDouble() || config.find("lowerout").isInt()) {
        this->setLowerBoundOut(config.find("lowerout").asDouble());
        success = true;
    }
    // set the desired outgoing bound (double)
    if(config.find("upperout").isDouble() || config.find("upperout").isInt()) {
        this->setUpperBoundOut(config.find("upperout").asDouble());
        success = true;
    }

    if(!config.findGroup("in").isNull()) {
        Bottle& bot = config.findGroup("in");
        if(bot.size() == 3 && (bot.get(1).isInt() || bot.get(1).isDouble()) && 
           (bot.get(2).isInt() || bot.get(2).isDouble())) {

            this->setLowerBoundIn(bot.get(1).asDouble());
            this->setUpperBoundIn(bot.get(2).asDouble());
            success = true;
        }
    }

    if(!config.findGroup("out").isNull()) {
        std::cout << "out: " << config.findGroup("out").toString() << std::endl;
        Bottle& bot = config.findGroup("out");
        if(bot.size() == 3 && (bot.get(1).isInt() || bot.get(1).isDouble()) && 
           (bot.get(2).isInt() || bot.get(2).isDouble())) {

            this->setLowerBoundOut(bot.get(1).asDouble());
            this->setUpperBoundOut(bot.get(2).asDouble());
            success = true;
        }
    }

    // set the desired outgoing upper bound (double)
    //std::cout << config.toString() << std::endl;
    
    return success;
}

} // learningmachine
} // contrib
} // iCub

