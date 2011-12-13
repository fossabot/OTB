/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __otbImageRegionSquareTileSplitter_txx
#define __otbImageRegionSquareTileSplitter_txx

#include "otbImageRegionSquareTileSplitter.h"
#include "otbMath.h"
#include "otbMacro.h"

namespace otb
{

template <unsigned int VImageDimension>
unsigned int
ImageRegionSquareTileSplitter<VImageDimension>
::GetNumberOfSplits(const RegionType& region, unsigned int requestedNumber)
{
  unsigned int theoricalNbPixelPerTile = region.GetNumberOfPixels() / requestedNumber;
  unsigned int theoricalTileDimension = static_cast<unsigned int> (vcl_sqrt(static_cast<double>(theoricalNbPixelPerTile)) );

  m_TileDimension = m_TileHint;

  // If we have less than one jpeg 2000 tile, split in sub-tiles
  if(theoricalTileDimension < m_TileHint)
    {
    while(m_TileDimension > theoricalTileDimension)
      {
      m_TileDimension/=2;
      }
    }
    else
      {
      while(2*m_TileDimension < theoricalTileDimension)
        {
        m_TileDimension*=2;
        }
      }

  // Minimal tile size is m_TileSizeAlignment * m_TileSizeAlignment
  if (m_TileDimension < m_TileSizeAlignment)
    {
    otbMsgDevMacro(<< "Warning: clamping tile size to " << m_TileSizeAlignment << " * " << m_TileSizeAlignment);
    m_TileDimension = m_TileSizeAlignment;
    }

  unsigned int numPieces = 1;
  const SizeType&  regionSize = region.GetSize();
  for (unsigned int j = 0; j < VImageDimension; ++j)
    {
    m_SplitsPerDimension[j] = (regionSize[j] + m_TileDimension - 1) / m_TileDimension;
    numPieces *= m_SplitsPerDimension[j];
    }

  otbMsgDevMacro(<< "Tile dimension : " << m_TileDimension)
  otbMsgDevMacro(<< "Number of splits per dimension : " << m_SplitsPerDimension[0] << " " <<  m_SplitsPerDimension[1])

  return numPieces;
}

template <unsigned int VImageDimension>
itk::ImageRegion<VImageDimension>
ImageRegionSquareTileSplitter<VImageDimension>
::GetSplit(unsigned int i, unsigned int itkNotUsed(numberOfPieces), const RegionType& region)
{
  RegionType splitRegion;
  IndexType  splitIndex;

  // Compute the actual number of splits
  unsigned int numPieces = 1;
  for (unsigned int j = 0; j < VImageDimension; ++j)
    {
    numPieces *= m_SplitsPerDimension[j];
    }

  // Sanity check
  if (i >= numPieces)
    {
    itkExceptionMacro("Asked for split number " << i << " but region contains only " << numPieces << " splits");
    }

  // First, find the megatile index

  unsigned int tilesPerMegaTilesPerDim = m_TileHint / m_TileDimension;

  if(tilesPerMegaTilesPerDim == 0)
    {
    tilesPerMegaTilesPerDim = 1;
    }

  // Can't use more tiles than the number of splits per dim
  if (tilesPerMegaTilesPerDim > m_SplitsPerDimension[0])
    tilesPerMegaTilesPerDim = m_SplitsPerDimension[0];

  if (tilesPerMegaTilesPerDim > m_SplitsPerDimension[1])
    tilesPerMegaTilesPerDim = m_SplitsPerDimension[1];


  unsigned int tilesPerMegaTiles = tilesPerMegaTilesPerDim*tilesPerMegaTilesPerDim;

  unsigned int remaining = i / tilesPerMegaTiles;
  for (unsigned int j = VImageDimension - 1; j > 0; --j)
    {
    splitIndex[j] = tilesPerMegaTilesPerDim * remaining / (m_SplitsPerDimension[VImageDimension - 1 - j] / tilesPerMegaTilesPerDim);
    remaining = remaining % (m_SplitsPerDimension[VImageDimension - 1 - j] / tilesPerMegaTilesPerDim);
    }
  splitIndex[0] = tilesPerMegaTilesPerDim * remaining;

  // Now splitIndex contains the megaTile index
  remaining = i % tilesPerMegaTiles;
  for (unsigned int j = VImageDimension - 1; j > 0; --j)
    {
    splitIndex[j] += remaining / tilesPerMegaTilesPerDim;
    remaining = remaining % (tilesPerMegaTilesPerDim);
    }
  splitIndex[0]+= remaining;
  
  // Now split index contains the tile index
    
  // Transform the split index to the actual coordinates
  for (unsigned int j = 0; j < VImageDimension; ++j)
    {
    splitRegion.SetIndex(j, region.GetIndex(j) + m_TileDimension * splitIndex[j]);
    splitRegion.SetSize(j, m_TileDimension);
    }
    
    // Handle the borders
    splitRegion.Crop(region);
    
    return splitRegion;
}

/**
 *
 */
template <unsigned int VImageDimension>
void
ImageRegionSquareTileSplitter<VImageDimension>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "SplitsPerDimension : " << m_SplitsPerDimension << std::endl;
  os << indent << "TileDimension      : " << m_TileDimension << std::endl;
  os << indent << "TileSizeAlignment  : " << m_TileSizeAlignment << std::endl;

}

} // end namespace itk

#endif
