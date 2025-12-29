
#include "tap_user.h"

/*
Implement circuit behavior on a TapUser

Responsible for taking a list of relations and turning that into a circuit. 
Assumes component definitions do not change while it is active, and does not
record connections to non-sensitive inputs.

The circuit can be updated by adding and removing components, and updating the
connections of existing components. This relatively simple interface should cut
down on development time relative to previous iterations.

The circuit is also responsible for propogating events through this circuit, 
implementing at least one function to propogate the lowest event and return the
current time. This can be used by audio output to run simulations.
*/
class TapCircuit : public TapUser {

};