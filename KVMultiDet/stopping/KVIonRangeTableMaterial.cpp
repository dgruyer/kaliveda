//Created by KVClassFactory on Fri Sep 28 11:34:14 2012
//Author: John Frankland,,,

#include "KVIonRangeTableMaterial.h"
#include "TF1.h"
#include "KVNucleus.h"
#include "TGeoManager.h"
#include "TGeoMaterial.h"

ClassImp(KVIonRangeTableMaterial)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIonRangeTableMaterial</h2>
<h4>Material for use in energy loss & range calculations</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIonRangeTableMaterial::KVIonRangeTableMaterial()
   : KVBase(), 
   fTable(0),
   fState("unknown"),
   fComposition(0),
     fDens(0.),
     fZmat(0),
     fAmat(0),
     fMoleWt(0),
     fDeltaE(0),
     fEres(0),
     fRange(0),
     fStopping(0)
{
   // Default constructor
}

KVIonRangeTableMaterial::KVIonRangeTableMaterial(const KVIonRangeTable*tab,const Char_t* name, const Char_t* symbol,
      const Char_t* state, Double_t density, Double_t Z, Double_t A, Double_t MoleWt)
   : KVBase(name,symbol),
      fTable(tab),
      fState(state),
      fComposition(0),
     fDens(density),
     fZmat(Z),
     fAmat(A),
     fMoleWt(MoleWt),
     fDeltaE(0),
     fEres(0),
     fRange(0),
     fStopping(0)
{
   // Create new material with given (long) name and symbol
   //        symbol convention: for elements, use element symbol. for compounds, use chemical formula.
   //        e.g. silicon:    name="Silicon"  symbol="Si"
   //              C4H10:     name="Isobutane",  symbol="C4H10"
   // state="solid", "liquid", "gas"
   // density in g/cm**3
   // 
}

//________________________________________________________________

KVIonRangeTableMaterial::KVIonRangeTableMaterial (const KVIonRangeTableMaterial& obj)  : KVBase(),
   fTable(0),
      fState("unknown"),
   fComposition(0),
     fDens(0.),
     fZmat(0),
     fAmat(0),
     fMoleWt(0),
     fDeltaE(0),
     fEres(0),
     fRange(0),
     fStopping(0)      
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

KVIonRangeTableMaterial::~KVIonRangeTableMaterial()
{
   // Destructor
   SafeDelete(fComposition);
   SafeDelete(fRange);
   SafeDelete(fEres);
   SafeDelete(fDeltaE);
   SafeDelete(fStopping);
}

//________________________________________________________________

void KVIonRangeTableMaterial::Copy (TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVIonRangeTableMaterial::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVBase::Copy(obj);
   //KVIonRangeTableMaterial& CastedObj = (KVIonRangeTableMaterial&)obj;
}

void KVIonRangeTableMaterial::AddCompoundElement(Int_t Z, Int_t A, Int_t Natoms)
{
   // Add an element to a compound material
   // Example: to define C3F8 gas:
   //      toto.AddCompoundElement(6,12,3);
   //      toto.AddCompoundElement(9,19,8);
   
   KVNucleus n(Z,A);
   Int_t nel = 0;
   if(!fComposition) {fComposition=new KVList;}
   else nel=fComposition->GetEntries();
   KVNameValueList* l = new KVNameValueList(Form("Compound element %d",nel+1));
   l->SetValue("Z",Z);
   l->SetValue("A",A);
   l->SetValue("Ar", n.GetAtomicMass());
   l->SetValue("Weight", (1.0*Natoms));
   l->SetValue("Weight*Ar", n.GetAtomicMass()*(1.0*Natoms));
   fComposition->Add(l);
}

void KVIonRangeTableMaterial::AddMixtureElement(Int_t Z, Int_t A, Int_t Natoms, Double_t Proportion)
{
   // Add an element to a mixed material
   // Example: to define air, assuming 78% nitrogen (N2) and 21% oxygen (O2) and 1% argon (Ar):
   //      toto.AddMixtureElement(7,14, 2, 0.78);
   //      toto.AddMixtureElement(8,16, 2, 0.21);
   //      toto.AddMixtureElement(18, 40, 2, 0.01);
   
   KVNucleus n(Z,A);
   Int_t nel = 0;
   if(!fComposition) fComposition=new KVList;
   else nel=fComposition->GetEntries();
   KVNameValueList* l = new KVNameValueList(Form("Mixture element %d",nel+1));
   l->SetValue("Z",Z);
   l->SetValue("A",A);
   l->SetValue("Ar", n.GetAtomicMass());
   l->SetValue("Weight", Natoms*Proportion);
   l->SetValue("Weight*Ar", n.GetAtomicMass()*Proportion*Natoms);
   fComposition->Add(l);
}

