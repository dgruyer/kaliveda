//Created by KVClassFactory on Fri Apr 13 12:31:16 2012
//Author: John Frankland

#include "KVTreeAnalyzer.h"
#include "TString.h"
#include "TDirectory.h"
#include "TEntryList.h"
#include "Riostream.h"
#include "KVCanvas.h"
#include "TCutG.h"
#include "TLeaf.h"
#include "TPaveStats.h"

ClassImp(KVTreeAnalyzer)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVTreeAnalyzer</h2>
<h4>KVTreeAnalyzer</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

#define MAX_COLOR_INDEX 5
Int_t my_color_array[] = {
   kBlue+2,
   kRed+2,
   kGreen+3,
   kCyan-2,
   kOrange+7,
   kViolet+2
};
   
KVTreeAnalyzer::KVTreeAnalyzer(Bool_t nogui)
   : TNamed(), fTree(0), fSelections(kTRUE), fHistoNumber(1), fSelectionNumber(1), fAliasNumber(1)
{
   // Default constructor
   fMain_histolist=0;
   fMain_leaflist=0;
   fMain_selectionlist=0;
   fDrawSame = fDrawLog = fApplySelection = fSwapLeafExpr = fNormHisto = kFALSE;
   fNewCanvas = kTRUE;
   fSameColorIndex=0;
   fSelectedSelections=0;
   fSelectedLeaves=0;
   fSelectedHistos=0;
   ipscale=0;
      GDfirst=new KVGumbelDistribution("Gum1",1);
      GDsecond=new KVGumbelDistribution("Gum2",2);
      GDthird=new KVGumbelDistribution("Gum3",3);
      GD4=new KVGumbelDistribution("Gum4",4);
      GD5=new KVGumbelDistribution("Gum5",5);
      GausGum=new KVGausGumDistribution("GausGum");
      fNoGui=nogui;
}


KVTreeAnalyzer::KVTreeAnalyzer(TTree*t,Bool_t nogui)
   : TNamed("KVTreeAnalyzer", t->GetTitle()), fTree(t), fSelections(kTRUE), fHistoNumber(1), fSelectionNumber(1), fAliasNumber(1)
{
   fMain_histolist=0;
   fMain_leaflist=0;
   fMain_selectionlist=0;
   fTreeName = t->GetName();
   fTreeFileName = t->GetCurrentFile()->GetName();
   fDrawSame = fDrawLog = fApplySelection = fSwapLeafExpr = fNormHisto = kFALSE;
   fNewCanvas = kTRUE;
   fNoGui=nogui;
   OpenGUI();
   fSameColorIndex=0;
   fSelectedSelections=0;
   fSelectedLeaves=0;
   fSelectedHistos=0;
   ipscale=0;
      GDfirst=new KVGumbelDistribution("Gum1",1);
      GDsecond=new KVGumbelDistribution("Gum2",2);
      GDthird=new KVGumbelDistribution("Gum3",3);
      GD4=new KVGumbelDistribution("Gum4",4);
      GD5=new KVGumbelDistribution("Gum5",5);
      GausGum=new KVGausGumDistribution("GausGum");
}

KVTreeAnalyzer::~KVTreeAnalyzer()
{
   // Destructor
   SafeDelete(fSelectedSelections);
   SafeDelete(fSelectedLeaves);
   SafeDelete(fSelectedHistos);
   SafeDelete(ipscale);
   SafeDelete(fMain_histolist);
   SafeDelete(fMain_leaflist);
   SafeDelete(fMain_selectionlist);
}

void KVTreeAnalyzer::GenerateHistoTitle(TString& title, const Char_t* expr, const Char_t* selection)
{
   TString _selection(selection);
   TString _elist;
   if(fTree->GetEntryList()) _elist = fTree->GetEntryList()->GetTitle();
   if(_selection!="" && _elist!="")
      title.Form("%s {(%s) && (%s)}", expr, _elist.Data(), selection);
   else if(_selection!="")
      title.Form("%s {%s}", expr, selection);
   else if(_elist!="")
      title.Form("%s {%s}", expr, _elist.Data());
   else
      title.Form("%s", expr);
}

TH1* KVTreeAnalyzer::MakeHisto(const Char_t* expr, const Char_t* selection, Int_t nX, Int_t nY)
{
   //CurrentSelection();
   TString name;
   name.Form("h%d",fHistoNumber);
   TString drawexp(expr), histo, histotitle;
   GenerateHistoTitle(histotitle, expr, selection);
   if(nY)
      histo.Form(">>%s(%d,0.,0.,%d,0.,0.)", name.Data(), nX, nY);
   else
      histo.Form(">>%s(%d,0.,0.)", name.Data(), nX);
   drawexp += histo;
   fTree->Draw(drawexp, selection, "goff");
   TH1* h = (TH1*)gDirectory->Get(name);
   h->SetTitle(histotitle);
   if(h->InheritsFrom("TH2")) h->SetOption("col");
   h->SetDirectory(0);
   AddHisto(h);
   fHistoNumber++;
      if(fNormHisto){
         h->Sumw2();
         h->Scale(1./h->Integral());
      }
   return h;
}

TH1* KVTreeAnalyzer::MakeIntHisto(const Char_t* expr, const Char_t* selection, Int_t Xmin, Int_t Xmax)
{
   //CurrentSelection();
   TString name;
   name.Form("Ih%d",fHistoNumber);
   TString drawexp(expr), histo, histotitle;
   GenerateHistoTitle(histotitle, expr, selection);
   histo.Form(">>%s(%d,%f,%f)", name.Data(), (Xmax-Xmin)+1, Xmin-0.5, Xmax+0.5);
   drawexp += histo;
   fTree->Draw(drawexp, selection, "goff");
   TH1* h = (TH1*)gDirectory->Get(name);
   h->SetTitle(histotitle);
   if(h->InheritsFrom("TH2")) h->SetOption("col");
   h->SetDirectory(0);
   AddHisto(h);
   fHistoNumber++;
      if(fNormHisto){
         h->Sumw2();
         h->Scale(1./h->Integral());
      }
   return h;
}
   
