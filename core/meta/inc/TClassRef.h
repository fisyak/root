// @(#)root/meta:$Id$
// Author: Philippe Canal 15/03/2005

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TClassRef
#define ROOT_TClassRef

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TClassRef                                                            //
//                                                                      //
// Reference to a TClass object and intrusive list of other             //
// to thise same TClass object references                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TClass.h"

#include <string>
#include <atomic>

class TClassRef {

private:
   std::string                 fClassName; //Name of referenced class
   std::atomic<TClass *const*> fClassPtr;  //! Ptr to the permanent TClass ptr/reference

   friend class TClass;

   void Assign(const TClassRef &);
   void Assign(TClass *);
   TClass   *InternalGetClass() const;
public:
   TClassRef() : fClassName(), fClassPtr(nullptr) {}
   TClassRef(TClass *cl);
   TClassRef(const char *classname);
   TClassRef(const TClassRef&);
   inline TClassRef &operator=(const TClassRef &rhs) {
      // Inline implementation of operator= to speed the no-op case.
      if (this != &rhs && (!fClassPtr || fClassPtr != rhs.fClassPtr)) {
         this->Assign(rhs);
      }
      return *this;
   }
   inline TClassRef &operator=(TClass *rhs) {
      // Inline implementation of operator= to speed the no-op case.
      if ( !this->fClassPtr|| *(this->fClassPtr) != rhs) {
         this->Assign(rhs);
      }
      return *this;
   }

   ~TClassRef() { };

   void SetName(const char* new_name) {
      if ( fClassPtr && fClassName != new_name ) Reset();
      fClassName = new_name;
   }
   const char *GetClassName() { return fClassName.c_str(); }
   TClass *GetClass()  const { return (fClassPtr && *fClassPtr) ? *fClassPtr : InternalGetClass(); }
   void Reset() { fClassPtr = nullptr; }

   TClass* operator->() const { return (fClassPtr && *fClassPtr) ? *fClassPtr : InternalGetClass(); }
   operator TClass*() const { return (fClassPtr && *fClassPtr )? *fClassPtr : InternalGetClass(); }

};

#endif
