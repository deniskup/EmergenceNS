// #include "kissat/src/kissat.h"

#include "Simulation.h"
#include "EntityManager.h"
#include "ReactionManager.h"
#include "Generation.h"

using namespace std;

juce_ImplementSingleton(Simulation)

    Simulation::Simulation() : ControllableContainer("Simulation"),
                               Thread("Simulation"),
                               curStep(0),
                               ready(false),
                               simNotifier(1000) // max messages async that can be sent at once
{
  simNotifier.dropMessageOnOverflow = false;

  dt = addFloatParameter("dt", "time step in ms", .001, 0.f);
  totalTime = addFloatParameter("Total Time", "Total simulated time in seconds", 1.f, 0.f);
  pointsDrawn = addIntParameter("Points drawn", "Number of points drawn on curves", 100, 1, 500);
  perCent = addIntParameter("Progress", "Percentage of the simulation done", 0, 0, 100);
  perCent->setControllableFeedbackOnly(true);
  finished = addBoolParameter("Finished", "Finished", false);
  finished->setControllableFeedbackOnly(true);
  maxConcent = addFloatParameter("Max. Concent.", "Maximal concentration displayed on the graph", 5.f);
  realTime = addBoolParameter("Real Time", "Print intermediary steps of the simulation", false);
  generated = addBoolParameter("Generated", "Are the entities manually chosen or generated ?", false);
  genTrigger = addTrigger("Generate", "Generate");
  startTrigger = addTrigger("Start/Continue", "Start/Continue");
  genstartTrigger = addTrigger("Gen. & Start", "Gen. & Start");
  restartTrigger = addTrigger("Restart", "Restart");
  cancelTrigger = addTrigger("Cancel", "Cancel");
  autoScale = addBoolParameter("Auto Scale", "Automatically scale to maximal concentration reached", true);
  // ready = addBoolParameter("Ready","Can the simulation be launched ?", false);
}

Simulation::~Simulation()
{
  // Destructor
  stopThread(500);
}

// maybe rand functions to move to a file Util.c later
int randInt(int i, int j)
{
  if (j < i)
  {
    LOGWARNING("Range [" << i << "," << j << "] incorrect, setting to " << i);
    return i;
  }
  if (i == j)
    return i;
  return Random::getSystemRandom().nextInt(Range(i, j + 1));
}

float randFloat()
{
  return Random::getSystemRandom().nextFloat();
}

float randFloat(float a, float b)
{
  if (b < a)
  {
    LOGWARNING("Range [" << a << "," << b << "] incorrect, setting to " << a);
    return a;
  }
  if (a == b)
    return a;
  float r = randFloat();
  return (r * a + (1 - r) * b);
}

void Simulation::clearParams()
{
  entities.clear();
  entitiesDrawn.clear();
  primEnts.clear();
  reactions.clear();
  cycles.clear();
}

void Simulation::updateParams()
{
  simNotifier.addMessage(new SimulationEvent(SimulationEvent::UPDATEPARAMS, this));
}

// to save additional data, different from getJSONdata()
var Simulation::toJSONData()
{
  var data = new DynamicObject();
  data.getDynamicObject()->setProperty("ready", ready);
  data.getDynamicObject()->setProperty("recordConcent", recordConcent);
  data.getDynamicObject()->setProperty("recordEntity", recordEntity);
  data.getDynamicObject()->setProperty("recordDrawn", recordDrawn);
  data.getDynamicObject()->setProperty("recordDrawnEntity", recordDrawnEntity);
  data.getDynamicObject()->setProperty("numLevels", numLevels);

  // entities
  var ents;
  for (auto &e : entities)
  {
    var ent = e->toJSONData();
    ents.append(ent);
  }
  data.getDynamicObject()->setProperty("entities", ents);

  // reactions
  var reacs;
  for (auto &r : reactions)
  {
    var reac = r->toJSONData();
    reacs.append(reac);
  }
  data.getDynamicObject()->setProperty("reactions", reacs);

  // primary entities
  var prim_ents;
  for (auto &e : primEnts)
  {
    var coord = new DynamicObject();
    coord.getDynamicObject()->setProperty("ent_id", e->id);
    prim_ents.append(coord);
  }
  data.getDynamicObject()->setProperty("primEnts", prim_ents);

  // entitiesDrawn
  var entDrawn;
  for (auto &e : entitiesDrawn)
  {
    var coord = new DynamicObject();
    coord.getDynamicObject()->setProperty("ent_id", e->id);
    entDrawn.append(coord);
  }
  data.getDynamicObject()->setProperty("entitiesDrawn", entDrawn);

  return data;
}

