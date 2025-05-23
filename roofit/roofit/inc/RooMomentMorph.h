/*
 * Project: RooFit
 *
 * Copyright (c) 2023, CERN
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted according to the terms
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)
 */

#ifndef ROOMOMENTMORPH
#define ROOMOMENTMORPH

#include "RooAbsPdf.h"
#include "RooRealProxy.h"
#include "RooCategoryProxy.h"
#include "RooAbsReal.h"
#include "RooAbsCategory.h"
#include "RooSetProxy.h"
#include "RooListProxy.h"
#include "RooArgList.h"

#include "TMatrixD.h"
#include "TVectorD.h"

class RooChangeTracker;

class RooMomentMorph : public RooAbsPdf {
public:

  enum Setting { Linear, NonLinear, NonLinearPosFractions, NonLinearLinFractions, SineLinear } ;

  RooMomentMorph() ;

  RooMomentMorph(const char *name, const char *title, RooAbsReal& _m, const RooArgList& varList,
          const RooArgList& pdfList, const RooArgList& mrefList, Setting setting = NonLinearPosFractions);
  RooMomentMorph(const char *name, const char *title, RooAbsReal& _m, const RooArgList& varList,
          const RooArgList& pdfList, const TVectorD& mrefpoints, Setting setting = NonLinearPosFractions );
  RooMomentMorph(const RooMomentMorph& other, const char* name=nullptr) ;
  TObject* clone(const char* newname=nullptr) const override { return new RooMomentMorph(*this,newname); }
  ~RooMomentMorph() override;

  void     setMode(const Setting& setting) { _setting = setting; }

  void useHorizontalMorphing(bool val) { _useHorizMorph = val; }

  bool selfNormalized() const override {
    // P.d.f is self normalized
    return true ;
  }

  double getValV(const RooArgSet* set=nullptr) const override;
  RooAbsPdf* sumPdf(const RooArgSet* nset) ;


protected:

  class CacheElem : public RooAbsCacheElement {
  public:
    CacheElem(std::unique_ptr<RooAbsPdf> && sumPdf,
              std::unique_ptr<RooChangeTracker> && tracker,
              const RooArgList& flist);
    ~CacheElem() override ;
    RooArgList containedArgs(Action) override ;
    std::unique_ptr<RooAbsPdf> _sumPdf ;
    std::unique_ptr<RooChangeTracker> _tracker ;
    RooArgList _frac ;

    RooRealVar* frac(Int_t i ) ;
    const RooRealVar* frac(Int_t i ) const ;
    void calculateFractions(const RooMomentMorph& self, bool verbose=true) const;
  } ;
  mutable RooObjCacheManager _cacheMgr ; //! The cache manager
  mutable RooArgSet* _curNormSet = nullptr; //! Current normalization set

  friend class CacheElem ; // Cache needs to be able to clear _norm pointer

  double evaluate() const override ;

  void     initialize();
  CacheElem* getCache(const RooArgSet* nset) const ;

  inline   Int_t ij(const Int_t& i, const Int_t& j) const { return (i*_varList.size()+j); }
  int      idxmin(const double& m) const;
  int      idxmax(const double& m) const;

  RooRealProxy m ;
  RooSetProxy  _varList ;
  RooListProxy _pdfList ;
  mutable TVectorD* _mref = nullptr;

  mutable TMatrixD* _M = nullptr;

  Setting _setting;

  bool _useHorizMorph = true;

  ClassDefOverride(RooMomentMorph,3);
};

#endif


