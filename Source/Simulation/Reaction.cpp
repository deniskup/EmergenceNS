#include "Settings.h"
#include "Reaction.h"
#include "EntityManager.h"
#include "Simulation.h"

#define CONSTANTS_PRECISION 7

Reaction::Reaction(var params) : BaseItem(getTypeString() + " 1")
{
  addParams();

  // 2->1 by default
  if (!Engine::mainEngine->isLoadingFile)
  {
    addReactant(nullptr);
    addReactant(nullptr);
    addProduct(nullptr);
  }
}

Reaction::Reaction(SimReaction *r) : BaseItem(r->name)
{
  addParams();

  simReac = r;

  // Entity *e1 = r->reactant1->entity;
  // Entity *e2 = r->reactant2->entity;
  // Entity *e3 = r->product->entity;

  // Entity *e1 = r->reactants[0]->entity;
  // Entity *e2 = r->reactants[1]->entity;
  // Entity *e3 = r->products[0]->entity;
  // do the above with safeguards

  // to change
  //  Entity *e1 = nullptr;
  //  Entity *e2 = nullptr;
  //  Entity *e3 = nullptr;
  //  if (r->reactants.size() > 0)
  //    e1 = r->reactants[0]->entity;
  //  if (r->reactants.size() > 1)
  //    e2 = r->reactants[1]->entity;
  //  if (r->products.size() > 0)
  //    e3 = r->products[0]->entity;

  // reactant1->setValueFromTarget(e1, false);
  // reactant2->setValueFromTarget(e2, false);
  // product->setValueFromTarget(e3, false);

  for (auto e : r->reactants)
  {
    addReactant(e->entity);
  }

  for (auto e : r->products)
  {
    addProduct(e->entity);
  }

  if (r->energy < -.5f)
    r->computeBarrier();
  energy->setValue(r->energy);
  assocRate->setValue(r->assocRate);
  dissocRate->setValue(r->dissocRate);
  r->reaction = this;

  // updateLinks();
}

Reaction::~Reaction()
{
}

void Reaction::addParams()
{
  saveAndLoadRecursiveData = true;

  // reactant1 = addTargetParameter("Reactant 1", "Reactant 1", EntityManager::getInstance());
  // reactant2 = addTargetParameter("Reactant 2", "Reactant 2", EntityManager::getInstance());
  // product = addTargetParameter("Product", "Product", EntityManager::getInstance());

  // reactant1->targetType = TargetParameter::CONTAINER;
  // reactant1->maxDefaultSearchLevel = 0;

  // reactant2->targetType = TargetParameter::CONTAINER;
  // reactant2->maxDefaultSearchLevel = 0;

  // product->targetType = TargetParameter::CONTAINER;
  // product->maxDefaultSearchLevel = 0;

  reactants.reset(new ControllableContainer("Reactants"));
  reactants->userCanAddControllables = true;
  reactants->userAddControllablesFilters.add(TargetParameter::getTypeStringStatic());
  addChildControllableContainer(reactants.get());

  products.reset(new ControllableContainer("Products"));
  products->userCanAddControllables = true;
  products->userAddControllablesFilters.add(TargetParameter::getTypeStringStatic());
  addChildControllableContainer(products.get());

  energy = addFloatParameter("Energy Barrier", "Additional energy barrier of the reaction", 1.f, 0.f, 100.f);

  assocRate = addFloatParameter("Association rate", "Reaction speed left to right", .5f);
  dissocRate = addFloatParameter("Dissociation rate", "Reaction speed right to left", .5f);
  assocRate->setControllableFeedbackOnly(true);
  dissocRate->setControllableFeedbackOnly(true);
  assocRate->setAttributeInternal("stringDecimals", CONSTANTS_PRECISION);
  dissocRate->setAttributeInternal("stringDecimals", CONSTANTS_PRECISION);

  showWarningInUI = true;
}

