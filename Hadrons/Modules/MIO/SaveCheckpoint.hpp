/*
 * SaveField.cpp, part of Hadrons (https://github.com/aportelli/Hadrons)
 *
 * Copyright (C) 2015 - 2023
 *
 * Author: Antonin Portelli <antonin.portelli@me.com>
 * Author: Michael Marshall <43034299+mmphys@users.noreply.github.com>
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
#ifndef Hadrons_MIO_SaveCheckpoint_hpp_
#define Hadrons_MIO_SaveCheckpoint_hpp_

#include <Hadrons/Global.hpp>
#include <Hadrons/Module.hpp>
#include <Hadrons/ModuleFactory.hpp>
#include <Hadrons/FieldIo.hpp>
#include <Hadrons/EmField.hpp>

BEGIN_HADRONS_NAMESPACE

/******************************************************************************
 *                         SaveCheckpoint                                 *
 ******************************************************************************/
BEGIN_MODULE_NAMESPACE(MIO)

class SaveCheckpointPar: Serializable
{
public:
    GRID_SERIALIZABLE_CLASS_MEMBERS(SaveCheckpointPar,
                                    std::string, name,
                                    std::string, fileStem);
};

template <typename Field>
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

MODULE_REGISTER_TMP(SavePropagatorCheckpoint, TSaveCheckpoint<FIMPL::PropagatorField>, MIO);

/******************************************************************************
 *                 TSaveCheckpoint implementation                             *
 ******************************************************************************/
// constructor /////////////////////////////////////////////////////////////////
template <typename Field>
TSaveCheckpoint<Field>::TSaveCheckpoint(const std::string name)
: Module<SaveCheckpointPar>(name)
{}

// dependencies/products ///////////////////////////////////////////////////////
template <typename Field>
std::vector<std::string> TSaveCheckpoint<Field>::getInput(void)
{
    return {par().name};
}

template <typename Field>
std::vector<std::string> TSaveCheckpoint<Field>::getOutput(void)
{
    return {};
}

// setup ///////////////////////////////////////////////////////////////////////
template <typename Field>
void TSaveCheckpoint<Field>::setup(void)
{
    // if require memory allocations then fill this section
}

// execution ///////////////////////////////////////////////////////////////////
template <typename Field>
void TSaveCheckpoint<Field>::execute(void)
{
    auto grid = env().getGrid();
    const std::string rankStr = std::to_string(grid->ThisRank());
    const std::string filename = resultFilename(par().fileStem + "." + rankStr, "bin");
    size_t size, sizec;
    uint32_t crc;
    GridStopWatch ioWatch, crcWatch;

    // Open checkpoint
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file.is_open())
    {
        HADRONS_ERROR(Definition, "Could not open file: " + filename);
    }

    // Local lattice data size
    size = grid->lSites()*sizeof(typename Field::scalar_object);
    sizec = size/sizeof(char);
    
    // Allocate field in Hadrons Environment
    auto &vec = envGet(Field, par().name);

    // Write lattice data
    autoView(vec_v, vec, CpuRead);
    crcWatch.Start();
    crc = GridChecksum::crc32(vec_v.cpu_ptr, size);
    file.write(reinterpret_cast<char *>(&crc), sizeof(uint32_t)/sizeof(char));
    crcWatch.Stop();
    LOG(Message) << "SaveCheckpoint: Data CRC32 " << std::hex << crc << std::dec << std::endl;

    ioWatch.Start();
    file.write(reinterpret_cast<char *>(vec_v.cpu_ptr), sizec);
    file.flush();
    ioWatch.Stop();
    if (!file)
    {
        HADRONS_ERROR(Definition, "Failed to write lattice data in checkpoint: " + filename);
    }
    file.close();

    // Performance
    size *= grid->ProcessorCount();
    auto &p = BinaryIO::lastPerf;
    p.size = size;
    p.time = ioWatch.useconds();
    p.mbytesPerSecond = size/1024.0/1024.0/(ioWatch.useconds()/1.e6);
    LOG(Message) << "SaveCheckpoint: Wrote " << p.size << " bytes in " << ioWatch.Elapsed() 
                 << ", " << p.mbytesPerSecond << " MB/s" << std::endl;
    LOG(Message) << "SaveCheckpoint: checksum overhead " << crcWatch.Elapsed() << std::endl;
}
END_MODULE_NAMESPACE

END_HADRONS_NAMESPACE

#endif // Hadrons_MIO_SaveCheckpoint_hpp_