void Simulation::importJSONData(var data)
{
  clearParams();
  if (data.isVoid())
    return;
  if (data.getDynamicObject() == nullptr)
    return;

  if (data.getDynamicObject()->hasProperty("ready"))
    ready = data.getDynamicObject()->getProperty("ready");
  if (data.getDynamicObject()->hasProperty("recordConcent"))
    recordConcent = data.getDynamicObject()->getProperty("recordConcent");
  if (data.getDynamicObject()->hasProperty("recordEntity"))
    recordEntity = data.getDynamicObject()->getProperty("recordEntity");
  if (data.getDynamicObject()->hasProperty("recordDrawn"))
    recordDrawn = data.getDynamicObject()->getProperty("recordDrawn");
  if (data.getDynamicObject()->hasProperty("recordDrawnEntity"))
    recordDrawnEntity = data.getDynamicObject()->getProperty("recordDrawnEntity");
  if (data.getDynamicObject()->hasProperty("numLevels"))
    numLevels = data.getDynamicObject()->getProperty("numLevels");

  // entities
  entities.clear();
  if (data.getDynamicObject()->hasProperty("entities"))
  {
    auto ents = data.getDynamicObject()->getProperty("entities").getArray();
    for (auto &evar : *ents)
    {
      entities.add(new SimEntity(evar));
    }
  }

  // reactions
  reactions.clear();
  if (data.getDynamicObject()->hasProperty("reactions"))
  {
    auto reacs = data.getDynamicObject()->getProperty("reactions").getArray();
    for (auto &rvar : *reacs)
    {
      reactions.add(new SimReaction(rvar));
    }
  }

  // entitiesDrawn
  entitiesDrawn.clear();
  if (data.getDynamicObject()->hasProperty("entitiesDrawn"))
  {
    // todo verify array
    auto entDrawns = data.getDynamicObject()->getProperty("entitiesDrawn").getArray();
    for (auto &coord : *entDrawns)
    {
      entitiesDrawn.add(getSimEntityForID(coord["ent_id"]));
    }
  }
  // primEnts
  primEnts.clear();
  if (data.getDynamicObject()->hasProperty("primEnts"))
  {
    auto prim_ents = data.getDynamicObject()->getProperty("primEnts").getArray();
    for (auto &coord : *prim_ents)
    {
      primEnts.add(getSimEntityForID(coord["ent_id"]));
    }
  }

  updateParams();
}

void Simulation::fetchManual()
{
  clearParams();
  for (auto &e : EntityManager::getInstance()->items)
  {
    if (!e->enabled->boolValue())
      continue;
    entities.add(new SimEntity(e));
  }

  for (auto &r : ReactionManager::getInstance()->items)
  {
    if (!r->shouldIncludeInSimulation())
      continue;
    reactions.add(new SimReaction(r));
  }
}

