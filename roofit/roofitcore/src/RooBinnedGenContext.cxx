/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 * @(#)root/roofitcore:$Id$
 * Authors:                                                                  *
 *   WV, Wouter Verkerke, UC Santa Barbara, verkerke@slac.stanford.edu       *
 *   DK, David Kirkby,    UC Irvine,         dkirkby@uci.edu                 *
 *                                                                           *
 * Copyright (c) 2000-2005, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/

/**
\file RooBinnedGenContext.cxx
\class RooBinnedGenContext
\ingroup Roofitcore

Efficient implementation of the generator context specific for binned pdfs.
**/

#include "Riostream.h"


#include "RooMsgService.h"
#include "RooBinnedGenContext.h"
#include "RooAbsPdf.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooDataSet.h"
#include "RooRandom.h"

using std::endl, std::vector, std::ostream;



////////////////////////////////////////////////////////////////////////////////
/// Constructor

RooBinnedGenContext::RooBinnedGenContext(const RooAbsPdf &model, const RooArgSet &vars,
               const RooDataSet *prototype, const RooArgSet* auxProto,
               bool verbose) :
  RooAbsGenContext(model,vars,prototype,auxProto,verbose)
{
  cxcoutI(Generation) << "RooBinnedGenContext::ctor() setting up event special generator context for sum p.d.f. " << model.GetName()
         << " for generation of observable(s) " << vars ;
  if (prototype) ccxcoutI(Generation) << " with prototype data for " << *prototype->get() ;
  if (auxProto && !auxProto->empty())  ccxcoutI(Generation) << " with auxiliary prototypes " << *auxProto ;
  ccxcoutI(Generation) << std::endl ;

  // Constructor. Build an array of generator contexts for each product component PDF
  RooArgSet(model).snapshot(_pdfSet, true);
  _pdf = static_cast<RooAbsPdf*>(_pdfSet.find(model.GetName())) ;
  _pdf->setOperMode(RooAbsArg::ADirty,true) ;

  // Fix normalization set of this RooAddPdf
  if (prototype)
    {
      RooArgSet coefNSet(vars) ;
      coefNSet.add(*prototype->get()) ;
      _pdf->fixAddCoefNormalization(coefNSet) ;
    }

  _pdf->recursiveRedirectServers(_theEvent) ;
  _vars = std::unique_ptr<RooArgSet>{_pdf->getObservables(vars)};

  // Create empty RooDataHist
  _hist = std::make_unique<RooDataHist>("genData","genData",*_vars);

  _expectedData = false ;
}


RooBinnedGenContext::~RooBinnedGenContext() = default;


////////////////////////////////////////////////////////////////////////////////
/// Attach given set of variables to internal p.d.f. clone

void RooBinnedGenContext::attach(const RooArgSet& args)
{
  _pdf->recursiveRedirectServers(args) ;
}



////////////////////////////////////////////////////////////////////////////////
/// One-time initialization of generator context. Attach theEvent
/// to internal p.d.f clone and forward initialization call to
/// the component generators.

void RooBinnedGenContext::initGenerator(const RooArgSet &theEvent)
{
  _pdf->recursiveRedirectServers(theEvent) ;

}


////////////////////////////////////////////////////////////////////////////////

void RooBinnedGenContext::setExpectedData(bool flag)
{
  _expectedData = flag ;
}


////////////////////////////////////////////////////////////////////////////////

RooDataSet *RooBinnedGenContext::generate(double nEvt, bool /*skipInit*/, bool extended)
{
  // Scale to number of events and introduce Poisson fluctuations
  _hist->reset() ;

  double nEvents = nEvt ;

  if (nEvents<=0) {
    if (!_pdf->canBeExtended()) {
      coutE(InputArguments) << "RooAbsPdf::generateBinned(" << GetName()
             << ") ERROR: No event count provided and p.d.f does not provide expected number of events" << std::endl ;
      return nullptr ;
    } else {
      // Don't round in expectedData mode
      if (_expectedData || extended) {
   nEvents = _pdf->expectedEvents(_vars.get());
      } else {
   nEvents = Int_t(_pdf->expectedEvents(_vars.get())+0.5) ;
      }
    }
  }

  // Sample p.d.f. distribution
  _pdf->fillDataHist(_hist.get(),_vars.get(),1,true) ;

  // Output container
  RooDataSet* wudata = new RooDataSet("wu","wu",*_vars,RooFit::WeightVar()) ;

  vector<int> histOut(_hist->numEntries()) ;
  double histMax(-1) ;
  Int_t histOutSum(0) ;
  for (int i=0 ; i<_hist->numEntries() ; i++) {
    _hist->get(i) ;
    if (_expectedData) {

      // Expected data, multiply p.d.f by nEvents
      double w=_hist->weight()*nEvents ;
      wudata->add(*_hist->get(),w) ;

    } else if (extended) {

      // Extended mode, set contents to Poisson(pdf*nEvents)
      double w = RooRandom::randomGenerator()->Poisson(_hist->weight()*nEvents) ;
      wudata->add(*_hist->get(),w) ;

    } else {

      // Regular mode, fill array of weights with Poisson(pdf*nEvents), but to not fill
      // histogram yet.
      if (_hist->weight()>histMax) {
   histMax = _hist->weight() ;
      }
      histOut[i] = RooRandom::randomGenerator()->Poisson(_hist->weight()*nEvents) ;
      histOutSum += histOut[i] ;
    }
  }


  if (!_expectedData && !extended) {

    // Second pass for regular mode - Trim/Extend dataset to exact number of entries

    // Calculate difference between what is generated so far and what is requested
    Int_t nEvtExtra = std::abs(Int_t(nEvents)-histOutSum) ;
    Int_t wgt = (histOutSum>nEvents) ? -1 : 1 ;

    // Perform simple binned accept/reject procedure to get to exact event count
    while(nEvtExtra>0) {

      Int_t ibinRand = RooRandom::randomGenerator()->Integer(_hist->numEntries()) ;
      _hist->get(ibinRand) ;
      double ranY = RooRandom::randomGenerator()->Uniform(histMax) ;

      if (ranY<_hist->weight()) {
   if (wgt==1) {
     histOut[ibinRand]++ ;
   } else {
     // If weight is negative, prior bin content must be at least 1
     if (histOut[ibinRand]>0) {
       histOut[ibinRand]-- ;
     } else {
       continue ;
     }
   }
   nEvtExtra-- ;
      }
    }

    // Transfer working array to histogram
    for (int i=0 ; i<_hist->numEntries() ; i++) {
      _hist->get(i) ;
      wudata->add(*_hist->get(),histOut[i]) ;
    }

  }

  return wudata ;

}



////////////////////////////////////////////////////////////////////////////////
/// this method is not implemented for this context

void RooBinnedGenContext::generateEvent(RooArgSet&, Int_t)
{
  assert(0) ;
}



////////////////////////////////////////////////////////////////////////////////
/// Print the details of the context

void RooBinnedGenContext::printMultiline(ostream &os, Int_t content, bool verbose, TString indent) const
{
  RooAbsGenContext::printMultiline(os,content,verbose,indent) ;
  os << indent << "--- RooBinnedGenContext ---" << std::endl ;
  os << indent << "Using PDF ";
  _pdf->printStream(os,kName|kArgs|kClassName,kSingleLine,indent);
}
