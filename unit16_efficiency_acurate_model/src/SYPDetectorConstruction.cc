
/// \file B1DetectorConstruction.cc
/// \brief Implementation of the B1DetectorConstruction class

#include <G4GenericTrap.hh>
#include "SYPDetectorConstruction.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Trd.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "CADMesh.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SYPDetectorConstruction::SYPDetectorConstruction()
: G4VUserDetectorConstruction(),
  fScoringVolume(0)
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SYPDetectorConstruction::~SYPDetectorConstruction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


G4VPhysicalVolume* SYPDetectorConstruction::Construct()
{
  // Material definition
  G4NistManager* nistManager = G4NistManager::Instance();

  // Air definition using NIST Manager
  nistManager->FindOrBuildMaterial("G4_AIR");

  // Parameter
  G4double  z, a, fractionmass, density;
  G4String name, symbol;
  G4int ncomponents;

  // 95WNiFe
  G4Element* elW  = new G4Element
          (name="Tungsten", symbol = "W", z= 74., a=183.84*g/mole);
  G4Element* elNi = new G4Element
          (name="Nickle",symbol="Ni",z=28.,a=58.69*g/mole);
  G4Element* elFe = new G4Element
          (name="Ferrum",symbol="Fe",z=26.,a=55.845*g/mole);
  density = 18.75*g/cm3;
  G4Material* W95NiFe = new G4Material
          (name="95WNiFe",density,ncomponents=3);
  W95NiFe->AddElement(elW, fractionmass=95*perCent);
  W95NiFe->AddElement(elNi,fractionmass=2.5*perCent);
  W95NiFe->AddElement(elFe,fractionmass=2.5*perCent);

  // Stainless Steel
  G4Element* elCr = new G4Element
          (name="chromium",symbol="Cr",z=24,a=51.996*g/mole);
  G4Element* elMn = new G4Element
          (name="Manganese",symbol="Mn",z=25,a=54.938*g/mole);
  density = 7.93*g/cm3;
  G4Material* SS = new G4Material
          (name="StainlessSteel",density,ncomponents=4);
  SS->AddElement(elFe,fractionmass=68*perCent);
  SS->AddElement(elCr,fractionmass=20*perCent);
  SS->AddElement(elNi,fractionmass=10*perCent);
  SS->AddElement(elMn,fractionmass= 2*perCent);

  // Xe-48atm
  new G4Material("Xe_48atm", z=54., a=131.2*g/mole,density= 0.3723*g/cm3,
                 kStateGas, 2.93*kelvin, 48*atmosphere);

  // Option to switch on/off checking of volumes overlaps
  //
  G4bool checkOverlaps = true;

  // Parameter

  G4double world_sizeX = 10*m; G4double world_sizeY = 10*m; G4double world_sizeZ = 10*m;
  //
  G4Material* world_mat = nistManager->FindOrBuildMaterial("G4_AIR");
  G4Material* cham_mat = G4Material::GetMaterial("Xe_48atm");
  G4Material* shell_mat = G4Material::GetMaterial("StainlessSteel");
  G4Material* electrodeSlice_mat = nistManager->FindOrBuildMaterial("95WNiFe");

  //
  // World
  //

  G4Box* solidWorld =    
    new G4Box("World",                       //its name
       0.5*world_sizeX, 0.5*world_sizeY, 0.5*world_sizeZ);     //its size
      
  G4LogicalVolume* logicWorld =                         
    new G4LogicalVolume(solidWorld,          //its solid
                        world_mat,           //its material
                        "World");            //its name
                                   
  G4VPhysicalVolume* physWorld = 
    new G4PVPlacement(0,                     //no rotation
                      G4ThreeVector(),       //at (0,0,0)
                      logicWorld,            //its logical volume
                      "World",               //its name
                      0,                     //its mother  volume
                      false,                 //no boolean operation
                      0,                     //copy number
                      checkOverlaps);        //overlaps checking


  //
  // Shell: window front to insulating back
  // dx = 235.5*mm dy1 = 163*mm dy2 = 169.1166*mm(point to point)
  //
  // generic trapezoid with optionally collapsing vertices
  //

  G4double shell_halfZ = 0.5*36*mm;
  const std::vector<G4TwoVector>& shell_vertices =
          {{0,81.5*mm},{235*mm, 84.5583*mm},
           {235*mm,-84.5583*mm},{0,-81.5*mm},
           {0,81.5*mm},{235*mm, 84.5583*mm},
           {235*mm,-84.5583*mm},{0,-81.5*mm}};

  G4GenericTrap* solid_shell =
    new G4GenericTrap( "Shell", shell_halfZ, shell_vertices);

  G4LogicalVolume* logic_shell =
          new G4LogicalVolume(solid_shell,shell_mat,"shell");

  G4VPhysicalVolume* phys_shell = new G4PVPlacement
          (

                  0,
                  G4ThreeVector(),
                  logic_shell,
                  "shell",
                  logicWorld,
                  false,
                  0,
                  checkOverlaps

          );
  //
  // working gas area
  // window back (precisely: according to the side window end gasket) to insulating front
  // z_start = 14.3887*mm z_end = 210.5*mm
  //
  G4double gas_halfZ = 0.5*16*mm;
  const std::vector<G4TwoVector>& gas_vertices =
          {{14.3887*mm,79.5983*mm},{220.5*mm,82.3*mm},
           {220.5*mm, -82.3*mm},{14.3887*mm,-79.5983*mm},
           {14.3887*mm,79.5983*mm},{220.5*mm,82.3*mm},
           {220.5*mm, -82.3*mm},{14.3887*mm,-79.5983*mm}};

  G4GenericTrap* solid_gas =
          new G4GenericTrap( "gas", gas_halfZ, gas_vertices);

  G4LogicalVolume* logic_gas = new G4LogicalVolume
          (solid_gas,cham_mat,"gas");

  G4VPhysicalVolume* phys_gas = new G4PVPlacement
          (
                  0,
                  G4ThreeVector(),
                  logic_gas,
                  "gas",
                  logic_shell,
                  false,
                  0,
                  checkOverlaps
          );

  // window2
  // there may be some spare room between rib and chamber,
  // so we need this window to count the photon that
  // enter into the chamber
  //
  // using GetPreStepPoint() && GetPostStepPoint() in
  // UserSteppingAction
  // the created window must be stuck to the chamber
  // we may refer to some point used before
  //
  // window and chamber
  G4double chamber_halfZ = 0.5*16*mm;
  const std::vector<G4TwoVector>& chamberAndWindow_vertices =
          {{-92.76855*mm,9.49997*mm},{92.76855*mm,9.80003*mm},
           {92.76855*mm, -9.80003*mm},{-92.76855*mm,-9.49997*mm},
           {-92.76855*mm,9.49997*mm},{92.76855*mm,9.80003*mm},
           {92.76855*mm, -9.80003*mm},{-92.76855*mm,-9.49997*mm}};

  G4GenericTrap* solid_chamberAndWindow = new G4GenericTrap
          ("chamberAndWindow",chamber_halfZ,chamberAndWindow_vertices);

  G4LogicalVolume* logic_chamberAndWindow = new G4LogicalVolume
          (solid_chamberAndWindow,cham_mat,"chamberAndWindow");


  // Chamber
  // contain electrode slice
  //

  // original location, centred at(0,0,0), need to be put in the right place with the gas
  const std::vector<G4TwoVector>& chamber_vertices =
          {{-92.751855*mm,9.499997*mm},{92.751855*mm, 9.800003*mm},
           {92.751855*mm,-9.800003*mm},{-92.751855*mm,-9.499997*mm},
           {-92.751855*mm,9.499997*mm},{92.751855*mm, 9.800003*mm},
           {92.751855*mm,-9.800003*mm},{-92.751855*mm,-9.499997*mm}};

  G4GenericTrap* solid_chamber = new G4GenericTrap
          ("Chamber",chamber_halfZ,chamber_vertices);

  G4LogicalVolume* logic_chamber = new G4LogicalVolume
          (solid_chamber,cham_mat,"Chamber");

  G4VPhysicalVolume* phys_chamber = new G4PVPlacement(

                  0,
                  G4ThreeVector(),
                  logic_chamber,
                  "Chamber",
                  logic_chamberAndWindow,
                  false,
                  0,
                  checkOverlaps
          );

  // divide a chamber into two parts
  // one chamber include two units
  // for easy to get copy number
  const std::vector<G4TwoVector>& chamber_vertices_left_half =
          {{-92.751855*mm,9.499997*mm},{92.751855*mm, 9.800003*mm},
           {92.751855*mm,0*mm},{-92.751855*mm,0*mm},
           {-92.751855*mm,9.499997*mm},{92.751855*mm, 9.800003*mm},
           {92.751855*mm,0*mm},{-92.751855*mm,0*mm}};

  const std::vector<G4TwoVector>& chamber_vertices_right_half =
          {{-92.751855*mm,0*mm},{92.751855*mm, 0*mm},
           {92.751855*mm,-9.800003*mm},{-92.751855*mm,-9.499997*mm},
           {-92.751855*mm,0*mm},{92.751855*mm, 0*mm},
           {92.751855*mm,-9.800003*mm},{-92.751855*mm,-9.499997*mm},};

  G4GenericTrap* solid_chamber_left = new G4GenericTrap
          ("chamber",chamber_halfZ,chamber_vertices_left_half);

  G4LogicalVolume* logic_chamber_left = new G4LogicalVolume
          (solid_chamber_left,cham_mat,"chamber");

  G4GenericTrap* solid_chamber_right = new G4GenericTrap
          ("chamber",chamber_halfZ,chamber_vertices_right_half);

  G4LogicalVolume* logic_chamber_right = new G4LogicalVolume
          (solid_chamber_right,cham_mat,"chamber");

  G4VPhysicalVolume* phys_chamber_left = new G4PVPlacement
          (
                  0,
                  G4ThreeVector(),
                  logic_chamber_left,
                  "chamber",
                  logic_chamber,
                  false,
                  0,
                  checkOverlaps
                  );

  G4VPhysicalVolume* phys_chamber_right = new G4PVPlacement
          (
                  0,
                  G4ThreeVector(),
                  logic_chamber_right,
                  "chamber",
                  logic_chamber,
                  false,
                  1,
                  checkOverlaps
                  );

  // place electrode slice in chamber(half)
  // there are 2 kinds of electrode slice: ES and EC
  // differ in height: 12*mm and 16*mm
  // total 8, 6 complete, 2 half

  // 1st middle half (left)  ES
  const std::vector<G4TwoVector>& electrode_slice1_left =
          {{-92.75*mm,0.25*mm},{92.75*mm, 0.25*mm},
           {92.75*mm,0*mm},{-92.75*mm,0*mm},
           {-92.75*mm,0.25*mm},{92.75*mm, 0.25*mm},
           {92.75*mm,0.*mm},{-92.75*mm,0.*mm}};

  G4GenericTrap* solid_electrode_slice1_left = new G4GenericTrap
          ("ES",chamber_halfZ,electrode_slice1_left);

  G4LogicalVolume* logic_electrode_slice1_left = new G4LogicalVolume
          (solid_electrode_slice1_left,electrodeSlice_mat,"ES");

  G4VPhysicalVolume* phys_electrode_slice1_left = new G4PVPlacement
            (
                    0,
                    G4ThreeVector(),
                    logic_electrode_slice1_left,
                    "ES",
                    logic_chamber_left,
                    false,
                    30,
                    checkOverlaps
            );

  // 2nd middle half (right)  ES
  const std::vector<G4TwoVector>& electrode_slice2_right =
          {{-92.75*mm,0.*mm},{92.75*mm, 0.*mm},
           {92.75*mm,-0.25*mm},{-92.75*mm,-0.25*mm},
           {-92.75*mm,0.*mm},{92.75*mm, 0.*mm},
           {92.75*mm,-0.25*mm},{-92.75*mm,-0.25*mm}};

  G4GenericTrap* solid_electrode_slice2_right = new G4GenericTrap
          ("ES",chamber_halfZ,electrode_slice2_right);

  G4LogicalVolume* logic_electrode_slice2_right = new G4LogicalVolume
          (solid_electrode_slice2_right,electrodeSlice_mat,"ES");

  G4VPhysicalVolume* phys_electrode_slice2_right = new G4PVPlacement
          (
                  0,
                  G4ThreeVector(),
                  logic_electrode_slice2_right,
                  "ES",
                  logic_chamber_right,
                  false,
                  31,
                  checkOverlaps
                  );

  // 3rd slice EC

  G4double EC_halfZ = 0.5*12*mm;
  const std::vector<G4TwoVector>& electrode_slice3 =
          {{-92.7503 *mm, 7.5625 *mm },
           {92.7496 *mm, 7.7875 *mm },
           {92.7502 *mm, 7.2875 *mm },
           {-92.7497 *mm, 7.0625 *mm },
           {-92.7503 *mm, 7.5625 *mm },
           {92.7496 *mm, 7.7875 *mm },
           {92.7502 *mm, 7.2875 *mm },
           {-92.7497 *mm, 7.0625 *mm }};

    G4GenericTrap* solid_electrode_slice3 = new G4GenericTrap
            ("EC",EC_halfZ,electrode_slice3);

    G4LogicalVolume* logic_electrode_slice3 = new G4LogicalVolume
            (solid_electrode_slice3,electrodeSlice_mat,"EC");

    G4VPhysicalVolume* phys_electrode_slice3 = new G4PVPlacement
            (
                    0,
                    G4ThreeVector(),
                    logic_electrode_slice3,
                    "EC",
                    logic_chamber_left,
                    false,
                    0,
                    checkOverlaps
            );

  // 4th slice ES
  const std::vector<G4TwoVector>& electrode_slice4 =
          {{-92.7501 *mm, 5.125 *mm },
           {92.7498 *mm, 5.275 *mm },
           {92.7502 *mm, 4.775 *mm },
           {-92.7498 *mm, 4.625 *mm },
           {-92.7501 *mm, 5.125 *mm },
           {92.7498 *mm, 5.275 *mm },
           {92.7502 *mm, 4.775 *mm },
           {-92.7498 *mm, 4.625 *mm }};

  G4GenericTrap* solid_electrode_slice4 = new G4GenericTrap
          ("ES",chamber_halfZ,electrode_slice4);

  G4LogicalVolume* logic_electrode_slice4 = new G4LogicalVolume
          (solid_electrode_slice4,electrodeSlice_mat,"ES");

  G4VPhysicalVolume* phys_electrode_slice4 = new G4PVPlacement
          (
                  0,
                  G4ThreeVector(),
                  logic_electrode_slice4,
                  "ES",
                  logic_chamber_left,
                  false,
                  1,
                  checkOverlaps
            );

  // 5th slice EC
  const std::vector<G4TwoVector>& electrode_slice5 =
          {{-92.7501 *mm, 2.6875 *mm },{92.7499 *mm, 2.7625 *mm },
           {92.7501 *mm, 2.2625 *mm },{-92.7499 *mm, 2.1875 *mm },
           {-92.7501 *mm, 2.6875 *mm },{92.7499 *mm, 2.7625 *mm },
           {92.7501 *mm, 2.2625 *mm },{-92.7499 *mm, 2.1875 *mm }};

  G4GenericTrap* solid_electrode_slice5 = new G4GenericTrap
          ("EC",EC_halfZ,electrode_slice5);

  G4LogicalVolume* logic_electrode_slice5 = new G4LogicalVolume
          (solid_electrode_slice5,electrodeSlice_mat,"EC");

  G4VPhysicalVolume* phys_electrode_slice5 = new G4PVPlacement
          (
                  0,
                  G4ThreeVector(),
                  logic_electrode_slice5,
                  "EC",
                  logic_chamber_left,
                  false,
                  2,
                  checkOverlaps
            );

  // 6th slice EC in right half
  const std::vector<G4TwoVector>& electrode_slice6 =
            {{-92.7499 *mm, -2.1875 *mm },
             {92.7501 *mm, -2.2625 *mm },
             {92.7499 *mm, -2.7625 *mm },
             {-92.7501 *mm, -2.6875 *mm },
             {-92.7499 *mm, -2.1875 *mm },
             {92.7501 *mm, -2.2625 *mm },
             {92.7499 *mm, -2.7625 *mm },
             {-92.7501 *mm, -2.6875 *mm }};

    G4GenericTrap* solid_electrode_slice6 = new G4GenericTrap
            ("EC",EC_halfZ,electrode_slice6);

    G4LogicalVolume* logic_electrode_slice6 = new G4LogicalVolume
            (solid_electrode_slice6,electrodeSlice_mat,"EC");

    G4VPhysicalVolume* phys_electrode_slice6 = new G4PVPlacement
            (
                    0,
                    G4ThreeVector(),
                    logic_electrode_slice6,
                    "EC",
                    logic_chamber_right,
                    false,
                    4,
                    checkOverlaps
            );

  // 7th slice ES in right half
  const std::vector<G4TwoVector>& electrode_slice7 =
          {{-92.7498 *mm, -4.625 *mm },{92.7502 *mm, -4.775 *mm },
           {92.7498 *mm, -5.275 *mm },{-92.7502 *mm, -5.125 *mm },
           {-92.7498 *mm, -4.625 *mm },{92.7502 *mm, -4.775 *mm },
           {92.7498 *mm, -5.275 *mm },{-92.7502 *mm, -5.125 *mm }};

  G4GenericTrap* solid_electrode_slice7 = new G4GenericTrap
          ("ES",chamber_halfZ,electrode_slice7);

  G4LogicalVolume* logic_electrode_slice7 = new G4LogicalVolume
          (solid_electrode_slice7,electrodeSlice_mat,"ES");

  G4VPhysicalVolume* phys_electrode_slice7 = new G4PVPlacement
          (
                  0,
                  G4ThreeVector(),
                  logic_electrode_slice7,
                  "ES",
                  logic_chamber_right,
                  false,
                  5,
                  checkOverlaps
            );

  // 8th slice EC in right half
  const std::vector<G4TwoVector>& electrode_slice8 =
          {{-92.7497 *mm, -7.0625 *mm },{92.7502 *mm, -7.2875 *mm },
           {92.7496 *mm, -7.7875 *mm },{-92.7503 *mm, -7.5625 *mm },
           {-92.7497 *mm, -7.0625 *mm },{92.7502 *mm, -7.2875 *mm },
           {92.7496 *mm, -7.7875 *mm },{-92.7503 *mm, -7.5625 *mm }};

  G4GenericTrap* solid_electrode_slice8 = new G4GenericTrap
          ("EC",EC_halfZ,electrode_slice8);

  G4LogicalVolume* logic_electrode_slice8 = new G4LogicalVolume
          (solid_electrode_slice8,electrodeSlice_mat,"EC");

  G4VPhysicalVolume* phys_electrode_slice8 = new G4PVPlacement
          (
                  0,
                  G4ThreeVector(),
                  logic_electrode_slice8,
                  "EC",
                  logic_chamber_right,
                  false,
                  6,
                  checkOverlaps
            );

  // place 8 chamberAndWindow in gas
  // from ---- the original location
  // middle ES location ---- left below, right below and right up
  G4Point3D v1 = { -92.75*mm,0.25*mm,8*mm },
            v2 = {-92.75*mm,-0.25*mm,8*mm},
            v3 = {92.75*mm,-0.25*mm,8*mm};

  // to ---- the transformed location
  // this is the chamber in the left middle, copy number = 3
  G4Point3D v1_after1 = {16.4996*mm,10.2664*mm,8*mm},
            v2_after1 = {16.5004*mm,9.7664*mm,8*mm},
            v3_after1 = {202*mm,10.0791*mm,8*mm};

  G4Transform3D trans_cham1 =
          G4Transform3D
          (v1,v2,v3,v1_after1,v2_after1,v3_after1);

  G4VPhysicalVolume* phys_chamber1 = new G4PVPlacement
          (
                  trans_cham1,
                  logic_chamberAndWindow,
                  "chamberAndWindow",
                  logic_gas,
                  false,
                  3,
                  checkOverlaps
                  );

  // to ---- the transformed location
  // copy number = 2
  G4Point3D v1_after2 = {16.4987*mm,30.2992*mm,8*mm},
            v2_after2 = {16.5012*mm,29.7992*mm,8*mm},
            v3_after2 = {201.999*mm,30.7373*mm,8*mm};

  G4Transform3D trans_cham2 =
          G4Transform3D
          (v1,v2,v3,v1_after2,v2_after2,v3_after2);

  G4VPhysicalVolume* phys_chamber2 = new G4PVPlacement
          (
                  trans_cham2,
                  logic_chamberAndWindow,
                  "chamberAndWindow",
                  logic_gas,
                  false,
                  2,
                  checkOverlaps
            );

  // copy number = 1
  G4Point3D v1_after3 = {16.4978*mm,50.332*mm,8*mm},
            v2_after3 = {16.502*mm,49.832*mm,8*mm},
            v3_after3 = {201.995*mm,51.3955*mm,8*mm};

  G4Transform3D trans_cham3 =
          G4Transform3D
          (v1,v2,v3,v1_after3,v2_after3,v3_after3);

  G4VPhysicalVolume* phys_chamber3 = new G4PVPlacement
          (
                  trans_cham3,
                  logic_chamberAndWindow,
                  "chamberAndWindow",
                  logic_gas,
                  false,
                  1,
                  checkOverlaps
            );

  // copy number = 0
  G4Point3D v1_after4 = {16.4969*mm,70.3724*mm,8*mm},
            v2_after4 = {16.5028*mm,69.8725*mm,8*mm},
            v3_after4 = {201.99*mm,72.0464*mm,8*mm};

  G4Transform3D trans_cham4 =
          G4Transform3D
          (v1,v2,v3,v1_after4,v2_after4,v3_after4);

  G4VPhysicalVolume* phys_chamber4 = new G4PVPlacement
          (
                  trans_cham4,
                  logic_chamberAndWindow,
                  "chamberAndWindow",
                  logic_gas,
                  false,
                  0,
                  checkOverlaps
            );

  // copy number = 4
  G4Point3D v1_after5 = {16.5004*mm,-9.7664*mm,8*mm},
            v2_after5 = {16.4996*mm,-10.2664*mm,8*mm},
            v3_after5 = {201.999*mm,-10.5791*mm,8*mm};

  G4Transform3D trans_cham5 =
          G4Transform3D
          (v1,v2,v3,v1_after5,v2_after5,v3_after5);

  G4VPhysicalVolume* phys_chamber5 = new G4PVPlacement
          (
                  trans_cham5,
                  logic_chamberAndWindow,
                  "chamberAndWindow",
                  logic_gas,
                  false,
                  4,
                  checkOverlaps
            );

  // copy number = 5
  G4Point3D v1_after6 = {16.5012*mm,-29.7992*mm,8*mm},
            v2_after6 = {16.4987*mm,-30.2992*mm,8*mm},
            v3_after6 = {201.996*mm,-31.237*mm,8*mm};

  G4Transform3D trans_cham6 =
          G4Transform3D
          (v1,v2,v3,v1_after6,v2_after6,v3_after6);

  G4VPhysicalVolume* phys_chamber6 = new G4PVPlacement
          (
                  trans_cham6,
                  logic_chamberAndWindow,
                  "chamberAndWindow",
                  logic_gas,
                  false,
                  5,
                  checkOverlaps
                  );

  // copy number = 6
  G4Point3D v1_after7 = {16.502*mm,-49.832*mm,8*mm},
            v2_after7 = {16.4978*mm,-50.3319*mm,8*mm},
            v3_after7 = {201.991*mm,-51.8954*mm,8*mm};

  G4Transform3D trans_cham7 =
          G4Transform3D
          (v1,v2,v3,v1_after7,v2_after7,v3_after7);

  G4VPhysicalVolume* phys_chamber7 = new G4PVPlacement
          (
                  trans_cham7,
                  logic_chamberAndWindow,
                  "chamberAndWindow",
                  logic_gas,
                  false,
                  6,
                  checkOverlaps
            );

  // copy number = 7
  G4Point3D v1_after8 = {16.5028*mm,-69.8724*mm,8*mm},
            v2_after8 = {16.4969*mm,-70.3723*mm,8*mm},
            v3_after8 = {201.984*mm,-72.5463*mm,8*mm};

  G4Transform3D trans_cham8 =
          G4Transform3D
          (v1,v2,v3,v1_after8,v2_after8,v3_after8);

  G4VPhysicalVolume* phys_chamber8 = new G4PVPlacement
          (
                  trans_cham8,
                  logic_chamberAndWindow,
                  "chamberAndWindow",
                  logic_gas,
                  false,
                  7,
                  checkOverlaps
            );

  // 7 ribs
  // middle  copy number = 4
  const std::vector<G4TwoVector>& rib4 =
          {{15.25*mm,0.5*mm},
           {208.25*mm,0.5*mm},
           {208.25*mm,-0.5*mm},
           {15.25*mm, -0.5*mm},
           {15.25*mm,0.5*mm},
           {208.25*mm,0.5*mm},
           {208.25*mm,-0.5*mm},
           {15.25*mm, -0.5*mm}
           };

  G4GenericTrap* solid_rib4 = new G4GenericTrap
          ("rib",chamber_halfZ,rib4);

  G4LogicalVolume* logic_rib4 = new G4LogicalVolume
          (solid_rib4,shell_mat,"rib");

  G4VPhysicalVolume* phys_rib4 = new G4PVPlacement
          (
                  0,
                  G4ThreeVector(),
                  logic_rib4,
                  "rib",
                  logic_gas,
                  false,
                  3,
                  checkOverlaps
                  );

  // middle  copy number = 3
  const std::vector<G4TwoVector>& rib3 =
          {{15.2494 *mm, 20.5286 *mm },
           {208.248 *mm, 21.1793 *mm },
           {208.252 *mm, 20.1793 *mm },
           {15.2528 *mm, 19.5286 *mm },
           {15.2494 *mm, 20.5286 *mm },
           {208.248 *mm, 21.1793 *mm },
           {208.252 *mm, 20.1793 *mm },
           {15.2528 *mm, 19.5286 *mm }
            };

  G4GenericTrap* solid_rib3 = new G4GenericTrap
          ("rib",chamber_halfZ,rib3);

  G4LogicalVolume* logic_rib3 = new G4LogicalVolume
          (solid_rib3,shell_mat,"rib");

  G4VPhysicalVolume* phys_rib3 = new G4PVPlacement
          (
                  0,
                  G4ThreeVector(),
                  logic_rib3,
                  "rib",
                  logic_gas,
                  false,
                  2,
                  checkOverlaps
            );

  // middle  copy number = 2
  const std::vector<G4TwoVector>& rib2 =
          {{15.249 *mm, 40.5572 *mm },
           {208.245 *mm, 41.8586 *mm },
           {208.251 *mm, 40.8586 *mm },
           {15.2557 *mm, 39.5572 *mm },
           {15.249 *mm, 40.5572 *mm },
           {208.245 *mm, 41.8586 *mm },
           {208.251 *mm, 40.8586 *mm },
           {15.2557 *mm, 39.5572 *mm }
          };

  G4GenericTrap* solid_rib2 = new G4GenericTrap
            ("rib",chamber_halfZ,rib2);

  G4LogicalVolume* logic_rib2 = new G4LogicalVolume
          (solid_rib2,shell_mat,"rib");

  G4VPhysicalVolume* phys_rib2 = new G4PVPlacement
          (
                  0,
                  G4ThreeVector(),
                  logic_rib2,
                  "rib",
                  logic_gas,
                  false,
                  1,
                  checkOverlaps
            );

  // middle  copy number = 1
  const std::vector<G4TwoVector>& rib1 =
          {{15.2502 *mm, 60.5858 *mm },
           {208.24 *mm, 62.5378 *mm },
           {208.25 *mm, 61.5378 *mm },
           {15.2603 *mm, 59.5859 *mm },
           {15.2502 *mm, 60.5858 *mm },
           {208.24 *mm, 62.5378 *mm },
           {208.25 *mm, 61.5378 *mm },
           {15.2603 *mm, 59.5859 *mm }
          };
  //
  G4GenericTrap* solid_rib1 = new G4GenericTrap
            ("rib",chamber_halfZ,rib1);

  G4LogicalVolume* logic_rib1 = new G4LogicalVolume
          (solid_rib1,shell_mat,"rib");

  G4VPhysicalVolume* phys_rib1 = new G4PVPlacement
          (
                  0,
                  G4ThreeVector(),
                  logic_rib1,
                  "rib",
                  logic_gas,
                  false,
                  1,
                  checkOverlaps
            );

  // middle  copy number = 4
  const std::vector<G4TwoVector>& rib5 =
          {{15.2494 *mm, -20.5286 *mm },
           {208.248 *mm, -21.1793 *mm },
           {208.252 *mm, -20.1793 *mm },
           {15.2528 *mm, -19.5286 *mm },
           {15.2494 *mm, -20.5286 *mm },
           {208.248 *mm, -21.1793 *mm },
           {208.252 *mm, -20.1793 *mm },
           {15.2528 *mm, -19.5286 *mm }
          };

  G4GenericTrap* solid_rib5 = new G4GenericTrap
          ("rib",chamber_halfZ,rib5);

  G4LogicalVolume* logic_rib5 = new G4LogicalVolume
          (solid_rib5,shell_mat,"rib");

  G4VPhysicalVolume* phys_rib5 = new G4PVPlacement
          (
                  0,
                  G4ThreeVector(),
                  logic_rib5,
                  "rib",
                  logic_gas,
                  false,
                  4,
                  checkOverlaps
            );

  // middle  copy number = 5
  const std::vector<G4TwoVector>& rib6 =
            {{15.249 *mm, -40.5572 *mm },
             {208.245 *mm, -41.8586 *mm },
             {208.251 *mm, -40.8586 *mm },
             {15.2557 *mm, -39.5572 *mm },
             {15.249 *mm, -40.5572 *mm },
             {208.245 *mm, -41.8586 *mm },
             {208.251 *mm, -40.8586 *mm },
             {15.2557 *mm, -39.5572 *mm }
            };

  G4GenericTrap* solid_rib6 = new G4GenericTrap
          ("rib",chamber_halfZ,rib6);

  G4LogicalVolume* logic_rib6 = new G4LogicalVolume
          (solid_rib6,shell_mat,"rib");

  G4VPhysicalVolume* phys_rib6 = new G4PVPlacement
          (
                  0,
                  G4ThreeVector(),
                  logic_rib6,
                  "rib",
                  logic_gas,
                  false,
                  1,
                  checkOverlaps
            );

  // middle  copy number = 6
  const std::vector<G4TwoVector>& rib7 =
            {{15.2502 *mm, -60.5858 *mm },
             {208.24 *mm, -62.5378 *mm },
             {208.25 *mm, -61.5378 *mm },
             {15.2603 *mm, -59.5859 *mm },
             {15.2502 *mm, -60.5858 *mm },
             {208.24 *mm, -62.5378 *mm },
             {208.25 *mm, -61.5378 *mm },
             {15.2603 *mm, -59.5859 *mm }
            };
  //
  G4GenericTrap* solid_rib7 = new G4GenericTrap
          ("rib",chamber_halfZ,rib7);

  G4LogicalVolume* logic_rib7 = new G4LogicalVolume
          (solid_rib7,shell_mat,"rib");

  G4VPhysicalVolume* phys_rib7 = new G4PVPlacement
          (
                  0,
                  G4ThreeVector(),
                  logic_rib7,
                  "rib",
                  logic_gas,
                  false,
                  6,
                  checkOverlaps
            );

  // window slice
  // within the shell
  G4double window_half_sizeX = 0.5*0.1*mm,
  window_half_sizeY = 0.5*163*mm,
  window_half_sizeZ = 0.5*20*mm;

  G4Box* solid_window = new G4Box
          ("window",window_half_sizeX,
           window_half_sizeY,window_half_sizeZ);

  // need to be update later
  G4LogicalVolume* logic_window = new G4LogicalVolume
          (solid_window,shell_mat,"window");

  //
  G4ThreeVector window_pos = G4ThreeVector (0.05*mm,0,0);
  G4VPhysicalVolume* phys_window = new G4PVPlacement
          (
                  0,
                  window_pos,
                  logic_window,
                  "window",
                  logic_shell,
                  false,
                  0,
                  checkOverlaps
            );

  // outside the shell
  G4double window_half_sizeX_out = 0.5*0.4*mm,
           window_half_sizeY_out = 0.5*163*mm,
           window_half_sizeZ_out = 0.5*20*mm;

  G4Box* solid_window_out = new G4Box
          ("window",window_half_sizeX_out,
           window_half_sizeY_out,window_half_sizeZ_out);

  G4LogicalVolume* logic_window_out = new G4LogicalVolume
          (solid_window_out,shell_mat,"window");

  //
  G4ThreeVector window_pos_out = G4ThreeVector (-0.2*mm,0,0);
  G4VPhysicalVolume* phys_window_out = new G4PVPlacement
          (
                  0,
                  window_pos_out,
                  logic_window_out,
                  "window",
                  logicWorld,
                  false,
                  0,
                  checkOverlaps
            );

  // dig a hole in window
  G4double hole_sizeX = 0.5*13.8*mm;
  G4double hole_sizeY = 0.5*160*mm;
  G4double hole_sizeZ = 0.5*10*mm;

  G4Box* solid_hole = new G4Box
          ("hole",hole_sizeX,hole_sizeY,hole_sizeZ);

  G4LogicalVolume* logic_hole = new G4LogicalVolume
          (solid_hole,world_mat,"hole");

  G4ThreeVector hole_pos = G4ThreeVector (7*mm,0*mm,0*mm);
  G4VPhysicalVolume* phy_hole = new G4PVPlacement
          (
                  0,
                  hole_pos,
                  logic_hole,
                  "hole",
                  logic_shell,
                  false,
                  0,
                  checkOverlaps
                  );

  // Set working gas as scoring volume
  //
  fScoringVolume = logic_shell;

  //
  //always return the physical World
  //
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
