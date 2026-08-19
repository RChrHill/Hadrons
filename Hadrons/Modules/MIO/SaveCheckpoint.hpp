/*
 * SaveField.cpp, part of Hadrons (https://github.com/aportelli/Hadrons)
 *
 * Copyright (C) 2015 - 2023
 *
 * Author: Antonin Portelli <antonin.portelli@me.com>
 * Author: Muhammad Asif <19404936+asifsamiarain@users.noreply.github.com>
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
    GridStopWatch ioWatch, crcWatch;

    // Open checkpoint
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file.is_open())
    {
        file.close();
        HADRONS_ERROR(Definition, "Could not open file: " + filename);
    }

    // Local lattice data size
    size = grid->lSites()*sizeof(typename Field::scalar_object);
    sizec = size/sizeof(char);
    
    // Allocate field in Hadrons Environment
    auto &vec = envGet(Field, par().name);

    // Write lattice data
    autoView(vec_v, vec, CpuRead);

    // Write header
    crcWatch.Start();
    uint32_t magic = 'CKPT';
    uint32_t datatype = 0; // datatype corresponding to Field
    uint32_t localCrc = GridChecksum::crc32(vec_v.cpu_ptr, size);
    uint32_t globalCrc = localCrc;
    #ifdef GRID_COMMS_MPI
        MPI_Allreduce(&localCrc, &globalCrc, 1, MPI_UINT32_T, MPI_BXOR, MPI_COMM_WORLD);
    #endif
    uint64_t dataOffset = 0x20;
    uint64_t dataSize = static_cast<uint64_t>(size);

    file.write(reinterpret_cast<char *>(&magic),      sizeof(uint32_t));
    file.write(reinterpret_cast<char *>(&datatype),   sizeof(uint32_t));
    file.write(reinterpret_cast<char *>(&localCrc),   sizeof(uint32_t));
    file.write(reinterpret_cast<char *>(&globalCrc),  sizeof(uint32_t));
    file.write(reinterpret_cast<char *>(&dataOffset), sizeof(uint64_t));
    file.write(reinterpret_cast<char *>(&dataSize),   sizeof(uint64_t));
    crcWatch.Stop();

    if (!file)
    {
        file.close();
        HADRONS_ERROR(Definition, "Failed to write checkpoint header: " + filename);
    }

    LOG(Message) << "SaveCheckpoint: magic "             << std::hex << magic      << std::dec << std::endl;
    LOG(Message) << "SaveCheckpoint: datatype "          << std::hex << datatype   << std::dec << std::endl;
    LOG(Message) << "SaveCheckpoint: Loacal Data CRC32 " << std::hex << localCrc   << std::dec << std::endl;
    LOG(Message) << "SaveCheckpoint: Global Data CRC32 " << std::hex << globalCrc  << std::dec << std::endl;
    LOG(Message) << "SaveCheckpoint: dataOffset "        << std::hex << dataOffset << std::dec << std::endl;
    LOG(Message) << "SaveCheckpoint: dataSize "          << std::hex << dataSize   << std::dec << std::endl;

    // Write payload
    ioWatch.Start();
    file.write(reinterpret_cast<char *>(vec_v.cpu_ptr), sizec);
    file.flush();
    ioWatch.Stop();

    if (!file)
    {
        file.close();
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
