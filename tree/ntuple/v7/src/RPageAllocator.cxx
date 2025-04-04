/// \file RPageAllocator.cxx
/// \ingroup NTuple ROOT7
/// \author Jakob Blomer <jblomer@cern.ch>
/// \date 2019-06-25
/// \warning This is part of the ROOT 7 prototype! It will change without notice. It might trigger earthquakes. Feedback
/// is welcome!

/*************************************************************************
 * Copyright (C) 1995-2019, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/


#include <ROOT/RPageAllocator.hxx>

#include <TError.h>

#include <algorithm>

ROOT::Internal::RPage ROOT::Internal::RPageAllocatorHeap::NewPage(std::size_t elementSize, std::size_t nElements)
{
   R__ASSERT((elementSize > 0) && (nElements > 0));
   auto nbytes = elementSize * nElements;
   auto buffer = new unsigned char[nbytes];
   return RPage(buffer, this, elementSize, nElements);
}

void ROOT::Internal::RPageAllocatorHeap::DeletePage(RPage &page)
{
   delete[] reinterpret_cast<unsigned char *>(page.GetBuffer());
}
