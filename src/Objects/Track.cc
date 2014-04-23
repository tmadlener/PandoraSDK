/**
 *  @file   PandoraPFANew/Framework/src/Objects/Track.cc
 * 
 *  @brief  Implementation of the track class.
 * 
 *  $Log: $
 */

#include "Helpers/GeometryHelper.h"

#include "Objects/Helix.h"
#include "Objects/Track.h"

#include <cmath>
#include <cstdlib>

namespace pandora
{

const MCParticle *Track::GetMainMCParticle() const
{
    float bestWeight(0.f);
    const MCParticle *pBestMCParticle = NULL;

    for (MCParticleWeightMap::const_iterator iter = m_mcParticleWeightMap.begin(), iterEnd = m_mcParticleWeightMap.end(); iter != iterEnd; ++iter)
    {
        if (iter->second > bestWeight)
        {
            bestWeight = iter->second;
            pBestMCParticle = iter->first;
        }
    }

    if (NULL == pBestMCParticle)
        throw StatusCodeException(STATUS_CODE_NOT_INITIALIZED);

    return pBestMCParticle;
}

//------------------------------------------------------------------------------------------------------------------------------------------

Track::Track(const PandoraApi::Track::Parameters &parameters) :
    m_d0(parameters.m_d0.Get()),
    m_z0(parameters.m_z0.Get()),
    m_particleId(parameters.m_particleId.Get()),
    m_charge(parameters.m_charge.Get()),
    m_mass(parameters.m_mass.Get()),
    m_momentumAtDca(parameters.m_momentumAtDca.Get()),
    m_momentumMagnitudeAtDca(m_momentumAtDca.GetMagnitude()),
    m_energyAtDca(std::sqrt(m_mass * m_mass + m_momentumMagnitudeAtDca * m_momentumMagnitudeAtDca)),
    m_trackStateAtStart(parameters.m_trackStateAtStart.Get()),
    m_trackStateAtEnd(parameters.m_trackStateAtEnd.Get()),
    m_trackStateAtCalorimeter(parameters.m_trackStateAtCalorimeter.Get()),
    m_timeAtCalorimeter(parameters.m_timeAtCalorimeter.Get()),
    m_reachesCalorimeter(parameters.m_reachesCalorimeter.Get()),
    m_isProjectedToEndCap(parameters.m_isProjectedToEndCap.Get()),
    m_canFormPfo(parameters.m_canFormPfo.Get()),
    m_canFormClusterlessPfo(parameters.m_canFormClusterlessPfo.Get()),
    m_pAssociatedCluster(NULL),
    m_pParentAddress(parameters.m_pParentAddress.Get()),
    m_isAvailable(true)
{
    // Consistency checks
    if (0.f == m_energyAtDca)
        throw StatusCodeException(STATUS_CODE_INVALID_PARAMETER);

    if (0 == m_charge)
        throw StatusCodeException(STATUS_CODE_INVALID_PARAMETER);

    // Obtain helix fit to track state at calorimeter
    static const float bField(GeometryHelper::GetBField(CartesianVector(0.f, 0.f, 0.f)));
    m_pHelixFitAtCalorimeter = new Helix(m_trackStateAtCalorimeter.GetPosition(), m_trackStateAtCalorimeter.GetMomentum(), static_cast<float>(m_charge), bField);
}

//------------------------------------------------------------------------------------------------------------------------------------------

Track::~Track()
{
    delete m_pHelixFitAtCalorimeter;

    m_parentTrackList.clear();
    m_siblingTrackList.clear();
    m_daughterTrackList.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------

void Track::SetMCParticleWeightMap(const MCParticleWeightMap &mcParticleWeightMap)
{
    m_mcParticleWeightMap = mcParticleWeightMap;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void Track::RemoveMCParticles()
{
    m_mcParticleWeightMap.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode Track::SetAssociatedCluster(Cluster *const pCluster)
{
    if (NULL == pCluster)
        return STATUS_CODE_INVALID_PARAMETER;

    if (NULL != m_pAssociatedCluster)
        return STATUS_CODE_ALREADY_INITIALIZED;

    m_pAssociatedCluster = pCluster;

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode Track::RemoveAssociatedCluster(Cluster *const pCluster)
{
    if (pCluster != m_pAssociatedCluster)
        return STATUS_CODE_NOT_FOUND;

    m_pAssociatedCluster = NULL;

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode Track::AddParent(Track *const pTrack)
{
    if (NULL == pTrack)
        return STATUS_CODE_INVALID_PARAMETER;

    if (!m_parentTrackList.insert(pTrack).second)
        return STATUS_CODE_ALREADY_PRESENT;

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode Track::AddDaughter(Track *const pTrack)
{
    if (NULL == pTrack)
        return STATUS_CODE_INVALID_PARAMETER;

    if (!m_daughterTrackList.insert(pTrack).second)
        return STATUS_CODE_ALREADY_PRESENT;

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode Track::AddSibling(Track *const pTrack)
{
    if (NULL == pTrack)
        return STATUS_CODE_INVALID_PARAMETER;

    if (!m_siblingTrackList.insert(pTrack).second)
        return STATUS_CODE_ALREADY_PRESENT;

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

std::ostream &operator<<(std::ostream &stream, const Track &track)
{
    stream  << " Track: " << std::endl
            << " d0     " << track.GetD0() << std::endl
            << " z0     " << track.GetZ0() << std::endl
            << " p0     " << track.GetMomentumAtDca() << std::endl;

    return stream;
}

} // namespace pandora
