
#include "Statistics.h"
#include "Generation.h"
#include "Util.h"

juce_ImplementSingleton(Statistics)

    Statistics::Statistics() : simul(Simulation::getInstance())
//    saveSimBT("Save"),
//    loadSimBT("Load")
// uiStep(1)
{
    simul->addAsyncSimulationListener(this);
}

Statistics::~Statistics()
{
    simul->removeAsyncSimulationListener(this);
    //  simul->removeAsyncContainerListener(this);
}

float isClose(const Array<float> &a, const Array<float> &b, float epsilon)
{
    if (a.size() != b.size())
        return false;
    for (int i = 0; i < a.size(); i++)
    {
        if (abs(a[i] - b[i]) > epsilon)
            return false;
    }
    return true;
}

float distance(const Array<float> &a, const Array<float> &b)
{
    if (a.size() != b.size())
        return false;
    float dist = 0;
    for (int i = 0; i < a.size(); i++)
    {
        dist += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sqrt(dist);
}

void Statistics::genStartConcents()
{
    // randomly choose starting concentrations

    const float initConcentBase = Generation::getInstance()->initConcent->x;
    const float initConcentVar = Generation::getInstance()->initConcent->y;

    // recall that energy of primary entities are normalized to 0
    for (auto &ent : simul->entities)
    {
        const float concent = jmax(0.f, initConcentBase + randFloat(-initConcentVar, initConcentVar));
        ent->startConcent = concent;
    }
}

void Statistics::launchSim()
{
    if (nbSim < maxNbSim)
    {
        // set initial values
        // launch simulation
        nbSim++;
        // LOG("nbSim " <<nbSim);
        genStartConcents();
        simul->start(true);
    }
    else
    {
        finishStats();
    }
}

void Statistics::computeStats()
{
    maxNbSim = Generation::getInstance()->numRuns->intValue();
    // we take epsFacto times the epsilon speed *dt.
    epsilon = Generation::getInstance()->epsFactor->intValue() * simul->epsilonEq->floatValue() * simul->dt->floatValue();

    LOG("Epsilon: " << epsilon);
    // processed so far
    nbSim = 0;

    steadyStates.clear();
    launchSim();
}

void Statistics::newMessage(const Simulation::SimulationEvent &ev)
{
    // ignore if not in express mode
    if (!simul->express)
        return;
    switch (ev.type)
    {
    case Simulation::SimulationEvent::FINISHED:
    {
        // process result
        // check if already in steadyStates
        bool found = false;
        for (int i = 0; i < steadyStates.size(); i++)
        {
            if (isClose(steadyStates[i], ev.entityValues, epsilon))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            steadyStates.add(ev.entityValues);
        }
        // launch next simulation
        launchSim();
    }
    break;
    }
}

void Statistics::printSteadyStates()
{
    LOG("Steady state 0: ");
    for (int j = 0; j < steadyStates[0].size(); j++)
    {
        LOG(j << ":  " << steadyStates[0][j]);
    }

    // print differential steady states
    for (int i = 1; i < steadyStates.size(); i++)
    {
        LOG("Diff Steady state " << i << ": ");
        for (int j = 0; j < steadyStates[i].size(); j++)
        {
            float d = steadyStates[i][j] - steadyStates[0][j];
            if (abs(d) > epsilon)
                LOG(j << ":  " << d);
        }
    }
}

void Statistics::finishStats()
{
    // display number of steady states
    LOG("Number of steady states: " << steadyStates.size());
    printSteadyStates();
}