void KVTreeAnalyzer::MakeSelection(const Char_t* selection)
{
   //CurrentSelection();
   TString name;
   name.Form("el%d",fSelectionNumber);
   TString drawexp(name.Data());
   drawexp.Prepend(">>");
   fTree->Draw(drawexp, selection, "entrylist");
   TEntryList*el = (TEntryList*)gDirectory->Get(name);
   if(fTree->GetEntryList()){
      TString _elist = fTree->GetEntryList()->GetTitle();
      TString title;
      title.Form("(%s) && (%s)", _elist.Data(), selection);
      el->SetTitle(title);
   }
   fSelectionNumber++;
   AddSelection(el);
}
   
void KVTreeAnalyzer::SetSelection(TObject* obj)
{
   if(!obj->InheritsFrom("TEntryList")) return;
   TEntryList* el = dynamic_cast<TEntryList*>(obj);
   if(fTree->GetEntryList()==el){
	   fTree->SetEntryList(0);
      G_selection_status->SetText("CURRENT SELECTION:",0);
	   return;
   }
   fTree->SetEntryList(el);
   G_selection_status->SetText(Form("CURRENT SELECTION: %s (%lld)",el->GetTitle(),el->GetN()),0);
}
   
void KVTreeAnalyzer::CurrentSelection()
{
   TString tmp;
   if(fTree->GetEntryList()) tmp = fTree->GetEntryList()->GetTitle();
   else tmp="";
   if(tmp!="") cout << "CURRENT SELECTION : " << tmp << endl;
}

void KVTreeAnalyzer::FillLeafList()
{
   TList stuff;
   stuff.AddAll(fTree->GetListOfLeaves());
   stuff.AddAll(&fAliasList);
   G_leaflist->Display(&stuff);   
}

