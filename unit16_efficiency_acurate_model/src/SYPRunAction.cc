
/// \file SYPRunAction.cc
/// \brief Implementation of the SYPRunAction class

#include "SYPRunAction.hh"
#include "SYPPrimaryGeneratorAction.hh"
#include "SYPDetectorConstruction.hh"
// #include "B1Run.hh"

#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4AccumulableManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4GeneralParticleSourceData.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SYPRunAction::SYPRunAction()
: G4UserRunAction(),
  fEdep(0.)
{
  // Register accumulable to the accumulable manager
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->RegisterAccumulable(fEdep);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SYPRunAction::~SYPRunAction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SYPRunAction::BeginOfRunAction(const G4Run*)
{ 
  // inform the runManager to save random number seed
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);

  // reset accumulables to their initial values
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Reset();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SYPRunAction::EndOfRunAction(const G4Run* run)
{
  G4int nofEvents = run->GetNumberOfEvent();
  if (nofEvents == 0) return;

  // Merge accumulables 
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Merge();
/*
  G4double edep  = fEdep.GetValue();
  G4double countSum, countPhotonSum;
  for (G4int i = 0; i < 16; i++)
  {
      countSum += count[i];
      countPhotonSum += countPhoton[i];
  }
*/

  // Run conditions
  //  note: There is no primary generator action object for "master"
  //        run manager for multi-threaded mode.
        
  // Print
  //  
  if (IsMaster()) {
    G4cout
     << G4endl
     << "--------------------End of Global Run-----------------------"
     << G4endl;
  }
  else {
    G4cout
     << G4endl
     << "--------------------End of Local Run------------------------"
     << G4endl;
  }
/*
  G4cout
     << " Number of photon is: "
     << countPhotonSum <<" "
     << G4endl

     G4cout
     << " Energy deposit in the working gas is: "
     << edep << " MeV"
     << G4endl;
*/
     G4double sum=0;
     G4double sumphoton=0;
     for( G4int i = 0; i < 16; i++ )
     {
        //
        G4cout
        <<" Detection Efficiency in Chamber[" << i << "] is: " << count[i] << " "<< countPhoton[i] << " " << (count[i])*100/(countPhoton[i]) << " % "
        << G4endl;

        sum += count[i];
        sumphoton += countPhoton[i];
/*
        G4cout
        << " Energy deposit in Chamber[" << i << "]is: " << Edep[i] << " MeV "
        << " The number of photon entered is: " << countPhoton[i] << " "
        << " Detection sensitivity is " << 3648.4*Edep[i]/countPhoton[i] << " pA/(cGy/h)"
        << G4endl;
*/
     }

     G4cout << "Global detection efficiency is " << sum*100/sumphoton << "%" <<G4endl;

    std::fstream dataFile;
    dataFile.open("DetectionEfficienvy.txt",std::ios::app|std::ios::out);
    for( G4int i = 0; i < 16; i++)
    {
        dataFile << count[i]*100/countPhoton[i] << G4endl;
    }



/*
     std::fstream dataFile;
     dataFile.open("DetectionEfficienvy.txt",std::ios::app|std::ios::out);
     for( G4int i = 0; i < 32; i++)
     {
         dataFile << count[i]  << G4endl;
     }

     G4cout
     << " Overall detection efficiency is: " << countSum*100/nofEvents <<" %"
     << G4endl

     G4cout
     << " Sensitivity is: "
     << 3648.4*edep/nofEvents << " pA/(cGy/h)"
     << G4endl */
    std::fstream dataFile1;
    dataFile1.open("sensitivity_read.txt",std::ios::app|std::ios::out);
    for( G4int i = 0; i < 16; i++)
    {
        dataFile1 << Edep[i] << " MeV" << "    " << countPhoton[i] << "    "<< 3648.4*Edep[i]/countPhoton[i]  << G4endl;
    }
     G4cout
     << "------------------------------------------------------------"
     << G4endl
     << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SYPRunAction::AddEdep( G4double edep, G4int copyno )
{
  Edep[copyno] += edep;
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