void KVIonRangeTableMaterial::Initialize()
{
   // Correctly initialize material ready for use
   // For compound materials, calculate normalised weights of components
   // and molar weight of substance
   
   fMoleWt=0.;
   if(IsCompound()){
      TIter next(fComposition);
      KVNameValueList* nvl;
      Double_t totW=0,totmW=0;
      while( (nvl = (KVNameValueList*)next()) ){
         Double_t war = nvl->GetDoubleValue("Weight*Ar");
         fMoleWt+=war;
         Double_t prop = nvl->GetDoubleValue("Weight");
         totmW+=war;
         totW+=prop;
      }
      next.Reset();
      while( (nvl = (KVNameValueList*)next()) ){
         Double_t war = nvl->GetDoubleValue("Weight*Ar");
         Double_t prop = nvl->GetDoubleValue("Weight");
         nvl->SetValue("NormWeight*Ar",war/totmW);
         nvl->SetValue("NormWeight",prop/totW);
      }
   }
}
   
const KVIonRangeTable* KVIonRangeTableMaterial::GetTable() const
{
   return fTable;
}

void KVIonRangeTableMaterial::ls(Option_t*) const
{
   printf("%s::%s     Material type : %s    State : %s\n", ClassName(), GetName(), GetType(), fState.Data());
   printf("\tZ=%f  A=%f  ", fZmat, fAmat);
   if (IsGas()) printf(" Mole Weight = %f g.", fMoleWt);
   if (fDens > 0) printf(" Density = %f g/cm**3", fDens);
   printf("\n\n");
}


void KVIonRangeTableMaterial::PrintRangeTable(Int_t Z, Int_t A, Double_t isoAmat, Double_t units, Double_t T, Double_t P)
{
   // Print range of element (in g/cm**2) as a function of incident energy (in MeV).
   // For solid elements, print also the linear range (in cm). To change the default units,
   // set optional argument units (e.g. to have linear range in microns, call with units = KVUnits::um).
   // For gaseous elements, give the temperature (in degrees) and the pressure (in torr)
   // in order to print the range in terms of length units.

   GetRangeFunction(Z, A, isoAmat);
   printf("  ****  %s Range Table  ****\n\n",GetTable()->GetName());
   ls();
   printf(" Element: Z=%d A=%d\n\n", Z, A);
   printf("\tENERGY (MeV)\t\tRANGE (g/cm**2)");
   if (!IsGas() || (IsGas() && T > 0 && P > 0)) printf("\t\tLIN. RANGE");
   SetTemperatureAndPressure(T, P);
   printf("\n\n");
   for (Double_t e = 0.1; (e <= 1.e+4 && e <=GetEmaxValid(Z,A)); e *= 10) {
      printf("\t%10.5g\t\t%10.5g", e, fRange->Eval(e));
      if (!IsGas() || (IsGas() && T > 0 && P > 0)) printf("\t\t\t%10.5g", fRange->Eval(e) / GetDensity() / units);
      printf("\n");
   }
}

Double_t KVIonRangeTableMaterial::GetRangeOfIon(Int_t Z, Int_t A, Double_t E, Double_t isoAmat)
{
   // Returns range (in g/cm**2) of ion (Z,A) with energy E (MeV) in material.
   // Give Amat to change default (isotopic) mass of material,

   TF1* f = GetRangeFunction(Z, A, isoAmat);
   return f->Eval(E);
}

Double_t KVIonRangeTableMaterial::GetLinearRangeOfIon(Int_t Z, Int_t A, Double_t E, Double_t isoAmat, Double_t T, Double_t P)
{
   // Returns range (in cm) of ion (Z,A) with energy E (MeV) in material.
   // Give Amat to change default (isotopic) mass of material,
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.

   SetTemperatureAndPressure(T, P);
   if (fDens > 0) return GetRangeOfIon(Z, A, E, isoAmat) / GetDensity();
   else return 0.;
}

Double_t KVIonRangeTableMaterial::GetDeltaEOfIon(Int_t Z, Int_t A, Double_t E, Double_t e, Double_t isoAmat)
{
   // Returns energy lost (in MeV) by ion (Z,A) with energy E (MeV) after thickness e (in g/cm**2).
   // Give Amat to change default (isotopic) mass of material,

   TF1* f = GetDeltaEFunction(e, Z, A, isoAmat);
   return f->Eval(E);
}

