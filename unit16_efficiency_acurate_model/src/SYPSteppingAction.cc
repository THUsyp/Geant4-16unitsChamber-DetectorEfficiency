
/// \file SYPSteppingAction.cc
/// \brief Implementation of the SYPSteppingAction class

#include "SYPSteppingAction.hh"
#include "SYPEventAction.hh"
#include "SYPRunAction.hh"
#include "SYPDetectorConstruction.hh"

#include "G4Step.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4LogicalVolume.hh"
#include "math.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SYPSteppingAction::SYPSteppingAction(SYPRunAction* fRunAction)
: G4UserSteppingAction(),
  fRunAction(fRunAction)
  //ScoringVolume(0)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SYPSteppingAction::~SYPSteppingAction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SYPSteppingAction::UserSteppingAction(const G4Step* step) {

    G4TouchableHandle touchableHandle = step->GetPreStepPoint()->GetTouchableHandle();
    G4LogicalVolume* volume = touchableHandle->GetVolume()->GetLogicalVolume();
    G4String volumeName = volume->GetName();
    G4Track* track = step->GetTrack();
    G4int trackID = track->GetTrackID();

    // To get the detection efficiency
    // first we should count the photon
    // that enter into a particular chamber
    if (trackID==1 && volumeName=="chamberAndWindow")
    {
        G4VPhysicalVolume* next_PV = step->GetPostStepPoint()->GetTouchableHandle()
                ->GetVolume();
        if(next_PV!=NULL)
        {
            G4String nextVolumeName = next_PV->GetName();
            if(nextVolumeName=="chamber")
            {
                G4int copyNo = step->GetPostStepPoint()->GetTouchableHandle()->GetCopyNumber();
                G4int motherCopyNo = step->GetPostStepPoint()->GetTouchableHandle()->GetCopyNumber(2);
                fRunAction->countPhoton[2*motherCopyNo+copyNo]++;
            }
        }
    }

    // kill the e- created by secondaries e-
    G4String createProcess = track->GetCreatorModelName();
    G4String particleName = track->GetParticleDefinition()->GetParticleName();

    if(particleName=="e-"&&createProcess=="eIoni"&&(volumeName=="EC"||volumeName=="ES"
            ||volumeName=="rib"||volumeName=="shell"))
    {
        // kill e- in metal
        track->SetTrackStatus(fKillTrackAndSecondaries);
    }

    // count the secondary in chamber

    if (volumeName=="chamber")
    {
        if(particleName=="e-" && createProcess!="eIoni")
        {
            G4int copyNo = touchableHandle->GetCopyNumber();
            G4int motherCopyNo = touchableHandle->GetCopyNumber(2);
            fRunAction->count[2*motherCopyNo+copyNo]++;
        }
        if(particleName=="e-")
        {
            track->SetTrackStatus(fKillTrackAndSecondaries);
        }
        if(particleName=="gamma"&&createProcess=="eBrem")
        {
            track->SetTrackStatus(fKillTrackAndSecondaries);
        }
    }

}