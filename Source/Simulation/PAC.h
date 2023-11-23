#pragma once

#include <JuceHeader.h>
using namespace juce;
using namespace std;

class SimEntity;
class SimReaction;
class Simulation;

class SATSolver{
public:
    SATSolver(){};
    ~SATSolver(){};
    SATSolver(String name, String command, string satLine, bool printsExtraString): name(name), command(command) , satLine(satLine), printsExtraString(printsExtraString){};
    String name;
    String command;
    string satLine; //displayed by the SAT solver: SAT or SATISFIABLE
    bool printsExtraString; //is there an extra string printed by the SAT solver before SAT or SATISFIABLE, and before the values
};

typedef Array<pair<SimEntity *,float>> witnessType; // a witness is a vector of concentrations

class PAC
{
public:
    PAC(){};
    PAC(var data, Simulation *simul); // import from JSON, TODO
    ~PAC(){
        entities.clear();
        reacDirs.clear();
    };
    var toJSONData();                 // save to JSON, TODO

    String toString(); // for printing

    bool constructionFailed=false; // if the PAC was not constructed because of a problem with the data (e.g. null ptrs to entities)

    //int id; //identifier

    Array<SimEntity *> entities;
    Array<pair<SimReaction *, bool>> reacDirs; // direction 0 is 2->1 and 1 is 1->2

    float flow; // min of reactions flows, 0 if one flow is in the wrong direction

    bool wasRAC = false; // was this PAC a RAC at some point

    bool includedIn(PAC *p, bool onlyEnts);

    //for CACs

    //bool isCAC = false; // is this PAC a CAC ?
    //Array<pair<SimEntity *,float>> witness; //for CAC: vector of concentrations witnessing the CAC

  

};


class PAClist : public Thread
{
public:
    PAClist() : Thread("PACs") {};
    PAClist(Simulation *simul) : Thread("PACs"), simul(simul){};
    ~PAClist();

    Simulation *simul;
    OwnedArray<PAC> cycles;
    OwnedArray<PAC> nonMinimals;
    float maxRAC = 0.0f;          // remember the max RAC for display
    bool includeOnlyWithEntities=false; // specify the rule for inclusion of PACs
    void addCycle(PAC *);
    void printPACs(); // print list of PACs to cout
    void printRACs();

      //the thread function
    void run() override;

    void affectSATIds(); // affect idSAT to the entities/reactions if not already done.

    void PACsWithSAT(); // compute PACs with SAT solver minisat (numSolver 0) or kissat (numSolver 1)
    void PACsWithZ3(); // compute PACs with SMT Solver Z3 (numSolver >1)

    int numSolver; // index of the current sat solver
    string z3path=""; // path to z3 executable
    void setZ3path(); // set the path to z3 executable

    void computePACs(int numSolv); // compute PACs from the simulation

    //CACs
    Array<pair<set<int>,witnessType>> CACs; // indexes of the PACs in "cycles" that are CACs. each is a vector because we also treat pairs, etc.
        //the Array is the witness concentrations
    bool computeCAC(set<int>); // test for CAC and compute witness if yes
    void computeCACS(); // compute CACs among the PACs
    void clear(); // clear everything

    // save/load to JSON
    var toJSONData();
    void fromJSONData(var data);
};