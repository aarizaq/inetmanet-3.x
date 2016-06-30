#include "inet/common/geometry/common/Coord.h"

#include "inet/mobility/traci/TraCIMobility.h"

using namespace inet;

void ifInetTraCIMobilityCallPreInitialize(cModule* mod, const std::string& nodeId, const Coord& position, const std::string& road_id, double speed, double angle) {
	::TraCIMobility* inetmm = dynamic_cast< ::TraCIMobility*>(mod);
	if (!inetmm) return;
	inetmm->preInitialize(nodeId, ::Coord(position.x, position.y), road_id, speed, angle);
}


void ifInetTraCIMobilityCallNextPosition(cModule* mod, const Coord& p, const std::string& edge, double speed, double angle) {
	::TraCIMobility *inetmm = dynamic_cast< ::TraCIMobility*>(mod);
	if (!inetmm) return;
	inetmm->nextPosition(::Coord(p.x, p.y), edge, speed, angle);
}