void KVTreeAnalyzer::OpenGUI()
{
   if(fNoGui) return;
   
   ULong_t red,cyan,green,yellow,magenta,gura,gurb,gurc,gurd,gure,gurf;
   gClient->GetColorByName("#ff00ff",magenta);
   gClient->GetColorByName("#ff0000",red);
   gClient->GetColorByName("#00ff00",green);
   gClient->GetColorByName("#00ffff",cyan);
   gClient->GetColorByName("#ffff00",yellow);
   gClient->GetColorByName("#cf14b2",gura);
   gClient->GetColorByName("#cd93e6",gurb);
   gClient->GetColorByName("#c1e91a",gurc);
   gClient->GetColorByName("#d1a45b",gurd);
   gClient->GetColorByName("#b54cfe",gure);
   gClient->GetColorByName("#a325ef",gurf);

   // main frame
   fMain_leaflist = new TGMainFrame(gClient->GetRoot(),10,10,kMainFrame | kVerticalFrame);
   fMain_leaflist->SetName("fMain_leaflist");
   fMain_leaflist->SetWindowName("VARIABLES");
   UInt_t lWidth = 200, lHeight = 500;
   /* leaf list */
   TGHorizontalFrame *fHorizontalFrame = new TGHorizontalFrame(fMain_leaflist,lWidth,36,kHorizontalFrame);
   G_leaf_draw = new TGPictureButton(fHorizontalFrame, "$ROOTSYS/icons/draw_t.xpm");
   G_leaf_draw->SetEnabled(kFALSE);
   G_leaf_draw->Connect("Clicked()", "KVTreeAnalyzer", this, "DrawLeafExpr()");
   fHorizontalFrame->AddFrame(G_leaf_draw, new TGLayoutHints(kLHintsTop|kLHintsLeft,2,2,2,2));
   G_leaf_swap = new TGCheckButton(fHorizontalFrame, "Swap");
   G_leaf_swap->SetToolTipText("Swap axes");
   G_leaf_swap->Connect("Toggled(Bool_t)", "KVTreeAnalyzer", this, "SetSwapLeafExpr(Bool_t)");
   fHorizontalFrame->AddFrame(G_leaf_swap, new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsCenterY,2,2,2,2));
   fLeafExpr="          ";
   G_leaf_expr = new TGLabel(fHorizontalFrame, fLeafExpr.Data());
   G_leaf_expr->Resize();
   fHorizontalFrame->AddFrame(G_leaf_expr, new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsCenterY,2,2,2,2));
   fMain_leaflist->AddFrame(fHorizontalFrame, new TGLayoutHints(kLHintsExpandX | kLHintsTop,1,1,1,1));
   /* make selection */
   fHorizontalFrame = new TGHorizontalFrame(fMain_leaflist,lWidth,36,kHorizontalFrame);
   TGLabel* lab = new TGLabel(fHorizontalFrame, "Make alias : ");
   fHorizontalFrame->AddFrame(lab, new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,2,2));
   G_alias_text = new TGTextEntry(fHorizontalFrame, new TGTextBuffer(50));
   G_alias_text->SetMaxLength(4096);
   G_alias_text->SetAlignment(kTextLeft);
   G_alias_text->Resize(500,G_alias_text->GetDefaultHeight());
   G_alias_text->Connect("ReturnPressed()", "KVTreeAnalyzer", this, "GenerateAlias()");
   fHorizontalFrame->AddFrame(G_alias_text, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX,2,5,2,2));
   fMain_leaflist->AddFrame(fHorizontalFrame, new TGLayoutHints(kLHintsExpandX | kLHintsTop,1,1,1,1));
   G_leaflist = new KVListView(TNamed::Class(), fMain_leaflist, lWidth, lHeight);
   G_leaflist->SetDataColumns(1);
   G_leaflist->SetDataColumn(0, "Title");
   G_leaflist->ActivateSortButtons();
   G_leaflist->SetDoubleClickAction("KVTreeAnalyzer", this, "DrawLeaf(TObject*)");
   G_leaflist->Connect("SelectionChanged()", "KVTreeAnalyzer", this, "LeafChanged()");
   fMain_leaflist->AddFrame(G_leaflist, new TGLayoutHints(kLHintsLeft|kLHintsTop|
                                       kLHintsExpandX|kLHintsExpandY,
				       5,5,5,5));
  
   fMain_leaflist->MapSubwindows();

   fMain_leaflist->Resize(fMain_leaflist->GetDefaultSize());
   fMain_leaflist->MapWindow();
   fMain_leaflist->Resize(lWidth,lHeight);
   FillLeafList();
   // main frame
   fMain_histolist = new TGMainFrame(gClient->GetRoot(),10,10,kMainFrame | kVerticalFrame);
   fMain_histolist->SetName("fMain_histolist");
   fMain_histolist->SetWindowName("HISTOS");
   UInt_t hWidth = 400, hHeight = 600;
   TGTextButton* save = new TGTextButton(fMain_histolist,Form("SAVE Analysis_%s",fTreeFileName.Data()));
   save->SetTextJustify(36);
   save->SetMargins(0,0,0,0);
   save->SetWrapLength(-1);
   save->Resize(hWidth,save->GetDefaultHeight());
   save->Connect("Clicked()", "KVTreeAnalyzer", this, "Save()");
   save->SetEnabled(kTRUE);
   save->ChangeBackground(yellow);
   fMain_histolist->AddFrame(save, new TGLayoutHints(kLHintsLeft | kLHintsTop| kLHintsExpandX,2,2,2,2));
   G_histo_del = new TGTextButton(fMain_histolist,"DELETE HISTO!");
   G_histo_del->SetTextJustify(36);
   G_histo_del->SetMargins(0,0,0,0);
   G_histo_del->SetWrapLength(-1);
   G_histo_del->Resize(hWidth,G_histo_del->GetDefaultHeight());
   G_histo_del->Connect("Clicked()", "KVTreeAnalyzer", this, "DeleteSelectedHisto()");
   G_histo_del->SetEnabled(kFALSE);
   G_histo_del->ChangeBackground(red);
   fMain_histolist->AddFrame(G_histo_del, new TGLayoutHints(kLHintsLeft | kLHintsTop| kLHintsExpandX,2,2,2,2));
   /* histo options */
   TGGroupFrame* histo_opts = new TGGroupFrame(fMain_histolist, "Options", kVerticalFrame);
   fHorizontalFrame = new TGHorizontalFrame(histo_opts,hWidth,50,kHorizontalFrame);
   G_histo_new_can = new TGCheckButton(fHorizontalFrame, "New canvas");
   G_histo_new_can->SetToolTipText("Draw in a new canvas");
   G_histo_new_can->Connect("Toggled(Bool_t)", "KVTreeAnalyzer", this, "SetNewCanvas(Bool_t)");
   G_histo_new_can->SetState(kButtonDown);
   fHorizontalFrame->AddFrame(G_histo_new_can, new TGLayoutHints(kLHintsLeft|kLHintsCenterX,2,2,2,2));
   G_histo_same = new TGCheckButton(fHorizontalFrame, "Same");
   G_histo_same->SetToolTipText("Draw in same pad");
   G_histo_same->Connect("Toggled(Bool_t)", "KVTreeAnalyzer", this, "SetDrawSame(Bool_t)");
   fHorizontalFrame->AddFrame(G_histo_same, new TGLayoutHints(kLHintsLeft|kLHintsCenterX,2,2,2,2));
   G_histo_app_sel = new TGCheckButton(fHorizontalFrame, "Apply selection");
   G_histo_app_sel->SetToolTipText("Apply current selection to generate new histo");
   G_histo_app_sel->Connect("Toggled(Bool_t)", "KVTreeAnalyzer", this, "SetApplySelection(Bool_t)");
   fHorizontalFrame->AddFrame(G_histo_app_sel, new TGLayoutHints(kLHintsLeft|kLHintsCenterX,2,2,2,2));
   histo_opts->AddFrame(fHorizontalFrame, new TGLayoutHints(kLHintsCenterX|kLHintsTop,2,2,2,2));
   fHorizontalFrame = new TGHorizontalFrame(histo_opts,hWidth,50,kHorizontalFrame);
   G_histo_log = new TGCheckButton(fHorizontalFrame, "Log scale");
   G_histo_log->SetToolTipText("Use log scale in Y (1D) or Z (2D)");
   G_histo_log->Connect("Toggled(Bool_t)", "KVTreeAnalyzer", this, "SetDrawLog(Bool_t)");
   fHorizontalFrame->AddFrame(G_histo_log, new TGLayoutHints(kLHintsLeft|kLHintsCenterX,2,2,2,2));
   G_histo_norm = new TGCheckButton(fHorizontalFrame, "Normalize");
   G_histo_norm->SetToolTipText("Generate normalized histogram with integral=1");
   G_histo_norm->Connect("Toggled(Bool_t)", "KVTreeAnalyzer", this, "SetNormHisto(Bool_t)");
   fHorizontalFrame->AddFrame(G_histo_norm, new TGLayoutHints(kLHintsLeft|kLHintsCenterX,2,2,2,2));
   histo_opts->AddFrame(fHorizontalFrame, new TGLayoutHints(kLHintsCenterX|kLHintsTop,2,2,2,2));
   fMain_histolist->AddFrame(histo_opts, new TGLayoutHints(kLHintsLeft|kLHintsTop|kLHintsExpandX,5,5,5,5));
   /* ip scale */
   histo_opts = new TGGroupFrame(fMain_histolist, "Impact parameter", kHorizontalFrame);
   G_make_ip_scale = new TGTextButton(histo_opts,"Make scale");
   G_make_ip_scale->SetTextJustify(36);
   G_make_ip_scale->SetMargins(0,0,0,0);
   G_make_ip_scale->SetWrapLength(-1);
   G_make_ip_scale->Resize();
   G_make_ip_scale->SetEnabled(kFALSE);
   G_make_ip_scale->Connect("Clicked()","KVTreeAnalyzer",this,"MakeIPScale()");
   G_make_ip_scale->ChangeBackground(green);
   histo_opts->AddFrame(G_make_ip_scale, new TGLayoutHints(kLHintsLeft|kLHintsTop,2,2,10,2));
   lab = new TGLabel(histo_opts,"b <");
   histo_opts->AddFrame(lab, new TGLayoutHints(kLHintsLeft|kLHintsTop,5,2,12,2));
   G_make_ip_selection = new TGTextEntry(histo_opts, new TGTextBuffer(5));
   G_make_ip_selection->SetMaxLength(10);
   G_make_ip_selection->SetAlignment(kTextLeft);
   G_make_ip_selection->Resize(50,G_make_ip_selection->GetDefaultHeight());
   G_make_ip_selection->Connect("ReturnPressed()", "KVTreeAnalyzer", this, "GenerateIPSelection()");
   G_make_ip_selection->SetEnabled(kFALSE);
   histo_opts->AddFrame(G_make_ip_selection, new TGLayoutHints(kLHintsLeft|kLHintsTop,2,2,10,2));
   G_ip_histo = new TGLabel(histo_opts,"-");
   histo_opts->AddFrame(G_ip_histo, new TGLayoutHints(kLHintsLeft|kLHintsTop,5,2,12,2));
   fMain_histolist->AddFrame(histo_opts, new TGLayoutHints(kLHintsLeft|kLHintsExpandX,5,5,5,5));
   /* ip scale */
   histo_opts = new TGGroupFrame(fMain_histolist, "Fits", kHorizontalFrame);
   lab = new TGLabel(histo_opts,"Gumbel : ");
   histo_opts->AddFrame(lab, new TGLayoutHints(kLHintsLeft|kLHintsTop,2,2,5,2));
   G_fit1 = new TGTextButton(histo_opts, " 1st ");
   G_fit1->SetTextJustify(36);
   G_fit1->SetMargins(0,0,0,0);
   G_fit1->SetWrapLength(-1);
   G_fit1->Resize();
   G_fit1->SetEnabled(kFALSE);
   G_fit1->ChangeBackground(gura);
   G_fit1->Connect("Clicked()", "KVTreeAnalyzer", this, "FitGum1()");
   histo_opts->AddFrame(G_fit1, new TGLayoutHints(kLHintsLeft|kLHintsTop,2,2,2,2));
   G_fit2 = new TGTextButton(histo_opts, " 2nd ");
   G_fit2->SetTextJustify(36);
   G_fit2->SetMargins(0,0,0,0);
   G_fit2->SetWrapLength(-1);
   G_fit2->Resize();
   G_fit2->SetEnabled(kFALSE);
   G_fit2->ChangeBackground(gurb);
   G_fit2->Connect("Clicked()", "KVTreeAnalyzer", this, "FitGum2()");
   histo_opts->AddFrame(G_fit2, new TGLayoutHints(kLHintsLeft|kLHintsTop,2,2,2,2));
   G_fit3 = new TGTextButton(histo_opts, " 3rd ");
   G_fit3->SetTextJustify(36);
   G_fit3->SetMargins(0,0,0,0);
   G_fit3->SetWrapLength(-1);
   G_fit3->Resize();
   G_fit3->SetEnabled(kFALSE);
   G_fit3->ChangeBackground(gurc);
   G_fit3->Connect("Clicked()", "KVTreeAnalyzer", this, "FitGum3()");
   histo_opts->AddFrame(G_fit3, new TGLayoutHints(kLHintsLeft|kLHintsTop,2,2,2,2));
   G_fit4 = new TGTextButton(histo_opts, " 4th ");
   G_fit4->SetTextJustify(36);
   G_fit4->SetMargins(0,0,0,0);
   G_fit4->SetWrapLength(-1);
   G_fit4->Resize();
   G_fit4->SetEnabled(kFALSE);
   G_fit4->ChangeBackground(gurd);
   G_fit4->Connect("Clicked()", "KVTreeAnalyzer", this, "FitGum4()");
   histo_opts->AddFrame(G_fit4, new TGLayoutHints(kLHintsLeft|kLHintsTop,2,2,2,2));
   G_fit5 = new TGTextButton(histo_opts, " 5th ");
   G_fit5->SetTextJustify(36);
   G_fit5->SetMargins(0,0,0,0);
   G_fit5->SetWrapLength(-1);
   G_fit5->Resize();
   G_fit5->SetEnabled(kFALSE);
   G_fit5->ChangeBackground(gure);
   G_fit5->Connect("Clicked()", "KVTreeAnalyzer", this, "FitGum5()");
   histo_opts->AddFrame(G_fit5, new TGLayoutHints(kLHintsLeft|kLHintsTop,2,2,2,2));
   lab = new TGLabel(histo_opts,"GausGum : ");
   histo_opts->AddFrame(lab, new TGLayoutHints(kLHintsLeft|kLHintsTop,10,2,5,2));
   G_fitGG = new TGTextButton(histo_opts, " 1 ");
   G_fitGG->SetTextJustify(36);
   G_fitGG->SetMargins(0,0,0,0);
   G_fitGG->SetWrapLength(-1);
   G_fitGG->Resize();
   G_fitGG->SetEnabled(kFALSE);
   G_fitGG->ChangeBackground(gurf);
   G_fitGG->Connect("Clicked()", "KVTreeAnalyzer", this, "FitGausGum()");
   histo_opts->AddFrame(G_fitGG, new TGLayoutHints(kLHintsLeft|kLHintsTop,2,2,2,2));
   fMain_histolist->AddFrame(histo_opts, new TGLayoutHints(kLHintsLeft|kLHintsExpandX,5,5,5,5));
   
   /* histo list */
   G_histolist = new KVListView(TH1::Class(), fMain_histolist, hWidth, hHeight);
   G_histolist->SetDataColumns(1);
   G_histolist->SetDataColumn(0, "Data", "GetTitle", kTextLeft);
   G_histolist->ActivateSortButtons();
   G_histolist->SetMaxColumnSize(500);
   G_histolist->SetDoubleClickAction("KVTreeAnalyzer", this, "DrawHisto(TObject*)");
   G_histolist->Connect("SelectionChanged()","KVTreeAnalyzer",this,"HistoSelectionChanged()");
   fMain_histolist->AddFrame(G_histolist, new TGLayoutHints(kLHintsLeft|kLHintsTop|
                                       kLHintsExpandX|kLHintsExpandY,
				       5,5,5,5));
  
   fMain_histolist->MapSubwindows();

   fMain_histolist->Resize(fMain_histolist->GetDefaultSize());
   fMain_histolist->MapWindow();
   fMain_histolist->Resize(hWidth,hHeight);
   G_histolist->Display(&fHistolist);
   // SELECTIONS main frame
   UInt_t sWidth = 600, sHeight = 360;
   fMain_selectionlist = new TGMainFrame(gClient->GetRoot(),10,10,kMainFrame | kVerticalFrame);
   fMain_selectionlist->SetName("fMain_selectionlist");
   fMain_selectionlist->SetWindowName("SELECTIONS");
   /* current selection */
   G_selection_status = new TGStatusBar(fMain_selectionlist, sWidth, 10);
   G_selection_status->SetText("CURRENT SELECTION:",0);
   fMain_selectionlist->AddFrame(G_selection_status, new TGLayoutHints(kLHintsExpandX, 0, 0, 10, 0));
   /* make selection */
   TGHorizontalFrame *fHorizontalFrame1614 = new TGHorizontalFrame(fMain_selectionlist,sWidth,36,kHorizontalFrame);
   lab = new TGLabel(fHorizontalFrame1614, "Make selection : ");
   fHorizontalFrame1614->AddFrame(lab, new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,2,2));
   G_selection_text = new TGTextEntry(fHorizontalFrame1614, new TGTextBuffer(50));
   G_selection_text->SetMaxLength(4096);
   G_selection_text->SetAlignment(kTextLeft);
   G_selection_text->Resize(500,G_selection_text->GetDefaultHeight());
   G_selection_text->Connect("ReturnPressed()", "KVTreeAnalyzer", this, "GenerateSelection()");
   fHorizontalFrame1614->AddFrame(G_selection_text, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX,2,5,2,2));
   fMain_selectionlist->AddFrame(fHorizontalFrame1614, new TGLayoutHints(kLHintsExpandX | kLHintsTop,1,1,1,1));
   G_selection_but = new TGTextButton(fMain_selectionlist,"Combine selections");
   G_selection_but->SetTextJustify(36);
   G_selection_but->SetMargins(0,0,0,0);
   G_selection_but->SetWrapLength(-1);
   G_selection_but->Resize(sWidth,G_selection_but->GetDefaultHeight());
   G_selection_but->Connect("Clicked()", "KVTreeAnalyzer", this, "CombineSelections()");
   G_selection_but->SetEnabled(kFALSE);
   G_selection_but->ChangeBackground(cyan);
   fMain_selectionlist->AddFrame(G_selection_but, new TGLayoutHints(kLHintsLeft | kLHintsTop| kLHintsExpandX,2,2,2,2));
   G_update_but = new TGTextButton(fMain_selectionlist,"Update entry lists");
   G_update_but->SetTextJustify(36);
   G_update_but->SetMargins(0,0,0,0);
   G_update_but->SetWrapLength(-1);
   G_update_but->Resize(sWidth,G_update_but->GetDefaultHeight());
   G_update_but->Connect("Clicked()", "KVTreeAnalyzer", this, "UpdateEntryLists()");
   G_update_but->SetEnabled(kTRUE);
   G_update_but->ChangeBackground(yellow);
   fMain_selectionlist->AddFrame(G_update_but, new TGLayoutHints(kLHintsLeft | kLHintsTop| kLHintsExpandX,2,2,2,2));
   /* selection list */
   G_selectionlist = new KVListView(TEntryList::Class(), fMain_selectionlist, sWidth, sHeight);
   G_selectionlist->SetDataColumns(2);
   G_selectionlist->SetDataColumn(0, "Selection", "GetTitle");
   G_selectionlist->SetDataColumn(1, "Events", "GetN", kTextRight);
   G_selectionlist->ActivateSortButtons();
   G_selectionlist->SetDoubleClickAction("KVTreeAnalyzer", this, "SetSelection(TObject*)");
   G_selectionlist->Connect("SelectionChanged()", "KVTreeAnalyzer", this, "SelectionChanged()");
   fMain_selectionlist->AddFrame(G_selectionlist, new TGLayoutHints(kLHintsLeft|kLHintsTop|
                                       kLHintsExpandX|kLHintsExpandY,
				       5,5,5,5));
   
   fMain_selectionlist->MapSubwindows();

   fMain_selectionlist->Resize(fMain_selectionlist->GetDefaultSize());
   fMain_selectionlist->MapWindow();
   fMain_selectionlist->Resize(sWidth,sHeight);
   G_selectionlist->Display(&fSelections);
}  
   