void Simulation::fetchGenerate()
{
  clearParams();
  Generation *gen = Generation::getInstance();

  numLevels = randInt(gen->numLevels->x, gen->numLevels->y);

  // primary entities
  int nbPrimEnts = randInt(gen->primEntities->x, gen->primEntities->y);

  // only rough approximation
  int totalEnts = numLevels * gen->entPerLevNum->x;
  float propShow = (gen->avgNumShow->floatValue()) / totalEnts;
  int nbShow = 0;

  int cur_id = 0;

  Array<Array<SimEntity *>> hierarchyEnt;
  // Array<SimEntity *> primEnts; primEnts is part of Simulation
  for (int idp = 0; idp < nbPrimEnts; idp++)
  {

    float concent = randFloat(gen->concentRange->x, gen->concentRange->y);
    float dRate = randFloat(0., gen->maxDestructionRate->floatValue());
    float cRate = randFloat(0., gen->maxCreationRate->floatValue());
    SimEntity *e = new SimEntity(true, concent, cRate, dRate, 0.f);
    e->level = 0;
    e->id = cur_id;
    cur_id++;
    e->color = Colour::fromHSV(0, 1, 1, 1);
    e->draw = false;
    if (nbShow < gen->avgNumShow->intValue() && randFloat() < propShow)
    { // proba to draw entity
      e->draw = true;
      nbShow++;
    }
    e->composition.insertMultiple(0, 0, nbPrimEnts);
    e->composition.set(idp, 1);
    e->nameFromCompo();
    entities.add(e);
    primEnts.add(e);
  }
  hierarchyEnt.add(primEnts);

  // composite entities

  // poly or exp growth
  float aGrowth = gen->entPerLevA->floatValue();
  float bGrowth = gen->entPerLevB->floatValue();
  float u = gen->entPerLevUncert->intValue();

  // reactions per entity
  int minReacPerEnt = gen->reactionsPerEntity->x;
  int maxReacPerEnt = gen->reactionsPerEntity->y;
  for (int level = 1; level < numLevels; level++)
  {
    Array<SimEntity *> levelEnt;
    int numEnts = 1;
    switch (gen->growthEntitiesPerLevel->getValueDataAsEnum<Generation::GrowthMode>())
    {
    case Generation::CONSTANT:
      numEnts = randInt(gen->entPerLevNum->x, gen->entPerLevNum->y);
      break;
    case Generation::POLYNOMIAL:
      numEnts = (int)(aGrowth * pow(level, bGrowth) + randInt(-u, u));
      break;
    case Generation::EXPONENTIAL:
      numEnts = (int)(aGrowth * pow(nbPrimEnts, level) + randInt(-u, u));
      break;
    }
    numEnts = jmax(1, numEnts); // at least one entity per level, maybe not necessary later but needed for now.

    Array<CompoDecomps *> compos; // first working thing, Hashtable or sorted array later ?
    for (int lev1 = 0; lev1 < level; lev1++)
    {
      int lev2 = level - lev1 - 1; // complement level
      if (lev2 < lev1)
        break; // no need to do the reverse cases
      // compute all combinations
      for (auto &ent1 : hierarchyEnt[lev1])
      {
        for (auto &ent2 : hierarchyEnt[lev2])
        {
          Array<int> newCompo;
          for (int i = 0; i < nbPrimEnts; i++)
          {
            newCompo.add(ent1->composition[i] + ent2->composition[i]);
          }
          // loop through existing ones
          bool exists = false;
          for (auto &cd : compos)
          {
            if (cd->compo == newCompo)
            { // if exists
              // NLOG("Compos","Exists "<<ent1->name<< " + "<<ent2->name);
              cd->add(ent1, ent2);
              exists = true;
              break;
            }
          }
          if (!exists)
          {
            //  NLOG("Compos","New "<<ent1->name<< " + "<<ent2->name);
            Array<Decomp> dec(make_pair(ent1, ent2));
            compos.add(new CompoDecomps(newCompo, dec));
          }
          if (lev1 == lev2 && ent1 == ent2)
            break; // to avoid duplicates, we stop when reaching the diagonal
        }
      }
    }
    // bound numEnts by the number of compositions.
    // NLOG("Compos", compos.size() << " at level " << level);
    numEnts = jmin(numEnts, compos.size());
    for (int ide = 0; ide < numEnts; ide++)
    {
      float concent = 0.; // no initial presence of composite entities
      float dRate = randFloat(0., gen->maxDestructionRate->floatValue()) / level;
      float uncert = gen->energyUncertainty->floatValue();
      float energy = level * gen->energyPerLevel->floatValue() + randFloat(-uncert, uncert);
      SimEntity *e = new SimEntity(false, concent, 0., dRate, energy);
      e->level = level;
      e->color = Colour::fromHSV(level * 1. / numLevels, 1, 1, 1); // color depends only on level
      e->draw = false;
      e->id = cur_id;
      cur_id++;
      if (nbShow < gen->avgNumShow->intValue() && randFloat() < propShow)
      { // proba to draw entity
        e->draw = true;
        nbShow++;
      }
      int idComp = randInt(0, compos.size() - 1);
      e->composition = compos[idComp]->compo;
      // NLOG("New entity", e->name << " = " << dec.first->name << " + " << dec.second->name);
      e->nameFromCompo();
      entities.add(e);
      levelEnt.add(e);

      // reactions producing e
      int nbReac = randInt(minReacPerEnt, maxReacPerEnt);
      int nbDecomps = compos[idComp]->decomps.size();
      nbReac = jmin(nbReac, nbDecomps);

      for (int ir = 0; ir < nbReac; ir++)
      {
        int idDecomp = randInt(0, compos[idComp]->decomps.size() - 1);
        SimEntity *e1 = compos[idComp]->decomps[idDecomp].first;
        SimEntity *e2 = compos[idComp]->decomps[idDecomp].second;
        // remove this decomposition
        compos[idComp]->decomps.remove(idDecomp);

        // choice of activation barrier
        float barrier = randFloat(0., gen->maxEnergyBarrier->floatValue());
        // GA+GB
        float energyLeft = e1->freeEnergy + e2->freeEnergy;
        // GAB
        float energyRight = e->freeEnergy;
        // total energy G* of the reaction: activation + max of GA+GB and GAB.
        float energyStar = barrier + jmax(energyLeft, energyRight);
        // k1=exp(GA+GB-G*)
        float aRate = exp(energyLeft - energyStar);
        // k2=exp(GAB-G*)
        float disRate = exp(energyRight - energyStar);
        reactions.add(new SimReaction(e1, e2, e, aRate, disRate));
        NLOG("New reaction", e->name << " = " << e1->name << " + " << e2->name);
      }
      compos.remove(idComp);
    }
    hierarchyEnt.add(levelEnt);
  }
  // ready->setValue(true);
  ready = true;
  entitiesDrawn.clear();
  for (auto &ent : entities)
  {
    if (ent->draw)
      entitiesDrawn.add(ent);
  }

  updateParams();
}