Double_t KVIonRangeTableMaterial::GetLinearDeltaEOfIon(Int_t Z, Int_t A, Double_t E, Double_t e,
                                                 Double_t isoAmat, Double_t T, Double_t P)
{
   // Returns energy lost (in MeV) by ion (Z,A) with energy E (MeV) after thickness e (in cm).
   // Give Amat to change default (isotopic) mass of material,
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.

   SetTemperatureAndPressure(T, P);
   e *= GetDensity();
   return GetDeltaEOfIon(Z, A, E, e, isoAmat);
}

Double_t KVIonRangeTableMaterial::GetEResOfIon(Int_t Z, Int_t A, Double_t E, Double_t e,
                                         Double_t isoAmat)
{
   // Returns energy lost (in MeV) by ion (Z,A) with energy E (MeV) after thickness e (in g/cm**2).
   // Give Amat to change default (isotopic) mass of material,
   
   TF1* f = GetEResFunction(e, Z, A, isoAmat);
   return f->Eval(E);
}

Double_t KVIonRangeTableMaterial::GetLinearEResOfIon(Int_t Z, Int_t A, Double_t E, Double_t e,
                                               Double_t isoAmat, Double_t T, Double_t P)
{
   // Returns energy lost (in MeV) by ion (Z,A) with energy E (MeV) after thickness e (in cm).
   // Give Amat to change default (isotopic) mass of material,
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.

   SetTemperatureAndPressure(T, P);
   e *= GetDensity();
   return GetEResOfIon(Z, A, E, e, isoAmat);
}

Double_t KVIonRangeTableMaterial::GetEIncFromEResOfIon(Int_t Z, Int_t A, Double_t Eres, Double_t e, Double_t isoAmat)
{
   // Calculates incident energy (in MeV) of an ion (Z,A) with residual energy Eres (MeV) after thickness e (in g/cm**2).
   // Give Amat to change default (isotopic) mass of material,
   GetRangeFunction(Z, A, isoAmat);
   Double_t R0 = fRange->Eval(Eres) + e;
   return fRange->GetX(R0);
}

Double_t KVIonRangeTableMaterial::GetLinearEIncFromEResOfIon(Int_t Z, Int_t A, Double_t Eres, Double_t e,
                                                       Double_t isoAmat, Double_t T, Double_t P)
{
   // Calculates incident energy (in MeV) of an ion (Z,A) with residual energy Eres (MeV) after thickness e (in cm).
   // Give Amat to change default (isotopic) mass of material,
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.

   SetTemperatureAndPressure(T, P);
   e *= GetDensity();
   return GetEIncFromEResOfIon(Z, A, Eres, e, isoAmat);
}

Double_t KVIonRangeTableMaterial::GetEIncFromDeltaEOfIon(Int_t Z, Int_t A, Double_t DeltaE, Double_t e, enum KVIonRangeTable::SolType type, Double_t isoAmat)
{
   // Calculates incident energy (in MeV) of an ion (Z,A) from energy loss DeltaE (MeV) in thickness e (in g/cm**2).
   // Give Amat to change default (isotopic) mass of material,
   GetDeltaEFunction(e, Z, A, isoAmat);
   Double_t e1,e2;
   fDeltaE->GetRange(e1,e2);
   switch(type){
      case KVIonRangeTable::kEmin:
         e2=GetEIncOfMaxDeltaEOfIon(Z,A,e,isoAmat);
         break;
      case KVIonRangeTable::kEmax:
         e1=GetEIncOfMaxDeltaEOfIon(Z,A,e,isoAmat);
         break;
   }
   return fDeltaE->GetX(DeltaE,e1,e2);
}

Double_t KVIonRangeTableMaterial::GetLinearEIncFromDeltaEOfIon(Int_t Z, Int_t A, Double_t deltaE, Double_t e, enum KVIonRangeTable::SolType type,
                                                         Double_t isoAmat, Double_t T, Double_t P)
{
   // Calculates incident energy (in MeV) of an ion (Z,A) from energy loss DeltaE (MeV) in thickness e (in cm).
   // Give Amat to change default (isotopic) mass of material,
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.

   SetTemperatureAndPressure(T, P);
   e *= GetDensity();
   return GetEIncFromDeltaEOfIon(Z, A, deltaE, e, type, isoAmat);
}