void KVTreeAnalyzer::AddHisto(TH1*h)
{
   fHistolist.Add(h);
   G_histolist->Display(&fHistolist);
}
   
void KVTreeAnalyzer::AddSelection(TEntryList*e)
{
   fSelections.Add(e);
   G_selectionlist->Display(&fSelections);
}

void KVTreeAnalyzer::SetTree(TTree* t)
{
   fTree = t;
   fTreeName = t->GetName();
   fTreeFileName = t->GetCurrentFile()->GetName();
}
   
void KVTreeAnalyzer::ReconnectTree()
{
   TFile*f = TFile::Open(fTreeFileName);
   TTree *t = (TTree*)f->Get(fTreeName);
   SetTree(t);
}

KVTreeAnalyzer* KVTreeAnalyzer::OpenFile(const Char_t* filename, Bool_t nogui)
{
   TFile* f = TFile::Open(filename);
   KVTreeAnalyzer* anal = (KVTreeAnalyzer*)f->Get("KVTreeAnalyzer");
   delete f;
   anal->fNoGui=nogui;
   anal->ReconnectTree();
   anal->OpenGUI();
   return anal;
}

void KVTreeAnalyzer::DrawHisto(TObject* obj)
{
   if(!obj->InheritsFrom("TH1")) return;
   TH1* histo = dynamic_cast<TH1*>(obj);
   if(gPad && gPad->GetListOfPrimitives()->FindObject(histo)){
      TIter next(gPad->GetListOfPrimitives());
      TObject* o;
      while( (o = next()) ){
         if(o->IsA() == TCutG::Class()){
            MakeSelection(o->GetName());
            return;
         }
      }
   }
   TString exp,sel;
   ParseHistoTitle(histo->GetTitle(),exp,sel);
   if(!IsCurrentSelection(sel) && fApplySelection) histo = RemakeHisto(histo,exp);
   if(fDrawSame)
   {
      histo->SetLineColor(my_color_array[++fSameColorIndex]);
      if(fSameColorIndex==MAX_COLOR_INDEX) fSameColorIndex=-1;
      histo->Draw("same");
      TObject* legend = gPad->GetListOfPrimitives()->FindObject("TPave");
      if(legend){
         gPad->GetListOfPrimitives()->Remove(legend);
         delete legend;
      }
      gPad->BuildLegend();
      if(histo->InheritsFrom("TH2")) gPad->SetLogz(fDrawLog);
      else gPad->SetLogy(fDrawLog);
      gPad->Modified();
      gPad->Update();
   }
   else
   {
      if(fNewCanvas) new KVCanvas;
      histo->SetLineColor(my_color_array[0]);
      if(histo->InheritsFrom("TH2")) gPad->SetLogz(fDrawLog);
      else gPad->SetLogy(fDrawLog);
      histo->Draw();
      gPad->Modified();gPad->Update();
   }
}
   
