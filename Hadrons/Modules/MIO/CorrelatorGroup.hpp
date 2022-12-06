/*
 * CorrelatorGroup.hpp, part of Hadrons (https://github.com/aportelli/Hadrons)
 *
 * Copyright (C) 2015 - 2022
 *
 * Author: Antonin Portelli <antonin.portelli@me.com>
 * Author: Ryan Hill <rchrys.hill@gmail.com>
 *
 * Hadrons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Hadrons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Hadrons.  If not, see <http://www.gnu.org/licenses/>.
 *
 * See the full license in the file "LICENSE" in the top level distribution 
 * directory.
 */

/*  END LEGAL */

#ifndef Hadrons_MIO_CorrelatorGroup_hpp_
#define Hadrons_MIO_CorrelatorGroup_hpp_

#include <Hadrons/Global.hpp>
#include <Hadrons/Module.hpp>
#include <Hadrons/ModuleFactory.hpp>
#include <Hadrons/Serialization.hpp>

BEGIN_HADRONS_NAMESPACE

/******************************************************************************
 *                 Module to load a single field from disk                    *
 ******************************************************************************/
BEGIN_MODULE_NAMESPACE(MIO)

class CorrelatorGroupPar: Serializable
{
public:
    GRID_SERIALIZABLE_CLASS_MEMBERS(CorrelatorGroupPar,
                                    std::vector<std::string>, contractions);
};

template<typename Impl>
class TCorrelatorGroup: public Module<CorrelatorGroupPar>
{
public:
    // constructor
    TCorrelatorGroup(const std::string name);
    // destructor
    virtual ~TCorrelatorGroup(void) {};
    // dependency relation
    virtual std::vector<std::string> getInput(void);
    virtual std::vector<std::string> getOutput(void);
    virtual std::vector<std::string> getOutputFiles(void);
    // setup
    virtual void setup(void);
    // execution
    virtual void execute(void);
};

MODULE_REGISTER_TMP(CorrelatorGroup, TCorrelatorGroup<FIMPL>, MIO);

/******************************************************************************
 *                 TCorrelatorGroup implementation                            *
 ******************************************************************************/
// constructor /////////////////////////////////////////////////////////////////
template <typename Impl>
TCorrelatorGroup<Impl>::TCorrelatorGroup(const std::string name)
: Module<CorrelatorGroupPar>(name)
{}

// dependencies/products ///////////////////////////////////////////////////////
template<typename Impl>
std::vector<std::string> TCorrelatorGroup<Impl>::getInput(void)
{
    // Convert a set of module names into the strings used to store their
    // HadronsSerializable objects
    std::vector<std::string> inputs;
    for (const auto& name : par().contractions)
        inputs.push_back(envModuleResultName(name));
    return inputs;
}

template<typename Impl>
std::vector<std::string> TCorrelatorGroup<Impl>::getOutput(void)
{
    // Output the module result under a standardised identifier
    std::vector<std::string> output = {envResultName()};
    
    return output;
}

template<typename Impl>
std::vector<std::string> TCorrelatorGroup<Impl>::getOutputFiles(void)
{
    std::vector<std::string> out = {};
    
    return out;
}

// setup ///////////////////////////////////////////////////////////////////////
template<typename Impl>
void TCorrelatorGroup<Impl>::setup(void)
{
    envCreateResult();
}

// execution ///////////////////////////////////////////////////////////////////
template<typename Impl>
void TCorrelatorGroup<Impl>::execute(void)
{
    auto &contractionList = par().contractions;
    auto &result          = envInitResultGroup();

    for (const auto &contractionModuleName : contractionList)
    {
        const auto& moduleResults = envGetModuleResult(contractionModuleName);
        result.append(contractionModuleName, moduleResults);
    }
}

END_MODULE_NAMESPACE

END_HADRONS_NAMESPACE

#endif // Hadrons_MIO_CorrelatorGroup_hpp_