void Simulation::start()
{
  startTrigger->setEnabled(false);
  simNotifier.addMessage(new SimulationEvent(SimulationEvent::WILL_START, this));
  // listeners.call(&SimulationListener::simulationWillStart, this);

  if (generated->boolValue())
  {
    if (!ready)
    {
      LOGWARNING("Start with no parameters, generating parameters");
      fetchGenerate();
    }
  }
  else
  {
    fetchManual();
  }

  Array<float> entityValues;
  Array<Colour> entityColors;

  for (auto &ent : entitiesDrawn)
  {
    entityValues.add(ent->concent);
    entityColors.add(ent->color);
  }

  simNotifier.addMessage(new SimulationEvent(SimulationEvent::STARTED, this, 0, entityValues, entityColors));
  // listeners.call(&SimulationListener::simulationStarted, this);
  recordConcent = 0.;
  recordDrawn = 0.;
  checkPoint = maxSteps / pointsDrawn->intValue(); // draw once every "chekpoints" steps
  checkPoint = jmax(1, checkPoint);
  startThread();
}

void Simulation::nextStep()
{

  bool isCheck = (curStep % checkPoint == 0);
  if (displayLog && isCheck)
  {
    NLOG(niceName, "New Step : " << curStep);
    wait(1);
  }
  if (curStep >= maxSteps)
  {
    stop();
    return;
  }
  // add primary->boolValue() entities
  for (auto &ent : entities)
  {
    if (ent->primary)
    {
      ent->concent += ent->creationRate * dt->floatValue();
    }
    ent->decrease(ent->concent * ent->destructionRate * dt->floatValue());
  }

  // loop through reactions
  for (auto &reac : reactions)
  {
    // shorter names
    float reac1Concent = reac->reactant1->concent;
    float reac2Concent = reac->reactant2->concent;
    float prodConcent = reac->product->concent;
    // compute product of reactants concentrations
    float reacConcent = reac1Concent * reac2Concent;

    float directIncr = reacConcent * reac->assocRate * dt->floatValue();
    float reverseIncr = prodConcent * reac->dissocRate * dt->floatValue();

    // adjust the increments depending on available entities
    directIncr = jmin(directIncr, reac1Concent, reac2Concent);
    reverseIncr = jmin(reverseIncr, prodConcent);

    // increase entities
    reac->reactant1->increase(reverseIncr);
    reac->reactant2->increase(reverseIncr);
    reac->product->increase(directIncr);

    // decrease entities
    reac->reactant1->decrease(directIncr);
    reac->reactant2->decrease(directIncr);
    reac->product->decrease(reverseIncr);
  }

  curStep++;
  perCent->setValue((int)((curStep * 100) / maxSteps));

  for (auto &ent : entities)
  {
    if (isinf(ent->concent))
    {
      NLOG(niceName, "Concentration of entity " << ent->name << " exceeded capacity");
      finished->setValue(true);
      return;
    }
    if (ent->concent > recordConcent)
    {
      recordConcent = ent->concent;
      recordEntity = ent->name;
    }

    if (ent->draw && ent->concent > recordDrawn)
    {
      recordDrawn = ent->concent;
      recordDrawnEntity = ent->name;
    }
  }

  if (displayLog)
  {
    for (auto &e : entities)
    {
      if (e->draw && displayLog)
        NLOG(niceName, e->toString());
    }
  }

  Array<float> entityValues;
  for (auto &ent : entitiesDrawn)
  {
    entityValues.add(ent->concent);
  }

  // call only pointsDrawn time
  if (isCheck)
  {
    simNotifier.addMessage(new SimulationEvent(SimulationEvent::NEWSTEP, this, curStep, entityValues));
    // listeners.call(&SimulationListener::newStep, this);
  }
}

