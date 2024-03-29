#include "Generation.h"

juce_ImplementSingleton(Generation);

Generation::Generation() : ControllableContainer("Generation")
{

    primEntities = addIntParameter("Primary entities", "Number of primary entities", 2, 1);
    // primEntities->x = 3;
    // primEntities->y = 10;

    numLevels = addIntParameter("Levels", "Number of levels", 4, 1);
    // numLevels->x = 10;
    // numLevels->y = 20;

    growthEntitiesPerLevel = addEnumParameter("Growth entities/level", "How the number of entities increase per level. Constant gives n+range[-u,u], Polynomial is a*level^b+range[-u,u], Propreactions fixed the number of entities just by selecting a proportion of reactions");
    growthEntitiesPerLevel->addOption("Constant", CONSTANT)->addOption("Polynomial", POLYNOMIAL)->addOption("PropReactions", PROPREACTIONS);
    // growthEntitiesPerLevel->hideInEditor = true;

    entPerLevNum = addIntParameter("Entities/level", "Base number of entities per level", 3, 1);


    entPerLevA = addFloatParameter("Coef a #entities", "Maximal creation rate of primary entities", .1, 0.);
    entPerLevA->hideInEditor = true;

    entPerLevB = addFloatParameter("Exponent level^b #entities", "Exponent of polynomial growth", 2., 0.);
    entPerLevB->hideInEditor = true;

    entPerLevUncert = addIntParameter("Plus minus u", "Uncertainty on entity per level: +range[-u,u]", 5, 0);
    entPerLevUncert->hideInEditor = true;

    // propEntities = addFloatParameter("Proportion entities", "Proportion of entities to include", .5, 0., 1.);
    // propEntities->hideInEditor = true;

    // old
    // maxReactionsPerEntity = addIntParameter("Max reactions per entity", "Maximal number of reactions forming an entity", 3, 1, 100);

    // growthReactionsPerLevel = addEnumParameter("Growth reactions/level", "How the number of reactions increase per level. #perEntity gives an average of reactions to form each entity, Percentage gives a chance for every reaction to be included");
    // growthReactionsPerLevel->addOption("Percentage", PERCENTAGE)->addOption("#perEntity", PERENTITY);

    propReactions = addFloatParameter("Proportion reactions", "Proportion of reactions to include", .5, 0., 1.);
    propReactions->hideInEditor = true;

    reactionsPerEntity = addIntParameter("Reactions per entity", "Number of reactions forming an entity",1);
    reactionsPerEntity->hideInEditor = true;

    initConcent = addPoint2DParameter("Init. prim. concentration / var", "Initial concentrations for primary entities, and variance");
    initConcent->x = 1.f;
    initConcent->y = 0.f;
    creationRate = addPoint2DParameter(" creation rate / var", "Creation rate of primary entities, and variance");
    creationRate->x=1.f;
    creationRate->y=0.f;
    destructionRate = addPoint2DParameter(" destruction rate / var", "destruction rate of entities");
    destructionRate->x=.5f;
    destructionRate->y=0.f;
    energyPerLevel = addPoint2DParameter("Energy per level / var", "Base energy of composite entities is coef*level");
    energyPerLevel->x = 0.f;
    energyPerLevel->y = .5f;
    energyBarrier = addPoint2DParameter(" energy barrier / var", " energy barrier of reactions");
    energyBarrier->x = 0.f;
    energyBarrier->y = 0.f;

    showAll = addBoolParameter("Show all", "Show all entities", true);
    avgNumShow = addIntParameter("Max num of curves", "Expected number of entities to follow", 10, 1, 100);
    avgNumShow->hideInEditor = true;

    statistics = addBoolParameter("Statistics", "Compute statistics", false);
    numRuns = addIntParameter("Num runs", "Number of runs to compute statistics", 10, 1);
    numRuns->hideInEditor = true;
    epsFactor = addIntParameter("Eps factor", "Number of possible steps around equilibrium to identify steady states", 200, 1);
    epsFactor->hideInEditor = true;
    // minG = addFloatParameter("Min G", "Current step in the simulation", 0, 0, maxSteps->intValue());
}

Generation::~Generation()
{
}

void Generation::onContainerParameterChanged(Parameter *p)
{
    if (p == growthEntitiesPerLevel)
    {
        switch (growthEntitiesPerLevel->getValueDataAsEnum<GrowthMode>())
        {
        case CONSTANT:
            entPerLevNum->hideInEditor = false;
            entPerLevA->hideInEditor = true;
            entPerLevB->hideInEditor = true;
            entPerLevUncert->hideInEditor = true;
            propReactions->hideInEditor = true;
            reactionsPerEntity->hideInEditor = false;

            break;
        case POLYNOMIAL:
            entPerLevNum->hideInEditor = true;
            entPerLevA->hideInEditor = false;
            entPerLevB->hideInEditor = false;
            entPerLevUncert->hideInEditor = false;
            propReactions->hideInEditor = true;
            reactionsPerEntity->hideInEditor = false;
            break;

        case PROPREACTIONS:
            entPerLevNum->hideInEditor = true;
            entPerLevA->hideInEditor = true;
            entPerLevB->hideInEditor = true;
            entPerLevUncert->hideInEditor = true;
            propReactions->hideInEditor = false;
            reactionsPerEntity->hideInEditor = true;
            break;
        default:
            break;
        }
        listeners.call(&GenerationListener::updateGenUI, this);
    }
    else if (p == showAll)
    {
        avgNumShow->hideInEditor = showAll->boolValue();
        listeners.call(&GenerationListener::updateGenUI, this);
    }
    else if (p == statistics)
    {
        numRuns->hideInEditor = !statistics->boolValue();
        epsFactor->hideInEditor = !statistics->boolValue();
        listeners.call(&GenerationListener::updateGenUI, this);
    }
}