void Reaction::controllableAdded(Controllable *c)
{
  if (c->parentContainer == reactants.get() || c->parentContainer == products.get())
  {
    TargetParameter *tp = (TargetParameter *)c;
    // if (!isCurrentlyLoadingData)
    // { // rename only if not loading data
    //   String targetName;
    //   if (c->parentContainer == reactants.get())
    //   {
    //     targetName = "Reactant 1";
    //   }
    //   else
    //   {
    //     targetName = "Product 1";// + String(products->controllables.size());
    //   }
    //   tp->setNiceName(tp->parentContainer->getUniqueNameInContainer(targetName));
    // }
    tp->targetType = TargetParameter::CONTAINER;
    tp->maxDefaultSearchLevel = 0;
    tp->setRootContainer(EntityManager::getInstance());
  }
}

void Reaction::controllableRemoved(Controllable *)
{
  updateWarnAndRates();
}

void Reaction::addReactant(Entity *e)
{
  TargetParameter *tp = reactants->addTargetParameter("Reactant " + String(reactants->controllables.size() + 1), "Reactant " + String(reactants->controllables.size() + 1), EntityManager::getInstance());
  if (e != NULL)
    tp->setValueFromTarget(e, false);
  tp->saveValueOnly = false;
  tp->isRemovableByUser = true;
}

void Reaction::addProduct(Entity *e)
{
  TargetParameter *tp = products->addTargetParameter("Product " + String(products->controllables.size() + 1), "Product " + String(products->controllables.size() + 1), EntityManager::getInstance());
  if (e != NULL)
    tp->setValueFromTarget(e, false);
  tp->saveValueOnly = false;
  tp->isRemovableByUser = true;
}

void Reaction::clearReactants()
{
  while (!reactants->controllables.isEmpty())
    reactants->removeControllable(reactants->controllables.getLast());
}

void Reaction::clearProducts()
{
  while (!products->controllables.isEmpty())
    products->removeControllable(products->controllables.getLast());
}
// void Reaction::updateLinks()
// {
//   // update links to entity to listen to changes
//   linkedR1 = reactant1->targetContainer.get();
//   registerLinkedInspectable(linkedR1.get());
//   if (linkedR1 != nullptr)
//   {
//     ((Entity *)linkedR1.get())->enabled->addParameterListener(this);
//     ((Entity *)linkedR1.get())->freeEnergy->addParameterListener(this);
//   }
//   linkedR2 = reactant2->targetContainer.get();
//   registerLinkedInspectable(linkedR2.get());
//   if (linkedR2 != nullptr)
//   {
//     ((Entity *)linkedR2.get())->enabled->addParameterListener(this);
//     ((Entity *)linkedR2.get())->freeEnergy->addParameterListener(this);
//   }

//   linkedP = product->targetContainer.get();
//   registerLinkedInspectable(linkedP.get());
//   if (linkedP != nullptr)
//   {
//     ((Entity *)linkedP.get())->enabled->addParameterListener(this);
//     ((Entity *)linkedP.get())->freeEnergy->addParameterListener(this);
//   }

//   updateWarnAndRates();
// }

void Reaction::autoRename()
{

  // String newName = reactant1->targetContainer.get()->niceName + "+" + reactant2->targetContainer.get()->niceName + "=" + product->targetContainer.get()->niceName;

  String newName;
  for (auto c : reactants->controllables)
  {
    if (newName.isNotEmpty())
      newName += "+";
    newName += c->niceName;
  }
  newName += "=";
  for (auto c : products->controllables)
  {
    if (newName.isNotEmpty())
      newName += "+";
    newName += c->niceName;
  }

  if (newName != niceName)
  {
    setNiceName(newName);
  }
}

// commented old version with 2 reactants and 1products
// void Reaction::inferEntities()
// {
//   // cout << "trying to infer entities" << endl;
//   //  if name is A+B, infer entities
//   String name = niceName;
//   int pos = name.indexOfChar('+');
//   int pos2 = name.indexOfChar('=');
//   if (pos > 0)
//   {
//     String name1 = name.substring(0, pos).trim();
//     String name2;
//     String namep;
//     if (pos2 > 0)
//     {
//       name2 = name.substring(pos + 1, pos2).trim();
//       namep = name.substring(pos2 + 1).trim();
//     }
//     else
//     {
//       name2 = name.substring(pos + 1).trim();
//       for (int i = 0; i < name1.length(); i++)
//       {
//         // namep will be inferred from name1 and name2 by adding the corresponding digits
//         namep += (char)((name1[i] - '0') + (name2[i] - '0') + '0');
//       }
//     }

