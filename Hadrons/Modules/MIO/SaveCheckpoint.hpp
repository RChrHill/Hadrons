#ifndef Hadrons_MIO_SaveCheckpoint_hpp_
#define Hadrons_MIO_SaveCheckpoint_hpp_

#include <Hadrons/Global.hpp>
#include <Hadrons/Module.hpp>
#include <Hadrons/ModuleFactory.hpp>

BEGIN_HADRONS_NAMESPACE

/******************************************************************************
 *                         SaveCheckpoint                                 *
 ******************************************************************************/
BEGIN_MODULE_NAMESPACE(MIO)

class SaveCheckpointPar: Serializable
{
public:
    GRID_SERIALIZABLE_CLASS_MEMBERS(SaveCheckpointPar,
                                    unsigned int, i);
};

template <typename FImpl>
class TSaveCheckpoint: public Module<SaveCheckpointPar>
{
public:
    // constructor
    TSaveCheckpoint(const std::string name);
    // destructor
    virtual ~TSaveCheckpoint(void) {};
    // dependency relation
    virtual std::vector<std::string> getInput(void);
    virtual std::vector<std::string> getOutput(void);
    // setup
    virtual void setup(void);
    // execution
    virtual void execute(void);
};

MODULE_REGISTER_TMP(SaveCheckpoint, TSaveCheckpoint<FIMPL>, MIO);

/******************************************************************************
 *                 TSaveCheckpoint implementation                             *
 ******************************************************************************/
// constructor /////////////////////////////////////////////////////////////////
template <typename FImpl>
TSaveCheckpoint<FImpl>::TSaveCheckpoint(const std::string name)
: Module<SaveCheckpointPar>(name)
{}

// dependencies/products ///////////////////////////////////////////////////////
template <typename FImpl>
std::vector<std::string> TSaveCheckpoint<FImpl>::getInput(void)
{
    std::vector<std::string> in;
    
    return in;
}

template <typename FImpl>
std::vector<std::string> TSaveCheckpoint<FImpl>::getOutput(void)
{
    std::vector<std::string> out = {getName()};
    
    return out;
}

// setup ///////////////////////////////////////////////////////////////////////
template <typename FImpl>
void TSaveCheckpoint<FImpl>::setup(void)
{
    
}

// execution ///////////////////////////////////////////////////////////////////
template <typename FImpl>
void TSaveCheckpoint<FImpl>::execute(void)
{
    
}

END_MODULE_NAMESPACE

END_HADRONS_NAMESPACE

#endif // Hadrons_MIO_SaveCheckpoint_hpp_
