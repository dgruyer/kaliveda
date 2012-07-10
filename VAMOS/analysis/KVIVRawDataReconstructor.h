/*
$Id: KVIVRawDataReconstructor.h,v 1.3 2009/01/14 15:59:11 franklan Exp $
$Revision: 1.3 $
$Date: 2009/01/14 15:59:11 $
*/

//Created by KVClassFactory on Fri Jun  1 14:46:26 2007
//Author: e_ivamos

#ifndef __KVIVRAWDATARECONSTRUCTOR_H
#define __KVIVRAWDATARECONSTRUCTOR_H

#include "KVINDRARawDataReconstructor.h"
#include "GTGanilData.h"
#include "KVGANILDataReader.h"

class KVIVRawDataReconstructor : public KVINDRARawDataReconstructor
{
   public:

   KVIVRawDataReconstructor();
   virtual ~KVIVRawDataReconstructor();
   virtual void postInitRun()
	{
   	((KVGANILDataReader*)fRunFile)->GetGanTapeInterface()->SetUserTree( tree );
	};

   ClassDef(KVIVRawDataReconstructor,1)//Reconstructs raw data from INDRA-VAMOS experiments
};

#endif