void KVTreeAnalyzer::ParseHistoTitle(const Char_t* title, TString& exp, TString& sel)
{
   // Take histo title "VAREXP { SELECTION }"
   // and separate the two components
   
   exp="";
   sel="";
   TString tmp(title);
   Int_t ss = tmp.Index("{");
   if(ss>0){
      Int_t se = tmp.Index("}");
      sel = tmp(ss+1, se-ss-1);
      sel.Remove(TString::kBoth,' ');
      exp = tmp(0, ss);
      exp.Remove(TString::kBoth,' ');
   }
   else
   {
      exp = tmp;
      exp.Remove(TString::kBoth,' ');      
   }
}
   
Bool_t KVTreeAnalyzer::IsCurrentSelection(const Char_t* sel)
{
   // Returns kTRUE if "sel" corresponds to current active selection
   // (i.e. entry list of TTree)
   
   TString test_sel(sel);
   TString tree_sel;TEntryList* el;
   if((el = fTree->GetEntryList())) tree_sel = el->GetTitle();
   return (test_sel == tree_sel);
}
   
TH1* KVTreeAnalyzer::RemakeHisto(TH1* h, const Char_t* expr)
{
   // check histo not already in list
   TString htit;
   GenerateHistoTitle(htit,expr,"");
   TH1* histo = (TH1*)fHistolist.FindObjectWithMethod(htit,"GetTitle");
   if(histo) return histo;
   Int_t nx,ny=0;
   TString hname(h->GetName());
   if(hname.BeginsWith("I")){
      Int_t xmin = h->GetXaxis()->GetXmin()+0.5;
      Int_t xmax = h->GetXaxis()->GetXmax()-0.5;
      cout << "Remake histo with xmin = " << xmin << "  xmax = " << xmax << endl;
      h = MakeIntHisto(expr, "", xmin, xmax);
      return h;
   }
   nx = h->GetNbinsX();
   if(h->InheritsFrom("TH2")) ny = h->GetNbinsY();
   cout << "Remake histo with nx = " << nx << "  ny = " << ny << endl;
   h = MakeHisto(expr, "", nx, ny);
   return h;
}
void KVTreeAnalyzer::GenerateSelection()
{
   TString selection = G_selection_text->GetText();
   MakeSelection(selection);
   G_selection_text->Clear();
}