void Simulation::stop()
{
  finished->setValue(true);
}

void Simulation::cancel()
{
  stopThread(500);
}

void Simulation::run()
{
  curStep = 0;
  NLOG(niceName, "Start thread");
  finished->setValue(false);
  while (!finished->boolValue() && !threadShouldExit())
  {
    nextStep();
  }

  NLOG(niceName, "End thread");
  NLOG(niceName, "Record Concentration: " << recordConcent << " for entity " << recordEntity);
  NLOG(niceName, "Record Drawn Concentration: " << recordDrawn << " for entity " << recordDrawnEntity);
  simNotifier.addMessage(new SimulationEvent(SimulationEvent::FINISHED, this));
  // listeners.call(&SimulationListener::simulationFinished, this);
  startTrigger->setEnabled(true);
}

SimEntity *Simulation::getSimEntityForEntity(Entity *e)
{
  for (auto &se : entities)
  {
    if (se->entity == e)
      return se;
  }
  return nullptr;
}

SimEntity *Simulation::getSimEntityForID(int id)
{
  for (auto &se : entities)
  {
    if (se->id == id)
      return se;
  }
  return nullptr;
}

void Simulation::onContainerTriggerTriggered(Trigger *t)
{
  if (t == startTrigger)
    start();
  else if (t == genTrigger)
    fetchGenerate();
  else if (t == genstartTrigger)
  {
    fetchGenerate();
    start();
  }
  else if (t == restartTrigger)
  {
    for (auto &e : entities)
    {
      e->concent = e->startConcent;
    }
    start();
  }
  else if (t == cancelTrigger)
    cancel();
}

void Simulation::onContainerParameterChanged(Parameter *p)
{
  ControllableContainer::onContainerParameterChanged(p);
  if (p == dt || p == totalTime)
  {
    maxSteps = (int)(totalTime->floatValue() / dt->floatValue());
  }
  if (p == generated)
  {
    // set ready to false if we switched to generated
    ready = !generated->boolValue();
  }
}

