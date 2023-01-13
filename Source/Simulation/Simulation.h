

#pragma once

#include <JuceHeader.h>
using namespace juce;
using namespace std;

class Entity;
class Reaction;

class SimEntity
{
public:
  SimEntity(Entity *e);
  SimEntity(bool isPrimary, float concent, float cRate, float dRate, float freeEnergy);

  String name;
  Entity *entity; // sourceEntity

  Colour color;
  bool primary;
  float concent;
  float creationRate;
  float destructionRate;
  float freeEnergy;

  int level;
  bool draw=true;


  void increase(float incr);
  void decrease(float decr);

  String toString() const;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimEntity);
};

class SimReaction
{
public:
  SimReaction(Reaction *e);
  SimReaction(SimEntity *r1, SimEntity *r2, SimEntity *p, float assocRate, float dissocRate);

  SimEntity *reactant1;
  SimEntity *reactant2;
  SimEntity *product;

  float assocRate;
  float dissocRate;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimReaction);
};

class Simulation : public ControllableContainer,
                   public Thread

{
public:
  juce_DeclareSingleton(Simulation, true);
  Simulation();
  ~Simulation();

  IntParameter *maxSteps;
  IntParameter *curStep;
  IntParameter *perCent;
  BoolParameter *finished;
  FloatParameter *dt;
  FloatParameter *totalTime;
  BoolParameter *generated;
  BoolParameter *autoScale;

  float recordConcent; // record the higher concentration reached
  String recordEntity;
  int checkPoint; // every checkPoint steps, wait and log
  bool displayLog=false;
  
  // these ones are for display
  FloatParameter *maxConcent;
  BoolParameter *realTime; // do we print intermediary steps ?

  Trigger *startTrigger;
  Trigger *cancelTrigger;

  OwnedArray<SimEntity> entities;    // all entities
  OwnedArray<SimReaction> reactions; // all reactions

  void fetchGenerate();
  void fetchManual();
  void start();
  void nextStep();
  void stop();
  void cancel();

  void run() override;

  SimEntity *getSimEntityForEntity(Entity *e);

  void onContainerTriggerTriggered(Trigger *t) override;
  void onContainerParameterChanged(Parameter *p) override;

  class SimulationListener
  {
  public:
    virtual ~SimulationListener() {}
    virtual void newStep(Simulation *){};
    virtual void simulationWillStart(Simulation *){};
    virtual void simulationStarted(Simulation *){};
    virtual void simulationFinished(Simulation *){};
  };

  ListenerList<SimulationListener> listeners;
  void addSimulationListener(SimulationListener *newListener) { listeners.add(newListener); }
  void removeSimulationListener(SimulationListener *listener) { listeners.remove(listener); }
};
