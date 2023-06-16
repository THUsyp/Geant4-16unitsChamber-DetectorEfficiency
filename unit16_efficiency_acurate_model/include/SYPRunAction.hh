
/// \file SYPRunAction.hh
/// \brief Definition of the SYPRunAction class

#ifndef SYPRunAction_h
#define SYPRunAction_h 1

#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "globals.hh"

class G4Run;

/// Run action class
///
/// In EndOfRunAction(), it calculates the dose in the selected volume 
/// from the energy deposit accumulated via stepping and event actions.
/// The computed dose is then printed on the screen.

class SYPRunAction : public G4UserRunAction
{
  public:
    SYPRunAction();
    virtual ~SYPRunAction();

    // virtual G4Run* GenerateRun();
    virtual void BeginOfRunAction(const G4Run*);
    virtual void   EndOfRunAction(const G4Run*);

    void AddEdep ( G4double edep , G4int copyNo );
    G4double count[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    G4double countPhoton[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    G4double Edep[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  private:
    G4Accumulable<G4double> fEdep;

};

#endif