void KVTreeAnalyzer::GenerateAlias()
{
   TString alias = G_alias_text->GetText();
   TString name;
   name.Form("a%d", fAliasNumber++);
   SetAlias(name, alias);
   FillLeafList();
   G_alias_text->Clear();
}

void KVTreeAnalyzer::CombineSelections()
{
   if(fSelectedSelections){
      TEntryList* save_elist = fTree->GetEntryList();
      fTree->SetEntryList((TEntryList*)fSelectedSelections->First());
      TString newselect;
      int nsel = fSelectedSelections->GetEntries();
      for(int i=1; i<nsel; i++)
      {
         TString tmp; TEntryList* el = (TEntryList*)fSelectedSelections->At(i);
         tmp.Form("(%s)", el->GetTitle());
         if(i>1) newselect += " && ";
         newselect += tmp.Data();
      }
      MakeSelection(newselect);
      fTree->SetEntryList(save_elist);
   }
}

void KVTreeAnalyzer::SelectionChanged()
{
   SafeDelete(fSelectedSelections);
   fSelectedSelections = G_selectionlist->GetSelectedObjects();
   if(fSelectedSelections && fSelectedSelections->GetEntries()>1) G_selection_but->SetEnabled(kTRUE);
   else G_selection_but->SetEnabled(kFALSE);
}

void KVTreeAnalyzer::LeafChanged()
{
   SafeDelete(fSelectedLeaves);
   fSelectedLeaves = G_leaflist->GetSelectedObjects();
   fLeafExpr="-";
   fXLeaf=fYLeaf=0;
   G_leaf_draw->SetEnabled(kFALSE);
   if(fSelectedLeaves){
      Int_t nleaf = fSelectedLeaves->GetEntries();
      if(nleaf==1){
         fXLeaf = (TNamed*)fSelectedLeaves->First();
         fLeafExpr = (fXLeaf->InheritsFrom("TLeaf") ? fXLeaf->GetName():  fXLeaf->GetTitle()); 
         G_leaf_draw->SetEnabled(kTRUE);
      }
      else if(nleaf==2){
         if(fSwapLeafExpr){
            fXLeaf = (TNamed*)fSelectedLeaves->At(1);
            fYLeaf = (TNamed*)fSelectedLeaves->First();
         }
         else{
            fYLeaf = (TNamed*)fSelectedLeaves->At(1);
            fXLeaf = (TNamed*)fSelectedLeaves->First();
         }
         TString X,Y;
         X = (fXLeaf->InheritsFrom("TLeaf") ? fXLeaf->GetName():  fXLeaf->GetTitle());
         Y = (fYLeaf->InheritsFrom("TLeaf") ? fYLeaf->GetName():  fYLeaf->GetTitle());
         fLeafExpr.Form("%s:%s", Y.Data(), X.Data());
         G_leaf_draw->SetEnabled(kTRUE);
      }
      else{
         fLeafExpr="-";
      }
   }
   G_leaf_expr->SetText(fLeafExpr);
   G_leaf_expr->Resize();
}

