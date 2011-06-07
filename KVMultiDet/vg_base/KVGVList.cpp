//
//Author: Daniel Cussol 
//
// 17/02/2004
// Creation d'une classe Liste de  Variables Globales
// Elle est utilisee dans les analyses KaliVeda.
//

#include "Riostream.h"
#include "KVGVList.h"

ClassImp(KVGVList)
//////////////////////////////////////////////////////////////////////////////////
//    List of global variables
//
//   This class allows to process in a single call many KVVargGlob instances. This list 
// can be managed by using standard KVList methods. The methods used to initiate, reset 
// and fill such a liste are:
//
//      virtual void Init(void)                 initiates the global variables 
//      virtual void Reset(void)                resets the global variables before the treatment
//      virtual void Fill(KVNucleus *c)         compute all the 1-body global variables
//      virtual void Fill2(KVNucleus *n1, KVNucleus *n2)         compute all the 2-body global variables
//      virtual void FillN(KVEvent* e)         compute all the N-body global variables
//
// By default the KVGVList does not own the objects it contains (they may be on the stack).
// User's responsibility in this case to delete heap-based objects after use.
//
// Here is a short example of use:
//
//==================================================================================================
// // Declarations and initialisations
// ...
// KVEkin *Sekin=new KVEkin("SEkin");
// KVZmean zmean;
// KVZmax  zmax;
// KVGVList *gvlist=new KVGVList();
// gvlist->Add(Sekin);
// gvlist->Add(&zmean);
// gvlist->Add(&zmax);
// gvlist->Init();
//
// ...
// // Treatment loop for each event called for each event
// ...
// gvlist->Reset();
// KVINDRAReconNuc *part = 0;
// while( (part = GetEvent()->GetNextParticle("ok")) ){//loop over particles with correct codes
//  gvlist->Fill(part); 
// }
// cout << "Total kinetic energy : " << Sekin->GetValue() << endl; 
// cout << "Mean charge          : " << zmean() << endl; 
// cout << "Standard deviation   : " << zmean("RMS") << endl; 
// cout << "Charge of the heaviest   : " << zmax() << endl; 
// cout << "Vpar of the heaviest     : " << ((KVNucleus *)zmax.GetObject()).GetVpar() << endl; 
// ...
//
//////////////////////////////////////////////////////////////////////////////////
Int_t KVGVList::nb = 0;
Int_t KVGVList::nb_crea = 0;
Int_t KVGVList::nb_dest = 0;

//_________________________________________________________________
void KVGVList::init_KVGVList(void)
{
//
// Initialisation des champs de KVGVList
// Cette methode privee n'est appelee par les createurs
//
   nb++;
   nb_crea++;

   KVList::SetOwner(kFALSE);
   fNbBranch = 0;
}

//_________________________________________________________________
KVGVList::KVGVList(void):KVList()
{
//
// Createur par default
//
   init_KVGVList();
#ifdef DEBUG_KVGVList
   cout << nb << " crees...(defaut) " << endl;
#endif
}


//_________________________________________________________________
KVGVList::KVGVList(const KVGVList & a)
{
// 
// Contructeur par Copy
//
   init_KVGVList();
   a.Copy(*this);
#ifdef DEBUG_KVGVList
   cout << nb << " crees...(Copy) " << endl;
#endif
}

//_________________________________________________________________
KVGVList::~KVGVList(void)
{
// 
// Destructeur
//
#ifdef DEBUG_KVGVList
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;

   nb_dest++;
}


//_________________________________________________________________
void KVGVList::Init(void)
{
	// methode d'initialisation des
	// variables Internes
	
	this->R__FOR_EACH(KVVarGlob,Init)();
}

//_________________________________________________________________
void KVGVList::Reset(void)
{
	// Remise a zero avant le
	// traitement d'un evenement
	this->R__FOR_EACH(KVVarGlob,Reset)();
}

//_________________________________________________________________
void KVGVList::Fill(KVNucleus * c)
{
   // Calls Fill(KVNucleus*) method of all one-body variables in the list
   // for all KVNucleus satisfying the KVParticleCondition given to
   // SetSelection (if no selection given, all nuclei are used).
   
	fVG1.R__FOR_EACH(KVVarGlob,FillWithCondition)(c);
}


//_________________________________________________________________
void KVGVList::Fill2(KVNucleus * c1, KVNucleus* c2)
{
   // Calls Fill(KVNucleus*,KVNucleus*) method of all two-body variables in the list
   // for all pairs of KVNucleus (c1,c2) satisfying the KVParticleCondition given to
   // SetSelection (if no selection given, all nuclei are used).
	
	fVG2.R__FOR_EACH(KVVarGlob,Fill2WithCondition)(c1,c2);
}