//     Entity *e1 = EntityManager::getInstance()->getEntityFromName(name1);
//     Entity *e2 = EntityManager::getInstance()->getEntityFromName(name2);
//     if (e1 != nullptr && e2 != nullptr)
//     {
//       reactant1->setValueFromTarget(e1, false);
//       reactant2->setValueFromTarget(e2, false);
//       // compute product by summing up reactants

//       Entity *ep = EntityManager::getInstance()->getEntityFromName(namep);
//       if (ep != nullptr)
//       {

//         product->setValueFromTarget(ep, false);
//       }
//       else
//       {
//         NLOG("InferEntities", "Could not infer product " + namep + " for reaction " + name);
//         return;
//       }
//     }
//     // rename to follow convention
//     String newName = name1 + "+" + name2 + "=" + namep;
//     if (newName != name)
//       niceName = newName;
//     // updateLinks();
//   }

void Reaction::inferEntities()
{
  if (isCurrentlyLoadingData)
    return;
  // only infer for empty reactions
  // if (!newReac)
  //   return;
  // DBG("Infering entities for reaction " + niceName);
  String name = niceName;
  int pos = name.indexOfChar('+');
  int pos2 = name.indexOfChar('=');

  if (pos > 0)
  {
    StringArray reactantNames;
    StringArray productNames;

    // Parse reactants and products from name
    if (pos2 > 0)
    {
      String reactantsPart = name.substring(0, pos2).trim();
      String productsPart = name.substring(pos2 + 1).trim();

      reactantNames.addTokens(reactantsPart, "+", "\"");
      productNames.addTokens(productsPart, "+", "\"");
    }
    else
    {
      NLOG("InferEntities", "Invalid reaction name format: " + name);
      return;
    }

    // clear reactants and products
    clearReactants();
    clearProducts();

    // Infer entities from reactant names
    for (int i = 0; i < reactantNames.size(); ++i)
    {
      String reactantName = reactantNames[i].trim();
      Entity *reactantEntity = EntityManager::getInstance()->getEntityFromName(reactantName);
      if (reactantEntity != nullptr)
      {
        addReactant(reactantEntity);
      }
      else
      {
        NLOG("InferEntities", "Could not infer reactant " + reactantName);
        return;
      }
    }

    // Infer entities from product names
    for (int i = 0; i < productNames.size(); ++i)
    {
      String productName = productNames[i].trim();
      Entity *productEntity = EntityManager::getInstance()->getEntityFromName(productName);
      if (productEntity != nullptr)
      {
        addProduct(productEntity);
      }
      else
      {
        NLOG("InferEntities", "Could not infer product " + productName);
        return;
      }
    }

    // Construct the new reaction name following the convention
    String newName;
    for (int i = 0; i < reactantNames.size(); ++i)
    {
      if (i > 0)
        newName += "+";
      newName += reactantNames[i];
    }
    newName += "=";
    for (int i = 0; i < productNames.size(); ++i)
    {
      if (i > 0)
        newName += "+";
      newName += productNames[i];
    }

    // Update the reaction name if it has changed
    if (newName != name)
    {
      niceName = newName;
    }
  }
}

void Reaction::onContainerNiceNameChanged()
{
  // inferEntities();
  // change name of SimReaction if exists
  if (simReac != nullptr)
  {
    simReac->name = niceName;
  }
}

void Reaction::onControllableFeedbackUpdateInternal(ControllableContainer *cc, Controllable *c)
{
  if (cc == reactants.get() || cc == products.get())
  {
    TargetParameter *tp = (TargetParameter *)c;
    if (Entity *e = dynamic_cast<Entity *>(tp->previousTargetContainer.get()))
    {

      // unregister old one
      unregisterLinkedInspectable(e);
      e->enabled->removeParameterListener(this);
      e->freeEnergy->removeParameterListener(this);
    }

    if (Entity *e = tp->getTargetContainerAs<Entity>())
    {

      registerLinkedInspectable(e);

      e->enabled->addParameterListener(this);
      e->freeEnergy->addParameterListener(this);
    }
  }
  updateWarnAndRates();
}

