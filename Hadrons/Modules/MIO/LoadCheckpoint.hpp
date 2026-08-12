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
#ifndef Hadrons_MIO_LoadCheckpoint_hpp_
#define Hadrons_MIO_LoadCheckpoint_hpp_

#include <Hadrons/Global.hpp>
#include <Hadrons/Module.hpp>
#include <Hadrons/ModuleFactory.hpp>
#include <Hadrons/FieldIo.hpp> 
#include <Hadrons/EmField.hpp>

BEGIN_HADRONS_NAMESPACE

/******************************************************************************
 *                         LoadCheckpoint                                 *
 ******************************************************************************/
BEGIN_MODULE_NAMESPACE(MIO)

class LoadCheckpointPar: Serializable
{
public:
    GRID_SERIALIZABLE_CLASS_MEMBERS(LoadCheckpointPar,
                                    std::string, name,
                                    unsigned int, Ls,
                                    std::string, fileStem);
};

template <typename Field>
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

MODULE_REGISTER_TMP(LoadPropagatorCheckpoint, TLoadCheckpoint<FIMPL::PropagatorField>, MIO);

/******************************************************************************
 *                 TLoadCheckpoint implementation                             *
 ******************************************************************************/
// constructor /////////////////////////////////////////////////////////////////
template <typename Field>
TLoadCheckpoint<Field>::TLoadCheckpoint(const std::string name)
: Module<LoadCheckpointPar>(name)
{}

// dependencies/products ///////////////////////////////////////////////////////
template <typename Field>
std::vector<std::string> TLoadCheckpoint<Field>::getInput(void)
{
    return {};
}

template <typename Field>
std::vector<std::string> TLoadCheckpoint<Field>::getOutput(void)
{
    return {par().name};
}

// setup ///////////////////////////////////////////////////////////////////////
template <typename Field>
void TLoadCheckpoint<Field>::setup(void)
{
    if (par().Ls > 1)
    {
        envCreateLat(Field, par().name, par().Ls);
        LOG(Message) << "Crated 5d fields " << par().name << std::endl;
    }
    else
    {
        envCreateLat(Field, par().name);
        LOG(Message) << "Crated 4d fields " << par().name << std::endl;
    }
}

// execution ///////////////////////////////////////////////////////////////////
template <typename Field>
void TLoadCheckpoint<Field>::execute(void)
{
    auto grid = env().getGrid();
    const std::string rankStr = std::to_string(grid->ThisRank());
    const std::string filename = resultFilename(par().fileStem + "." + rankStr, "bin");
    size_t size, sizec;
    uint32_t crcRead, crcData;
    GridStopWatch ioWatch, crcWatch;

    // Open checkpoint
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        HADRONS_ERROR(Definition, "Could not open file: " + filename);
    }

    // Local lattice data size
    size = grid->lSites()*sizeof(typename Field::scalar_object);
    sizec = size/sizeof(char);

    // Allocate field in Hadrons Environment
    auto &vec = envGet(Field, par().name);

    // Read lattice data
    crcWatch.Start();
    file.read(reinterpret_cast<char *>(&crcRead), sizeof(uint32_t)/sizeof(char));
    if (!file)
    {
        HADRONS_ERROR(Definition, "Failed to read CRC from checkpoint: " + filename);
    }
    crcWatch.Stop();

    {
        autoView(vec_v, vec, CpuWrite);
        ioWatch.Start();
        file.read(reinterpret_cast<char *>(vec_v.cpu_ptr), sizec);
        if (!file)
        {
            HADRONS_ERROR(Definition, "Failed to read lattice data from checkpoint: " + filename);
        }
        ioWatch.Stop();
    }
    file.close();

    // Calculate CRC
    {
        autoView(vec_v, vec, CpuRead);
        crcWatch.Start();
        crcData = GridChecksum::crc32(vec_v.cpu_ptr, size);
        crcWatch.Stop();
    }
    
    // Validate CRC
    LOG(Message) << "LoadCheckpoint: stored CRC32 " << std::hex << crcRead << std::dec << std::endl;
    LOG(Message) << "LoadCheckpoint: calculated CRC32 " << std::hex << crcData << std::dec << std::endl;
    if (crcRead != crcData)
    {
        HADRONS_ERROR(Definition, "Checkpoint CRC32 mismatch for " + filename);
    }

    // Performance
    size *= grid->ProcessorCount();
    auto &p = BinaryIO::lastPerf;
    p.size = size;
    p.time = ioWatch.useconds();
    p.mbytesPerSecond = size/1024.0/1024.0/(ioWatch.useconds()/1.e6);
    LOG(Message) << "LoadCheckpoint: Read " << p.size << " bytes in " << ioWatch.Elapsed()
                 << ", " << p.mbytesPerSecond << " MB/s" << std::endl;
    LOG(Message) << "LoadCheckpoint: checksum overhead " << crcWatch.Elapsed() << std::endl;
}
END_MODULE_NAMESPACE

END_HADRONS_NAMESPACE

#endif // Hadrons_MIO_LoadCheckpoint_hpp_
