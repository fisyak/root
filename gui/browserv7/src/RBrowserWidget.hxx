// Author: Sergey Linev <S.Linev@gsi.de>
// Date: 2021-01-22
// Warning: This is part of the ROOT 7 prototype! It will change without notice. It might trigger earthquakes. Feedback is welcome!

/*************************************************************************
 * Copyright (C) 1995-2021, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT7_RBrowserWidget
#define ROOT7_RBrowserWidget

#include <memory>
#include <map>
#include <string>

#include <ROOT/Browsable/RElement.hxx>
#include <ROOT/RWebWindow.hxx>

namespace ROOT {

class RBrowser;

/** \class ROOT::RBrowserWidget
\ingroup rbrowser
Abstract Web-based widget, which can be used in the RBrowser
Used to embed canvas, geometry viewer and potentially any other widgets
*/

class RBrowserWidget {

   friend class RBrowser;

   std::string fName;   ///<!  widget name

   Browsable::RElementPath_t  fPath;  ///<! path of drawn element

   RBrowser  *fBrowser{nullptr};

public:

   explicit RBrowserWidget(const std::string &name) : fName(name) {};
   virtual ~RBrowserWidget() = default;

   RBrowser *GetBrowser() const { return fBrowser; }

   virtual std::shared_ptr<RWebWindow> GetWindow() { return nullptr; }

   virtual void ResetConn() {}

   virtual void SetActive() {}

   void SetPath(const Browsable::RElementPath_t &path) { fPath = path; }
   const Browsable::RElementPath_t &GetPath() const { return fPath; }

   const std::string &GetName() const { return fName; }
   virtual std::string GetKind() const = 0;
   virtual std::string GetUrl() { return ""; }
   virtual std::string GetTitle() { return ""; }

   virtual bool DrawElement(std::shared_ptr<Browsable::RElement> &, const std::string & = "") { return false; }
   virtual std::string SendWidgetContent() { return ""; }
   std::string SendWidgetTitle();

   virtual void CheckModified() {}

   virtual bool IsValid() { return true; }
};

class RBrowserWidgetProvider {
protected:
   using ProvidersMap_t = std::map<std::string, RBrowserWidgetProvider*>;

   virtual std::shared_ptr<RBrowserWidget> Create(const std::string &) = 0;

   virtual std::shared_ptr<RBrowserWidget> CreateFor(const std::string &, std::shared_ptr<Browsable::RElement> &) { return nullptr; }

   virtual std::shared_ptr<RBrowserWidget> DetectWindow(RWebWindow &) { return nullptr; }

   static RBrowserWidgetProvider *GetProvider(const std::string &kind);

   static ProvidersMap_t& GetMap();

public:

   explicit RBrowserWidgetProvider(const std::string &kind);
   virtual ~RBrowserWidgetProvider();

   static std::shared_ptr<RBrowserWidget> CreateWidget(const std::string &kind, const std::string &name);

   static std::shared_ptr<RBrowserWidget> CreateWidgetFor(const std::string &kind, const std::string &name, std::shared_ptr<Browsable::RElement> &element);

   static std::shared_ptr<RBrowserWidget> DetectCatchedWindow(const std::string &kind, RWebWindow &win);
};

} // namespace ROOT

#endif