SimEntity::SimEntity(Entity *e) : SimEntity(e->primary->boolValue(), e->concent->floatValue(), e->creationRate->floatValue(), e->destructionRate->floatValue(), e->freeEnergy->floatValue())
{
  name = e->niceName;
  entity = e;
  color = e->itemColor->getColor();
}

SimEntity::SimEntity(bool isPrimary, float concent, float cRate, float dRate, float freeEnergy) : primary(isPrimary), concent(concent), startConcent(concent), creationRate(cRate), destructionRate(dRate), freeEnergy(freeEnergy),
                                                                                                  name("New entity"), entity(nullptr)
{
}

var color2JSON(Colour col)
{
  var data = new DynamicObject();
  data.getDynamicObject()->setProperty("H", col.getHue());
  data.getDynamicObject()->setProperty("S", col.getSaturation());
  data.getDynamicObject()->setProperty("B", col.getBrightness());
  data.getDynamicObject()->setProperty("A", col.getAlpha());
  return data;
}

Colour JSON2Color(var data)
{
  return Colour::fromHSV(data["H"], data["S"], data["B"], data["A"]);
}

SimEntity::SimEntity(var data)
{
  if (data.isVoid())
    return;
  if (data.getDynamicObject() == nullptr)
    return;

  if (data.getDynamicObject()->hasProperty("name"))
    name = data.getDynamicObject()->getProperty("name");

  if (data.getDynamicObject()->hasProperty("color"))
    color = JSON2Color(data.getDynamicObject()->getProperty("color"));

  if (data.getDynamicObject()->hasProperty("primary"))
    primary = data.getDynamicObject()->getProperty("primary");

  if (data.getDynamicObject()->hasProperty("id"))
    id = data.getDynamicObject()->getProperty("id");

  if (data.getDynamicObject()->hasProperty("concent"))
    concent = data.getDynamicObject()->getProperty("concent");

  if (data.getDynamicObject()->hasProperty("startConcent"))
    startConcent = data.getDynamicObject()->getProperty("startConcent");

  if (data.getDynamicObject()->hasProperty("creationRate"))
    creationRate = data.getDynamicObject()->getProperty("creationRate");

  if (data.getDynamicObject()->hasProperty("destructionRate"))
    destructionRate = data.getDynamicObject()->getProperty("destructionRate");

  if (data.getDynamicObject()->hasProperty("freeEnergy"))
    freeEnergy = data.getDynamicObject()->getProperty("freeEnergy");

  if (data.getDynamicObject()->hasProperty("level"))
    level = data.getDynamicObject()->getProperty("level");

  if (data.getDynamicObject()->hasProperty("draw"))
    draw = data.getDynamicObject()->getProperty("draw");

  if (data.getDynamicObject()->hasProperty("composition"))
  {
    Array<var> *comp = data.getDynamicObject()->getProperty("composition").getArray();
    for (auto &coord : *comp)
    {
      composition.add(data.getDynamicObject()->getProperty("coord"));
    }
  }
}

var SimEntity::toJSONData()
{
  var data = new DynamicObject();
  data.getDynamicObject()->setProperty("name", name);
  data.getDynamicObject()->setProperty("color", color2JSON(color));
  data.getDynamicObject()->setProperty("primary", primary);
  data.getDynamicObject()->setProperty("id", id);
  data.getDynamicObject()->setProperty("concent", concent);
  data.getDynamicObject()->setProperty("startConcent", startConcent);
  data.getDynamicObject()->setProperty("creationRate", creationRate);
  data.getDynamicObject()->setProperty("destructionRate", destructionRate);
  data.getDynamicObject()->setProperty("freeEnergy", freeEnergy);
  data.getDynamicObject()->setProperty("level", level);
  data.getDynamicObject()->setProperty("draw", draw);
  var comp;
  for (auto &i : composition)
  {
    var coord = new DynamicObject();
    coord.getDynamicObject()->setProperty("coord", i);
    comp.append(coord);
  }
  data.getDynamicObject()->setProperty("composition", comp);
  return data;
}

