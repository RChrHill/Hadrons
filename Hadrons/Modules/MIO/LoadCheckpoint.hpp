#ifndef Hadrons_MIO_LoadCheckpoint_hpp_
#define Hadrons_MIO_LoadCheckpoint_hpp_

#include <Hadrons/Global.hpp>
#include <Hadrons/Module.hpp>
#include <Hadrons/ModuleFactory.hpp>

BEGIN_HADRONS_NAMESPACE

/******************************************************************************
 *                         LoadCheckpoint                                 *
 ******************************************************************************/
BEGIN_MODULE_NAMESPACE(MIO)

class LoadCheckpointPar: Serializable
{
public:
    GRID_SERIALIZABLE_CLASS_MEMBERS(LoadCheckpointPar,
                                    unsigned int, i);
};

template <typename FImpl>
class TLoadCheckpoint: public Module<LoadCheckpointPar>
{
public:
    // constructor
    TLoadCheckpoint(const std::string name);
    // destructor
    virtual ~TLoadCheckpoint(void) {};
    // dependency relation
    virtual std::vector<std::string> getInput(void);
    virtual std::vector<std::string> getOutput(void);
    // setup
    virtual void setup(void);
    // execution
    virtual void execute(void);
};

MODULE_REGISTER_TMP(LoadCheckpoint, TLoadCheckpoint<FIMPL>, MIO);

/******************************************************************************
 *                 TLoadCheckpoint implementation                             *
 ******************************************************************************/
// constructor /////////////////////////////////////////////////////////////////
template <typename FImpl>
TLoadCheckpoint<FImpl>::TLoadCheckpoint(const std::string name)
: Module<LoadCheckpointPar>(name)
{}

// dependencies/products ///////////////////////////////////////////////////////
template <typename FImpl>
std::vector<std::string> TLoadCheckpoint<FImpl>::getInput(void)
{
    std::vector<std::string> in;
    
    return in;
}

template <typename FImpl>
std::vector<std::string> TLoadCheckpoint<FImpl>::getOutput(void)
{
    std::vector<std::string> out = {getName()};
    
    return out;
}

// setup ///////////////////////////////////////////////////////////////////////
template <typename FImpl>
void TLoadCheckpoint<FImpl>::setup(void)
{
    
}

// execution ///////////////////////////////////////////////////////////////////
template <typename FImpl>
void TLoadCheckpoint<FImpl>::execute(void)
{
    
}

END_MODULE_NAMESPACE

END_HADRONS_NAMESPACE

#endif // Hadrons_MIO_LoadCheckpoint_hpp_
