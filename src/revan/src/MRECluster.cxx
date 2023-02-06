/*
 * MRECluster.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MRECluster.cxx
//
// This class is a cluster of hits. Its position is the "center of energy" of 
// its hits.
// You can add hits and other clusters, but no tracks! If you add clusters
// only the hits of the cluster are assimilated, not the cluster itself.
// A cluster is for example generated by the class MREClusterAnalyzer.
//
//
// Example:
//
// MRECluster *Cluster = new MRECluster();
// Cluster->Add(new Hit(...));
//
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MRECluster.h"

// Standard libs:
#include <float.h>

// MEGAlib libs:
#include "MRESE.h"
#include "MREHit.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MRECluster)
#endif


////////////////////////////////////////////////////////////////////////////////


MRECluster::MRECluster() : MRESE()
{
  // Constructs a cluster without any hits

  m_SubElementType = MRESE::c_Cluster;

  // The following is done already in MRESE? 
  m_Position = MVector(0.0, 0.0, 0.0);
  m_Energy = 0.0;
  m_Detector = -1; // not adjusted
  
  m_IgnoreMissingDepth = true;
}


////////////////////////////////////////////////////////////////////////////////


MRECluster::MRECluster(MRECluster *Cluster) : MRESE((MRESE *) Cluster)
{
  // copy constructor:

  m_SubElementType = MRESE::c_Cluster;
}


////////////////////////////////////////////////////////////////////////////////


MRECluster::~MRECluster()
{
  // Delete this object, but do not delete its links and RESEs 
}


////////////////////////////////////////////////////////////////////////////////


double MRECluster::ComputeMinDistance(MRESE *RESE)
{
  // Compute the minimum distance between this cluster and the sub element:

  return ComputeMinDistanceVector(RESE).Mag();
}

////////////////////////////////////////////////////////////////////////////////


MVector MRECluster::ComputeMinDistanceVector(MRESE *RESE)
{
  // Compute the minimum distance between this cluster and the sub element.
  // The vector points to the RESE.

  int h, h_max;
  MVector Dist;
  MVector MinDist(DBL_MAX, DBL_MAX, DBL_MAX);

  h_max = GetNRESEs();

  if (RESE->GetType() == MRESE::c_Hit) {
    for (h = 0; h < h_max; h++) {
      Dist = (-1) * GetRESEAt(h)->ComputeMinDistanceVector(RESE);
      if (Dist.Mag() < MinDist.Mag()) {
        MinDist = Dist;
      }
    }
  } else if (RESE->GetType() == MRESE::c_Cluster) {
    for (h = 0; h < h_max; h++) {
      Dist = (-1) * GetRESEAt(h)->ComputeMinDistanceVector(RESE);
      if (Dist.Mag() < MinDist.Mag()) {
        MinDist = Dist;
      }
    }
  } else if (RESE->GetType() == MRESE::c_Track) {
    for (h = 0; h < h_max; h++) {
      Dist = (-1) * GetRESEAt(h)->ComputeMinDistanceVector(RESE);
      if (Dist.Mag() < MinDist.Mag()) {
        MinDist = Dist;
      }
    }
  }

  return MinDist;
}

////////////////////////////////////////////////////////////////////////////////


bool MRECluster::AreAdjacent(MRESE* RESE, double Sigma, int Level)
{
  // Compute the minimum distance between this cluster and the sub element.
  // The vector points to the RESE.

  int h, h_max;
  h_max = GetNRESEs();

  if (RESE->GetType() == MRESE::c_Hit) {
    for (h = 0; h < h_max; h++) {
      if (GetRESEAt(h)->AreAdjacent(RESE, Sigma, Level) == true) return true;
    }
  } else if (RESE->GetType() == MRESE::c_Cluster) {
    for (h = 0; h < h_max; h++) {
      if (GetRESEAt(h)->AreAdjacent(RESE, Sigma, Level) == true) return true;
    }
  } else if (RESE->GetType() == MRESE::c_Track) {
    for (h = 0; h < h_max; h++) {
      if (GetRESEAt(h)->AreAdjacent(RESE, Sigma, Level) == true) return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MRECluster::AddRESE(MRESE *RESE)
{
  // Add hits or clusters to this cluster.
  // Tracks are NOT allowed, right now


  if (RESE->GetType() == MRESE::c_Hit) {
    AddHit((MREHit *) RESE);
  } else if (RESE->GetType() == MRESE::c_Cluster) {
    int h, h_max;
    h_max = GetNRESEs();
    for (h = 0; h < h_max; h++) {
      AddHit((MREHit *) RESE->GetRESEAt(h));
    }
  } else {
    Fatal("MRECluster::AddRESE(MRESE *RESE)", 
          "Can't add the following MRESE to a cluster:\n%s", 
            RESE->ToString().Data());
  }
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRECluster::RemoveRESE(MRESE *RESE)
{
  // Remove a RESE from this cluster
  // RESE must be of type MREHit!!

  // TODO: This does not handle rotated detectors!
  
  if (ContainsRESE(RESE) == true) {
    //  Undo the changes in AddHit()
    if (m_AllowOverwrite == true) {
      if (m_IgnoreMissingDepth == true) {
        if (RESE->GetNoiseFlags().Contains("NODEPTH") == true) {
          m_Position[0] = 1.0/(m_Energy - RESE->GetEnergy())*(m_Energy*m_Position[0] - RESE->GetEnergy()*RESE->GetPosition()[0]);
          m_Position[1] = 1.0/(m_Energy - RESE->GetEnergy())*(m_Energy*m_Position[1] - RESE->GetEnergy()*RESE->GetPosition()[1]);
        } else {
          m_Position = 1.0/(m_Energy - RESE->GetEnergy())*(m_Energy*m_Position - RESE->GetEnergy()*RESE->GetPosition());
        }
      } else {
        m_Position = 1.0/(m_Energy - RESE->GetEnergy())*(m_Energy*m_Position - RESE->GetEnergy()*RESE->GetPosition());
      }
      m_Energy -= RESE->GetEnergy();
    }

    return MRESE::RemoveRESE(RESE);
  } else {
    Fatal("MRECluster::RemoveRESE(MRESE *RESE)",
          "This cluster does not contain the RESE!");
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MRECluster::AddHit(MREHit *Hit)
{
  // Add a hit to this cluster and recompute the cluster-parameters:

  if (m_AllowOverwrite == true) {
    if (GetNRESEs() == 0) {
      m_Time = Hit->GetTime();
      m_Position = Hit->GetPosition();
      m_Energy = Hit->GetEnergy();
      if (Hit->GetNoiseFlags().Contains("NODEPTH") == true) {
        m_NoiseFlags += " NODEPTH";
      }
    } else {
      m_Time = min(m_Time, Hit->GetTime());
      m_Energy += Hit->GetEnergy();
      
      if (m_IgnoreMissingDepth == false) {
        m_Position = (1.0/(m_Energy + Hit->GetEnergy()))*(m_Energy*m_Position + Hit->GetEnergy()*Hit->GetPosition());
        if (Hit->GetNoiseFlags().Contains("NODEPTH") == true) {
          if (m_NoiseFlags.Contains("NODEPTH") == false) {
            m_NoiseFlags += " NODEPTH";
          }
        }
      } else {
        // If we alreday have a good depth:
        cout<<"NF: "<<Hit->GetNoiseFlags()<<endl;
        bool HasGoodDepth = false;
        for (int r = 0; r < GetNRESEs(); ++r) {
          if (GetRESEAt(r)->GetNoiseFlags().Contains("NODEPTH") == false) {
            HasGoodDepth = true; 
          }
        }
        if (HasGoodDepth == true) {
          if (Hit->GetNoiseFlags().Contains("NODEPTH") == false) {
            m_Position = (1.0/(m_Energy + Hit->GetEnergy()))*(m_Energy*m_Position + Hit->GetEnergy()*Hit->GetPosition());
          } else {
            m_Position[0] = (1.0/(m_Energy + Hit->GetEnergy()))*(m_Energy*m_Position[0] + Hit->GetEnergy()*Hit->GetPosition())[0];
            m_Position[1] = (1.0/(m_Energy + Hit->GetEnergy()))*(m_Energy*m_Position[1] + Hit->GetEnergy()*Hit->GetPosition())[1];
            // m_Position[2] unchanged
            cout<<"Ignore new depth"<<endl;
          }
        } else {
          // If we have no good depth, but this RESE has one, the we take the depth of this RESE
          if (Hit->GetNoiseFlags().Contains("NODEPTH") == false) {
            m_Position[0] = (1.0/(m_Energy + Hit->GetEnergy()))*(m_Energy*m_Position[0] + Hit->GetEnergy()*Hit->GetPosition())[0];
            m_Position[1] = (1.0/(m_Energy + Hit->GetEnergy()))*(m_Energy*m_Position[1] + Hit->GetEnergy()*Hit->GetPosition())[1];
            m_Position[2] = Hit->GetPosition()[2];
            m_NoiseFlags.RemoveAllInPlace(" NODEPTH");
            cout<<"Ignore old depth"<<endl;
          } else {
            m_Position = (1.0/(m_Energy + Hit->GetEnergy()))*(m_Energy*m_Position + Hit->GetEnergy()*Hit->GetPosition());
          }
        }
      }
    }
  }

  MRESE::AddRESE(Hit);

  if (m_AllowOverwrite == true) {
    if (m_Detector == -1) {
      m_Detector = Hit->GetDetector();
    } else if (m_Detector != Hit->GetDetector()) {
      m_Detector = 0;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


MString MRECluster::ToString(bool WithLink, int Level)
{
  // Returns a MString containing the relevant data of this object
  //
  // WithLink: Display the links
  // Level:    A level of N displays 3*N blancs before the text

  const int Length = 5000;
  char Text[Length];

  MString String("");
  snprintf(Text, Length, "Cluster (%d) at (%.3f, %.3f, %.3f)+-(%.3f, %.3f, %.3f) with %.2f+-%.2f keV in %d the following hits:\n",
          m_ID, m_Position.X(), m_Position.Y(), m_Position.Z(), 
          m_PositionResolution.X(), m_PositionResolution.Y(), m_PositionResolution.Z(), 
          m_Energy, m_EnergyResolution, m_Detector);
  for (int i = 0; i < Level; i++) {
    String += MString("   ");
  }
  String += MString(Text);

  for (int h = 0; h < GetNRESEs(); h++) {
    String += GetRESEAt(h)->ToString(WithLink, Level+1);
  }
 
  if (WithLink == true && GetNLinks() > 0) {
    for (int i = 0; i < Level+1; i++) {
      String += MString("   ");
    }
    String += MString("Linked with: ");  
    for (int a = 0; a < GetNLinks(); a++) {
      snprintf(Text, Length, "  (%d)", GetLinkAt(a)->GetID());
      String += MString(Text);
    }
    String += MString("\n");
  }

  return String;
}


////////////////////////////////////////////////////////////////////////////////


MRECluster* MRECluster::Duplicate()
{
  // Duplicate this cluster

  return new MRECluster(this);
}


////////////////////////////////////////////////////////////////////////////////


bool MRECluster::IsValid()
{
  // Check if this cluster and all its hits are valid:

  for (int i = 0; i < GetNRESEs(); ++i) {
    if (GetRESEAt(i)->IsValid() == false) return false;
  }

  return true;
}



// MRECluster.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