void Reaction::onExternalParameterValueChanged(Parameter *p)
{
  updateWarnAndRates();
}

void Reaction::afterLoadJSONDataInternal()
{
  updateWarnAndRates();
}

void Reaction::updateWarnAndRates()
{
  if (isCurrentlyLoadingData)
    return;
  bool someWarn = false;

  StringArray disabledReactantsNames;
  StringArray disabledProductsNames;

  float energyLeft = 0;
  for (auto c : reactants->controllables)
  {

    Entity *e = ((TargetParameter *)c)->getTargetContainerAs<Entity>();

    if (e == NULL || !e->enabled->boolValue())
    {
      disabledReactantsNames.add(c->niceName);
    }
    else
    {
      energyLeft += e->freeEnergy->floatValue();
    }
  }

  if (disabledReactantsNames.size() > 0)
  {
    setWarningMessage("Some reactants are disabled or null: " + disabledReactantsNames.joinIntoString(", "), "DisabledReactants");
    someWarn = true;
  }
  else
    clearWarning("DisabledReactants");

  float energyRight = 0;

  for (auto c : products->controllables)
  {
    Entity *e = ((TargetParameter *)c)->getTargetContainerAs<Entity>();

    if (e == NULL || !e->enabled->boolValue())
    {
      disabledProductsNames.add(c->niceName);
    }
    else
    {
      energyRight += e->freeEnergy->floatValue();
    }
  }

  if (disabledProductsNames.size() > 0)
  {
    setWarningMessage("Some products are disabled or null: " + disabledProductsNames.joinIntoString(", "), "DisabledProducts");
    someWarn = true;
  }
  else
    clearWarning("DisabledProducts");

  if (someWarn)
    return;
  // energies of reactants and products
  // GA+GB

  // to update with new system
  // float energyLeft = ((Entity *)linkedR1.get())->freeEnergy->floatValue() + ((Entity *)linkedR2.get())->freeEnergy->floatValue();
  // GAB
  // float energyRight = ((Entity *)linkedP.get())->freeEnergy->floatValue();
  // energyLeft and right have been computed above

  // total energy G* of the reaction: activation + max of GA+GB and GAB.
  float energyStar = energy->floatValue() + jmax(energyLeft, energyRight);
  // k1=exp(GA+GB-G*)
  assocRate->setValue(exp(energyLeft - energyStar));
  // k2=exp(GAB-G*)
  float newDissocRate = exp(energyRight - energyStar);
  dissocRate->setValue(newDissocRate);

  // to implement better later, via listening functions
  if (simReac != nullptr)
  {
    simReac->toImport = true;
    if (Settings::getInstance()->autoLoadLists->boolValue())
    {
      simReac->importFromManual();
    }
    simReac->reaction = this;
  }
}

bool Reaction::shouldIncludeInSimulation()
{
  if (!enabled->boolValue())
    return false;

  // test that all entities are enabled

  // if (linkedR1 == nullptr || !((Entity *)linkedR1.get())->enabled->boolValue())
  //   return false;
  // if (linkedR2 == nullptr || !((Entity *)linkedR2.get())->enabled->boolValue())
  //   return false;
  // if (linkedP == nullptr || !((Entity *)linkedP.get())->enabled->boolValue())
  //   return false;

  for (auto c : reactants->controllables)
  {
    Entity *e = ((TargetParameter *)c)->getTargetContainerAs<Entity>();
    if (e == NULL || !e->enabled->boolValue())
      return false;
  }

  for (auto c : products->controllables)
  {
    Entity *e = ((TargetParameter *)c)->getTargetContainerAs<Entity>();
    if (e == NULL || !e->enabled->boolValue())
      return false;
  }

  return true;
}

// when a parameter is changed, update the reaction
void Reaction::onContainerParameterChanged(Parameter *p)
{
  updateWarnAndRates();
}