void KVTreeAnalyzer::DrawLeafExpr()
{
   if(fLeafExpr=="-") return;
   if(fSelectedLeaves->GetEntries()==1){
      DrawLeaf(fSelectedLeaves->First());
      return;
   }
   int nx=500,ny=500;
   double xmin,xmax,ymin,ymax;
   xmin=xmax=ymin=ymax=0;
   TString Xexpr,Yexpr;
   Xexpr = (fXLeaf->InheritsFrom("TLeaf") ? fXLeaf->GetName():fXLeaf->GetTitle());
   Yexpr = (fYLeaf->InheritsFrom("TLeaf") ? fYLeaf->GetName():fYLeaf->GetTitle());
   
   
      xmin = fTree->GetMinimum(Xexpr);
      xmax = fTree->GetMaximum(Xexpr);
   if(fXLeaf->InheritsFrom("TLeaf") && 
         (!strcmp(((TLeaf*)fXLeaf)->GetTypeName(),"Int_t")||!strcmp(((TLeaf*)fXLeaf)->GetTypeName(),"Short_t")||!strcmp(((TLeaf*)fXLeaf)->GetTypeName(),"Char_t"))
         ){
      xmin -= 0.5;
      xmax += 0.5;
      nx = xmax-xmin;
   }
      ymin = fTree->GetMinimum(Yexpr);
      ymax = fTree->GetMaximum(Yexpr);
   if(fYLeaf->InheritsFrom("TLeaf") && 
         (!strcmp(((TLeaf*)fYLeaf)->GetTypeName(),"Int_t")||!strcmp(((TLeaf*)fYLeaf)->GetTypeName(),"Short_t")||!strcmp(((TLeaf*)fYLeaf)->GetTypeName(),"Char_t"))
         ){
      ymin -= 0.5;
      ymax += 0.5;
      ny = ymax-ymin;
   }
   if(fXLeaf->InheritsFrom("TLeaf") && !strcmp(((TLeaf*)fXLeaf)->GetTypeName(),"Char_t")){
      TString tmp = Xexpr;
      Xexpr.Form("int(%s)",tmp.Data());
   }
   if(fYLeaf->InheritsFrom("TLeaf") && !strcmp(((TLeaf*)fYLeaf)->GetTypeName(),"Char_t")){
      TString tmp = Yexpr;
      Yexpr.Form("int(%s)",tmp.Data());
   }
   
   fLeafExpr.Form("%s:%s", Yexpr.Data(), Xexpr.Data());
   TString name;
   name.Form("h%d",fHistoNumber);
   TString drawexp(fLeafExpr), histo, histotitle;
   GenerateHistoTitle(histotitle, fLeafExpr, "");
   histo.Form(">>%s(%d,%f,%f,%d,%f,%f)", name.Data(), nx,xmin,xmax,ny,ymin,ymax);
   drawexp += histo;
   fTree->Draw(drawexp, "", "goff");
   TH1* h = (TH1*)gDirectory->Get(name);
   h->SetTitle(histotitle);
   if(h->InheritsFrom("TH2")) h->SetOption("col");
   h->SetDirectory(0);
   AddHisto(h);
   fHistoNumber++;
   DrawHisto(h);
}


void KVTreeAnalyzer::DrawLeaf(TObject* obj)
{
   TH1* histo = 0;
   if(obj->InheritsFrom("TLeaf")){
      TLeaf* leaf = dynamic_cast<TLeaf*>(obj);
      TString expr = leaf->GetName();
      TString type = leaf->GetTypeName();
      // check histo not already in list
//       TString htit;
//       GenerateHistoTitle(htit,expr,"");
//       histo = (TH1*)fHistolist.FindObjectWithMethod(htit,"GetTitle");
      if(!histo){
         if(type=="Int_t"||type=="Char_t"||type=="Short_t"){
            Int_t xmin = fTree->GetMinimum(expr);
            Int_t xmax = fTree->GetMaximum(expr);
            if(type=="Char_t"){
               TString tmp; tmp.Form("int(%s)",expr.Data());
               expr=tmp.Data();
            }
            histo = MakeIntHisto(expr, "", xmin, xmax);
         }
         else
         {
            histo = MakeHisto(expr, "", 500);
         }
      }
      if(fNewCanvas) new KVCanvas;
      histo->Draw();
      if(histo->InheritsFrom("TH2")) gPad->SetLogz(fDrawLog);
      else gPad->SetLogy(fDrawLog);
      gPad->Modified();gPad->Update();
   }
   else{
      TString expr = obj->GetTitle();
      // check histo not already in list
//       TString htit;
//       GenerateHistoTitle(htit,expr,"");
//       histo = (TH1*)fHistolist.FindObjectWithMethod(htit,"GetTitle");
      if(!histo) histo = MakeHisto(expr, "", 500);
      if(fNewCanvas) new KVCanvas;
      histo->Draw();
      if(histo->InheritsFrom("TH2")) gPad->SetLogz(fDrawLog);
      else gPad->SetLogy(fDrawLog);
      gPad->Modified();gPad->Update();
   }
}

void KVTreeAnalyzer::HistoSelectionChanged()
{
   SafeDelete(fSelectedHistos);
   G_make_ip_scale->SetEnabled(kFALSE);
         G_fit1->SetEnabled(kFALSE);
         G_histo_del->SetEnabled(kFALSE);
         G_fit2->SetEnabled(kFALSE);
         G_fit3->SetEnabled(kFALSE);
         G_fit4->SetEnabled(kFALSE);
         G_fit5->SetEnabled(kFALSE);
         G_fitGG->SetEnabled(kFALSE);
   fSelectedHistos = G_histolist->GetSelectedObjects();
   if(fSelectedHistos && fSelectedHistos->GetEntries()==1)
   {
         G_histo_del->SetEnabled(kTRUE);
      if(fSelectedHistos->First()->IsA()==TH1F::Class()){
         G_make_ip_scale->SetEnabled(kTRUE);
         G_fit1->SetEnabled(kTRUE);
         G_fit2->SetEnabled(kTRUE);
         G_fit3->SetEnabled(kTRUE);
         G_fit4->SetEnabled(kTRUE);
         G_fit5->SetEnabled(kTRUE);
         G_fitGG->SetEnabled(kTRUE);
      }
   }
}

void KVTreeAnalyzer::GenerateIPSelection()
{
   TString bmax = G_make_ip_selection->GetText();
   Double_t Bmax = bmax.Atof();
   TGString histotit = G_ip_histo->GetText();
   TString ipvar,ipsel;
   ParseHistoTitle(histotit.Data(),ipvar,ipsel);
   Double_t varCut = ipscale->GetObservable(Bmax);
   TString selection;
   selection.Form("%s>%f", ipvar.Data(), varCut);
   TEntryList* save_elist = fTree->GetEntryList();
   SetSelection(ipsel);
   MakeSelection(selection);
   fTree->SetEntryList(save_elist);
}

void KVTreeAnalyzer::MakeIPScale()
{
   SafeDelete(ipscale);
   TH1* histo = dynamic_cast<TH1*>(fSelectedHistos->First());
   if(!histo) return;
   ipscale = new KVImpactParameter(histo);
   ipscale->MakeScale();
   G_ip_histo->SetText(histo->GetTitle());
   G_ip_histo->Resize();
   G_make_ip_selection->SetEnabled(kTRUE);
}

void KVTreeAnalyzer::SetSelection(const Char_t* sel)
{
   TObject* obj = fSelections.FindObjectWithMethod(sel,"GetTitle");
   fTree->SetEntryList((TEntryList*)obj);
}

void KVTreeAnalyzer::Save()
{
   TString filename;
   filename.Form("Analysis_%s", fTreeFileName.Data());
   SaveAs(filename);
}

