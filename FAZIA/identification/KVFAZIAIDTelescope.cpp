//Created by KVClassFactory on Mon Feb 17 13:51:39 2014
//Author: John Frankland,,,

#include "KVFAZIAIDTelescope.h"
#include "KVFAZIADetector.h"

ClassImp(KVFAZIAIDTelescope)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIAIDTelescope</h2>
<h4>Identification for FAZIA array</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIAIDTelescope::KVFAZIAIDTelescope()
{
   // Default constructor
}

KVFAZIAIDTelescope::~KVFAZIAIDTelescope()
{
   // Destructor
}

void KVFAZIAIDTelescope::AddDetector(KVDetector* d)
{
   //Add a detector to the telescope.
   //The first detector added is the "DeltaE" member, the second the "Eresidual" member.
   //Update name of telescope to "ID_[name of DE]_[name of E]"

   if (d) {
      fDetectors->Add(d);
      d->AddIDTelescope(this);
      if (GetSize() > 1)
         SetName(Form("ID_%s_%s_%d", GetDetector(1)->GetLabel(), GetDetector(2)->GetLabel(), ((KVFAZIADetector*)GetDetector(1))->GetIndex()));
      else SetName(Form("ID_%s_%d", GetDetector(1)->GetLabel(), ((KVFAZIADetector*)GetDetector(1))->GetIndex()));
   } else {
      Warning("AddDetector", "Called with null pointer");
   }
}

//________________________________________________________________
const Char_t* KVFAZIAIDTelescope::GetNewName(KVString oldname)
{

   KVString tmp = "";
   KVString lab = "";
   oldname.Begin("_");
   KVString id = oldname.Next();
   if (id != "ID") return "unkonwn";
   if (oldname.End()) return "unkonwn";
   KVString det1 = oldname.Next();
   KVString det2 = "";
   if (!oldname.End()) {
      det2 = oldname.Next();
   }
   KVString newdet1 = KVFAZIADetector::GetNewName(det1.Data());
   newdet1.Begin("-");
   KVString labdet1 = newdet1.Next();
   KVString idxdet1 = newdet1.Next();

   static KVString newname;
   if (det2 == "") {
      newname.Form("ID_%s_%s", labdet1.Data(), idxdet1.Data());
   } else {
      KVString newdet2 = KVFAZIADetector::GetNewName(det2.Data());
      newdet2.Begin("-");
      KVString labdet2 = newdet2.Next();
      newname.Form("ID_%s_%s_%s", labdet1.Data(), labdet2.Data(), idxdet1.Data());
   }

   return newname.Data();

}