//_________________________________________________________________
void KVGVList::FillN(KVEvent* r)
{
   // Calls FillN(KVEvent*) method of all N-body variables in the list
   TObjLink *lnk = fVGN.FirstLink();
   while (lnk) {
      KVVarGlob *vg = (KVVarGlob *) lnk->GetObject();
      vg->FillN(r);
      lnk = lnk->Next();
   }
}

//_________________________________________________________________
KVVarGlob *KVGVList::GetGV(const Char_t * nom)
{
   //Return pointer to global variable in list with name 'nom'

   return (KVVarGlob *) FindObject(nom);
}

//_________________________________________________________________
void KVGVList::Add(TObject* obj)
{
   // Overrides TList::Add(TObject*) so that global variable pointers are sorted
   // between the 3 lists used for 1-body, 2-body & N-body variables.
   
   if( obj->InheritsFrom("KVVarGlob") ){
      // put global variable pointer in appropriate list
      KVVarGlob* vg = (KVVarGlob*)obj;
      if( vg->IsOneBody() ) fVG1.Add( vg );
      else if( vg->IsTwoBody() ) fVG2.Add( vg );
      else if( vg->IsNBody() ) fVGN.Add( vg );
   }
   // add object to main list
   KVList::Add(obj);
}

//_________________________________________________________________

TObject** KVGVList::GetGVRef(const Char_t* name)
{
   // Returns pointer to pointer holding address of GV in list with given name.
   // This can be used in order to store global variables used in a KVSelector in a TTree:
   //
   // [ in MySelector::InitAnalysis... ]
   //    AddGV("KVZmax", "zmax");
   //    TTree* tree = new TTree(...);
   //    tree->Branch("zmax", "KVZmax", GetGVList()->GetGVRef("zmax"));
   //
   // This will create a leaf called "zmax" containing the global variable for each event.
   // In the Tree Viewer you can plot the values of this leaf variable in the same way as
   // you would with any simple variable.
   // 
   // Note that if you want to store all of the global variables in the tree, you just need to
   // do the following:
   //
   // [in MySelector::InitAnalysis... ]
   //    AddGV("KVZmax", "zmax");
   //    AddGV("KVZtot", "ztot");
   //    AddGV("KVEtrans", "etrans");
   //    etc. etc.
   //    TTree* tree = new TTree(...);
   //    tree->Branch(GetGVList());
   //
   // This will create a leaf for each global variable in the list.

	TObject* obj = FindObject(name);
	if(obj) return GetObjectRef(obj);
   return 0;
}

//_________________________________________________________________

void KVGVList::MakeBranches(TTree* tree)
{
   // Create a branch in the TTree for each global variable in the list.
   // A leaf with the name of each global variable will be created to hold the
   // value of the variable (result of GetValue() method).
   // For multi-valued global variables we add a branch for each value with name
   //   GVname.ValueName
   
   if(!tree) return;
   if(fNbBranch>=MAX_CAP_BRANCHES) return;
   
	TIter next(this); TObject*ob;
   while ((ob = next()) && fNbBranch<MAX_CAP_BRANCHES) {
      
      if(((KVVarGlob*)ob)->GetNumberOfValues()>1){
      	// multi-valued variable
      	for(int i=0; i<((KVVarGlob*)ob)->GetNumberOfValues(); i++){
      		// replace any nasty mathematical symbols which could pose problems
      		// in names of TTree leaves/branches
      	   TString sane_name( ((KVVarGlob*)ob)->GetValueName(i) );
      	   sane_name.ReplaceAll("*", "star");
      		tree->Branch( Form("%s.%s", ob->GetName(), sane_name.Data()),
      			&fBranchVar[ fNbBranch++ ], Form("%s_%s/D", ob->GetName(), sane_name.Data()));
      	}
      }
      else
      	tree->Branch( ob->GetName(), &fBranchVar[ fNbBranch++ ], Form("%s/D", ob->GetName()));
      
   }
}

//_________________________________________________________________

void KVGVList::FillBranches()
{
   // Use this method ONLY if you first use MakeBranches(TTree*) in order to
   // automatically create branches for your global variables.
   // Call this method for each event in order to put the values of the variables
   // in the branches ready for TTree::Fill to be called (note that Fill() is not
   // called in this method: you should do it after this).
   
   if( !fNbBranch ) return; // MakeBranches has not been called
   
   int i=0;
	TIter next(this); KVVarGlob*ob;
   while ((ob = (KVVarGlob*)next()) && fNbBranch<MAX_CAP_BRANCHES) {
            
      if(((KVVarGlob*)ob)->GetNumberOfValues()>1){
      	// multi-valued variable
      	for(int j=0; j<((KVVarGlob*)ob)->GetNumberOfValues(); j++){
      		fBranchVar[ i++ ] = ob->GetValue(j);
      	}
      }
      else
      	fBranchVar[ i++ ] = ob->GetValue();      
		
   }
}