void KVTreeAnalyzer::FitGum1()
{
   TH1* histo = dynamic_cast<TH1*>(fSelectedHistos->First());
   if(!histo) return;
   GDfirst->SetParameters(histo->GetMean(),histo->GetRMS());
   histo->Fit(GDfirst,"EM");
   TPaveStats* stats = (TPaveStats*)histo->FindObject("stats");
   stats->SetFitFormat("10.9g");
   stats->SetOptFit(111);
   histo->SetOption("e1");
   histo->SetMarkerStyle(24);
   histo->SetMarkerColor(kBlue+2);
   gPad->Modified();gPad->Update();
}
void KVTreeAnalyzer::FitGum2()
{
   TH1* histo = dynamic_cast<TH1*>(fSelectedHistos->First());
   if(!histo) return;
   GDsecond->SetParameters(histo->GetMean(),histo->GetRMS());
   histo->Fit(GDsecond,"EM");
   TPaveStats* stats = (TPaveStats*)histo->FindObject("stats");
   stats->SetFitFormat("10.9g");
   stats->SetOptFit(111);
   histo->SetOption("e1");
   histo->SetMarkerStyle(24);
   histo->SetMarkerColor(kBlue+2);
   gPad->Modified();gPad->Update();
}
void KVTreeAnalyzer::FitGum3()
{
   TH1* histo = dynamic_cast<TH1*>(fSelectedHistos->First());
   if(!histo) return;
   GDthird->SetParameters(histo->GetMean(),histo->GetRMS());
   histo->Fit(GDthird,"EM");
   TPaveStats* stats = (TPaveStats*)histo->FindObject("stats");
   stats->SetFitFormat("10.9g");
   stats->SetOptFit(111);
   histo->SetOption("e1");
   histo->SetMarkerStyle(24);
   histo->SetMarkerColor(kBlue+2);
   gPad->Modified();gPad->Update();
}
void KVTreeAnalyzer::FitGum4()
{
   TH1* histo = dynamic_cast<TH1*>(fSelectedHistos->First());
   if(!histo) return;
   GD4->SetParameters(histo->GetMean(),histo->GetRMS());
   histo->Fit(GD4,"EM");
   TPaveStats* stats = (TPaveStats*)histo->FindObject("stats");
   stats->SetFitFormat("10.9g");
   stats->SetOptFit(111);
   histo->SetOption("e1");
   histo->SetMarkerStyle(24);
   histo->SetMarkerColor(kBlue+2);
   gPad->Modified();gPad->Update();
}
void KVTreeAnalyzer::FitGum5()
{
   TH1* histo = dynamic_cast<TH1*>(fSelectedHistos->First());
   if(!histo) return;
   GD5->SetParameters(histo->GetMean(),histo->GetRMS());
   histo->Fit(GD5,"EM");
   TPaveStats* stats = (TPaveStats*)histo->FindObject("stats");
   stats->SetFitFormat("10.9g");
   stats->SetOptFit(111);
   histo->SetOption("e1");
   histo->SetMarkerStyle(24);
   histo->SetMarkerColor(kBlue+2);
   gPad->Modified();gPad->Update();
}
void KVTreeAnalyzer::FitGausGum()
{
   TH1* histo = dynamic_cast<TH1*>(fSelectedHistos->First());
   if(!histo) return;
   GausGum->SetParameters(0.5,histo->GetMean()+histo->GetRMS(),1,histo->GetRMS(),1);
   histo->Fit(GausGum,"EM");
   TPaveStats* stats = (TPaveStats*)histo->FindObject("stats");
   stats->SetFitFormat("10.9g");
   stats->SetOptFit(111);
   histo->SetOption("e1");
   histo->SetMarkerStyle(24);
   histo->SetMarkerColor(kBlue+2);
   gPad->Modified();gPad->Update();
}
   
TList* KVTreeAnalyzer::GetHistosByData(const Char_t* expr)
{
   // fill and return TList with histos containing the given data
   // which may be 1D ("mult", "zmax", etc.) or 2D ("zmax:mult")
   // DELETE LIST AFTER USE
   TList* hlist = new TList;
   TIter next(&fHistolist);
   TString var,sel;
   TH1* h;
   while( (h=(TH1*)next()) ){
      ParseHistoTitle(h->GetTitle(),var,sel);
      if(var==expr) hlist->Add(h);
   }
   return hlist;
}
   
TList* KVTreeAnalyzer::GetHistosBySelection(const Char_t* expr)
{
   // fill and return TList with histos using the given selection criteria
   // DELETE LIST AFTER USE
   TList* hlist = new TList;
   TIter next(&fHistolist);
   TString var,sel;
   TH1* h;
   while( (h=(TH1*)next()) ){
      ParseHistoTitle(h->GetTitle(),var,sel);
      if(sel==expr) hlist->Add(h);
   }
   return hlist;
}
   
TH1* KVTreeAnalyzer::GetHisto(const Char_t* expr, const Char_t* selection)
{
   TIter next(&fHistolist);
   TString var,sel;
   TH1* h;
   while( (h=(TH1*)next()) ){
      ParseHistoTitle(h->GetTitle(),var,sel);
      if(var==expr&&sel==selection) return h;
   }
   return 0;
}
   
void KVTreeAnalyzer::DeleteHisto(const Char_t* expr, const Char_t* selection)
{
   TH1* h = GetHisto(expr,selection);
   if(h){
      cout << "Deleting histo " << h->GetName() << endl;
      fHistolist.Remove(h);
      delete h;
   }
   G_histolist->Display(&fHistolist);
}

void KVTreeAnalyzer::DeleteSelectedHisto()
{
   TH1* histo = dynamic_cast<TH1*>(fSelectedHistos->First());
   if(!histo) return;
   fHistolist.Remove(histo);
   delete histo;
   G_histolist->Display(&fHistolist);   
}


void KVTreeAnalyzer::UpdateEntryLists()
{
   // regenerate entry lists for all selections
   TList old_lists;
   old_lists.AddAll(&fSelections);
   fSelections.Clear();
   G_selectionlist->RemoveAll();
   TIter next(&old_lists);
   TEntryList* old_el;
   fTree->SetEntryList(0);
   SelectionChanged();
   while( (old_el = (TEntryList*)next()) ){
      cout << "REGENERATING SELECTION : " << old_el->GetTitle() << endl;
      MakeSelection(old_el->GetTitle());
   }
   old_lists.Delete();
}