void SimEntity::increase(float incr)
{
  concent += incr;
}

void SimEntity::decrease(float decr)
{
  concent = jmax(0.f, concent - decr);
}

void SimEntity::nameFromCompo()
{
  name = "";
  for (auto &i : composition)
  {
    name += String(i);
  }
}

String SimEntity::toString() const
{
  return "[Entity " + name + " : " + String(concent) + "]";
}

SimReaction::SimReaction(Reaction *r) : assocRate(r->assocRate->floatValue()),
                                        dissocRate(r->dissocRate->floatValue())
{
  reactant1 = Simulation::getInstance()->getSimEntityForEntity(dynamic_cast<Entity *>(r->reactant1->targetContainer.get()));
  reactant2 = Simulation::getInstance()->getSimEntityForEntity(dynamic_cast<Entity *>(r->reactant2->targetContainer.get()));
  product = Simulation::getInstance()->getSimEntityForEntity(dynamic_cast<Entity *>(r->product->targetContainer.get()));
}

SimReaction::SimReaction(SimEntity *r1, SimEntity *r2, SimEntity *p, float aRate, float dRate) : reactant1(r1), reactant2(r2), product(p), assocRate(aRate), dissocRate(dRate)
{
}

SimReaction::SimReaction(var data)
{
  if (data.isVoid())
    return;
  if (data.getDynamicObject() == nullptr)
    return;

  Simulation *simul = Simulation::getInstance();
  if (data.getDynamicObject()->hasProperty("reactant1_id"))
    reactant1 = simul->getSimEntityForID(data["reactant1_id"]);

  if (data.getDynamicObject()->hasProperty("reactant2_id"))
    reactant2 = simul->getSimEntityForID(data["reactant2_id"]);

  if (data.getDynamicObject()->hasProperty("product_id"))
    product = simul->getSimEntityForID(data["product_id"]);

  if (data.getDynamicObject()->hasProperty("assocRate"))
    assocRate = data["assocRate"];

  if (data.getDynamicObject()->hasProperty("dissocRate"))
    dissocRate = data["dissocRate"];
}

var SimReaction::toJSONData()
{
  var data = new DynamicObject();
  data.getDynamicObject()->setProperty("reactant1_id", reactant1->id);
  data.getDynamicObject()->setProperty("reactant2_id", reactant2->id);
  data.getDynamicObject()->setProperty("product_id", product->id);

  data.getDynamicObject()->setProperty("assocRate", assocRate);
  data.getDynamicObject()->setProperty("dissocRate", dissocRate);

  return data;
}

PAC::PAC()
{
}

String PAC::toString()
{
  String res;
  for (auto rd : reacDirs)
  {
    auto r = rd.first;
    bool d = rd.second;
    auto p = r->product;
    auto r1 = r->reactant1;
    auto r2 = r->reactant2;
    String plus = "+";
    String r1name = r1->name;
    String r2name = r2->name;
    if (!entities.contains(r1))
    {
      plus = "";
      r1name = "";
    }
    if (!entities.contains(r2))
    {
      plus = "";
      r2name = "";
    }
    if (d)
    { // 1->2
      res += r->product->name + "->" + r1name + plus + r2name+ " ";
    }
    else
    { // 2->1
      res += r1name + plus + r2name + "->" + r->product->name+" ";
    }
  }
  return res;
}

bool PAC::includedIn(PAC *pac)
{
  for (auto e : entities)
  {
    if (!pac->entities.contains(e))
      return false;
  }
  for (auto rd : reacDirs)
  {
    if (!pac->reacDirs.contains(rd))
      return false;
  }
  return true;
}

void Simulation::addCycle(PAC *newpac)
{
  // we only test if is is included in existing one, other direction is taken care of by SAT solver
  for (int i = 0; i < cycles.size(); i++)
  {
    if (newpac->includedIn(cycles[i]))
    {
      cycles.remove(i);
    }
  }
  cycles.add(newpac);
}

void Simulation::printPACs()
{
  for(auto pac:cycles){
    cout<<pac->toString()<<endl;
  }
}
