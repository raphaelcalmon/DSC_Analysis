#ifndef __Configuration_h
#define __Configuration_h

#include <sstream>
#include "PkModelingCLP.h"

//! Simple struct encapsulating all configuration options comming from the command line
struct Configuration {
public:
  float T1PreBloodValue;
  float T1PreTissueValue;
  float RelaxivityValue;
  float S0GradValue;
  float FTolerance;
  float GTolerance;
  float XTolerance;
  float Epsilon;
  float MaxIter;
  float Hematocrit;
  float AUCTimeInterval;
  bool ComputeFpv;
  std::string AIFMode;
  std::string InputFourDImageFileName;
  std::string ROIMaskFileName;
  std::string T1MapFileName;
  std::string AIFMaskFileName;
  std::string PrescribedAIFFileName;
  std::string OutputKtransFileName;
  std::string OutputVeFileName;
  std::string OutputFpvFileName;
  std::string OutputMaxSlopeFileName;
  std::string OutputAUCFileName;
  std::string BATCalculationMode;
  int ConstantBAT;
  std::string OutputRSquaredFileName;
  std::string OutputBolusArrivalTimeImageFileName;
  std::string OutputConcentrationsImageFileName;
  std::string OutputFittedDataImageFileName;
  std::string OutputOptimizerDiagnosticsImageFileName;

  ModuleProcessInformation* CLPProcessInformation;
};

// Parse cmdl args into a Configuration struct.
// Avoids pollution of scope by single variables for each cmdl argument.
#define PARSE_ARGS_INTO_CONFIG \
  Configuration configuration; \
  { \
    PARSE_ARGS; \
    configuration.T1PreBloodValue = T1PreBloodValue; \
    configuration.T1PreTissueValue = T1PreTissueValue; \
    configuration.RelaxivityValue = RelaxivityValue; \
    configuration.S0GradValue = S0GradValue; \
    configuration.FTolerance = FTolerance; \
    configuration.GTolerance = GTolerance; \
    configuration.XTolerance = XTolerance; \
    configuration.Epsilon = Epsilon; \
    configuration.MaxIter = MaxIter; \
    configuration.Hematocrit = Hematocrit; \
    configuration.AUCTimeInterval = AUCTimeInterval; \
    configuration.ComputeFpv = ComputeFpv; \
    configuration.AIFMode = AIFMode; \
    configuration.InputFourDImageFileName = InputFourDImageFileName; \
    configuration.ROIMaskFileName = ROIMaskFileName; \
    configuration.T1MapFileName = T1MapFileName; \
    configuration.AIFMaskFileName = AIFMaskFileName; \
    configuration.PrescribedAIFFileName = PrescribedAIFFileName; \
    configuration.OutputKtransFileName = OutputKtransFileName; \
    configuration.OutputVeFileName = OutputVeFileName; \
    configuration.OutputFpvFileName = OutputFpvFileName; \
    configuration.OutputMaxSlopeFileName = OutputMaxSlopeFileName; \
    configuration.OutputAUCFileName = OutputAUCFileName; \
    configuration.BATCalculationMode = BATCalculationMode; \
    configuration.ConstantBAT = ConstantBAT; \
    configuration.OutputRSquaredFileName = OutputRSquaredFileName; \
    configuration.OutputBolusArrivalTimeImageFileName = OutputBolusArrivalTimeImageFileName; \
    configuration.OutputConcentrationsImageFileName = OutputConcentrationsImageFileName; \
    configuration.OutputFittedDataImageFileName = OutputFittedDataImageFileName; \
    configuration.OutputOptimizerDiagnosticsImageFileName = OutputOptimizerDiagnosticsImageFileName; \
    \
    configuration.CLPProcessInformation = CLPProcessInformation; \
  } \


#endif