Double_t KVIonRangeTableMaterial::GetPunchThroughEnergy(Int_t Z, Int_t A, Double_t e, Double_t isoAmat)
{
   // Calculate incident energy (in MeV) for ion (Z,A) for which the range is equal to the
   // given thickness e (in g/cm**2). At this energy the residual energy of the ion is (just) zero,
   // for all energies above this energy the residual energy is > 0.
   // Give Amat to change default (isotopic) mass of material.
   
   return GetRangeFunction(Z,A,isoAmat)->GetX(e);
}

Double_t KVIonRangeTableMaterial::GetLinearPunchThroughEnergy(Int_t Z, Int_t A, Double_t e, Double_t isoAmat, Double_t T, Double_t P) 
{
   // Calculate incident energy (in MeV) for ion (Z,A) for which the range is equal to the
   // given thickness e (in cm). At this energy the residual energy of the ion is (just) zero,
   // for all energies above this energy the residual energy is > 0.
   // Give Amat to change default (isotopic) mass of material.
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.
   
   SetTemperatureAndPressure(T, P);
   e *= GetDensity();
   return GetPunchThroughEnergy(Z,A,e,isoAmat);
}

Double_t KVIonRangeTableMaterial::GetMaxDeltaEOfIon(Int_t Z, Int_t A, Double_t e, Double_t isoAmat)
{
   // Calculate maximum energy loss (in MeV) of ion (Z,A) in given thickness e (in g/cm**2).
   // Give Amat to change default (isotopic) mass of material.
   
   return GetDeltaEFunction(e,Z,A,isoAmat)->GetMaximum();
}

Double_t KVIonRangeTableMaterial::GetEIncOfMaxDeltaEOfIon(Int_t Z, Int_t A, Double_t e, Double_t isoAmat)
{
   // Calculate incident energy (in MeV) corresponding to maximum energy loss of ion (Z,A)
   // in given thickness e (in g/cm**2).
   // Give Amat to change default (isotopic) mass of material.
   
   return GetDeltaEFunction(e,Z,A,isoAmat)->GetMaximumX();
}

Double_t KVIonRangeTableMaterial::GetLinearMaxDeltaEOfIon(Int_t Z, Int_t A, Double_t e, Double_t isoAmat, Double_t T, Double_t P)
{
   // Calculate maximum energy loss (in MeV) of ion (Z,A) in given thickness e (in cm).
   // Give Amat to change default (isotopic) mass of material.
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.
   
   SetTemperatureAndPressure(T, P);
   e *= GetDensity();
   return GetMaxDeltaEOfIon(Z,A,e,isoAmat);
}

Double_t KVIonRangeTableMaterial::GetLinearEIncOfMaxDeltaEOfIon(Int_t Z, Int_t A, Double_t e, Double_t isoAmat, Double_t T, Double_t P)
{
   // Calculate incident energy (in MeV) corresponding to maximum energy loss of ion (Z,A)
   // in given thickness e (in cm).
   // Give Amat to change default (isotopic) mass of material.
   // give temperature (degrees C) & pressure (torr) (T,P) for gaseous materials.
   
   SetTemperatureAndPressure(T, P);
   e *= GetDensity();
   return GetEIncOfMaxDeltaEOfIon(Z,A,e,isoAmat);
}

TGeoMaterial* KVIonRangeTableMaterial::GetTGeoMaterial() const
{
	// Create and return pointer to a TGeoMaterial or TGeoMixture (for compound materials)
	// with the properties of this material.
	// gGeoManager must exist.
	
	TGeoMaterial* gmat=0x0;
	if(!gGeoManager) return gmat;
	if(IsCompound()){
		gmat = new TGeoMixture(GetTitle(), GetComposition()->GetEntries(), GetDensity());
		TIter next(GetComposition());
		KVNameValueList* nvl;
		while((nvl = (KVNameValueList*)next())){
         KVNucleus n( nvl->GetIntValue("Z"), nvl->GetIntValue("A") );
			TGeoElement* gel = gGeoManager->GetElementTable()->FindElement( n.GetSymbol("EL") );
			float poids = nvl->GetDoubleValue( "NormWeight" );
			((TGeoMixture*)gmat)->AddElement(gel, poids);
		}
	}
	else
	{
		gmat = new TGeoMaterial( GetTitle(), GetMass(), GetZ(), GetDensity() );
	}
   // set state of material
   if(IsGas()) gmat->SetState(TGeoMaterial::kMatStateGas);
   else gmat->SetState(TGeoMaterial::kMatStateSolid);
	return gmat;
}